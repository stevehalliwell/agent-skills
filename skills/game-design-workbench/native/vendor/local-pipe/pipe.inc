/* SPDX-License-Identifier: MIT */
/* Extracted proto-framework pipe helpers; see PIPE-PROVENANCE.md. */
/* Local message pipes. Caller owns zeroed channel and all I/O storage. One
   outstanding operation per channel; keep buffers alive until terminal poll.
   Windows named-pipe and macOS/Linux Unix-socket backends; others unsupported.
   Server grants current user only and rejects remote clients; no heap storage.
   Windows access-policy calls loaded from OS advapi32, with no caller link flag. */
typedef enum pipe_error {
    pipe_error_ok, pipe_error_pending, pipe_error_invalid, pipe_error_unavailable,
    pipe_error_denied, pipe_error_capacity, pipe_error_io, pipe_error_timeout,
    pipe_error_unsupported
} pipe_error;
typedef struct pipe_channel {
    void *handle, *event;
#if defined(_WIN32)
    OVERLAPPED operation;
#elif defined(__APPLE__) || defined(__linux__)
    int descriptor, listener; /* fd + 1; zero means absent. */
    int operation, cancelled;
    void *bytes;
    usize size, used, header_used;
    u32 message_size;
    u8 header[4];
    char path[104];
    dev_t device;
    ino_t inode;
#endif
    int pending; /* 0 idle, 1 OS pending, 2 completed synchronously. */
    usize transferred;
} pipe_channel;

static const char *pipe_error_message(pipe_error error) {
    switch (error) {
    case pipe_error_ok: return NULL;
    case pipe_error_pending: return "pipe operation pending";
    case pipe_error_invalid: return "invalid pipe operation";
    case pipe_error_unavailable: return "pipe unavailable or busy";
    case pipe_error_denied: return "pipe access denied";
    case pipe_error_capacity: return "pipe message capacity exceeded";
    case pipe_error_io: return "pipe I/O failed or disconnected";
    case pipe_error_timeout: return "pipe operation timed out";
    case pipe_error_unsupported: return "pipes are unsupported on this platform";
    }
    return "unknown pipe error";
}

#if defined(_WIN32)
#ifndef PIPE_REJECT_REMOTE_CLIENTS
#define PIPE_REJECT_REMOTE_CLIENTS 0x00000008
#endif
static pipe_error pipe_windows_error(DWORD error) {
    if (error == ERROR_ACCESS_DENIED)
        return pipe_error_denied;
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PIPE_BUSY || error == ERROR_SEM_TIMEOUT)
        return pipe_error_unavailable;
    if (error == ERROR_MORE_DATA)
        return pipe_error_capacity;
    return pipe_error_io;
}
static int pipe_windows_path_valid(const char *path) {
    if (!path || strncmp(path, "\\\\.\\pipe\\", 9) || !path[9])
        return 0;
    for (usize index = 9; index < 256; ++index) {
        unsigned char byte = (unsigned char)path[index];
        if (!byte)
            return 1;
        if (byte < 32 || byte >= 127 || byte == '\\' || byte == '/')
            return 0;
    }
    return 0;
}
#endif

static int process_executable(char *output, usize capacity) {
    if (!output || !capacity || capacity > UINT32_MAX)
        return 0;
#if defined(_WIN32)
    wchar_t path[1024];
    DWORD length = GetModuleFileNameW(NULL, path, sizeof(path) / sizeof(path[0]));
    return length && length < sizeof(path) / sizeof(path[0]) &&
           WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, path, -1, output, (int)capacity, NULL, NULL);
#elif defined(__linux__)
    ssize_t length = readlink("/proc/self/exe", output, capacity - 1);
    if (length < 0 || (usize)length >= capacity - 1) return 0;
    output[length] = 0; return 1;
#elif defined(__APPLE__)
    u32 size = (u32)capacity;
    return _NSGetExecutablePath(output, &size) == 0;
#else
    return 0;
#endif
}

static u32 process_id(void) {
#if defined(_WIN32)
    return (u32)GetCurrentProcessId();
#else
    return (u32)getpid();
#endif
}

