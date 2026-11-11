/* Controlled native child for test.h command contracts. No Attendant bindings. */
#include "base.h"
#if defined(_WIN32)
#include <shellapi.h>
#include <fcntl.h>
#include <io.h>
#else
#include <signal.h>
#endif
static void child_sleep(unsigned milliseconds) {
#if defined(_WIN32)
    Sleep(milliseconds);
#else
    struct timespec delay = {milliseconds / 1000, (long)(milliseconds % 1000) * 1000000};
    nanosleep(&delay, NULL);
#endif
}
int main(int argc, char **argv) {
#if defined(_WIN32)
    static char text[PROCESS_COMMAND_SIZE * 3];
    static char *arguments[PROCESS_MAX_ARGUMENTS];
    int count = 0; wchar_t **wide = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!wide || count > PROCESS_MAX_ARGUMENTS) return 90;
    usize offset = 0;
    for (int i = 0; i < count; ++i) {
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide[i], -1, text + offset, sizeof(text) - offset, NULL, NULL);
        if (!size) { LocalFree(wide); return 90; }
        arguments[i] = text + offset; offset += size;
    }
    LocalFree(wide); argc = count; argv = arguments;
    _setmode(_fileno(stdin), _O_BINARY); _setmode(_fileno(stdout), _O_BINARY); _setmode(_fileno(stderr), _O_BINARY);
#endif
    if (argc < 2) return 91;
    if (!strcmp(argv[1], "args")) {
        for (int i = 2; i < argc; ++i) printf("%zu:%s\n", strlen(argv[i]), argv[i]);
    } else if (!strcmp(argv[1], "echo")) {
        u8 buffer[8192]; usize size;
        while ((size = fread(buffer, 1, sizeof(buffer), stdin))) if (fwrite(buffer, 1, size, stdout) != size) return 92;
        if (ferror(stdin)) return 92;
        fputs("error café Ω\n", stderr);
    } else if (!strcmp(argv[1], "environment") && argc == 3) {
        u8 memory[131072]; char value[32768]; usize size;
        environment_error error = environment_get(arena_from_buffer(memory, sizeof(memory)), argv[2], value, sizeof(value), &size);
        if (error == environment_error_not_found) fputs("<absent>", stdout);
        else if (error != environment_error_ok) return 93;
        else if (size && fwrite(value, 1, size, stdout) != size) return 93;
    } else if (!strcmp(argv[1], "cwd")) {
        char path[NATIVE_PATH_SIZE];
#if defined(_WIN32)
        wchar_t wide[NATIVE_PATH_SIZE];
        DWORD count = GetCurrentDirectoryW(NATIVE_PATH_SIZE, wide);
        if (!count || count >= NATIVE_PATH_SIZE || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide, -1, path, sizeof(path), NULL, NULL)) return 94;
        for (char *p = path; *p; ++p) if (*p == '\\') *p = '/';
#else
        if (!getcwd(path, sizeof(path))) return 94;
#endif
        fputs(path, stdout);
    } else if (!strcmp(argv[1], "live") && argc == 3) {
        fputs("rea", stdout); fflush(stdout); child_sleep(25);
        fputs("dy\n", stdout); fflush(stdout);
        fputs("live-errors\n", stderr); fflush(stderr);
        unsigned attempt;
        for (attempt = 0; attempt < 10000 && file_stat((cstring){argv[2]}).error != file_error_ok; ++attempt) child_sleep(1);
        if (attempt == 10000) return 101;
        fputs("done\n", stdout);
    } else if (!strcmp(argv[1], "sleep")) child_sleep(10000);
#if !defined(_WIN32)
    else if (!strcmp(argv[1], "join-group") && argc == 3) {
        if (setpgid(0, (pid_t)strtol(argv[2], NULL, 10))) return 95;
        child_sleep(10000);
    }
