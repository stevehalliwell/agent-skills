/* SPDX-License-Identifier: MIT */
#ifndef INK99_PLAYER_NATIVE_H
#define INK99_PLAYER_NATIVE_H
/* Include before other headers for POSIX credentials declarations. */
#ifndef _WIN32
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif
#ifdef __APPLE__
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE 1
#endif
#endif
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <poll.h>
#include <dirent.h>
#include <spawn.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif
typedef size_t usize;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
#if defined(__linux__)
#define PLAYER_SEND_FLAGS MSG_NOSIGNAL
static int player_socket_policy(int descriptor) { (void)descriptor; return 0; }
#elif defined(__APPLE__)
#define PLAYER_SEND_FLAGS 0
static int player_socket_policy(int descriptor) {
    int enabled = 1;
    return setsockopt(descriptor, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled));
}
#endif
#include "pipe.inc"
#endif