static u32 system_milliseconds(void) {
#if defined(_WIN32)
    return (u32)GetTickCount();
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
        abort(); /* OS clock failure breaks finite deadline invariant. */
    return (u32)((u64)now.tv_sec * 1000 + (u64)now.tv_nsec / 1000000);
#endif
}

static void system_sleep(u32 milliseconds) {
#if defined(_WIN32)
    Sleep(milliseconds);
#else
    struct timespec delay = {(time_t)(milliseconds / 1000), (long)(milliseconds % 1000) * 1000000};
    nanosleep(&delay, NULL);
#endif
}

static int pipe_local_name(char *output, usize capacity, const char *label, u32 pid) {
    if (!output || !label || !*label || !pid)
        return 0;
    for (usize i = 0; label[i]; ++i)
        if (!((label[i] >= 'a' && label[i] <= 'z') || label[i] == '-'))
            return 0;
#if defined(_WIN32)
    int length = snprintf(output, capacity, "\\\\.\\pipe\\%s-%lu", label, (unsigned long)pid);
#else
    int length = snprintf(output, capacity, "/tmp/proto-pipes-%lu/%s-%lu",
                           (unsigned long)getuid(), label, (unsigned long)pid);
#endif
    return length > 0 && (usize)length < capacity;
}

#if defined(__APPLE__) || defined(__linux__)
static pipe_error pipe_macos_error(void) {
    if (errno == EACCES || errno == EPERM)
        return pipe_error_denied;
    if (errno == ENOENT || errno == ECONNREFUSED || errno == EADDRINUSE)
        return pipe_error_unavailable;
    return pipe_error_io;
}

static int pipe_macos_path(const char *path, char *directory, usize capacity) {
    int length = snprintf(directory, capacity, "/tmp/proto-pipes-%lu", (unsigned long)getuid());
    if (length <= 0 || (usize)length + 1 >= capacity || strlen(path) >= 104 ||
        strncmp(path, directory, (usize)length) || path[length] != '/' || !path[length + 1])
        return 0;
    for (usize i = (usize)length + 1; path[i]; ++i)
        if (!((path[i] >= 'a' && path[i] <= 'z') || (path[i] >= '0' && path[i] <= '9') || path[i] == '-'))
            return 0;
    return 1;
}

static int pipe_macos_directory(const char *directory) {
    struct stat status;
    return !lstat(directory, &status) && S_ISDIR(status.st_mode) &&
           status.st_uid == getuid() && (status.st_mode & 0777) == 0700;
}

static int pipe_macos_socket(void) {
    int descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (descriptor < 0)
        return -1;
    if (fcntl(descriptor, F_SETFL, O_NONBLOCK) || fcntl(descriptor, F_SETFD, FD_CLOEXEC) ||
        player_socket_policy(descriptor)) {
        close(descriptor);
        return -1;
    }
    return descriptor;
}

static int pipe_macos_owner(int descriptor) {
#ifdef __linux__
    struct ucred credentials;
    socklen_t size = sizeof(credentials);
    return !getsockopt(descriptor, SOL_SOCKET, SO_PEERCRED, &credentials, &size) &&
        size == sizeof(credentials) && credentials.uid == getuid();
#else
    uid_t user;
    gid_t group;
    return !getpeereid(descriptor, &user, &group) && user == getuid();
#endif
}
#endif