#endif
    else if (!strcmp(argv[1], "exit") && argc == 3) return atoi(argv[2]);
    else if (!strcmp(argv[1], "crash")) {
#if defined(_WIN32)
        TerminateProcess(GetCurrentProcess(), 0xc0000005UL);
#else
        raise(SIGKILL);
#endif
        return 95;
    } else if (!strcmp(argv[1], "flood")) {
        u8 bytes[4096]; memset(bytes, 'x', sizeof(bytes));
        for (unsigned i = 0; i < 256; ++i)
            if (fwrite(bytes, 1, sizeof(bytes), stdout) != sizeof(bytes) || fwrite(bytes, 1, sizeof(bytes), stderr) != sizeof(bytes)) return 96;
    } else if ((!strcmp(argv[1], "heartbeat") || !strcmp(argv[1], "heartbeat-closed")) && argc == 3) {
        FILE *file = file_open_utf8(argv[2], "wb");
        if (!file) return 97;
        child_sleep(25); /* Force creation-before-readiness scheduling window. */
        if (!strcmp(argv[1], "heartbeat-closed")) {
#if defined(_WIN32)
            CloseHandle(GetStdHandle(STD_INPUT_HANDLE)); CloseHandle(GetStdHandle(STD_OUTPUT_HANDLE)); CloseHandle(GetStdHandle(STD_ERROR_HANDLE));
            SetStdHandle(STD_INPUT_HANDLE, INVALID_HANDLE_VALUE); SetStdHandle(STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE); SetStdHandle(STD_ERROR_HANDLE, INVALID_HANDLE_VALUE);
#else
            close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO);
#endif
        }
#if defined(_WIN32)
        unsigned long process = GetCurrentProcessId();
#else
        unsigned long process = (unsigned long)getpid();
#endif
        if (fprintf(file, "%lu\n", process) < 0 || fflush(file)) { fclose(file); return 97; }
        char ready[NATIVE_PATH_SIZE];
        if (snprintf(ready, sizeof(ready), "%s.ready", argv[2]) >= (int)sizeof(ready) ||
            file_create_text((cstring){ready}, "ready") != file_error_ok) { fclose(file); return 97; }
        for (unsigned i = 0; i < 1500; ++i) { fputc('x', file); fflush(file); child_sleep(20); }
        if (fclose(file)) return 97;
    } else if ((!strcmp(argv[1], "spawn-heartbeat") || !strcmp(argv[1], "spawn-heartbeat-closed")) &&
               (argc == 3 || (argc == 4 && !strcmp(argv[3], "hold")))) {
        int closed = !strcmp(argv[1], "spawn-heartbeat-closed");
#if defined(_WIN32)
        wchar_t self[NATIVE_PATH_SIZE], target[NATIVE_PATH_SIZE], line[NATIVE_PATH_SIZE * 2 + 32];
        if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, argv[0], -1, self, NATIVE_PATH_SIZE) ||
            !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, argv[2], -1, target, NATIVE_PATH_SIZE)) return 98;
        _snwprintf(line, sizeof(line) / sizeof(line[0]), L"\"%ls\" %ls \"%ls\"", self, closed ? L"heartbeat-closed" : L"heartbeat", target);
        STARTUPINFOW startup = {0}; PROCESS_INFORMATION process = {0};
        startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE); startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE); startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        if (!CreateProcessW(self, line, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startup, &process)) return 98;
        CloseHandle(process.hThread); CloseHandle(process.hProcess);
#else
        extern char **environ;
        char *arguments[] = {argv[0], closed ? "heartbeat-closed" : "heartbeat", argv[2], NULL}; pid_t process;
        if (posix_spawn(&process, argv[0], NULL, NULL, arguments, environ)) return 98;
#endif
        char ready[NATIVE_PATH_SIZE];
        if (snprintf(ready, sizeof(ready), "%s.ready", argv[2]) >= (int)sizeof(ready)) return 98;
        for (unsigned i = 0; i < 1000 && file_stat((cstring){ready}).error != file_error_ok; ++i) child_sleep(1);
        if (file_stat((cstring){ready}).error != file_error_ok) return 98;
        fputs("spawned\n", stdout); fflush(stdout);
        if (argc == 4) {
            char release[NATIVE_PATH_SIZE];
            if (snprintf(release, sizeof(release), "%s.release", argv[2]) >= (int)sizeof(release)) return 98;
            unsigned attempt;
            for (attempt = 0; attempt < 10000 && file_stat((cstring){release}).error != file_error_ok; ++attempt) child_sleep(1);
            if (attempt == 10000) return 102;
        }
    } else return 99;
    return ferror(stdout) || ferror(stderr) ? 100 : 0;
}