static pipe_error pipe_server_open(const char *path, usize input_capacity,
                                   usize output_capacity, pipe_channel *channel) {
    if (!channel || channel->handle || channel->event || !path || !input_capacity ||
        !output_capacity || input_capacity > UINT32_MAX || output_capacity > UINT32_MAX)
        return pipe_error_invalid;
#if defined(_WIN32)
    if (!pipe_windows_path_valid(path))
        return pipe_error_invalid;
    /* CJIT emits unused static functions too: dynamic OS imports keep merely
       including base.h from imposing an advapi32 link requirement. */
    HMODULE security = LoadLibraryA("advapi32.dll");
    if (!security)
        return pipe_error_io;
    typedef BOOL (WINAPI *open_token_type)(HANDLE, DWORD, PHANDLE);
    typedef BOOL (WINAPI *token_information_type)(HANDLE, TOKEN_INFORMATION_CLASS, void *, DWORD, PDWORD);
    typedef BOOL (WINAPI *initialize_acl_type)(PACL, DWORD, DWORD);
    typedef BOOL (WINAPI *add_ace_type)(PACL, DWORD, DWORD, PSID);
    typedef BOOL (WINAPI *initialize_descriptor_type)(PSECURITY_DESCRIPTOR, DWORD);
    typedef BOOL (WINAPI *set_dacl_type)(PSECURITY_DESCRIPTOR, BOOL, PACL, BOOL);
    open_token_type open_token = (open_token_type)GetProcAddress(security, "OpenProcessToken");
    token_information_type token_information = (token_information_type)GetProcAddress(security, "GetTokenInformation");
    initialize_acl_type initialize_acl = (initialize_acl_type)GetProcAddress(security, "InitializeAcl");
    add_ace_type add_ace = (add_ace_type)GetProcAddress(security, "AddAccessAllowedAce");
    initialize_descriptor_type initialize_descriptor = (initialize_descriptor_type)GetProcAddress(security, "InitializeSecurityDescriptor");
    set_dacl_type set_dacl = (set_dacl_type)GetProcAddress(security, "SetSecurityDescriptorDacl");
    if (!open_token || !token_information || !initialize_acl || !add_ace || !initialize_descriptor || !set_dacl) {
        FreeLibrary(security);
        return pipe_error_io;
    }
    HANDLE token;
    union { u64 alignment; u8 bytes[1024]; } token_buffer, acl_buffer;
    DWORD needed = 0;
    if (!open_token(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        FreeLibrary(security);
        return pipe_error_denied;
    }
    BOOL got_token = token_information(token, TokenUser, token_buffer.bytes, sizeof(token_buffer.bytes), &needed);
    CloseHandle(token);
    if (!got_token) {
        FreeLibrary(security);
        return needed > sizeof(token_buffer.bytes) ? pipe_error_capacity : pipe_error_denied;
    }
    TOKEN_USER *user = (TOKEN_USER *)token_buffer.bytes;
    ACL *acl = (ACL *)acl_buffer.bytes;
    SECURITY_DESCRIPTOR descriptor;
    BOOL policy_ready = initialize_acl(acl, sizeof(acl_buffer.bytes), ACL_REVISION) &&
                        add_ace(acl, ACL_REVISION, GENERIC_READ | GENERIC_WRITE, user->User.Sid) &&
                        initialize_descriptor(&descriptor, SECURITY_DESCRIPTOR_REVISION) &&
                        set_dacl(&descriptor, TRUE, acl, FALSE);
    FreeLibrary(security);
    if (!policy_ready)
        return pipe_error_denied;
    SECURITY_ATTRIBUTES attributes = {sizeof(attributes), &descriptor, FALSE};
    HANDLE event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!event)
        return pipe_error_io;
    HANDLE handle = CreateNamedPipeA(path, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED |
                                     FILE_FLAG_FIRST_PIPE_INSTANCE,
                                     PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT |
                                     PIPE_REJECT_REMOTE_CLIENTS,
                                     1, (DWORD)output_capacity, (DWORD)input_capacity,
                                     0, &attributes);
    if (handle == INVALID_HANDLE_VALUE) {
        pipe_error error = pipe_windows_error(GetLastError());
        CloseHandle(event);
        return error;
    }
    *channel = (pipe_channel){.handle = handle, .event = event};
    return pipe_error_ok;
#elif defined(__APPLE__) || defined(__linux__)
    char directory[104];
    if (!pipe_macos_path(path, directory, sizeof(directory)))
        return pipe_error_invalid;
    if (mkdir(directory, 0700) && errno != EEXIST)
        return pipe_macos_error();
    if (!pipe_macos_directory(directory))
        return pipe_error_denied;
    int descriptor = pipe_macos_socket();
    if (descriptor < 0)
        return pipe_macos_error();
    struct sockaddr_un address = {0};
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    /* Refuse live or stale existing endpoints; never unlink another listener. */
    if (bind(descriptor, (struct sockaddr *)&address, sizeof(address))) {
        pipe_error error = pipe_macos_error();
        close(descriptor);
        return error;
    }
    struct stat status;
    if (chmod(path, 0600) || lstat(path, &status) || listen(descriptor, 1)) {
        close(descriptor);
        unlink(path);
        return pipe_error_io;
    }
    channel->listener = descriptor + 1;
    channel->handle = channel->event = (void *)1;
    channel->device = status.st_dev;
    channel->inode = status.st_ino;
    strcpy(channel->path, path);
    return pipe_error_ok;
#else
    return pipe_error_unsupported;
#endif
}

static pipe_error pipe_client_open(const char *path, u32 timeout_ms, pipe_channel *channel) {
    if (!channel || channel->handle || channel->event || !path || timeout_ms == UINT32_MAX)
        return pipe_error_invalid;
#if defined(_WIN32)
    if (!pipe_windows_path_valid(path))
        return pipe_error_invalid;
    if (!WaitNamedPipeA(path, timeout_ms))
        return pipe_windows_error(GetLastError());
    HANDLE handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                FILE_FLAG_OVERLAPPED, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        return pipe_windows_error(GetLastError());
    DWORD mode = PIPE_READMODE_MESSAGE;
    HANDLE event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!event || !SetNamedPipeHandleState(handle, &mode, NULL, NULL)) {
        if (event)
            CloseHandle(event);
        CloseHandle(handle);
        return pipe_error_io;
    }
    *channel = (pipe_channel){.handle = handle, .event = event};
    return pipe_error_ok;
#elif defined(__APPLE__) || defined(__linux__)
    /* poll() uses signed milliseconds; never let a large timeout become infinite. */
    if (timeout_ms > INT32_MAX)
        return pipe_error_invalid;
    char directory[104];
    if (!pipe_macos_path(path, directory, sizeof(directory)))
        return pipe_error_invalid;
    if (!pipe_macos_directory(directory))
        return pipe_error_unavailable;
    int descriptor = pipe_macos_socket();
    if (descriptor < 0)
        return pipe_macos_error();
    struct sockaddr_un address = {0};
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    if (connect(descriptor, (struct sockaddr *)&address, sizeof(address))) {
        if (errno != EINPROGRESS) {
            pipe_error error = pipe_macos_error();
            close(descriptor);
            return error;
        }
        struct pollfd wait = {descriptor, POLLOUT, 0};
        int result = poll(&wait, 1, (int)timeout_ms);
        int error = 0;
        socklen_t size = sizeof(error);
        if (result <= 0 || getsockopt(descriptor, SOL_SOCKET, SO_ERROR, &error, &size) || error) {
            close(descriptor);
            return result == 0 ? pipe_error_timeout : pipe_error_unavailable;
        }
    }
    if (!pipe_macos_owner(descriptor)) {
        close(descriptor);
        return pipe_error_denied;
    }
    channel->descriptor = descriptor + 1;
    channel->handle = channel->event = (void *)1;
    return pipe_error_ok;
#else
    return pipe_error_unsupported;
#endif
}

typedef enum pipe_operation { pipe_operation_connect, pipe_operation_read, pipe_operation_write } pipe_operation;
static pipe_error pipe_begin(pipe_channel *channel, pipe_operation operation,
                             void *bytes, usize size) {
    if (!channel || !channel->handle || !channel->event || channel->pending ||
        operation < pipe_operation_connect || operation > pipe_operation_write ||
        (operation != pipe_operation_connect && (!bytes || !size || size > UINT32_MAX)))
        return pipe_error_invalid;
#if defined(_WIN32)
    ResetEvent(channel->event);
    channel->operation = (OVERLAPPED){0};
    channel->operation.hEvent = channel->event;
    DWORD transferred = 0;
    BOOL started = operation == pipe_operation_connect ? ConnectNamedPipe(channel->handle, &channel->operation) :
                   operation == pipe_operation_read ? ReadFile(channel->handle, bytes, (DWORD)size, &transferred, &channel->operation) :
                   WriteFile(channel->handle, bytes, (DWORD)size, &transferred, &channel->operation);
    DWORD error = started ? ERROR_SUCCESS : GetLastError();
    if (started || (operation == pipe_operation_connect && error == ERROR_PIPE_CONNECTED)) {
        channel->transferred = transferred;
        channel->pending = 2;
        return pipe_error_pending;
    }
    if (error == ERROR_IO_PENDING) {
        channel->pending = 1;
        return pipe_error_pending;
    }
    return pipe_windows_error(error);
#elif defined(__APPLE__) || defined(__linux__)
    if (operation == pipe_operation_connect && (!channel->listener || channel->descriptor))
        return pipe_error_invalid;
    if (operation != pipe_operation_connect && !channel->descriptor)
        return pipe_error_invalid;
    channel->operation = operation;
    channel->bytes = bytes;
    channel->size = size;
    channel->used = channel->header_used = 0;
    channel->cancelled = 0;
    channel->message_size = operation == pipe_operation_write ? (u32)size : 0;
    for (usize i = 0; i < 4; ++i)
        channel->header[i] = (u8)(channel->message_size >> (i * 8));
    channel->pending = 1;
    return pipe_error_pending;
#else
    return pipe_error_unsupported;
#endif
}

static pipe_error pipe_poll(pipe_channel *channel, usize *transferred) {
    if (!channel || !channel->handle || !channel->pending || !transferred)
        return pipe_error_invalid;
#if defined(_WIN32)
    DWORD bytes = (DWORD)channel->transferred;
    if (channel->pending == 1 && !GetOverlappedResult(channel->handle, &channel->operation, &bytes, FALSE)) {
        DWORD error = GetLastError();
        if (error == ERROR_IO_INCOMPLETE)
            return pipe_error_pending;
        channel->pending = 0;
        return pipe_windows_error(error);
    }
    channel->pending = 0;
    channel->transferred = *transferred = bytes;
    return pipe_error_ok;
#elif defined(__APPLE__) || defined(__linux__)
    if (channel->cancelled) {
        channel->pending = 0;
        return pipe_error_io;
    }
    if (channel->operation == pipe_operation_connect) {
        int descriptor = accept(channel->listener - 1, NULL, NULL);
        if (descriptor < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return pipe_error_pending;
            channel->pending = 0;
            return pipe_error_io;
        }
        if (!pipe_macos_owner(descriptor) || fcntl(descriptor, F_SETFL, O_NONBLOCK) ||
            fcntl(descriptor, F_SETFD, FD_CLOEXEC) || player_socket_policy(descriptor)) {
            close(descriptor);
            channel->pending = 0;
            return pipe_error_denied;
        }
        channel->descriptor = descriptor + 1;
        channel->pending = 0;
        *transferred = 0;
        return pipe_error_ok;
    }
    int descriptor = channel->descriptor - 1;
    int writing = channel->operation == pipe_operation_write;
    /* At most header and payload syscalls per poll, even with partial I/O. */
    if (channel->header_used < 4) {
        ssize_t count = writing ? send(descriptor, channel->header + channel->header_used,
                                       4 - channel->header_used, PLAYER_SEND_FLAGS) :
                                  recv(descriptor, channel->header + channel->header_used,
                                       4 - channel->header_used, 0);
        if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return pipe_error_pending;
        if (count <= 0) {
            channel->pending = 0;
            return pipe_error_io;
        }
        channel->header_used += (usize)count;
        if (channel->header_used < 4)
            return pipe_error_pending;
        if (!writing) {
            channel->message_size = 0;
            for (usize i = 0; i < 4; ++i)
                channel->message_size |= (u32)channel->header[i] << (i * 8);
            if (!channel->message_size || channel->message_size > channel->size) {
                channel->pending = 0;
                return channel->message_size ? pipe_error_capacity : pipe_error_io;
            }
        }
    }
    usize remaining = channel->message_size - channel->used;
    ssize_t count = writing ? send(descriptor, (u8 *)channel->bytes + channel->used, remaining, PLAYER_SEND_FLAGS) :
                              recv(descriptor, (u8 *)channel->bytes + channel->used, remaining, 0);
    if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return pipe_error_pending;
    if (count <= 0) {
        channel->pending = 0;
        return pipe_error_io;
    }
    channel->used += (usize)count;
    if (channel->used != channel->message_size)
        return pipe_error_pending;
    channel->pending = 0;
    channel->transferred = *transferred = channel->used;
    return pipe_error_ok;
#else
    return pipe_error_unsupported;
#endif
}

/* Timeout leaves operation pending. Cancel, then drain before reusing storage. */
static pipe_error pipe_wait(pipe_channel *channel, u32 timeout_ms, usize *transferred) {
    if (!channel || !channel->pending || !transferred || timeout_ms == UINT32_MAX)
        return pipe_error_invalid;
#if defined(_WIN32)
    if (channel->pending == 1) {
        DWORD result = WaitForSingleObject(channel->event, timeout_ms);
        if (result == WAIT_TIMEOUT)
            return pipe_error_timeout;
        if (result != WAIT_OBJECT_0)
            return pipe_error_io;
    }
    return pipe_poll(channel, transferred);
#elif defined(__APPLE__) || defined(__linux__)
    u32 started = system_milliseconds();
    for (;;) {
        pipe_error error = pipe_poll(channel, transferred);
        if (error != pipe_error_pending)
            return error;
        u32 elapsed = system_milliseconds() - started;
        if (elapsed >= timeout_ms)
            return pipe_error_timeout;
        int descriptor = channel->operation == pipe_operation_connect ? channel->listener : channel->descriptor;
        struct pollfd wait = {descriptor - 1, channel->operation == pipe_operation_write ? POLLOUT : POLLIN, 0};
        u32 remaining = timeout_ms - elapsed;
        int delay = remaining > INT32_MAX ? INT32_MAX : (int)remaining;
        int result = poll(&wait, 1, delay);
        if (result < 0 && errno != EINTR)
            return pipe_error_io;
    }
#else
    return pipe_error_unsupported;
#endif
}
static pipe_error pipe_cancel(pipe_channel *channel) {
    if (!channel || !channel->handle)
        return pipe_error_invalid;
#if defined(_WIN32)
    /* Operations must be begun/cancelled on same caller thread. */
    if (channel->pending == 1 && !CancelIo(channel->handle))
        return pipe_error_io;
    return pipe_error_ok;
#elif defined(__APPLE__) || defined(__linux__)
    if (channel->pending)
        channel->cancelled = 1;
    return pipe_error_ok;
#else
    return pipe_error_unsupported;
#endif
}
static pipe_error pipe_server_disconnect(pipe_channel *channel) {
    if (!channel || !channel->handle || channel->pending)
        return pipe_error_invalid;
#if defined(_WIN32)
    if (!DisconnectNamedPipe(channel->handle) && GetLastError() != ERROR_PIPE_NOT_CONNECTED)
        return pipe_error_io;
    return pipe_error_ok;
#elif defined(__APPLE__) || defined(__linux__)
    if (!channel->listener)
        return pipe_error_invalid;
    int result = !channel->descriptor || close(channel->descriptor - 1) == 0;
    channel->descriptor = 0;
    return result ? pipe_error_ok : pipe_error_io;
#else
    return pipe_error_unsupported;
#endif
}

/* Close is bounded. Cancellation timeout retains handles/backing for later
   drain/close; do not free channel or I/O buffers while operation remains live. */
static pipe_error pipe_close(pipe_channel *channel, u32 timeout_ms) {
    if (!channel || timeout_ms == UINT32_MAX)
        return pipe_error_invalid;
#if defined(_WIN32)
    if (channel->pending == 1) {
        pipe_cancel(channel);
        usize ignored;
        pipe_error error = pipe_wait(channel, timeout_ms, &ignored);
        if (channel->pending)
            return error;
    }
    int failed = 0;
    if (channel->handle && !CloseHandle(channel->handle))
        failed = 1;
    if (channel->event && !CloseHandle(channel->event))
        failed = 1;
    *channel = (pipe_channel){0};
    return failed ? pipe_error_io : pipe_error_ok;
#elif defined(__APPLE__) || defined(__linux__)
    int failed = 0;
    if (channel->descriptor && close(channel->descriptor - 1))
        failed = 1;
    if (channel->listener) {
        if (close(channel->listener - 1))
            failed = 1;
        struct stat status;
        if (!lstat(channel->path, &status) && status.st_dev == channel->device &&
            status.st_ino == channel->inode && S_ISSOCK(status.st_mode) &&
            status.st_uid == getuid() && unlink(channel->path))
            failed = 1;
    }
    *channel = (pipe_channel){0};
    return failed ? pipe_error_io : pipe_error_ok;
#else
    return pipe_error_unsupported;
#endif
}
