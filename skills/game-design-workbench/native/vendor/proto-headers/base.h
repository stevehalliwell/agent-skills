/*
base.h - single-file, header-only C base library

README
Copy this file into a project and #include "base.h". No companion project
headers or generated source files are needed. Uses the C standard library and
platform branches for Windows and POSIX systems; collection macros require a
compiler with __typeof__. Keep application- and language-specific features out.

Contents:
- arena: header-in-block allocator, caller-buffer or owned virtual-memory
  backed, with optional chaining and free-list reuse
- free_list: fixed caller-backed variable-size allocation and individual free;
  compile-time fit/alignment/minimum payload, splitting and coalescing
- string: byte strings, views, builder
- file: whole-file and caller-buffer read/write/stat helpers, Windows mutable
  file mapping with explicit flush/unmap
- memory_region: dedicated page-isolated OS backing with read-only/read-write protection
- file_watcher: allocator-backed multiple-file watching and caller-owned change bitset
- collections: fixed views, dynamic arrays, and hash maps with shared operations

Collection use: Array(T) is a non-owning view. DynamicArray(T) and HashMap(K, V)
are arena-backed; initialize .raw.arena before use. Maps also need .raw.hash
and .raw.equal. Pass lvalue pointers to operations and treat incompatible-pointer
warnings as errors. Growth invalidates pointers into collection storage; do not
use that storage after resetting or releasing its arena.

Allocation initialization: arena_alloc and free_list_alloc zero requested bytes,
including reused allocations. The corresponding _uninit functions leave bytes
uninitialized; _pattern functions fill requested bytes with the supplied pattern.
Alignment/minimum-size padding is not part of this initialization guarantee.

Free-list use: supply one buffer whose lifetime covers all returned pointers.
free_list *memory = free_list_from_buffer(backing, sizeof(backing));
arena_result allocation = free_list_alloc(memory, 128);
Handle initialization/allocation failure before using allocation.ptr.
free_list_free(memory, allocation.ptr);
free_list_reset(memory);
Reset zeros backing and invalidates every outstanding pointer.
Allocation fills only requested bytes; alignment/minimum-size padding is not
initialized. Free requires an exact live pointer from the same allocator.

CHANGELOG
- 2026-10-14: Added protected memory regions and multiple-file watching.
- 2026-10-12: Added caller-backed free-list allocator and compile-time policy.
- 2026-09-25: Replaced per-type generated collection functions with Array(T)
  views and shared, type-checked DynamicArray(T) and HashMap(K, V) operations.
  Kept the implementation self-contained.

REFERENCES
- Daniel Chase Hooper, "Type Safe Generic Data Structures in C":
  https://danielchasehooper.com/posts/typechecked-generic-c-data-structures/
- Ryan Fleury, RAD Debugger arena allocator (types and implementation):
  https://github.com/EpicGames/raddebugger/blob/master/src/base/base_arena.h
  https://github.com/EpicGames/raddebugger/blob/master/src/base/base_arena.c
*/

#ifndef BASE_H
#define BASE_H

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
/* raylib exports Rectangle, CloseWindow and ShowCursor; exclude Win32 GDI/user declarations. */
#define NOGDI
#define NOUSER
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <dirent.h>
#include <fcntl.h>
#include <spawn.h>
#include <strings.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <sys/mman.h>
#if defined(__APPLE__)
#include <sys/event.h>
#include <mach-o/dyld.h>
#endif
#include <time.h>
#include <unistd.h>
#endif

#if defined(__APPLE__) && defined(__TINYC__)
/* Apple's float.h expects compiler builtins absent from CJIT 1.2.0. */
#ifndef __DBL_MAX__
#define __DBL_MAX__ 1.7976931348623157e+308
#endif
#ifndef __FLT_MAX__
#define __FLT_MAX__ 3.4028234663852886e+38F
#endif
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef ptrdiff_t isize;
typedef uintptr_t uptr;

#if defined(__SIZEOF_INT128__) && !defined(__TINYC__)
typedef unsigned __int128 u128;
#else
typedef struct u128 {
    u64 low;
    u64 high;
} u128;
#endif

static u128 u128_from_u64(u64 value);
static u128 u128_add(u128 left, u128 right);
static u128 u128_mul_u64(u64 left, u64 right);
static u128 u128_div_u64(u128 numerator, u64 divisor);
static u64 u128_low(u128 value);
static u64 u128_isqrt(u128 value);

/* Fundamental scalar alignment on supported targets; not for explicitly over-aligned types. */
typedef union base_max_align {
    long double ld;
    long long integer;
    void *pointer;
    void (*function)(void);
} base_max_align;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(_Alignof(base_max_align) >= _Alignof(max_align_t),
               "base_max_align must cover the platform's fundamental alignment");
#endif

typedef enum arena_error {
    arena_error_ok = 0,
    arena_error_invalid,
    arena_error_out_of_memory,
} arena_error;

typedef struct arena_result {
    void *ptr;
    arena_error error;
} arena_result;

#define ARENA_DEFAULT_RESERVE_SIZE (64u * 1024u * 1024u)
#define ARENA_DEFAULT_COMMIT_SIZE (64u * 1024u)

typedef u32 arena_flags;

enum {
    arena_flag_no_chain = 1u << 0,
    arena_flag_free_list = 1u << 1,
    arena_flag_backing_buffer = 1u << 2,
};

typedef struct arena {
    struct arena *prev;
    struct arena *current;
    arena_flags flags;
    usize reserve_size;
    usize commit_size;
    usize base_pos;
    usize pos;
    usize committed;
    usize reserved;
    struct arena *free_last;
} arena;

typedef struct arena_params {
    arena_flags flags;
    usize reserve_size;
    usize commit_size;
    void *base_address;
} arena_params;

/* Free-list constants must match in every translation unit sharing an allocator.
   Alignment applies to block starts and payloads; minimum counts payload bytes.
   First-fit stops at first fitting block; best-fit scans for smallest fit. */
#define FREE_LIST_FIRST_FIT 0
#define FREE_LIST_BEST_FIT 1
#ifndef FREE_LIST_FIT_POLICY
#define FREE_LIST_FIT_POLICY FREE_LIST_FIRST_FIT
#endif
#ifndef FREE_LIST_ALIGNMENT
#define FREE_LIST_ALIGNMENT 16u
#endif
#ifndef FREE_LIST_MIN_PAYLOAD
#define FREE_LIST_MIN_PAYLOAD 16u
#endif

#if FREE_LIST_FIT_POLICY != FREE_LIST_FIRST_FIT && FREE_LIST_FIT_POLICY != FREE_LIST_BEST_FIT
#error "Invalid FREE_LIST_FIT_POLICY"
#endif

/* Two machine words before each payload. Size is total block span, including
   aligned header storage. Next is used only while block is free. */
typedef struct free_list_block {
    usize size;
    struct free_list_block *next;
} free_list_block;

/* Metadata lives in supplied backing. Backing size counts whole caller block;
   used counts allocated spans, including block headers. Managed region/capacity
   derive from backing and aligned metadata size. No growth, relocation or
   synchronization. Search/return work is bounded by capacity / minimum block
   span, not a separate arbitrary block-count limit. */
typedef struct free_list {
    void *backing;
    usize backing_size;
    usize used;
    free_list_block *first;
} free_list;

_Static_assert(FREE_LIST_ALIGNMENT != 0 &&
                   (FREE_LIST_ALIGNMENT & (FREE_LIST_ALIGNMENT - 1u)) == 0,
               "FREE_LIST_ALIGNMENT must be a power of two");
_Static_assert(FREE_LIST_ALIGNMENT >= _Alignof(free_list) &&
                   FREE_LIST_ALIGNMENT >= _Alignof(free_list_block),
               "FREE_LIST_ALIGNMENT must align allocator metadata");
_Static_assert(sizeof(free_list_block) == 2 * sizeof(usize),
               "Free-list header must occupy two machine words");
_Static_assert(FREE_LIST_MIN_PAYLOAD > 0 &&
                   FREE_LIST_MIN_PAYLOAD <=
                       SIZE_MAX - ((sizeof(free_list_block) + FREE_LIST_ALIGNMENT - 1u) &
                                   ~(usize)(FREE_LIST_ALIGNMENT - 1u)) -
                           (FREE_LIST_ALIGNMENT - 1u),
               "FREE_LIST_MIN_PAYLOAD must fit with aligned header");

/* Typed, non-owning fixed view. Use a typedef when passing one by value. */
#define Array(type)                                                                                \
    struct {                                                                                       \
        type *data;                                                                                \
        usize len;                                                                                 \
    }

#define array_raw_count(raw)                                                                       \
    (sizeof(raw) / sizeof((raw)[0]) +                                                              \
     0 * sizeof(struct {                                                                           \
         int array_from_raw_requires_array                                                         \
             : !__builtin_types_compatible_p(__typeof__(raw), __typeof__(&(raw)[0]))               \
               ? 1                                                                                 \
               : -1;                                                                               \
     }))

#define array_from_raw(raw) {(raw), array_raw_count(raw)}

typedef struct string {
    u8 *data;
    usize len;
} string;

typedef struct cstring {
    char *data;
} cstring;

typedef struct string_view {
    const u8 *data;
    usize len;
} string_view;

typedef struct duration_unit {
    u64 divisor;
    const char *suffix;
} duration_unit;

#if defined(__GNUC__) || defined(__clang__)
#define COMPILER_DO_NOT_OPTIMIZE(value) __asm__ volatile("" : : "r,m"(value) : "memory")
#define COMPILER_CLOBBER_MEMORY() __asm__ volatile("" : : : "memory")
#else
#define COMPILER_DO_NOT_OPTIMIZE(value) ((void)(value))
#define COMPILER_CLOBBER_MEMORY() ((void)0)
#endif

/* Arena */
static int arena_alignment_valid(usize align);
static arena_result arena_fail(arena_error error);
static usize arena_header_size(void);
static usize arena_page_size(void);
static usize arena_page_align(usize size);
static void *arena_memory_reserve_at(void *address, usize size);
static void *arena_memory_reserve(usize size);
static int arena_memory_commit(void *ptr, usize size);
static void arena_memory_release(void *ptr, usize size);
typedef struct arena_position_marker {
    arena *arena;
    usize pos;
} arena_position_marker;

static arena *arena_from_buffer(void *buffer, usize size);
static arena *arena_create_block(arena_params params, usize base_pos);
static arena *arena_create(arena_params params);
static void arena_release(arena *state);
static int arena_commit_to(arena *state, usize pos);
static usize arena_pos(arena *state);
static arena_error arena_pop_to(arena *state, usize pos);
static arena_position_marker arena_position_mark(arena *state);
static arena_error arena_pop_to_marker(arena_position_marker marker);
static void arena_fill_pattern(void *ptr, usize size, u32 pattern);
static arena_result
arena_alloc_internal(arena *state, usize size, usize align, int fill, u32 pattern);
static arena_result arena_alloc_pattern(arena *state, usize size, usize align, u32 pattern);
static arena_result arena_alloc(arena *state, usize size, usize align);
static arena_result arena_alloc_uninit(arena *state, usize size, usize align);
static void arena_reset(arena *state);

/* Free list: from_buffer aligns within caller's block, returns NULL if too small
   or invalid. Zero-size allocation is invalid; exhausted capacity returns error.
   Free requires exact live pointer from this allocator; NULL is a no-op.
   Reset zeros entire supplied backing and rebuilds one free span, invalidating
   all allocations. Never reset while a client retains live pointers. */
static usize free_list_header_size(void);
static usize free_list_min_block_size(void);
static u8 *free_list_memory(free_list *state);
static usize free_list_capacity(free_list *state);
static free_list *free_list_from_buffer(void *buffer, usize size);
static arena_result free_list_alloc_pattern(free_list *state, usize size, u32 pattern);
static arena_result free_list_alloc(free_list *state, usize size);
static arena_result free_list_alloc_uninit(free_list *state, usize size);
static void free_list_free(free_list *state, void *pointer);
static void free_list_reset(free_list *state);

/* Hash */
static u64 hash_fnv1a_bytes(const void *data, usize len);

/* Stats */
static u64 u64_mean(const u64 *values, u64 count);
static u64 u64_stddev(const u64 *values, u64 count, u64 mean);
static u64 u64_mean_kept(const u64 *values, const int *kept, u64 count, u64 *out_kept);
static u64 u64_stddev_kept(const u64 *values, const int *kept, u64 count, u64 mean);
static u64 u64_mode_kept(const u64 *values, const int *kept, u64 count);
static u64 u64_mark_outliers_stddev(const u64 *values, int *kept, u64 count, double sigma);

/* String */
static string string_from_raw(u8 *data, usize len);
static cstring cstring_from_raw(char *data);
static string string_from_cstring(cstring value);
static cstring cstring_from_string(arena *arena, string value);
static string_view string_view_from_cstr(const char *data);
static string_view string_view_from_cstring(cstring value);
static string_view string_view_from_string_full(string value);
static string_view string_view_from_view(string_view value, usize start, usize len);
static string_view string_view_from_string(string value, usize start, usize len);
static int string_view_eq(string_view left, string_view right);

/* Clock / duration / argv */
static u64 clock_now_ns(void);
static u64 clock_now_us(void);
static duration_unit duration_unit_for_ps(u64 picoseconds);
static int
duration_format_in_unit(char *buffer, usize buffer_size, u64 picoseconds, duration_unit unit);
static int duration_format_ps(char *buffer, usize buffer_size, u64 picoseconds);
static int argv_has_flag(int argc, char **argv, const char *flag);
static char *argv_last(int argc, char **argv);

/* Shared, arena-backed collections. Typed union members carry compile-time
   type information; callers pass lvalue pointers and treat incompatible-pointer
   diagnostics as errors (for example, Clang -Werror=pointer-type-mismatch).
   Growth retains old storage until arena reset/release. Do not use collection
   storage after resetting its arena. */
typedef struct dynamic_array_raw {
    void *data;
    usize len;
    usize cap;
    arena *arena;
} dynamic_array_raw;

#define DynamicArray(type)                                                                         \
    union {                                                                                        \
        dynamic_array_raw raw;                                                                     \
        type *payload;                                                                             \
    }
#define dynamic_array_data(array) ((__typeof__((array)->payload))((array)->raw.data))

static arena_error
_dynamic_array_reserve_raw(dynamic_array_raw *array, usize min_cap, usize size, usize align) {
    arena_result allocation;
    if (array == NULL || array->arena == NULL || size == 0 || array->len > array->cap ||
        (array->len != 0 && array->data == NULL)) {
        return arena_error_invalid;
    }
    if (min_cap <= array->cap) {
        return arena_error_ok;
    }
    if (min_cap > (usize)-1 / size) {
        return arena_error_out_of_memory;
    }
    allocation = arena_alloc_uninit(array->arena, min_cap * size, align);
    if (allocation.error != arena_error_ok) {
        return allocation.error;
    }
    if (array->len != 0) {
        memcpy(allocation.ptr, array->data, array->len * size);
    }
    array->data = allocation.ptr;
    array->cap = min_cap;
    return arena_error_ok;
}

static arena_error
_dynamic_array_push_raw(dynamic_array_raw *array, const void *item, usize size, usize align) {
    arena_error error;
    usize max_cap;
    usize next_cap;
    if (array == NULL || array->arena == NULL || item == NULL || size == 0 ||
        array->len > array->cap || (array->len != 0 && array->data == NULL)) {
        return arena_error_invalid;
    }
    max_cap = (usize)-1 / size;
    if (array->len >= max_cap) {
        return arena_error_out_of_memory;
    }
    if (array->len == array->cap) {
        next_cap = array->cap == 0 ? 8 : array->cap;
        if (next_cap > max_cap / 2) {
            next_cap = max_cap;
        } else if (array->cap != 0) {
            next_cap *= 2;
        }
        error = _dynamic_array_reserve_raw(array, next_cap, size, align);
        if (error != arena_error_ok) {
            return error;
        }
    }
    memcpy((u8 *)array->data + array->len * size, item, size);
    array->len++;
    return arena_error_ok;
}

static void _dynamic_array_pop_raw(dynamic_array_raw *array) {
    if (array != NULL && array->len > 0) {
        array->len--;
    }
}

static void _dynamic_array_pop_swap_back_raw(dynamic_array_raw *array, usize index, usize size) {
    if (array == NULL || index >= array->len) {
        return;
    }
    array->len--;
    if (index != array->len) {
        memcpy((u8 *)array->data + index * size, (u8 *)array->data + array->len * size, size);
    }
}

#define dynamic_array_reserve(array, cap)                                                          \
    _dynamic_array_reserve_raw(&(array)->raw,                                                      \
                               (cap),                                                              \
                               sizeof(*(array)->payload),                                          \
                               _Alignof(__typeof__(*(array)->payload)))
#define dynamic_array_push(array, item_ptr)                                                        \
    _dynamic_array_push_raw(&(array)->raw,                                                         \
                            (1 ? (item_ptr) : (array)->payload),                                   \
                            sizeof(*(array)->payload),                                             \
                            _Alignof(__typeof__(*(array)->payload)))
#define dynamic_array_pop(array) _dynamic_array_pop_raw(&(array)->raw)
#define dynamic_array_pop_swap_back(array, index)                                                  \
    _dynamic_array_pop_swap_back_raw(&(array)->raw, (index), sizeof(*(array)->payload))

typedef u64 (*hash_map_hash)(const void *key);
typedef int (*hash_map_equal)(const void *left, const void *right);

typedef struct hash_map_slot {
    u64 hash;
    int occupied;
} hash_map_slot;

typedef struct hash_map_raw {
    hash_map_slot *slots;
    void *keys;
    void *values;
    usize len;
    usize cap;
    arena *arena;
    hash_map_hash hash;
    hash_map_equal equal;
} hash_map_raw;

#define HashMap(key_type, value_type)                                                              \
    union {                                                                                        \
        hash_map_raw raw;                                                                          \
        key_type *key;                                                                             \
        value_type *value;                                                                         \
    }

static usize
_hash_map_find_raw(const hash_map_raw *map, const void *key, u64 hash, usize key_size) {
    usize i;
    usize start = hash & (map->cap - 1u);
    for (i = 0; i < map->cap; i++) {
        usize index = (start + i) & (map->cap - 1u);
        if (!map->slots[index].occupied || (map->slots[index].hash == hash &&
                                            map->equal((u8 *)map->keys + index * key_size, key))) {
            return index;
        }
    }
    return map->cap;
}

static arena_error _hash_map_reserve_raw(hash_map_raw *map,
                                         usize cap,
                                         usize key_size,
                                         usize key_align,
                                         usize value_size,
                                         usize value_align) {
    hash_map_raw next;
    arena_result slots;
    arena_result keys;
    arena_result values;
    usize i;
    if (map == NULL || map->arena == NULL || map->hash == NULL || map->equal == NULL ||
        key_size == 0 || value_size == 0) {
        return arena_error_invalid;
    }
    if (cap <= map->cap) {
        return arena_error_ok;
    }
    if ((cap & (cap - 1u)) != 0 || cap > (usize)-1 / sizeof(hash_map_slot) ||
        cap > (usize)-1 / key_size || cap > (usize)-1 / value_size) {
        return arena_error_invalid;
    }
    slots = arena_alloc_uninit(map->arena, cap * sizeof(hash_map_slot), _Alignof(hash_map_slot));
    if (slots.error != arena_error_ok)
        return slots.error;
    keys = arena_alloc_uninit(map->arena, cap * key_size, key_align);
    if (keys.error != arena_error_ok)
        return keys.error;
    values = arena_alloc_uninit(map->arena, cap * value_size, value_align);
    if (values.error != arena_error_ok)
        return values.error;

    next = *map;
    next.slots = (hash_map_slot *)slots.ptr;
    next.keys = keys.ptr;
    next.values = values.ptr;
    next.cap = cap;
    memset(next.slots, 0, cap * sizeof(hash_map_slot));
    for (i = 0; i < map->cap; i++) {
        if (map->slots[i].occupied) {
            const void *key = (const u8 *)map->keys + i * key_size;
            usize dest = _hash_map_find_raw(&next, key, map->slots[i].hash, key_size);
            next.slots[dest] = map->slots[i];
            memcpy((u8 *)next.keys + dest * key_size, key, key_size);
            memcpy((u8 *)next.values + dest * value_size,
                   (const u8 *)map->values + i * value_size,
                   value_size);
        }
    }
    *map = next;
    return arena_error_ok;
}

static arena_error _hash_map_put_raw(hash_map_raw *map,
                                     const void *key,
                                     const void *value,
                                     usize key_size,
                                     usize key_align,
                                     usize value_size,
                                     usize value_align) {
    usize index;
    u64 hash;
    arena_error error;
    if (map == NULL || map->arena == NULL || map->hash == NULL || map->equal == NULL ||
        key == NULL || value == NULL || key_size == 0 || value_size == 0 || map->len > map->cap) {
        return arena_error_invalid;
    }
    hash = map->hash(key);
    if (map->cap != 0) {
        index = _hash_map_find_raw(map, key, hash, key_size);
        if (index < map->cap && map->slots[index].occupied) {
            memmove((u8 *)map->values + index * value_size, value, value_size);
            return arena_error_ok;
        }
    }
    /* Equivalent to (len + 1) * 10 > cap * 7, without overflow. */
    if (map->cap == 0 || map->len + 1 > (map->cap / 10) * 7 + (map->cap % 10) * 7 / 10) {
        if (map->cap > (usize)-1 / 2)
            return arena_error_out_of_memory;
        error = _hash_map_reserve_raw(map,
                                      map->cap == 0 ? 8 : map->cap * 2,
                                      key_size,
                                      key_align,
                                      value_size,
                                      value_align);
        if (error != arena_error_ok)
            return error;
    }
    index = _hash_map_find_raw(map, key, hash, key_size);
    if (index == map->cap)
        return arena_error_out_of_memory;
    map->slots[index] = (hash_map_slot){.hash = hash, .occupied = 1};
    memcpy((u8 *)map->keys + index * key_size, key, key_size);
    memcpy((u8 *)map->values + index * value_size, value, value_size);
    map->len++;
    return arena_error_ok;
}

static void *
_hash_map_get_raw(hash_map_raw *map, const void *key, usize key_size, usize value_size) {
    usize index;
    if (map == NULL || key == NULL || map->cap == 0 || map->hash == NULL || map->equal == NULL)
        return NULL;
    index = _hash_map_find_raw(map, key, map->hash(key), key_size);
    if (index == map->cap || !map->slots[index].occupied)
        return NULL;
    return (u8 *)map->values + index * value_size;
}

#define hash_map_reserve(map, cap)                                                                 \
    _hash_map_reserve_raw(&(map)->raw,                                                             \
                          (cap),                                                                   \
                          sizeof(*(map)->key),                                                     \
                          _Alignof(__typeof__(*(map)->key)),                                       \
                          sizeof(*(map)->value),                                                   \
                          _Alignof(__typeof__(*(map)->value)))
#define hash_map_put(map, key_ptr, value_ptr)                                                      \
    _hash_map_put_raw(&(map)->raw,                                                                 \
                      (1 ? (key_ptr) : (map)->key),                                                \
                      (1 ? (value_ptr) : (map)->value),                                            \
                      sizeof(*(map)->key),                                                         \
                      _Alignof(__typeof__(*(map)->key)),                                           \
                      sizeof(*(map)->value),                                                       \
                      _Alignof(__typeof__(*(map)->value)))
#define hash_map_get(map, key_ptr)                                                                 \
    ((__typeof__((map)->value))_hash_map_get_raw(&(map)->raw,                                      \
                                                 (1 ? (key_ptr) : (map)->key),                     \
                                                 sizeof(*(map)->key),                              \
                                                 sizeof(*(map)->value)))
#define hash_map_has(map, key_ptr) (hash_map_get((map), (key_ptr)) != NULL)

typedef DynamicArray(string_view) string_view_array;
typedef DynamicArray(string) string_array;

typedef struct string_result {
    string value;
    arena_error error;
} string_result;

typedef enum file_error {
    file_error_ok = 0,
    file_error_invalid,
    file_error_open_failed,
    file_error_read_failed,
    file_error_write_failed,
    file_error_out_of_memory,
    file_error_not_found,
    file_error_capacity,
    file_error_size_mismatch,
    file_error_mapping_failed,
    file_error_flush_failed,
    file_error_cleanup_failed,
    file_error_unsupported,
} file_error;

/* Caller-owned, zero-initialized metadata. A Windows mapping owns its OS file,
   mapping handle and view until file_unmap; do not copy ownership or resize the
   file while mapped. Persistence scheduling belongs to the caller. */
typedef struct file_mapping {
    void *data;
    usize size;
    void *file_handle;
    void *mapping_handle;
} file_mapping;

/* Dedicated page-isolated OS backing. Metadata must live outside region.
   Zero-initialize; do not copy ownership. Unsupported targets fail explicitly. */
typedef enum memory_region_error {
    memory_region_ok,
    memory_region_invalid,
    memory_region_allocation_failed,
    memory_region_protection_failed,
    memory_region_release_failed,
    memory_region_unsupported,
} memory_region_error;

typedef struct memory_region {
    void *data;
    usize size, mapped_size;
} memory_region;

static memory_region_error memory_region_create(memory_region *region, usize size);
static memory_region_error memory_region_set_read_only(memory_region *region);
static memory_region_error memory_region_set_read_write(memory_region *region);
static memory_region_error memory_region_release(memory_region *region);

/* Startup allocator owns watch metadata/paths/buffers; close before rewinding.
   IDs are sequential. Poll ORs notifications into caller's bitset; never clears.
   Windows uses directory notifications, including replacement saves and missing
   leaves. macOS uses kqueue vnode notifications and reattaches replaced files.
   No background thread, debounce, automatic retry, or runtime backing growth. */
typedef struct file_watch_entry file_watch_entry;

typedef struct file_watcher {
    file_watch_entry *first, *last;
    usize count;
} file_watcher;

struct file_watch_entry {
    file_watch_entry *next;
    char *path;
#if defined(_WIN32)
    HANDLE directory, event;
    OVERLAPPED operation;
    WCHAR *name;
    usize name_units;
    DWORD buffer[4096]; /* 16 KiB; overflow explicitly reconciles every ID. */
#elif defined(__APPLE__)
    int queue, directory, file;
    char *directory_path; /* Prepared buffer for nearest existing parent lookup. */
    struct stat previous;
    int exists;
#endif
};

static file_error
file_watcher_add(file_watcher *watcher, arena *allocator, const char *path, usize *id);
static file_error file_watcher_poll(file_watcher *watcher, u64 *changed, usize word_count);
static file_error file_watcher_close(file_watcher *watcher);

typedef struct file_result {
    string value;
    file_error error;
} file_result;

typedef struct file_stat_result {
    u64 size;
    i64 modified_time;
    file_error error;
} file_stat_result;

typedef struct file_list_result {
    string_array paths;
    file_error error;
} file_list_result;

typedef struct string_builder {
    string_view_array chunks;
    arena *arena;
} string_builder;

typedef struct tsv_row {
    string_view_array fields;
    arena_error error;
} tsv_row;

/* File / path */
static file_error file_error_from_arena_error(arena_error error);
static file_result file_read_all(arena *arena, cstring path);
static file_error file_write_all(cstring path, string value);
/* No allocation. On failure size is unchanged; output can contain a partial
   read after an I/O failure. Writes can leave partial/truncated files. */
static file_error file_read_into(cstring path, void *output, usize capacity, usize *size);
static file_error file_write_bytes(cstring path, const void *bytes, usize size);
static const char *file_error_message(file_error error);
/* Windows only; other platforms return unsupported. New files are zeroed;
   existing files must match size exactly, including empty files. No migration,
   atomic transaction or crash-durability guarantee. Failed map leaves metadata
   unchanged; unmap attempts all cleanup and clears it even on cleanup failure. */
static file_error file_map(cstring path, usize size, file_mapping *mapping);
static file_error file_mapping_flush(const file_mapping *mapping);
static file_error file_unmap(file_mapping *mapping);
static file_stat_result file_stat(cstring path);
static int path_is_dir(string_view path, int *out_is_dir);
static int path_matches_pattern(string_view path, string_view pattern);
static string_result path_join(arena *arena, string_view dir, string_view name);
/* No matches in a valid directory succeed with an empty list. Windows directory,
   enumeration, and cleanup failures are explicit; paths may be partial on error. */
static file_list_result file_list(arena *arena, string_view dir, string_view pattern);

static arena_error string_builder_append(string_builder *builder, string_view value);
static string_result string_from_string_builder(string_builder *builder);
static cstring cstring_from_string_builder(string_builder *builder);
static arena_error tsv_write_field(string_builder *builder, string_view value);
static arena_error tsv_write_cstr_field(string_builder *builder, const char *value);
static arena_error tsv_write_u64_field(string_builder *builder, u64 value);
static arena_error tsv_write_separator(string_builder *builder);
static arena_error tsv_write_record_end(string_builder *builder);
static tsv_row tsv_read_row(arena *arena, string_view line);

/* Arena */
static int arena_alignment_valid(usize align) { return align != 0 && (align & (align - 1u)) == 0; }

static arena_result arena_fail(arena_error error) {
    arena_result result = {0};

    result.error = error;
    return result;
}

static usize arena_header_size(void) {
    usize align = _Alignof(base_max_align);

    return (sizeof(arena) + align - 1u) & ~(align - 1u);
}

static usize arena_page_size(void) {
#if defined(_WIN32)
    SYSTEM_INFO info;

    GetSystemInfo(&info);
    return (usize)info.dwPageSize;
#elif defined(_SC_PAGESIZE)
    long size = sysconf(_SC_PAGESIZE);

    if (size > 0) {
        return (usize)size;
    }
    return 4096u;
#else
    return 4096u;
#endif
}

static usize arena_page_align(usize size) {
    usize page_size = arena_page_size();

    if (size == 0 || page_size == 0) {
        return 0;
    }
    if (size > ((usize)-1) - (page_size - 1u)) {
        return 0;
    }
    return (size + page_size - 1u) & ~(page_size - 1u);
}

static void *arena_memory_reserve_at(void *address, usize size) {
    usize aligned_size = arena_page_align(size);

    if (aligned_size == 0) {
        return NULL;
    }
#if defined(_WIN32)
    return VirtualAlloc(address, aligned_size, MEM_RESERVE, PAGE_READWRITE);
#elif defined(MAP_PRIVATE) && (defined(MAP_ANONYMOUS) || defined(MAP_ANON))
    {
#if defined(MAP_ANONYMOUS)
        int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#else
        int flags = MAP_PRIVATE | MAP_ANON;
#endif
        void *ptr;

#if defined(MAP_FIXED_NOREPLACE)
        if (address != NULL) {
            flags |= MAP_FIXED_NOREPLACE;
        }
#endif
        ptr = mmap(address, aligned_size, PROT_READ | PROT_WRITE, flags, -1, 0);
        if (ptr == MAP_FAILED) {
            return NULL;
        }
        if (address != NULL && ptr != address) {
            munmap(ptr, aligned_size);
            return NULL;
        }
        return ptr;
    }
#else
    if (address != NULL) {
        return NULL;
    }
    return malloc(aligned_size);
#endif
}

static void *arena_memory_reserve(usize size) { return arena_memory_reserve_at(NULL, size); }

static int arena_memory_commit(void *ptr, usize size) {
    usize aligned_size = arena_page_align(size);

    if (ptr == NULL || aligned_size == 0) {
        return 0;
    }
#if defined(_WIN32)
    return VirtualAlloc(ptr, aligned_size, MEM_COMMIT, PAGE_READWRITE) != NULL;
#else
    (void)ptr;
    return 1;
#endif
}

static void arena_memory_release(void *ptr, usize size) {
    usize aligned_size;

    if (ptr == NULL) {
        return;
    }
    aligned_size = arena_page_align(size);
#if defined(_WIN32)
    (void)aligned_size;
    VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(MAP_PRIVATE) && (defined(MAP_ANONYMOUS) || defined(MAP_ANON))
    if (aligned_size != 0) {
        munmap(ptr, aligned_size);
    }
#else
    (void)aligned_size;
    free(ptr);
#endif
}

static memory_region_error memory_region_create(memory_region *region, usize size) {
    if (!region || region->data || !size)
        return memory_region_invalid;
    usize mapped = arena_page_align(size);
    if (!mapped)
        return memory_region_invalid;
#if defined(_WIN32)
    void *data = VirtualAlloc(NULL, mapped, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#elif defined(__APPLE__)
    void *data = mmap(NULL, mapped, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (data == MAP_FAILED)
        data = NULL;
#else
    return memory_region_unsupported;
#endif
#if defined(_WIN32) || defined(__APPLE__)
    if (!data)
        return memory_region_allocation_failed;
    *region = (memory_region){data, size, mapped};
    return memory_region_ok;
#endif
}

static memory_region_error memory_region_protect(memory_region *region, int read_only) {
    if (!region || !region->data || !region->mapped_size)
        return memory_region_invalid;
#if defined(_WIN32)
    DWORD previous;
    if (!VirtualProtect(region->data,
                        region->mapped_size,
                        read_only ? PAGE_READONLY : PAGE_READWRITE,
                        &previous))
        return memory_region_protection_failed;
#elif defined(__APPLE__)
    if (mprotect(region->data, region->mapped_size, read_only ? PROT_READ : PROT_READ | PROT_WRITE))
        return memory_region_protection_failed;
#else
    return memory_region_unsupported;
#endif
    return memory_region_ok;
}

static memory_region_error memory_region_set_read_only(memory_region *region) {
    return memory_region_protect(region, 1);
}

static memory_region_error memory_region_set_read_write(memory_region *region) {
    return memory_region_protect(region, 0);
}

static memory_region_error memory_region_release(memory_region *region) {
    if (!region)
        return memory_region_invalid;
    if (!region->data)
        return memory_region_ok;
#if defined(_WIN32)
    if (!VirtualFree(region->data, 0, MEM_RELEASE))
        return memory_region_release_failed;
#elif defined(__APPLE__)
    if (munmap(region->data, region->mapped_size))
        return memory_region_release_failed;
#else
    return memory_region_unsupported;
#endif
    *region = (memory_region){0};
    return memory_region_ok;
}

#if defined(_WIN32)
static int file_watch_arm(file_watch_entry *entry) {
    memset(&entry->operation, 0, sizeof(entry->operation));
    entry->operation.hEvent = entry->event;
    return ResetEvent(entry->event) &&
           ReadDirectoryChangesW(entry->directory,
                                 entry->buffer,
                                 sizeof(entry->buffer),
                                 TRUE,
                                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                     FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
                                 NULL,
                                 &entry->operation,
                                 NULL);
}
#elif defined(__APPLE__)
static file_error file_watch_macos_close(file_watch_entry *entry) {
    file_error error = file_error_ok;
    if (entry->file >= 0 && close(entry->file))
        error = file_error_cleanup_failed;
    if (entry->directory >= 0 && close(entry->directory))
        error = file_error_cleanup_failed;
    if (entry->queue >= 0 && close(entry->queue))
        error = file_error_cleanup_failed;
    entry->file = entry->directory = entry->queue = -1;
    return error;
}

/* Takes ownership of descriptor, including on failure. Keep unchanged vnode
   watches attached so pending events are not discarded on unrelated saves. */
static file_error file_watch_macos_attach(file_watch_entry *entry, int descriptor, int directory) {
    int *current = directory ? &entry->directory : &entry->file;
    struct stat previous, next;
    if (fstat(descriptor, &next)) {
        close(descriptor);
        return file_error_read_failed;
    }
    if (*current >= 0 && !fstat(*current, &previous) && previous.st_dev == next.st_dev &&
        previous.st_ino == next.st_ino) {
        return close(descriptor) ? file_error_cleanup_failed : file_error_ok;
    }
    struct kevent change;
    EV_SET(&change,
           descriptor,
           EVFILT_VNODE,
           EV_ADD | EV_CLEAR,
           NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE,
           0,
           (void *)(uptr)(directory ? 2 : 1));
    if (kevent(entry->queue, &change, 1, NULL, 0, NULL) < 0) {
        close(descriptor);
        return file_error_open_failed;
    }
    int old = *current;
    *current = descriptor;
    /* Closing old descriptor removes its vnode filter from kqueue. */
    return old >= 0 && close(old) ? file_error_cleanup_failed : file_error_ok;
}

/* Run only at startup or after a kernel notification, never on quiet polls.
   Directory events carry no filename; inspect the target to filter unrelated
   entries, descend through newly created parents, and recover replacement saves. */
static file_error file_watch_macos_refresh(file_watch_entry *entry, int *changed) {
    strcpy(entry->directory_path, entry->path);
    usize split = strlen(entry->directory_path);
    while (split && entry->directory_path[split - 1] != '/')
        --split;
    if (split)
        entry->directory_path[split == 1 ? 1 : split - 1] = 0;
    else
        strcpy(entry->directory_path, ".");
    int parent = -1;
    for (;;) {
        parent = open(entry->directory_path, O_EVTONLY | O_CLOEXEC);
        if (parent >= 0)
            break;
        if ((errno != ENOENT && errno != ENOTDIR) || split <= 1)
            return file_error_open_failed;
        usize end = split - 1;
        while (end && entry->directory_path[end - 1] != '/')
            --end;
        split = end;
        if (split)
            entry->directory_path[split == 1 ? 1 : split - 1] = 0;
        else
            strcpy(entry->directory_path, ".");
    }
    struct stat directory;
    if (fstat(parent, &directory) || !S_ISDIR(directory.st_mode)) {
        close(parent);
        return file_error_open_failed;
    }
    file_error error = file_watch_macos_attach(entry, parent, 1);
    if (error)
        return error;
    int file = open(entry->path, O_EVTONLY | O_CLOEXEC);
    if (file < 0 && errno != ENOENT && errno != ENOTDIR)
        return file_error_open_failed;
    struct stat next = {0};
    int exists = file >= 0;
    if (exists && fstat(file, &next)) {
        close(file);
        return file_error_read_failed;
    }
    *changed |= exists != entry->exists ||
                (exists &&
                 (next.st_dev != entry->previous.st_dev || next.st_ino != entry->previous.st_ino ||
                  next.st_size != entry->previous.st_size ||
                  next.st_mtimespec.tv_sec != entry->previous.st_mtimespec.tv_sec ||
                  next.st_mtimespec.tv_nsec != entry->previous.st_mtimespec.tv_nsec));
    if (exists) {
        error = file_watch_macos_attach(entry, file, 0);
        if (error)
            return error;
    } else if (entry->file >= 0) {
        int old = entry->file;
        entry->file = -1;
        if (close(old))
            return file_error_cleanup_failed;
    }
    entry->previous = next;
    entry->exists = exists;
    return file_error_ok;
}
#endif

static file_error
file_watcher_add(file_watcher *watcher, arena *allocator, const char *path, usize *id) {
    if (!watcher || !allocator || !path || !*path || !id || watcher->count == SIZE_MAX)
        return file_error_invalid;
    arena_position_marker start = arena_position_mark(allocator);
    arena_result allocation =
        arena_alloc(allocator, sizeof(file_watch_entry), _Alignof(file_watch_entry));
    usize length = strlen(path);
    if (length > INT_MAX - 1 || length > SIZE_MAX / sizeof(u16) - 2) {
        arena_pop_to_marker(start);
        return file_error_invalid;
    }
    arena_result path_memory = arena_alloc(allocator, length + 1, 1);
    if (allocation.error || path_memory.error) {
        arena_pop_to_marker(start);
        return file_error_out_of_memory;
    }
    file_watch_entry *entry = allocation.ptr;
    memset(entry, 0, sizeof(*entry));
    entry->path = path_memory.ptr;
    memcpy(entry->path, path, length + 1);
    file_error error = file_error_ok;
#if defined(_WIN32)
    /* Watch nearest existing parent, so a missing nested file can later appear. */
    arena_result directory_memory = arena_alloc(allocator, length + 2, 1);
    arena_result name_memory =
        arena_alloc(allocator, (length + 1) * sizeof(WCHAR), _Alignof(WCHAR));
    if (directory_memory.error || name_memory.error)
        error = file_error_out_of_memory;
    if (!error) {
        char *directory = directory_memory.ptr;
        memcpy(directory, path, length + 1);
        for (usize index = 0; index < length; ++index)
            if (directory[index] == '\\')
                directory[index] = '/';
        usize split = length;
        while (split && directory[split - 1] != '/')
            --split;
        usize suffix = split;
        if (split)
            directory[split == 1 || (split == 3 && directory[1] == ':') ? split : split - 1] = 0;
        else
            strcpy(directory, ".");
        entry->directory = INVALID_HANDLE_VALUE;
        for (;;) {
            entry->directory = CreateFileA(directory,
                                           FILE_LIST_DIRECTORY,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                           NULL,
                                           OPEN_EXISTING,
                                           FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                           NULL);
            if (entry->directory != INVALID_HANDLE_VALUE)
                break;
            DWORD native_error = GetLastError();
            if ((native_error != ERROR_PATH_NOT_FOUND && native_error != ERROR_FILE_NOT_FOUND) ||
                !split) {
                error = file_error_open_failed;
                break;
            }
            usize end = split - 1;
            while (end && directory[end - 1] != '/')
                --end;
            if (split == 1 || (split == 3 && directory[1] == ':') ||
                (!end && directory[1] == ':')) {
                error = file_error_open_failed;
                break;
            }
            split = end;
            suffix = split;
            if (split)
                directory[split == 1 || (split == 3 && directory[1] == ':') ? split : split - 1] =
                    0;
            else
                strcpy(directory, ".");
        }
        if (!error) {
            entry->name = name_memory.ptr;
            int units = MultiByteToWideChar(CP_UTF8,
                                            MB_ERR_INVALID_CHARS,
                                            path + suffix,
                                            -1,
                                            entry->name,
                                            (int)(length + 1));
            if (!units)
                error = file_error_invalid;
            else {
                entry->name_units = (usize)units - 1;
                for (usize index = 0; index < entry->name_units; ++index)
                    if (entry->name[index] == '/')
                        entry->name[index] = '\\';
                entry->event = CreateEventA(NULL, TRUE, FALSE, NULL);
                if (!entry->event || !file_watch_arm(entry))
                    error = file_error_open_failed;
            }
        }
    }
    if (error) {
        if (entry->directory && entry->directory != INVALID_HANDLE_VALUE)
            CloseHandle(entry->directory);
        if (entry->event)
            CloseHandle(entry->event);
    }
#elif defined(__APPLE__)
    entry->file = entry->directory = entry->queue = -1;
    arena_result directory_memory = arena_alloc(allocator, length + 2, 1);
    if (directory_memory.error)
        error = file_error_out_of_memory;
    else {
        entry->directory_path = directory_memory.ptr;
        entry->queue = kqueue();
        if (entry->queue < 0 || fcntl(entry->queue, F_SETFD, FD_CLOEXEC) < 0)
            error = file_error_open_failed;
        else {
            int ignored = 0;
            error = file_watch_macos_refresh(entry, &ignored);
        }
    }
    if (error)
        file_watch_macos_close(entry);
#else
    error = file_error_unsupported;
#endif
    if (error) {
        arena_pop_to_marker(start);
        return error;
    }
    if (watcher->last)
        watcher->last->next = entry;
    else
        watcher->first = entry;
    watcher->last = entry;
    *id = watcher->count++;
    return file_error_ok;
}

static file_error file_watcher_poll(file_watcher *watcher, u64 *changed, usize word_count) {
    if (!watcher || word_count < watcher->count / 64 + !!(watcher->count % 64) ||
        (watcher->count && !changed))
        return file_error_invalid;
    usize id = 0;
    for (file_watch_entry *entry = watcher->first; entry; entry = entry->next, ++id) {
        int stale = 0;
#if defined(_WIN32)
        DWORD state = WaitForSingleObject(entry->event, 0);
        if (state == WAIT_TIMEOUT)
            continue;
        if (state != WAIT_OBJECT_0)
            return file_error_read_failed;
        DWORD bytes = 0;
        if (!GetOverlappedResult(entry->directory, &entry->operation, &bytes, FALSE)) {
            if (GetLastError() != ERROR_NOTIFY_ENUM_DIR)
                return file_error_read_failed;
            bytes = 0;
        }
        if (!bytes) {
            for (usize index = 0; index < watcher->count; ++index)
                changed[index / 64] |= (u64)1 << (index % 64);
        } else {
            usize offset = 0;
            for (;;) {
                usize header = offsetof(FILE_NOTIFY_INFORMATION, FileName);
                if (offset > bytes || bytes - offset < header)
                    return file_error_read_failed;
                FILE_NOTIFY_INFORMATION *note = (void *)((u8 *)entry->buffer + offset);
                if (note->FileNameLength % sizeof(WCHAR) ||
                    note->FileNameLength > bytes - offset - header)
                    return file_error_read_failed;
                usize units = note->FileNameLength / sizeof(WCHAR);
                if (units == entry->name_units) {
                    stale = 1;
                    for (usize index = 0; index < units; ++index) {
                        WCHAR left = note->FileName[index], right = entry->name[index];
                        if (left >= 'A' && left <= 'Z')
                            left += 'a' - 'A';
                        if (right >= 'A' && right <= 'Z')
                            right += 'a' - 'A';
                        if (left != right) {
                            stale = 0;
                            break;
                        }
                    }
                    if (stale)
                        break;
                }
                if (!note->NextEntryOffset)
                    break;
                if (note->NextEntryOffset < header + note->FileNameLength ||
                    note->NextEntryOffset > bytes - offset)
                    return file_error_read_failed;
                offset += note->NextEntryOffset;
            }
        }
        if (!file_watch_arm(entry))
            return file_error_read_failed;
#elif defined(__APPLE__)
        struct kevent events[2]; /* At most one file and one directory filter per entry. */
        const struct timespec no_wait = {0};
        int count = kevent(entry->queue, NULL, 0, events, 2, &no_wait);
        if (count < 0) {
            if (errno == EINTR)
                continue; /* Keep pending events for next host boundary. */
            return file_error_read_failed;
        }
        if (!count)
            continue; /* No filesystem metadata work on quiet polls. */
        for (int index = 0; index < count; ++index) {
            if (events[index].flags & EV_ERROR)
                return file_error_read_failed;
            if ((uptr)events[index].udata == 1)
                stale = 1; /* Content notifications do not depend on timestamps. */
        }
        file_error error = file_watch_macos_refresh(entry, &stale);
        if (error)
            return error;
#else
        return file_error_unsupported;
#endif
        if (stale)
            changed[id / 64] |= (u64)1 << (id % 64);
    }
    return file_error_ok;
}

static file_error file_watcher_close(file_watcher *watcher) {
    if (!watcher)
        return file_error_invalid;
    file_error error = file_error_ok;
#if defined(_WIN32)
    for (file_watch_entry *entry = watcher->first; entry; entry = entry->next) {
        DWORD ignored;
        /* Drain cancellation before caller reuses OVERLAPPED/buffer storage. */
        CancelIo(entry->directory);
        GetOverlappedResult(entry->directory, &entry->operation, &ignored, TRUE);
        if (!CloseHandle(entry->directory))
            error = file_error_cleanup_failed;
        if (!CloseHandle(entry->event))
            error = file_error_cleanup_failed;
    }
#elif defined(__APPLE__)
    for (file_watch_entry *entry = watcher->first; entry; entry = entry->next) {
        file_error cleanup = file_watch_macos_close(entry);
        if (cleanup)
            error = cleanup;
    }
#endif
    *watcher = (file_watcher){0};
    return error;
}

static arena *arena_from_buffer(void *buffer, usize size) {
    uptr raw;
    uptr aligned;
    usize padding;
    usize header_size;
    arena *result;

    if (buffer == NULL || size == 0) {
        return NULL;
    }

    raw = (uptr)buffer;
    aligned = (raw + _Alignof(arena) - 1u) & ~(uptr)(_Alignof(arena) - 1u);
    padding = (usize)(aligned - raw);
    header_size = arena_header_size();
    if (padding > size || size - padding <= header_size) {
        return NULL;
    }

    result = (arena *)aligned;
    *result = (arena){
        .current = result,
        .flags = arena_flag_no_chain | arena_flag_backing_buffer,
        .reserve_size = size - padding,
        .commit_size = size - padding,
        .pos = header_size,
        .committed = size - padding,
        .reserved = size - padding,
    };
    return result;
}

static arena *arena_create_block(arena_params params, usize base_pos) {
    usize header_size = arena_header_size();
    usize reserve_size =
        params.reserve_size == 0 ? ARENA_DEFAULT_RESERVE_SIZE : params.reserve_size;
    usize commit_size = params.commit_size == 0 ? ARENA_DEFAULT_COMMIT_SIZE : params.commit_size;
    void *memory;
    arena *result;

    reserve_size = arena_page_align(reserve_size);
    commit_size = arena_page_align(commit_size);
    if (reserve_size == 0 || commit_size == 0 || reserve_size < header_size) {
        return NULL;
    }
    if (commit_size < header_size) {
        commit_size = arena_page_align(header_size);
    }
    if (commit_size > reserve_size) {
        commit_size = reserve_size;
    }

    memory = arena_memory_reserve_at(params.base_address, reserve_size);
    if (memory == NULL) {
        return NULL;
    }
    if (!arena_memory_commit(memory, commit_size)) {
        arena_memory_release(memory, reserve_size);
        return NULL;
    }

    result = (arena *)memory;
    *result = (arena){
        .current = result,
        .flags = params.flags,
        .reserve_size = reserve_size,
        .commit_size = commit_size,
        .base_pos = base_pos,
        .pos = header_size,
        .committed = commit_size,
        .reserved = reserve_size,
    };
    return result;
}

static arena *arena_create(arena_params params) { return arena_create_block(params, 0); }

static void arena_release_chain(arena *chain) {
    arena *current;
    arena *prev;

    current = chain;
    while (current != NULL) {
        prev = current->prev;
        arena_memory_release(current, current->reserved);
        current = prev;
    }
}

static void arena_release(arena *state) {
    arena *active_chain;
    arena *free_chain;

    if (state == NULL || (state->flags & arena_flag_backing_buffer)) {
        return;
    }

    active_chain = state->current != NULL ? state->current : state;
    free_chain = state->free_last;
    arena_release_chain(active_chain);
    arena_release_chain(free_chain);
}

static int arena_commit_to(arena *state, usize pos) {
    usize commit_end;
    usize commit_size;

    if (state == NULL || pos > state->reserved) {
        return 0;
    }
    if (pos <= state->committed) {
        return 1;
    }

    commit_end = pos;
    if (state->commit_size != 0) {
        if (commit_end > ((usize)-1) - (state->commit_size - 1u)) {
            return 0;
        }
        commit_end =
            (commit_end + state->commit_size - 1u) / state->commit_size * state->commit_size;
    }
    if (commit_end > state->reserved) {
        commit_end = state->reserved;
    }

    commit_size = commit_end - state->committed;
    if (!arena_memory_commit((u8 *)state + state->committed, commit_size)) {
        return 0;
    }
    state->committed = commit_end;
    return 1;
}

static usize arena_pos(arena *state) {
    arena *current;
    usize header_size;

    if (state == NULL || state->current == NULL) {
        return 0;
    }

    current = state->current;
    header_size = arena_header_size();
    if (current->pos < header_size) {
        return 0;
    }

    return current->base_pos + current->pos - header_size;
}

static arena_error arena_pop_to(arena *state, usize pos) {
    arena *current;
    usize header_size;

    if (state == NULL || state->current == NULL) {
        return arena_error_invalid;
    }
    if (pos > arena_pos(state)) {
        return arena_error_invalid;
    }

    current = state->current;
    header_size = arena_header_size();
    while (current != state && pos <= current->base_pos) {
        arena *prev = current->prev;

        if (state->flags & arena_flag_free_list) {
            current->pos = header_size;
            current->prev = state->free_last;
            state->free_last = current;
        } else if (!(current->flags & arena_flag_backing_buffer)) {
            arena_memory_release(current, current->reserved);
        }
        current = prev;
    }
    if (current == NULL || pos < current->base_pos) {
        return arena_error_invalid;
    }

    state->current = current;
    current->pos = header_size + pos - current->base_pos;
    return arena_error_ok;
}

static arena_position_marker arena_position_mark(arena *state) {
    return (arena_position_marker){state, arena_pos(state)};
}

static arena_error arena_pop_to_marker(arena_position_marker marker) {
    return arena_pop_to(marker.arena, marker.pos);
}

static void arena_fill_pattern(void *ptr, usize size, u32 pattern) {
    u8 bytes[4];
    usize offset = 0;

    if (ptr == NULL || size == 0) {
        return;
    }

    bytes[0] = (u8)(pattern & 0xffu);
    bytes[1] = (u8)((pattern >> 8u) & 0xffu);
    bytes[2] = (u8)((pattern >> 16u) & 0xffu);
    bytes[3] = (u8)((pattern >> 24u) & 0xffu);

    while (size - offset >= 4u) {
        memcpy((u8 *)ptr + offset, bytes, 4u);
        offset += 4u;
    }
    if (offset < size) {
        memcpy((u8 *)ptr + offset, bytes, size - offset);
    }
}

static arena_result
arena_alloc_internal(arena *state, usize size, usize align, int fill, u32 pattern) {
    arena *current;
    usize pos_pre;
    usize pos_pst;
    uptr base;
    uptr ptr;
    uptr aligned;

    if (state == NULL || state->current == NULL || size == 0 || !arena_alignment_valid(align)) {
        return arena_fail(arena_error_invalid);
    }

    current = state->current;
    base = (uptr)current;
    ptr = base + current->pos;
    aligned = (ptr + align - 1u) & ~(uptr)(align - 1u);
    if (aligned < ptr) {
        return arena_fail(arena_error_out_of_memory);
    }

    pos_pre = (usize)(aligned - base);
    pos_pst = pos_pre + size;
    if (pos_pst < pos_pre) {
        return arena_fail(arena_error_out_of_memory);
    }

    if (pos_pst > current->reserved) {
        arena *next = NULL;
        arena *free_prev = NULL;
        usize header_size = arena_header_size();
        usize min_reserve;
        arena_params params;

        if (state->flags & arena_flag_no_chain) {
            return arena_fail(arena_error_out_of_memory);
        }
        min_reserve = header_size + (align - 1u) + size;
        if (min_reserve < header_size || min_reserve < size) {
            return arena_fail(arena_error_out_of_memory);
        }
        if (state->flags & arena_flag_free_list) {
            for (next = state->free_last; next != NULL; free_prev = next, next = next->prev) {
                if (next->reserved >= min_reserve) {
                    if (free_prev != NULL) {
                        free_prev->prev = next->prev;
                    } else {
                        state->free_last = next->prev;
                    }
                    next->prev = NULL;
                    next->pos = header_size;
                    break;
                }
            }
        }
        params = (arena_params){
            .flags = state->flags,
            .reserve_size =
                current->reserve_size > min_reserve ? current->reserve_size : min_reserve,
            .commit_size = current->commit_size > min_reserve ? current->commit_size : min_reserve,
        };
        if (next == NULL) {
            next = arena_create_block(params, current->base_pos + current->reserved - header_size);
        }
        if (next == NULL) {
            return arena_fail(arena_error_out_of_memory);
        }
        next->base_pos = current->base_pos + current->reserved - header_size;
        next->prev = current;
        state->current = next;
        current = next;
        base = (uptr)current;
        ptr = base + current->pos;
        aligned = (ptr + align - 1u) & ~(uptr)(align - 1u);
        if (aligned < ptr) {
            return arena_fail(arena_error_out_of_memory);
        }
        pos_pre = (usize)(aligned - base);
        pos_pst = pos_pre + size;
    }

    if (pos_pst > current->reserved) {
        return arena_fail(arena_error_out_of_memory);
    }
    if (pos_pst > current->committed && !arena_commit_to(current, pos_pst)) {
        return arena_fail(arena_error_out_of_memory);
    }

    current->pos = pos_pst;
    if (fill) {
        arena_fill_pattern((void *)aligned, size, pattern);
    }
    return (arena_result){(void *)aligned, arena_error_ok};
}

static arena_result arena_alloc_pattern(arena *state, usize size, usize align, u32 pattern) {
    return arena_alloc_internal(state, size, align, 1, pattern);
}

static arena_result arena_alloc(arena *state, usize size, usize align) {
    return arena_alloc_pattern(state, size, align, 0);
}

static arena_result arena_alloc_uninit(arena *state, usize size, usize align) {
    return arena_alloc_internal(state, size, align, 0, 0);
}

static void arena_reset(arena *state) { (void)arena_pop_to(state, 0); }

/* Free list */
static usize free_list_header_size(void) {
    return (sizeof(free_list_block) + FREE_LIST_ALIGNMENT - 1u) &
           ~(usize)(FREE_LIST_ALIGNMENT - 1u);
}

static usize free_list_min_block_size(void) {
    return free_list_header_size() +
           ((FREE_LIST_MIN_PAYLOAD + FREE_LIST_ALIGNMENT - 1u) &
            ~(usize)(FREE_LIST_ALIGNMENT - 1u));
}

/* Require an initialized allocator for managed-region queries. */
static u8 *free_list_memory(free_list *state) {
    usize metadata = (sizeof(free_list) + FREE_LIST_ALIGNMENT - 1u) &
                     ~(usize)(FREE_LIST_ALIGNMENT - 1u);
    return (u8 *)state + metadata;
}

static usize free_list_capacity(free_list *state) {
    usize offset = (usize)(free_list_memory(state) - (u8 *)state->backing);
    return (state->backing_size - offset) & ~(usize)(FREE_LIST_ALIGNMENT - 1u);
}

static free_list *free_list_from_buffer(void *buffer, usize size) {
    if (!buffer || !size || (uptr)buffer > UINTPTR_MAX - size)
        return NULL;
    usize padding = (FREE_LIST_ALIGNMENT - (uptr)buffer % FREE_LIST_ALIGNMENT) %
                    FREE_LIST_ALIGNMENT;
    usize metadata = (sizeof(free_list) + FREE_LIST_ALIGNMENT - 1u) &
                     ~(usize)(FREE_LIST_ALIGNMENT - 1u);
    if (padding > size || metadata > size - padding)
        return NULL;
    usize capacity = (size - padding - metadata) & ~(usize)(FREE_LIST_ALIGNMENT - 1u);
    if (capacity < free_list_min_block_size())
        return NULL;
    free_list *state = (void *)((u8 *)buffer + padding);
    *state = (free_list){.backing = buffer,
                        .backing_size = size,
                        .first = (void *)free_list_memory(state)};
    *state->first = (free_list_block){.size = capacity};
    return state;
}

static arena_result free_list_alloc_uninit(free_list *state, usize size) {
    if (!state || !state->backing || !state->backing_size || !size)
        return arena_fail(arena_error_invalid);
    usize payload = size < FREE_LIST_MIN_PAYLOAD ? FREE_LIST_MIN_PAYLOAD : size;
    usize header = free_list_header_size();
    if (payload > SIZE_MAX - (FREE_LIST_ALIGNMENT - 1u))
        return arena_fail(arena_error_out_of_memory);
    payload = (payload + FREE_LIST_ALIGNMENT - 1u) & ~(usize)(FREE_LIST_ALIGNMENT - 1u);
    if (payload > SIZE_MAX - header)
        return arena_fail(arena_error_out_of_memory);
    usize needed = header + payload;
    free_list_block **selected = NULL;
    for (free_list_block **link = &state->first; *link; link = &(*link)->next) {
        if ((*link)->size < needed)
            continue;
#if FREE_LIST_FIT_POLICY == FREE_LIST_FIRST_FIT
        selected = link;
        break;
#else
        if (!selected || (*link)->size < (*selected)->size)
            selected = link;
        if ((*link)->size == needed)
            break;
#endif
    }
    if (!selected)
        return arena_fail(arena_error_out_of_memory);
    free_list_block *block = *selected;
    if (block->size - needed >= free_list_min_block_size()) {
        free_list_block *remaining = (void *)((u8 *)block + needed);
        *remaining = (free_list_block){.size = block->size - needed, .next = block->next};
        block->size = needed;
        *selected = remaining;
    } else {
        *selected = block->next;
    }
    block->next = NULL;
    state->used += block->size;
    return (arena_result){(u8 *)block + header, arena_error_ok};
}

static arena_result free_list_alloc_pattern(free_list *state, usize size, u32 pattern) {
    arena_result result = free_list_alloc_uninit(state, size);
    if (result.error == arena_error_ok)
        arena_fill_pattern(result.ptr, size, pattern);
    return result;
}

static arena_result free_list_alloc(free_list *state, usize size) {
    return free_list_alloc_pattern(state, size, 0);
}

static void free_list_free(free_list *state, void *pointer) {
    if (!pointer)
        return;
    assert(state && state->backing && state->backing_size);
    u8 *memory = free_list_memory(state);
    usize capacity = free_list_capacity(state);
    assert((uptr)pointer % FREE_LIST_ALIGNMENT == 0);
    assert((uptr)pointer >= (uptr)memory + free_list_header_size() &&
           (uptr)pointer < (uptr)memory + capacity);
    free_list_block *block = (void *)((u8 *)pointer - free_list_header_size());
    assert(block->size >= free_list_min_block_size() &&
           block->size % FREE_LIST_ALIGNMENT == 0);
    assert(block->size <= capacity - ((uptr)block - (uptr)memory));
    free_list_block **link = &state->first;
    free_list_block *previous = NULL;
    while (*link && (uptr)*link < (uptr)block) {
        previous = *link;
        link = &previous->next;
    }
    assert(!previous || (u8 *)previous + previous->size <= (u8 *)block);
    assert(!*link || (u8 *)block + block->size <= (u8 *)*link);
    assert(state->used >= block->size);
    state->used -= block->size;
    block->next = *link;
    *link = block;
    if (block->next && (u8 *)block + block->size == (u8 *)block->next) {
        block->size += block->next->size;
        block->next = block->next->next;
    }
    if (previous && (u8 *)previous + previous->size == (u8 *)block) {
        previous->size += block->size;
        previous->next = block->next;
    }
}

static void free_list_reset(free_list *state) {
    assert(state && state->backing && state->backing_size);
    void *backing = state->backing;
    usize size = state->backing_size;
    memset(backing, 0, size);
    free_list *restored = free_list_from_buffer(backing, size);
    assert(restored == state);
    (void)restored;
}

/* Hash */
/*
FNV-1a 64-bit hash.
Best suited to small generic in-memory hashing like hash tables, IDs, and lightweight fingerprints.
Not ideal as durable file-content hash when collision quality matters.
*/
static u64 hash_fnv1a_bytes(const void *data, usize len) {
    const u8 *bytes = (const u8 *)data;
    u64 hash = 14695981039346656037ull;
    usize i;

    if (data == NULL && len != 0) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ull;
    }

    return hash;
}

/* Wide arithmetic: addition wraps at 128 bits, as the native unsigned type does.
   Division requires a nonzero divisor. */
static u128 u128_from_u64(u64 value) {
#if defined(__SIZEOF_INT128__) && !defined(__TINYC__)
    return (u128)value;
#else
    return (u128){value, 0};
#endif
}

static u128 u128_add(u128 left, u128 right) {
#if defined(__SIZEOF_INT128__) && !defined(__TINYC__)
    return left + right;
#else
    u128 result;
    result.low = left.low + right.low;
    result.high = left.high + right.high + (result.low < left.low);
    return result;
#endif
}

static u128 u128_mul_u64(u64 left, u64 right) {
#if defined(__SIZEOF_INT128__) && !defined(__TINYC__)
    return (u128)left * (u128)right;
#else
    u64 low_product = (left & 0xffffffffull) * (right & 0xffffffffull);
    u64 cross_left = (left >> 32) * (right & 0xffffffffull);
    u64 cross_right = (left & 0xffffffffull) * (right >> 32);
    u64 middle = (low_product >> 32) + (cross_left & 0xffffffffull) + (cross_right & 0xffffffffull);
    u128 result;
    result.low = (low_product & 0xffffffffull) | (middle << 32);
    result.high =
        (left >> 32) * (right >> 32) + (cross_left >> 32) + (cross_right >> 32) + (middle >> 32);
    return result;
#endif
}

static u128 u128_div_u64(u128 numerator, u64 divisor) {
#if defined(__SIZEOF_INT128__) && !defined(__TINYC__)
    return numerator / divisor;
#else
    u128 quotient = {0};
    u64 remainder = 0;
    unsigned bit;

    /* Binary long division. The carry is the 65th bit of the remainder. */
    for (bit = 128; bit > 0; bit--) {
        u64 next_bit =
            bit > 64 ? (numerator.high >> (bit - 65)) & 1u : (numerator.low >> (bit - 1)) & 1u;
        u64 carry = remainder >> 63;
        remainder = (remainder << 1) | next_bit;
        if (carry || remainder >= divisor) {
            remainder -= divisor;
            if (bit > 64) {
                quotient.high |= 1ull << (bit - 65);
            } else {
                quotient.low |= 1ull << (bit - 1);
            }
        }
    }
    return quotient;
#endif
}

static u64 u128_low(u128 value) {
#if defined(__SIZEOF_INT128__) && !defined(__TINYC__)
    return (u64)value;
#else
    return value.low;
#endif
}

static u64 u128_isqrt(u128 value) {
    u64 low = 0;
    u64 high = (u64)-1;

    while (low < high) {
        u64 span = high - low;
        u64 mid = low + span / 2 + span % 2;
        u128 square = u128_mul_u64(mid, mid);
#if defined(__SIZEOF_INT128__) && !defined(__TINYC__)
        int fits = square <= value;
#else
        int fits =
            square.high < value.high || (square.high == value.high && square.low <= value.low);
#endif
        if (fits) {
            low = mid;
        } else {
            high = mid - 1;
        }
    }
    return low;
}

/* Stats */

static u64 u64_mean(const u64 *values, u64 count) {
    u128 sum = u128_from_u64(0);
    u64 i;
    if (values == NULL || count == 0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        sum = u128_add(sum, u128_from_u64(values[i]));
    }
    return u128_low(u128_div_u64(sum, count));
}

static u64 u64_stddev(const u64 *values, u64 count, u64 mean) {
    u128 sum = u128_from_u64(0);
    u64 i;
    if (values == NULL || count == 0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        u64 delta = values[i] >= mean ? values[i] - mean : mean - values[i];
        sum = u128_add(sum, u128_mul_u64(delta, delta));
    }
    return u128_isqrt(u128_div_u64(sum, count));
}

static u64 u64_mean_kept(const u64 *values, const int *kept, u64 count, u64 *out_kept) {
    u128 sum = u128_from_u64(0);
    u64 kept_count = 0;
    u64 i;
    if (values == NULL || kept == NULL || count == 0) {
        if (out_kept != NULL) {
            *out_kept = 0;
        }
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (kept[i]) {
            sum = u128_add(sum, u128_from_u64(values[i]));
            kept_count++;
        }
    }
    if (out_kept != NULL) {
        *out_kept = kept_count;
    }
    if (kept_count == 0) {
        return 0;
    }
    return u128_low(u128_div_u64(sum, kept_count));
}

static u64 u64_stddev_kept(const u64 *values, const int *kept, u64 count, u64 mean) {
    u128 sum = u128_from_u64(0);
    u64 kept_count = 0;
    u64 i;
    if (values == NULL || kept == NULL || count == 0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (kept[i]) {
            u64 delta = values[i] >= mean ? values[i] - mean : mean - values[i];
            sum = u128_add(sum, u128_mul_u64(delta, delta));
            kept_count++;
        }
    }
    if (kept_count == 0) {
        return 0;
    }
    return u128_isqrt(u128_div_u64(sum, kept_count));
}

static u64 u64_mode_kept(const u64 *values, const int *kept, u64 count) {
    u64 best_value = 0;
    u64 best_hits = 0;
    u64 i;
    if (values == NULL || kept == NULL || count == 0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        u64 hits = 0;
        u64 j;
        if (!kept[i]) {
            continue;
        }
        for (j = 0; j < count; j++) {
            if (kept[j] && values[j] == values[i]) {
                hits++;
            }
        }
        if (hits > best_hits || (hits == best_hits && values[i] < best_value)) {
            best_hits = hits;
            best_value = values[i];
        }
    }
    return best_value;
}

static u64 u64_mark_outliers_stddev(const u64 *values, int *kept, u64 count, double sigma) {
    u64 mean = u64_mean(values, count);
    u64 stddev = u64_stddev(values, count, mean);
    u64 removed = 0;
    u64 i;
    if (values == NULL || kept == NULL || count < 5 || stddev == 0 || sigma <= 0.0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        u64 delta = values[i] >= mean ? values[i] - mean : mean - values[i];
        if ((double)delta > sigma * (double)stddev) {
            if (kept[i]) {
                removed++;
            }
            kept[i] = 0;
        }
    }
    return removed;
}

/* String */
static string string_from_raw(u8 *data, usize len) { return (string){data, len}; }

static cstring cstring_from_raw(char *data) { return (cstring){data}; }

static string string_from_cstring(cstring value) {
    if (value.data == NULL) {
        return (string){0};
    }

    return string_from_raw((u8 *)value.data, strlen(value.data));
}

static cstring cstring_from_string(arena *arena, string value) {
    arena_result result = arena_alloc(arena, value.len + 1u, 1);
    cstring out = {0};

    if (result.error != arena_error_ok) {
        return out;
    }

    memcpy(result.ptr, value.data, value.len);
    ((char *)result.ptr)[value.len] = '\0';
    out.data = (char *)result.ptr;
    return out;
}

static string_view string_view_from_raw(u8 *data, usize len) { return (string_view){data, len}; }

static string_view string_view_from_cstr(const char *data) {
    return data != NULL ? string_view_from_raw((u8 *)data, strlen(data)) : (string_view){0};
}

static string_view string_view_from_cstring(cstring value) {
    return string_view_from_string_full(string_from_cstring(value));
}

static string_view string_view_from_string_full(string value) {
    return string_view_from_raw(value.data, value.len);
}

static string_view string_view_from_view(string_view value, usize start, usize len) {
    if (start > value.len || len > value.len - start) {
        return (string_view){0};
    }

    return (string_view){value.data + start, len};
}

static string_view string_view_from_string(string value, usize start, usize len) {
    return string_view_from_view(string_view_from_string_full(value), start, len);
}

static int string_view_eq(string_view left, string_view right) {
    if (left.len != right.len) {
        return 0;
    }

    return left.len == 0 || memcmp(left.data, right.data, left.len) == 0;
}

static arena_error string_builder_append(string_builder *builder, string_view value) {
    if (builder == NULL || builder->arena == NULL) {
        return arena_error_invalid;
    }

    builder->chunks.raw.arena = builder->arena;
    return dynamic_array_push(&builder->chunks, &value);
}

static arena_error tsv_write_field(string_builder *builder, string_view value) {
    usize segment_start = 0;
    usize i;

    if (builder == NULL || builder->arena == NULL) {
        return arena_error_invalid;
    }

    for (i = 0; i < value.len; i++) {
        string_view escape = {0};

        if (value.data[i] == '\t') {
            escape = string_view_from_raw((u8 *)"\\t", 2);
        } else if (value.data[i] == '\n') {
            escape = string_view_from_raw((u8 *)"\\n", 2);
        } else if (value.data[i] == '\r') {
            escape = string_view_from_raw((u8 *)"\\r", 2);
        } else if (value.data[i] == '\\') {
            escape = string_view_from_raw((u8 *)"\\\\", 2);
        }

        if (escape.data != NULL) {
            if (i > segment_start) {
                arena_error error = string_builder_append(
                    builder,
                    string_view_from_raw((u8 *)value.data + segment_start, i - segment_start));
                if (error != arena_error_ok) {
                    return error;
                }
            }
            {
                arena_error error = string_builder_append(builder, escape);
                if (error != arena_error_ok) {
                    return error;
                }
            }
            segment_start = i + 1u;
        }
    }

    if (segment_start < value.len) {
        return string_builder_append(
            builder,
            string_view_from_raw((u8 *)value.data + segment_start, value.len - segment_start));
    }

    return arena_error_ok;
}

static arena_error tsv_write_cstr_field(string_builder *builder, const char *value) {
    if (value == NULL) {
        return tsv_write_field(builder, (string_view){0});
    }

    return tsv_write_field(builder, string_view_from_raw((u8 *)value, strlen(value)));
}

static arena_error tsv_write_u64_field(string_builder *builder, u64 value) {
    char stack_buffer[32];
    int len;
    arena_result allocation;

    if (builder == NULL || builder->arena == NULL) {
        return arena_error_invalid;
    }

    len = snprintf(stack_buffer, sizeof(stack_buffer), "%llu", (unsigned long long)value);
    if (len < 0 || (usize)len >= sizeof(stack_buffer)) {
        return arena_error_out_of_memory;
    }

    allocation = arena_alloc(builder->arena, (usize)len, _Alignof(u8));
    if (allocation.error != arena_error_ok) {
        return allocation.error;
    }
    memcpy(allocation.ptr, stack_buffer, (usize)len);
    return tsv_write_field(builder, string_view_from_raw(allocation.ptr, (usize)len));
}

static arena_error tsv_write_separator(string_builder *builder) {
    return string_builder_append(builder, string_view_from_raw((u8 *)"\t", 1));
}

static arena_error tsv_write_record_end(string_builder *builder) {
    return string_builder_append(builder, string_view_from_raw((u8 *)"\n", 1));
}

static tsv_row tsv_read_row(arena *arena, string_view line) {
    tsv_row result = {0};
    usize field_start = 0;
    usize i;

    if (arena == NULL) {
        result.error = arena_error_invalid;
        return result;
    }
    result.fields.raw.arena = arena;

    if (line.len > 0 && line.data[line.len - 1u] == '\n') {
        line.len--;
    }
    if (line.len > 0 && line.data[line.len - 1u] == '\r') {
        line.len--;
    }

    for (i = 0; i <= line.len; i++) {
        if (i == line.len || line.data[i] == '\t') {
            string_view raw = string_view_from_raw((u8 *)line.data + field_start, i - field_start);
            int needs_unescape = 0;
            usize j;

            for (j = 0; j < raw.len; j++) {
                if (raw.data[j] == '\\') {
                    needs_unescape = 1;
                    break;
                }
            }

            if (needs_unescape) {
                arena_result allocation = arena_alloc(arena, raw.len, _Alignof(u8));
                usize out_len = 0;

                if (allocation.error != arena_error_ok) {
                    result.error = allocation.error;
                    return result;
                }
                for (j = 0; j < raw.len; j++) {
                    u8 byte = raw.data[j];

                    if (byte == '\\' && j + 1u < raw.len) {
                        j++;
                        if (raw.data[j] == 't') {
                            byte = '\t';
                        } else if (raw.data[j] == 'n') {
                            byte = '\n';
                        } else if (raw.data[j] == 'r') {
                            byte = '\r';
                        } else if (raw.data[j] == '\\') {
                            byte = '\\';
                        } else {
                            ((u8 *)allocation.ptr)[out_len++] = '\\';
                            byte = raw.data[j];
                        }
                    }
                    ((u8 *)allocation.ptr)[out_len++] = byte;
                }
                raw = string_view_from_raw(allocation.ptr, out_len);
            }

            result.error = dynamic_array_push(&result.fields, &raw);
            if (result.error != arena_error_ok) {
                return result;
            }
            field_start = i + 1u;
        }
    }

    result.error = arena_error_ok;
    return result;
}

static int string_format_parse_uint(string_view text, usize *index, usize *out) {
    usize value = 0;
    int any = 0;

    if (index == NULL || out == NULL) {
        return 0;
    }
    while (*index < text.len && text.data[*index] >= '0' && text.data[*index] <= '9') {
        usize digit = (usize)(text.data[*index] - '0');

        if (value > ((usize)-1 - digit) / 10u) {
            return 0;
        }
        value = value * 10u + digit;
        (*index)++;
        any = 1;
    }
    *out = value;
    return any;
}

static arena_error string_builder_append_repeat(string_builder *builder, u8 byte, usize count) {
    arena_result allocation;

    if (builder == NULL || builder->arena == NULL) {
        return arena_error_invalid;
    }
    if (count == 0) {
        return arena_error_ok;
    }
    allocation = arena_alloc(builder->arena, count, _Alignof(u8));
    if (allocation.error != arena_error_ok) {
        return allocation.error;
    }
    memset(allocation.ptr, byte, count);
    return string_builder_append(builder, string_view_from_raw(allocation.ptr, count));
}

static arena_error string_builder_append_grouped_digits(string_builder *builder,
                                                        string_view digits) {
    usize first_group;
    usize i;

    if (digits.len <= 3) {
        return string_builder_append(builder, digits);
    }
    first_group = digits.len % 3u;
    if (first_group == 0) {
        first_group = 3;
    }
    if (string_builder_append(builder, string_view_from_view(digits, 0, first_group)) !=
        arena_error_ok) {
        return arena_error_out_of_memory;
    }
    for (i = first_group; i < digits.len; i += 3) {
        if (string_builder_append(builder, string_view_from_raw((u8 *)",", 1)) != arena_error_ok ||
            string_builder_append(builder, string_view_from_view(digits, i, 3)) != arena_error_ok) {
            return arena_error_out_of_memory;
        }
    }
    return arena_error_ok;
}

/*
Format helpers use invariant separators forever: '.' decimal, ',' grouping, no locale/culture.

Numeric specs for string_format_f64:
- F<n>    fixed decimals, no grouping: F2 -> 12.30
- N<n>    fixed decimals with comma grouping: N2 -> 1,234.50
- P<n>    percent, value * 100 plus '%': P1 -> 50.0%
- G<n>    general snprintf %g style precision: G6 -> 1234.5
- E<n>    scientific snprintf %E style precision: E2 -> 1.23E+03
- R       f64 bit dump: sign:<s> exponent:<e> mantissa:<m>
- Z<n>    optional suffix for F/N/P: minimum integer digits before decimal/sign/suffix.
          Examples: F2Z5 -> 00012.30, N2Z8 -> 00,001,234.50, P1Z3 -> 050.0%
Deferred intentionally: C/currency never; D/B/X until integer semantics; custom separators; parsing.

String specs for string_format_string:
- sl<width><fill> left select/pad:  sl2- trims to left 2 bytes, sl6- pads right
- sr<width><fill> right select/pad: sr2- trims to right 2 bytes, sr6- pads left
- sc<width><fill> center select/pad: sc6- pads both sides
Width is byte count; Unicode grapheme handling deferred.
*/
static string_result string_format_f64(arena *arena, f64 number, string_view spec) {
    string_result out = {0};
    char raw[256];
    char format[16];
    char *digits;
    char *dot;
    int written;
    int negative = 0;
    int grouped = 0;
    int percent = 0;
    usize precision = 0;
    usize zero_width = 0;
    usize index = 1;
    string_builder builder = {0};

    if (arena == NULL || spec.len == 0) {
        out.error = arena_error_invalid;
        return out;
    }
    builder.arena = arena;

    if (spec.data[0] == 'R' || spec.data[0] == 'r') {
        u64 bits = 0;
        u64 sign;
        u64 exponent;
        u64 mantissa;

        if (spec.len != 1) {
            out.error = arena_error_invalid;
            return out;
        }
        memcpy(&bits, &number, sizeof(bits));
        sign = bits >> 63u;
        exponent = (bits >> 52u) & 0x7ffu;
        mantissa = bits & 0x000fffffffffffffull;
        written = snprintf(raw,
                           sizeof(raw),
                           "sign:%llu exponent:%llu mantissa:%013llx",
                           (unsigned long long)sign,
                           (unsigned long long)exponent,
                           (unsigned long long)mantissa);
        if (written < 0 || (usize)written >= sizeof(raw)) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        if (string_builder_append(&builder, string_view_from_raw((u8 *)raw, (usize)written)) !=
            arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        return string_from_string_builder(&builder);
    }

    if (spec.data[0] == 'G' || spec.data[0] == 'g' || spec.data[0] == 'E' || spec.data[0] == 'e') {
        if (index < spec.len && !string_format_parse_uint(spec, &index, &precision)) {
            out.error = arena_error_invalid;
            return out;
        }
        if (index != spec.len) {
            out.error = arena_error_invalid;
            return out;
        }
        if (precision == 0) {
            precision = spec.data[0] == 'G' || spec.data[0] == 'g' ? 17 : 6;
        }
        if (precision > 30) {
            out.error = arena_error_invalid;
            return out;
        }
        snprintf(format,
                 sizeof(format),
                 spec.data[0] == 'E' || spec.data[0] == 'e' ? "%%.%zuE" : "%%.%zug",
                 precision);
        written = snprintf(raw, sizeof(raw), format, number);
        if (written < 0 || (usize)written >= sizeof(raw)) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        if (string_builder_append(&builder, string_view_from_raw((u8 *)raw, (usize)written)) !=
            arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        return string_from_string_builder(&builder);
    }

    grouped = spec.data[0] == 'N' || spec.data[0] == 'n';
    percent = spec.data[0] == 'P' || spec.data[0] == 'p';
    if (!(spec.data[0] == 'F' || spec.data[0] == 'f' || grouped || percent)) {
        out.error = arena_error_invalid;
        return out;
    }
    if (!string_format_parse_uint(spec, &index, &precision)) {
        precision = 2;
    }
    if (index < spec.len) {
        if (spec.data[index++] != 'Z' || !string_format_parse_uint(spec, &index, &zero_width)) {
            out.error = arena_error_invalid;
            return out;
        }
    }
    if (index != spec.len || precision > 30) {
        out.error = arena_error_invalid;
        return out;
    }

    if (number < 0) {
        negative = 1;
        number = -number;
    }
    if (percent) {
        number *= 100.0;
    }
    snprintf(format, sizeof(format), "%%.%zuf", precision);
    written = snprintf(raw, sizeof(raw), format, number);
    if (written < 0 || (usize)written >= sizeof(raw)) {
        out.error = arena_error_out_of_memory;
        return out;
    }
    digits = raw;
    dot = strchr(raw, '.');
    if (negative &&
        string_builder_append(&builder, string_view_from_raw((u8 *)"-", 1)) != arena_error_ok) {
        out.error = arena_error_out_of_memory;
        return out;
    }
    if (dot != NULL) {
        *dot = 0;
    }
    if (grouped) {
        string_builder digit_builder = {.arena = arena};
        string_result padded_digits;

        if (zero_width > strlen(digits) &&
            string_builder_append_repeat(&digit_builder, '0', zero_width - strlen(digits)) !=
                arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        if (string_builder_append(
                &digit_builder,
                string_view_from_string_full(string_from_raw((u8 *)digits, strlen(digits)))) !=
            arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        padded_digits = string_from_string_builder(&digit_builder);
        if (padded_digits.error != arena_error_ok ||
            string_builder_append_grouped_digits(
                &builder,
                string_view_from_string_full(padded_digits.value)) != arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
    } else {
        if (zero_width > strlen(digits) &&
            string_builder_append_repeat(&builder, '0', zero_width - strlen(digits)) !=
                arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        if (string_builder_append(
                &builder,
                string_view_from_string_full(string_from_raw((u8 *)digits, strlen(digits)))) !=
            arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
    }
    if (dot != NULL &&
        string_builder_append(&builder, string_view_from_raw((u8 *)".", 1)) != arena_error_ok) {
        out.error = arena_error_out_of_memory;
        return out;
    }
    if (dot != NULL &&
        string_builder_append(
            &builder,
            string_view_from_string_full(string_from_raw((u8 *)dot + 1, strlen(dot + 1)))) !=
            arena_error_ok) {
        out.error = arena_error_out_of_memory;
        return out;
    }
    if (percent &&
        string_builder_append(&builder, string_view_from_raw((u8 *)"%", 1)) != arena_error_ok) {
        out.error = arena_error_out_of_memory;
        return out;
    }
    return string_from_string_builder(&builder);
}

static string_result string_format_string(arena *arena, string_view value, string_view spec) {
    string_result out = {0};
    usize index = 2;
    usize width = 0;
    usize left_pad = 0;
    usize right_pad = 0;
    u8 fill;
    string_view bytes = value;
    string_builder builder = {0};

    if (arena == NULL || spec.len < 4 || spec.data[0] != 's' ||
        (spec.data[1] != 'l' && spec.data[1] != 'r' && spec.data[1] != 'c') ||
        !string_format_parse_uint(spec, &index, &width) || index + 1u != spec.len) {
        out.error = arena_error_invalid;
        return out;
    }
    fill = spec.data[index];
    if (width <= bytes.len) {
        if (spec.data[1] == 'r') {
            bytes = string_view_from_view(bytes, bytes.len - width, width);
        } else if (spec.data[1] == 'c') {
            bytes = string_view_from_view(bytes, (bytes.len - width) / 2u, width);
        } else {
            bytes = string_view_from_view(bytes, 0, width);
        }
        builder.arena = arena;
        if (string_builder_append(&builder, bytes) != arena_error_ok) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        return string_from_string_builder(&builder);
    }

    if (spec.data[1] == 'l') {
        right_pad = width - bytes.len;
    } else if (spec.data[1] == 'r') {
        left_pad = width - bytes.len;
    } else {
        left_pad = (width - bytes.len) / 2u;
        right_pad = width - bytes.len - left_pad;
    }
    builder.arena = arena;
    if (string_builder_append_repeat(&builder, fill, left_pad) != arena_error_ok ||
        string_builder_append(&builder, bytes) != arena_error_ok ||
        string_builder_append_repeat(&builder, fill, right_pad) != arena_error_ok) {
        out.error = arena_error_out_of_memory;
        return out;
    }
    return string_from_string_builder(&builder);
}

static string_result string_from_string_builder(string_builder *builder) {
    string_result out = {0};
    arena_result allocation;
    usize total = 0;
    usize offset = 0;
    usize i;

    if (builder == NULL) {
        out.error = arena_error_invalid;
        return out;
    }

    for (i = 0; i < builder->chunks.raw.len; i++) {
        usize next = total + dynamic_array_data(&builder->chunks)[i].len;

        if (next < total) {
            out.error = arena_error_out_of_memory;
            return out;
        }
        total = next;
    }

    if (total == 0) {
        out.error = arena_error_ok;
        return out;
    }

    allocation = arena_alloc(builder->arena, total, 1);
    if (allocation.error != arena_error_ok) {
        out.error = allocation.error;
        return out;
    }

    for (i = 0; i < builder->chunks.raw.len; i++) {
        memcpy((u8 *)allocation.ptr + offset,
               dynamic_array_data(&builder->chunks)[i].data,
               dynamic_array_data(&builder->chunks)[i].len);
        offset += dynamic_array_data(&builder->chunks)[i].len;
    }

    out.value = string_from_raw(allocation.ptr, total);
    out.error = arena_error_ok;
    return out;
}

static cstring cstring_from_string_builder(string_builder *builder) {
    cstring out = {0};
    arena_result allocation;
    usize total = 0;
    usize offset = 0;
    usize i;

    if (builder == NULL) {
        return out;
    }

    for (i = 0; i < builder->chunks.raw.len; i++) {
        usize next = total + dynamic_array_data(&builder->chunks)[i].len;

        if (next < total) {
            return out;
        }
        total = next;
    }

    allocation = arena_alloc(builder->arena, total + 1u, 1);
    if (allocation.error != arena_error_ok) {
        return out;
    }

    for (i = 0; i < builder->chunks.raw.len; i++) {
        memcpy((char *)allocation.ptr + offset,
               dynamic_array_data(&builder->chunks)[i].data,
               dynamic_array_data(&builder->chunks)[i].len);
        offset += dynamic_array_data(&builder->chunks)[i].len;
    }

    ((char *)allocation.ptr)[total] = '\0';
    out.data = allocation.ptr;
    return out;
}

/* Clock / argv */
#if defined(_WIN32)
/* Positive frequency; converted nanoseconds must fit u64. Keep the product
   wide: multiplying the raw QPC counter in u64 wraps at normal system uptimes. */
static u64 clock_ticks_to_ns(u64 ticks, u64 frequency) {
    return u128_low(u128_div_u64(u128_mul_u64(ticks, 1000000000ull), frequency));
}
#endif

static u64 clock_now_ns(void) {
#if defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;

    if (!QueryPerformanceCounter(&counter) || !QueryPerformanceFrequency(&frequency) ||
        frequency.QuadPart <= 0) {
        return 0;
    }

    return clock_ticks_to_ns((u64)counter.QuadPart, (u64)frequency.QuadPart);
#elif defined(CLOCK_MONOTONIC)
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }

    return (u64)ts.tv_sec * 1000000000ull + (u64)ts.tv_nsec;
#else
    return 0;
#endif
}

static u64 clock_now_us(void) { return clock_now_ns() / 1000u; }

static duration_unit duration_unit_for_ps(u64 picoseconds) {
    if (picoseconds >= 1000000000000ull) {
        return (duration_unit){1000000000000ull, "s"};
    }
    if (picoseconds >= 1000000000ull) {
        return (duration_unit){1000000000ull, "ms"};
    }
    if (picoseconds >= 1000000ull) {
        return (duration_unit){1000000ull, "us"};
    }
    if (picoseconds >= 1000ull) {
        return (duration_unit){1000ull, "ns"};
    }
    return (duration_unit){1ull, "ps"};
}

static int
duration_format_in_unit(char *buffer, usize buffer_size, u64 picoseconds, duration_unit unit) {
    u64 whole;
    u64 frac;
    if (buffer == NULL || buffer_size == 0 || unit.divisor == 0 || unit.suffix == NULL) {
        return 0;
    }
    whole = picoseconds / unit.divisor;
    frac = (picoseconds % unit.divisor) * 1000ull / unit.divisor;
    if (frac == 0) {
        return snprintf(buffer, buffer_size, "%llu%s", (unsigned long long)whole, unit.suffix) > 0;
    }
    return snprintf(buffer,
                    buffer_size,
                    "%llu.%03llu%s",
                    (unsigned long long)whole,
                    (unsigned long long)frac,
                    unit.suffix) > 0;
}

static int duration_format_ps(char *buffer, usize buffer_size, u64 picoseconds) {
    return duration_format_in_unit(buffer,
                                   buffer_size,
                                   picoseconds,
                                   duration_unit_for_ps(picoseconds));
}

static int argv_has_flag(int argc, char **argv, const char *flag) {
    int i;

    if (argc < 0 || argv == NULL || flag == NULL) {
        return 0;
    }

    for (i = 1; i < argc; i++) {
        if (argv[i] != NULL && strcmp(argv[i], flag) == 0) {
            return 1;
        }
    }

    return 0;
}

static char *argv_last(int argc, char **argv) {
    if (argc <= 1 || argv == NULL) {
        return NULL;
    }

    return argv[argc - 1];
}

/* File */
static file_error file_error_from_arena_error(arena_error error) {
    if (error == arena_error_out_of_memory) {
        return file_error_out_of_memory;
    }

    return file_error_invalid;
}

static file_result file_read_all(arena *arena, cstring path) {
    file_result out = {0};
    FILE *file;
    long size_long;
    usize size;
    arena_result allocation;

    if (path.data == NULL) {
        out.error = file_error_invalid;
        return out;
    }

    file = fopen(path.data, "rb");
    if (file == NULL) {
        out.error = file_error_open_failed;
        return out;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        out.error = file_error_read_failed;
        return out;
    }

    size_long = ftell(file);
    if (size_long < 0) {
        fclose(file);
        out.error = file_error_read_failed;
        return out;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        out.error = file_error_read_failed;
        return out;
    }

    size = (usize)size_long;
    if ((long)size != size_long) {
        fclose(file);
        out.error = file_error_read_failed;
        return out;
    }

    if (size == 0) {
        fclose(file);
        out.error = file_error_ok;
        return out;
    }

    allocation = arena_alloc(arena, size, 1);
    if (allocation.error != arena_error_ok) {
        fclose(file);
        out.error = file_error_from_arena_error(allocation.error);
        return out;
    }

    if (fread(allocation.ptr, 1, size, file) != size) {
        fclose(file);
        out.error = file_error_read_failed;
        return out;
    }

    fclose(file);
    out.value = string_from_raw((u8 *)allocation.ptr, size);
    out.error = file_error_ok;
    return out;
}

static file_error file_write_all(cstring path, string value) {
    return file_write_bytes(path, value.data, value.len);
}

static file_error file_read_into(cstring path, void *output, usize capacity, usize *size) {
    if (!path.data || !size || (capacity && !output)) {
        return file_error_invalid;
    }

    FILE *file = fopen(path.data, "rb");
    if (!file) {
        return errno == ENOENT ? file_error_not_found : file_error_open_failed;
    }

    file_error error = file_error_ok;
    long length = -1;
    usize read = 0;
    if (fseek(file, 0, SEEK_END) || (length = ftell(file)) < 0) {
        error = file_error_read_failed;
    } else if ((u64)length > capacity) {
        error = file_error_capacity;
    } else if (fseek(file, 0, SEEK_SET)) {
        error = file_error_read_failed;
    } else {
        read = length ? fread(output, 1, (usize)length, file) : 0;
        if (read != (usize)length || ferror(file)) {
            error = file_error_read_failed;
        }
    }
    if (fclose(file) && error == file_error_ok) {
        error = file_error_read_failed;
    }
    if (error == file_error_ok) {
        *size = read;
    }
    return error;
}

static file_error file_write_bytes(cstring path, const void *bytes, usize size) {
    if (!path.data || (size && !bytes)) {
        return file_error_invalid;
    }

    FILE *file = fopen(path.data, "wb");
    if (!file) {
        return file_error_open_failed;
    }
    usize written = size ? fwrite(bytes, 1, size, file) : 0;
    int failed = written != size || ferror(file);
    if (fclose(file)) {
        failed = 1;
    }
    return failed ? file_error_write_failed : file_error_ok;
}

static const char *file_error_message(file_error error) {
    switch (error) {
    case file_error_ok:
        return NULL;
    case file_error_invalid:
        return "invalid file operation";
    case file_error_open_failed:
        return "file open failed";
    case file_error_read_failed:
        return "file read failed or was short";
    case file_error_write_failed:
        return "file write failed or was short";
    case file_error_out_of_memory:
        return "file storage exhausted";
    case file_error_not_found:
        return "file not found";
    case file_error_capacity:
        return "file read capacity exhausted";
    case file_error_size_mismatch:
        return "mapped file size mismatch";
    case file_error_mapping_failed:
        return "file mapping failed";
    case file_error_flush_failed:
        return "mapped file flush failed";
    case file_error_cleanup_failed:
        return "mapped file cleanup failed";
    case file_error_unsupported:
        return "file mapping is unsupported on this platform";
    }
    return "unknown file error";
}

/* Local message pipes. Caller owns zeroed channel and all I/O storage. One
   outstanding operation per channel; keep buffers alive until terminal poll.
   Windows backend only. Other platforms return pipe_error_unsupported.
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
#elif defined(__APPLE__)
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
           WideCharToMultiByte(CP_UTF8, 0, path, -1, output, (int)capacity, NULL, NULL);
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

#if defined(__APPLE__)
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
    int enabled = 1;
    if (fcntl(descriptor, F_SETFL, O_NONBLOCK) || fcntl(descriptor, F_SETFD, FD_CLOEXEC) ||
        setsockopt(descriptor, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled))) {
        close(descriptor);
        return -1;
    }
    return descriptor;
}

static int pipe_macos_owner(int descriptor) {
    uid_t user;
    gid_t group;
    return !getpeereid(descriptor, &user, &group) && user == getuid();
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
#elif defined(__APPLE__)
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
#elif defined(__APPLE__)
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
#elif defined(__APPLE__)
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
#elif defined(__APPLE__)
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
        int enabled = 1;
        if (!pipe_macos_owner(descriptor) || fcntl(descriptor, F_SETFL, O_NONBLOCK) ||
            fcntl(descriptor, F_SETFD, FD_CLOEXEC) ||
            setsockopt(descriptor, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled))) {
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
                                       4 - channel->header_used, 0) :
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
    ssize_t count = writing ? send(descriptor, (u8 *)channel->bytes + channel->used, remaining, 0) :
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
#elif defined(__APPLE__)
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
#elif defined(__APPLE__)
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
#elif defined(__APPLE__)
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
#elif defined(__APPLE__)
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

static file_error file_map(cstring path, usize size, file_mapping *mapping) {
    if (!path.data || !size || (u64)size > INT64_MAX || !mapping || mapping->data ||
        mapping->file_handle || mapping->mapping_handle || mapping->size) {
        return file_error_invalid;
    }
#if defined(_WIN32)
    HANDLE file = CreateFileA(path.data,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return file_error_open_failed;
    }
    DWORD created = GetLastError();
    LARGE_INTEGER length;
    if (!GetFileSizeEx(file, &length)) {
        CloseHandle(file);
        return file_error_read_failed;
    }
    if (created == ERROR_ALREADY_EXISTS && (u64)length.QuadPart != size) {
        CloseHandle(file);
        return file_error_size_mismatch;
    }
    HANDLE handle =
        CreateFileMappingA(file, NULL, PAGE_READWRITE, (DWORD)((u64)size >> 32), (DWORD)size, NULL);
    if (!handle) {
        CloseHandle(file);
        return file_error_mapping_failed;
    }
    void *data = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!data) {
        CloseHandle(handle);
        CloseHandle(file);
        return file_error_mapping_failed;
    }
    *mapping =
        (file_mapping){.data = data, .size = size, .file_handle = file, .mapping_handle = handle};
    return file_error_ok;
#elif defined(__APPLE__)
    int descriptor = open(path.data, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    int created = descriptor >= 0;
    if (!created && errno == EEXIST)
        descriptor = open(path.data, O_RDWR | O_NOFOLLOW);
    if (descriptor < 0)
        return file_error_open_failed;
    struct stat status;
    if (fstat(descriptor, &status) || !S_ISREG(status.st_mode)) {
        close(descriptor);
        return file_error_read_failed;
    }
    if (!created && (u64)status.st_size != size) {
        close(descriptor);
        return file_error_size_mismatch;
    }
    if (created && ftruncate(descriptor, (off_t)size)) {
        close(descriptor);
        return file_error_mapping_failed;
    }
    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
    if (data == MAP_FAILED) {
        close(descriptor);
        return file_error_mapping_failed;
    }
    *mapping = (file_mapping){.data = data, .size = size,
                              .file_handle = (void *)(uptr)(descriptor + 1), .mapping_handle = (void *)1};
    return file_error_ok;
#else
    return file_error_unsupported;
#endif
}

static file_error file_mapping_flush(const file_mapping *mapping) {
    if (!mapping || !mapping->data || !mapping->size || !mapping->file_handle ||
        !mapping->mapping_handle) {
        return file_error_invalid;
    }
#if defined(_WIN32)
    int failed = !FlushViewOfFile(mapping->data, mapping->size);
    if (!FlushFileBuffers(mapping->file_handle)) {
        failed = 1;
    }
    return failed ? file_error_flush_failed : file_error_ok;
#elif defined(__APPLE__)
    int failed = msync(mapping->data, mapping->size, MS_SYNC) != 0;
    if (fsync((int)(uptr)mapping->file_handle - 1))
        failed = 1;
    return failed ? file_error_flush_failed : file_error_ok;
#else
    return file_error_unsupported;
#endif
}

static file_error file_unmap(file_mapping *mapping) {
    if (!mapping) {
        return file_error_invalid;
    }
#if defined(_WIN32)
    int failed = 0;
    if (mapping->data && !UnmapViewOfFile(mapping->data)) {
        failed = 1;
    }
    if (mapping->mapping_handle && !CloseHandle(mapping->mapping_handle)) {
        failed = 1;
    }
    if (mapping->file_handle && !CloseHandle(mapping->file_handle)) {
        failed = 1;
    }
    memset(mapping, 0, sizeof(*mapping));
    return failed ? file_error_cleanup_failed : file_error_ok;
#elif defined(__APPLE__)
    int failed = 0;
    if (mapping->data && munmap(mapping->data, mapping->size))
        failed = 1;
    if (mapping->file_handle && close((int)(uptr)mapping->file_handle - 1))
        failed = 1;
    memset(mapping, 0, sizeof(*mapping));
    return failed ? file_error_cleanup_failed : file_error_ok;
#else
    return file_error_unsupported;
#endif
}

/* Ordinary caller-owned native file stream, no stdio buffer/heap allocation.
   A handle is opaque (macOS encodes fd + 1). I/O transfers one bounded span;
   short transfers fail. Opening/truncation belongs outside simulation. */
static void *file_stream_open(const char *path, int writing) {
#if defined(_WIN32)
    HANDLE handle = CreateFileA(path, writing ? GENERIC_WRITE : GENERIC_READ, 0, NULL,
                                writing ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return handle == INVALID_HANDLE_VALUE ? NULL : handle;
#else
    int descriptor = open(path, writing ? O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW : O_RDONLY | O_NOFOLLOW, 0600);
    return descriptor < 0 ? NULL : (void *)(uptr)(descriptor + 1);
#endif
}

/* Exclusive output creation; never truncates an existing file. Caller closes
   returned stream. NULL means destination exists or OS creation failed. */
static void *file_stream_create(const char *path) {
    if (!path || !path[0])
        return NULL;
#if defined(_WIN32)
    HANDLE handle = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    return handle == INVALID_HANDLE_VALUE ? NULL : handle;
#else
    int descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    return descriptor < 0 ? NULL : (void *)(uptr)(descriptor + 1);
#endif
}

static int file_stream_transfer(void *handle, void *bytes, usize size, int writing) {
    if (!handle || (size && !bytes) || size > UINT32_MAX)
        return 0;
#if defined(_WIN32)
    DWORD transferred = 0;
    return (writing ? WriteFile(handle, bytes, (DWORD)size, &transferred, NULL) :
                      ReadFile(handle, bytes, (DWORD)size, &transferred, NULL)) && transferred == size;
#else
    int descriptor = (int)(uptr)handle - 1;
    ssize_t count = writing ? write(descriptor, bytes, size) : read(descriptor, bytes, size);
    return count >= 0 && (usize)count == size;
#endif
}

static int file_stream_size(void *handle, u64 *size) {
    if (!handle || !size)
        return 0;
#if defined(_WIN32)
    LARGE_INTEGER length;
    if (!GetFileSizeEx(handle, &length) || length.QuadPart < 0)
        return 0;
    *size = (u64)length.QuadPart;
#else
    struct stat status;
    if (fstat((int)(uptr)handle - 1, &status) || !S_ISREG(status.st_mode) || status.st_size < 0)
        return 0;
    *size = (u64)status.st_size;
#endif
    return 1;
}

static int file_stream_seek(void *handle, u64 offset) {
    if (!handle || offset > INT64_MAX)
        return 0;
#if defined(_WIN32)
    LARGE_INTEGER position;
    position.QuadPart = (LONGLONG)offset;
    return SetFilePointerEx(handle, position, NULL, FILE_BEGIN);
#else
    return lseek((int)(uptr)handle - 1, (off_t)offset, SEEK_SET) == (off_t)offset;
#endif
}

static int file_stream_truncate(void *handle, u64 size) {
    if (!handle || size > INT64_MAX)
        return 0;
#if defined(_WIN32)
    return file_stream_seek(handle, size) && SetEndOfFile(handle);
#else
    return ftruncate((int)(uptr)handle - 1, (off_t)size) == 0;
#endif
}

static int file_stream_flush(void *handle) {
    if (!handle)
        return 0;
#if defined(_WIN32)
    return FlushFileBuffers(handle);
#else
    return fsync((int)(uptr)handle - 1) == 0;
#endif
}

static int file_stream_close(void *handle) {
    if (!handle)
        return 1;
#if defined(_WIN32)
    return CloseHandle(handle);
#else
    return close((int)(uptr)handle - 1) == 0;
#endif
}

static file_stat_result file_stat(cstring path) {
    file_stat_result out = {0};
    struct stat info;

    if (path.data == NULL) {
        out.error = file_error_invalid;
        return out;
    }

    if (stat(path.data, &info) != 0) {
        out.error = file_error_open_failed;
        return out;
    }

    out.size = (u64)info.st_size;
    out.modified_time = (i64)info.st_mtime;
    out.error = file_error_ok;
    return out;
}

static int path_is_dir(string_view path, int *out_is_dir) {
    struct stat info;

    if (path.data == NULL || out_is_dir == NULL) {
        return 0;
    }
    char path_data[path.len + 1u];
    memcpy(path_data, path.data, path.len);
    path_data[path.len] = '\0';
    if (stat(path_data, &info) != 0) {
        return 0;
    }
#if defined(S_ISDIR)
    *out_is_dir = S_ISDIR(info.st_mode) ? 1 : 0;
#else
    *out_is_dir = (info.st_mode & S_IFDIR) != 0;
#endif
    return 1;
}

static int path_matches_pattern(string_view path, string_view pattern) {
    if (path.data == NULL || pattern.data == NULL) {
        return 0;
    }
    if (pattern.len == 1u && pattern.data[0] == '*') {
        return 1;
    }
    if (pattern.len > 1u && pattern.data[0] == '*') {
        string_view suffix = {.data = pattern.data + 1u, .len = pattern.len - 1u};

        return path.len >= suffix.len &&
               memcmp(path.data + path.len - suffix.len, suffix.data, suffix.len) == 0;
    }
    return path.len == pattern.len && memcmp(path.data, pattern.data, pattern.len) == 0;
}

static string_result path_join(arena *arena, string_view dir, string_view name) {
    string_result out = {0};
    int needs_separator;
    arena_result allocation;

    if (arena == NULL || dir.data == NULL || name.data == NULL) {
        out.error = arena_error_invalid;
        return out;
    }
    needs_separator =
        dir.len != 0 && dir.data[dir.len - 1u] != '/' && dir.data[dir.len - 1u] != '\\';
    allocation = arena_alloc(arena, dir.len + (needs_separator ? 1u : 0u) + name.len, 1);
    if (allocation.error != arena_error_ok) {
        out.error = allocation.error;
        return out;
    }
    memcpy(allocation.ptr, dir.data, dir.len);
    if (needs_separator) {
        ((char *)allocation.ptr)[dir.len] = '/';
    }
    memcpy((char *)allocation.ptr + dir.len + (needs_separator ? 1u : 0u), name.data, name.len);
    out.value =
        string_from_raw((u8 *)allocation.ptr, dir.len + (needs_separator ? 1u : 0u) + name.len);
    out.error = arena_error_ok;
    return out;
}

static file_list_result file_list(arena *arena, string_view dir, string_view pattern) {
    file_list_result out = {0};

    if (arena == NULL || dir.data == NULL || pattern.data == NULL) {
        out.error = file_error_invalid;
        return out;
    }
    out.paths = (string_array){.raw = {.arena = arena}};
#if defined(_WIN32)
    {
        cstring directory = cstring_from_string(arena, string_from_raw((u8 *)dir.data, dir.len));
        cstring search_path_cstr;
        WIN32_FIND_DATAA data;
        HANDLE handle;
        DWORD attributes, error;

        if (directory.data == NULL) {
            out.error = file_error_out_of_memory;
            return out;
        }
        attributes = GetFileAttributesA(directory.data);
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            out.error = file_error_open_failed;
            return out;
        }
        string_result search_path = path_join(arena, dir, pattern);
        if (search_path.error != arena_error_ok) {
            out.error = file_error_from_arena_error(search_path.error);
            return out;
        }
        search_path_cstr = cstring_from_string(arena, search_path.value);
        if (search_path_cstr.data == NULL) {
            out.error = file_error_out_of_memory;
            return out;
        }
        handle = FindFirstFileA(search_path_cstr.data, &data);
        if (handle == INVALID_HANDLE_VALUE) {
            out.error = GetLastError() == ERROR_FILE_NOT_FOUND ? file_error_ok
                                                             : file_error_open_failed;
            return out;
        }
        do {
            string_result path;

            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                continue;
            }
            path = path_join(arena,
                             dir,
                             string_view_from_string_full(
                                 string_from_cstring(cstring_from_raw(data.cFileName))));
            if (path.error != arena_error_ok) {
                FindClose(handle);
                out.error = file_error_from_arena_error(path.error);
                return out;
            }
            if (dynamic_array_push(&out.paths, &path.value) != arena_error_ok) {
                FindClose(handle);
                out.error = file_error_out_of_memory;
                return out;
            }
        } while (FindNextFileA(handle, &data));
        error = GetLastError();
        int closed = FindClose(handle);
        if (error != ERROR_NO_MORE_FILES || !closed) {
            out.error = error != ERROR_NO_MORE_FILES ? file_error_read_failed
                                                    : file_error_cleanup_failed;
            return out;
        }
    }
#else
    {
        string dir_string = string_from_raw((u8 *)dir.data, dir.len);
        cstring dir_cstr = cstring_from_string(arena, dir_string);
        DIR *directory;
        struct dirent *entry;

        if (dir_cstr.data == NULL) {
            out.error = file_error_out_of_memory;
            return out;
        }
        directory = opendir(dir_cstr.data);
        if (directory == NULL) {
            out.error = file_error_open_failed;
            return out;
        }
        while ((entry = readdir(directory)) != NULL) {
            string_view entry_name = string_view_from_string_full(
                string_from_cstring(cstring_from_raw(entry->d_name)));
            string_result path;

            if (!path_matches_pattern(entry_name, pattern)) {
                continue;
            }
            path = path_join(arena, dir, entry_name);
            if (path.error != arena_error_ok) {
                closedir(directory);
                out.error = file_error_from_arena_error(path.error);
                return out;
            }
            {
                int is_dir = 0;
                if (path_is_dir(string_view_from_string_full(path.value), &is_dir) && is_dir) {
                    continue;
                }
            }
            if (dynamic_array_push(&out.paths, &path.value) != arena_error_ok) {
                closedir(directory);
                out.error = file_error_out_of_memory;
                return out;
            }
        }
        closedir(directory);
    }
#endif
    out.error = file_error_ok;
    return out;
}

/* Native filesystem/process helpers for tools. Caller owns iterator/workspace;
   fixed capacities never grow. No shell, retry, or dependency installation. */
enum { NATIVE_PATH_SIZE = 1024, PROCESS_MAX_ARGUMENTS = 192, PROCESS_COMMAND_SIZE = 32768 };

typedef struct file_directory {
#if defined(_WIN32)
    HANDLE handle;
    WIN32_FIND_DATAW entry;
    int first;
#else
    DIR *handle;
#endif
    char name[NATIVE_PATH_SIZE];
} file_directory;

#if defined(_WIN32)
static DWORD file_native_attributes(const char *path) {
    wchar_t wide[NATIVE_PATH_SIZE];
    if (!path || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1,
                                     wide, NATIVE_PATH_SIZE))
        return INVALID_FILE_ATTRIBUTES;
    return GetFileAttributesW(wide);
}
#endif

static int file_path_is_link(const char *path) {
#if defined(_WIN32)
    DWORD attributes = file_native_attributes(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_REPARSE_POINT);
#else
    struct stat status;
    return !lstat(path, &status) && S_ISLNK(status.st_mode);
#endif
}

static int file_is_directory(const char *path, int allow_links) {
#if defined(_WIN32)
    DWORD attributes = file_native_attributes(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) &&
           (allow_links || !(attributes & FILE_ATTRIBUTE_REPARSE_POINT));
#else
    struct stat status;
    return !(allow_links ? stat(path, &status) : lstat(path, &status)) && S_ISDIR(status.st_mode);
#endif
}

static int file_is_regular(const char *path, int allow_links) {
#if defined(_WIN32)
    DWORD attributes = file_native_attributes(path);
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY) &&
           (allow_links || !(attributes & FILE_ATTRIBUTE_REPARSE_POINT));
#else
    struct stat status;
    return !(allow_links ? stat(path, &status) : lstat(path, &status)) && S_ISREG(status.st_mode);
#endif
}

static int file_directory_create(const char *path, int exclusive) {
#if defined(_WIN32)
    wchar_t wide[NATIVE_PATH_SIZE];
    if (!path || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, NATIVE_PATH_SIZE))
        return 0;
    int created = CreateDirectoryW(wide, NULL);
#else
    int created = mkdir(path, 0755) == 0;
#endif
    return created || (!exclusive && file_is_directory(path, 1));
}

static int file_directory_open(file_directory *directory, const char *path) {
    if (!directory || !path || !file_is_directory(path, 0))
        return 0;
#if defined(_WIN32)
    char search[NATIVE_PATH_SIZE];
    wchar_t wide[NATIVE_PATH_SIZE];
    int length = snprintf(search, sizeof(search), "%s/*", path);
    if (length < 0 || length >= NATIVE_PATH_SIZE ||
        !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, search, -1, wide, NATIVE_PATH_SIZE))
        return 0;
    directory->handle = FindFirstFileW(wide, &directory->entry);
    directory->first = 1;
    return directory->handle != INVALID_HANDLE_VALUE || GetLastError() == ERROR_FILE_NOT_FOUND;
#else
    directory->handle = opendir(path);
    return directory->handle != NULL;
#endif
}

/* 1 entry, 0 end, -1 error. Names valid until next read; caller bounds visits. */
static int file_directory_read(file_directory *directory) {
#if defined(_WIN32)
    if (directory->handle == INVALID_HANDLE_VALUE)
        return 0;
    if (!directory->first && !FindNextFileW(directory->handle, &directory->entry))
        return GetLastError() == ERROR_NO_MORE_FILES ? 0 : -1;
    directory->first = 0;
    return WideCharToMultiByte(CP_UTF8, 0, directory->entry.cFileName, -1, directory->name,
                               sizeof(directory->name), NULL, NULL) ? 1 : -1;
#else
    errno = 0;
    struct dirent *entry = readdir(directory->handle);
    if (!entry)
        return errno ? -1 : 0;
    size_t size = strlen(entry->d_name);
    if (size >= sizeof(directory->name))
        return -1;
    memcpy(directory->name, entry->d_name, size + 1);
    return 1;
#endif
}

static int file_directory_close(file_directory *directory) {
#if defined(_WIN32)
    int success = directory->handle == INVALID_HANDLE_VALUE || FindClose(directory->handle);
    directory->handle = INVALID_HANDLE_VALUE;
#else
    int success = closedir(directory->handle) == 0;
    directory->handle = NULL;
#endif
    return success;
}

/* Partial/truncated destination possible on failure; no atomic publication. */
static int file_copy(const char *source, const char *destination, int exclusive) {
#if defined(_WIN32)
    wchar_t input[NATIVE_PATH_SIZE], output[NATIVE_PATH_SIZE];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, source, -1, input, NATIVE_PATH_SIZE) ||
        !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, destination, -1, output, NATIVE_PATH_SIZE))
        return 0;
    return CopyFileW(input, output, exclusive);
#else
    int input = open(source, O_RDONLY);
    if (input < 0)
        return 0;
    struct stat status;
    if (fstat(input, &status) || !S_ISREG(status.st_mode)) {
        close(input);
        return 0;
    }
    int output = open(destination, O_WRONLY | O_CREAT | (exclusive ? O_EXCL : O_TRUNC),
                      status.st_mode & 0777);
    if (output < 0) {
        close(input);
        return 0;
    }
    char buffer[16384];
    int success = 1;
    off_t remaining = status.st_size;
    while (remaining > 0) {
        size_t wanted = remaining > (off_t)sizeof(buffer) ? sizeof(buffer) : (size_t)remaining;
        ssize_t read_count = read(input, buffer, wanted);
        if (read_count <= 0) {
            success = 0;
            break;
        }
        for (ssize_t offset = 0; offset < read_count;) {
            ssize_t written = write(output, buffer + offset, (size_t)(read_count - offset));
            if (written <= 0) {
                success = 0;
                break;
            }
            offset += written;
        }
        if (!success)
            break;
        remaining -= read_count;
    }
    if (close(input))
        success = 0;
    if (close(output))
        success = 0;
    return success;
#endif
}

typedef struct process_workspace {
    char command[PROCESS_COMMAND_SIZE];
#if defined(_WIN32)
    wchar_t wide_command[PROCESS_COMMAND_SIZE];
    wchar_t wide_path[NATIVE_PATH_SIZE];
#endif
    unsigned long exit_code;
    unsigned long native_error;
    const char *error;
} process_workspace;

/* Wait for one child, no shell/timeouts. Child stdin is closed input; stdout and
   stderr append to log, or inherit both when log_path is NULL. Return means OS
   launch/wait succeeded, not child success. */
static int process_run(process_workspace *workspace,
                       const char *const *arguments,
                       usize count,
                       const char *log_path) {
    workspace->error = NULL;
    workspace->exit_code = 0;
    workspace->native_error = 0;
    if (!arguments || !count || count > PROCESS_MAX_ARGUMENTS) {
        workspace->error = "invalid process arguments";
        return 0;
    }
    usize position = 0;
    for (usize i = 0; i < count; ++i) {
        if (!arguments[i]) {
            workspace->error = "invalid process argument";
            return 0;
        }
        const char *text = arguments[i];
        usize slashes = 0;
        if (position + 3 >= PROCESS_COMMAND_SIZE)
            goto capacity;
        if (i)
            workspace->command[position++] = ' ';
        workspace->command[position++] = '"';
        for (;;) {
            char byte = *text++;
            if (byte == '\\') {
                ++slashes;
                if (slashes >= PROCESS_COMMAND_SIZE)
                    goto capacity;
                continue;
            }
            usize repeat = (byte == '"' || !byte) ? slashes * 2 : slashes;
            if (position + repeat + 3 >= PROCESS_COMMAND_SIZE)
                goto capacity;
            while (repeat--)
                workspace->command[position++] = '\\';
            slashes = 0;
            if (!byte)
                break;
            if (byte == '"')
                workspace->command[position++] = '\\';
            workspace->command[position++] = byte;
        }
        workspace->command[position++] = '"';
    }
    workspace->command[position] = 0;
    if (log_path) {
        FILE *record = fopen(log_path, "ab");
        if (!record) {
            workspace->error = "could not open process log";
            goto native_failure;
        }
        int saved = fprintf(record, "\n%s\n", workspace->command) >= 0;
        if (fclose(record))
            saved = 0;
        if (!saved) {
            workspace->error = "could not save process log";
            goto native_failure;
        }
    }
#if defined(_WIN32)
    if (!MultiByteToWideChar(CP_UTF8,
                             MB_ERR_INVALID_CHARS,
                             workspace->command,
                             -1,
                             workspace->wide_command,
                             PROCESS_COMMAND_SIZE) ||
        (log_path && !MultiByteToWideChar(CP_UTF8,
                                          MB_ERR_INVALID_CHARS,
                                          log_path,
                                          -1,
                                          workspace->wide_path,
                                          NATIVE_PATH_SIZE))) {
        workspace->error = "invalid UTF-8 process command/path";
        goto native_failure;
    }
    SECURITY_ATTRIBUTES security = {sizeof(security), NULL, TRUE};
    HANDLE log = log_path ? CreateFileW(workspace->wide_path,
                                        FILE_APPEND_DATA,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        &security,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL)
                          : GetStdHandle(STD_OUTPUT_HANDLE);
    if (log == INVALID_HANDLE_VALUE) {
        workspace->error = "could not open process log handle";
        goto native_failure;
    }
    HANDLE input = CreateFileW(L"NUL",
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               &security,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (input == INVALID_HANDLE_VALUE) {
        if (log_path)
            CloseHandle(log);
        workspace->error = "could not open process input";
        goto native_failure;
    }
    STARTUPINFOW startup = {0};
    PROCESS_INFORMATION child = {0};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input;
    startup.hStdOutput = log;
    startup.hStdError = log_path ? log : GetStdHandle(STD_ERROR_HANDLE);
    int launched = CreateProcessW(NULL,
                                  workspace->wide_command,
                                  NULL,
                                  NULL,
                                  TRUE,
                                  0,
                                  NULL,
                                  NULL,
                                  &startup,
                                  &child);
    CloseHandle(input);
    if (log_path)
        CloseHandle(log);
    if (!launched) {
        workspace->error = "could not launch process";
        goto native_failure;
    }
    DWORD code;
    int waited = WaitForSingleObject(child.hProcess, INFINITE) == WAIT_OBJECT_0 &&
                 GetExitCodeProcess(child.hProcess, &code);
    CloseHandle(child.hThread);
    CloseHandle(child.hProcess);
    if (!waited) {
        workspace->error = "could not wait for process";
        goto native_failure;
    }
    workspace->exit_code = code;
#else
    char *argv[PROCESS_MAX_ARGUMENTS + 1];
    for (usize i = 0; i < count; ++i)
        argv[i] = (char *)arguments[i];
    argv[count] = NULL;
    posix_spawn_file_actions_t actions;
    int error = posix_spawn_file_actions_init(&actions);
    if (error) {
        errno = error;
        workspace->error = "could not initialize process actions";
        goto native_failure;
    }
    error = posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    if (!error && log_path)
        error = posix_spawn_file_actions_addopen(&actions,
                                                 STDOUT_FILENO,
                                                 log_path,
                                                 O_WRONLY | O_APPEND,
                                                 0600);
    if (!error && log_path)
        error = posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);
    pid_t child;
    extern char **environ;
    if (!error)
        error = posix_spawnp(&child, argv[0], &actions, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    if (error) {
        errno = error;
        workspace->error = "could not launch process";
        goto native_failure;
    }
    int status;
    pid_t waited;
    do {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited != child) {
        workspace->error = "could not wait for process";
        goto native_failure;
    }
    workspace->exit_code = WIFEXITED(status) ? (unsigned long)WEXITSTATUS(status) : 128;
#endif
    return 1;
capacity:
    workspace->error = "process command capacity exceeded";
    return 0;
native_failure:
#if defined(_WIN32)
    workspace->native_error = GetLastError();
#else
    workspace->native_error = (unsigned long)errno;
#endif
    return 0;
}

/* Bounded lexical C generator helpers. Caller owns arena; no backing growth.
   TEST/BENCH namespaces are separate collections. Declaration-shaped lines in
   multiline comments or inactive preprocessor regions remain lexical matches. */
enum {
    GENERATOR_MAX_FILES = 1024,
    GENERATOR_MAX_DECLARATIONS = 4096,
    GENERATOR_LINE_SIZE = 4096,
    GENERATOR_PATH_SIZE = 1024,
    GENERATOR_NAME_SIZE = 256,
    GENERATOR_BACKING_SIZE = 4 * 1024 * 1024
};
/* A single discovery pass supplies both includes and calls, preserving declaration order. */
typedef struct generator_declaration {
    const char *source;
    const char *name;
} generator_declaration;

static int generator_write_literal(FILE *out, const char *prefix, const char *text) {
    const char *parts[] = {prefix, text};
    size_t i;
    if (fputc('"', out) == EOF)
        return 0;
    for (i = 0; i < 2; i++) {
        const unsigned char *byte = (const unsigned char *)parts[i];
        for (; *byte; byte++) {
            if (*byte == '"' || *byte == '\\' || *byte == '?') {
                if (fputc('\\', out) == EOF || fputc(*byte, out) == EOF)
                    return 0;
            } else if (*byte < 32 || *byte >= 127) {
                if (fprintf(out, "\\%03o", *byte) < 0)
                    return 0;
            } else if (fputc(*byte, out) == EOF) {
                return 0;
            }
        }
    }
    return fputc('"', out) != EOF;
}

/* Header names are not C string literals: escapes are not decoded by #include.
   Windows separators can be normalized; quotes/control bytes cannot name a header. */
static int generator_write_include(FILE *out, const char *prefix, const char *path) {
    const char *parts[] = {prefix, path};
    size_t i;
    if (fputs("#include \"", out) == EOF)
        return 0;
    for (i = 0; i < 2; i++) {
        const unsigned char *byte = (const unsigned char *)parts[i];
        for (; *byte; byte++) {
            if (*byte == '"' || *byte < 32 || *byte == 127) {
                fprintf(stderr, "path cannot be represented in an include directive: %s\n", path);
                return 0;
            }
            if (fputc(*byte == '\\' ? '/' : *byte, out) == EOF)
                return 0;
        }
    }
    return fputs("\"\n", out) != EOF;
}

static int generator_compare_paths(const void *left, const void *right) {
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}

#if defined(_WIN32)
/* Reject explicit paths tunneling through directory links. Fixed buffer, no growth. */
static int generator_path_is_unlinked(const char *path) {
    char full[GENERATOR_PATH_SIZE];
    DWORD length = GetFullPathNameA(path, sizeof(full), full, NULL);
    size_t i;
    if (length < 3 || length >= sizeof(full))
        return 0;
    for (i = 3; full[i]; i++) {
        if (full[i] == '/' || full[i] == '\\') {
            char separator = full[i];
            DWORD attributes;
            full[i] = 0;
            attributes = GetFileAttributesA(full);
            full[i] = separator;
            if (attributes == INVALID_FILE_ATTRIBUTES ||
                (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
                return 0;
        }
    }
    return 1;
}
#endif
static int generator_path_is_regular(const char *path) {
#if defined(_WIN32)
    DWORD attributes = GetFileAttributesA(path);
    if (!generator_path_is_unlinked(path))
        return 0;
    return attributes != INVALID_FILE_ATTRIBUTES &&
           !(attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT));
#else
    struct stat info;
    return lstat(path, &info) == 0 && S_ISREG(info.st_mode);
#endif
}

static int
generator_collect_dir(arena *storage, const char *directory, const char **files, size_t *count) {
    size_t first = *count;
    string_view dir = string_view_from_cstr(directory);
#if defined(_WIN32)
    DWORD attributes = GetFileAttributesA(directory);
    WIN32_FIND_DATAA entry;
    HANDLE handle;
    DWORD error;
    string_result search;
    cstring search_path;
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT) || !generator_path_is_unlinked(directory)) {
        fprintf(stderr, "invalid or linked generator directory: %s\n", directory);
        return 0;
    }
    search = path_join(storage, dir, string_view_from_cstr("*"));
    if (search.error != arena_error_ok ||
        (search_path = cstring_from_string(storage, search.value)).data == NULL) {
        fprintf(stderr, "generator storage exhausted\n");
        return 0;
    }
    handle = FindFirstFileA(search_path.data, &entry);
    if (handle == INVALID_HANDLE_VALUE) {
        error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND)
            return 1;
        fprintf(stderr, "failed to list %s (Windows error %lu)\n", directory, (unsigned long)error);
        return 0;
    }
    do {
        size_t length = strlen(entry.cFileName);
        string_result joined;
        cstring path;
        if ((entry.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
            length < 2 || entry.cFileName[length - 2] != '.' ||
            (entry.cFileName[length - 1] != 'c' && entry.cFileName[length - 1] != 'h')) {
            continue;
        }
        if (*count == GENERATOR_MAX_FILES) {
            fprintf(stderr, "generator file limit exceeded (%d)\n", GENERATOR_MAX_FILES);
            FindClose(handle);
            return 0;
        }
        joined = path_join(storage, dir, string_view_from_cstr(entry.cFileName));
        if (joined.error != arena_error_ok ||
            (path = cstring_from_string(storage, joined.value)).data == NULL) {
            fprintf(stderr, "generator storage exhausted\n");
            FindClose(handle);
            return 0;
        }
        files[(*count)++] = path.data;
    } while (FindNextFileA(handle, &entry));
    error = GetLastError();
    if (!FindClose(handle)) {
        fprintf(stderr, "failed to close listing for %s\n", directory);
        return 0;
    }
    if (error != ERROR_NO_MORE_FILES) {
        fprintf(stderr, "failed to list %s (Windows error %lu)\n", directory, (unsigned long)error);
        return 0;
    }
#else
    const char *patterns[] = {"*.c", "*.h"};
    size_t pattern;
    struct stat info;
    if (lstat(directory, &info) != 0 || !S_ISDIR(info.st_mode)) {
        fprintf(stderr, "invalid or linked generator directory: %s\n", directory);
        return 0;
    }
    for (pattern = 0; pattern < 2; pattern++) {
        file_list_result list = file_list(storage, dir, string_view_from_cstr(patterns[pattern]));
        usize i;
        if (list.error != file_error_ok) {
            fprintf(stderr,
                    "failed to list %s (error %d; bounded generator storage)\n",
                    directory,
                    list.error);
            return 0;
        }
        for (i = 0; i < list.paths.raw.len; i++) {
            cstring path = cstring_from_string(storage, dynamic_array_data(&list.paths)[i]);
            if (path.data == NULL) {
                fprintf(stderr, "generator storage exhausted\n");
                return 0;
            }
            if (!generator_path_is_regular(path.data))
                continue;
            if (*count == GENERATOR_MAX_FILES) {
                fprintf(stderr, "generator file limit exceeded (%d)\n", GENERATOR_MAX_FILES);
                return 0;
            }
            files[(*count)++] = path.data;
        }
    }
#endif
    qsort(files + first, *count - first, sizeof(*files), generator_compare_paths);
    return 1;
}

static int generator_read_declarations(arena *storage,
                                       const char *path,
                                       const char *macro,
                                       generator_declaration *declarations,
                                       size_t *count) {
    FILE *file = fopen(path, "r");
    char line[GENERATOR_LINE_SIZE];
    size_t line_number = 0;
    const char *kind = strcmp(macro, "TEST(") == 0 ? "test" : "bench";
    int ok = 1;
    if (file == NULL) {
        fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
        return 0;
    }
    /* ponytail: line-based declarations, not a C preprocessor/parser. Require
       Place declarations at the start of a line, after optional spaces/tabs. */
    while (fgets(line, sizeof(line), file) != NULL) {
        char *start = line;
        char name[GENERATOR_NAME_SIZE];
        size_t length = 0;
        size_t i;
        cstring stored;
        line_number++;
        if (strchr(line, '\n') == NULL && !feof(file) && fgetc(file) != EOF) {
            fprintf(stderr,
                    "%s:%zu: generator line limit exceeded (%d bytes)\n",
                    path,
                    line_number,
                    GENERATOR_LINE_SIZE - 1);
            ok = 0;
            break;
        }
        while (*start == ' ' || *start == '\t')
            start++;
        if (strncmp(start, macro, strlen(macro)) != 0)
            continue;
        start += strlen(macro);
        while (*start == ' ' || *start == '\t')
            start++;
        if (!(*start == '_' || (*start >= 'A' && *start <= 'Z') ||
              (*start >= 'a' && *start <= 'z'))) {
            fprintf(stderr, "%s:%zu: invalid %s identifier\n", path, line_number, kind);
            ok = 0;
            break;
        }
        while (*start == '_' || (*start >= '0' && *start <= '9') ||
               (*start >= 'A' && *start <= 'Z') || (*start >= 'a' && *start <= 'z')) {
            if (length + 1 == sizeof(name))
                break;
            name[length++] = *start++;
        }
        name[length] = 0;
        while (*start == ' ' || *start == '\t')
            start++;
        if (*start != ',' && *start != ')') {
            fprintf(stderr,
                    "%s:%zu: invalid or oversized %s identifier\n",
                    path,
                    line_number,
                    kind);
            ok = 0;
            break;
        }
        for (i = 0; i < *count; i++) {
            if (strcmp(declarations[i].name, name) == 0) {
                fprintf(stderr,
                        "duplicate %s identifier %s in %s and %s\n",
                        kind,
                        name,
                        declarations[i].source,
                        path);
                ok = 0;
                break;
            }
        }
        if (!ok)
            break;
        if (*count == GENERATOR_MAX_DECLARATIONS) {
            fprintf(stderr, "generator %s limit exceeded (%d)\n", kind, GENERATOR_MAX_DECLARATIONS);
            ok = 0;
            break;
        }
        stored = cstring_from_string(storage, string_from_raw((u8 *)name, length));
        if (stored.data == NULL) {
            fprintf(stderr, "generator storage exhausted\n");
            ok = 0;
            break;
        }
        declarations[(*count)++] = (generator_declaration){path, stored.data};
    }
    if (ferror(file)) {
        fprintf(stderr, "failed to read %s\n", path);
        ok = 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "failed to close %s\n", path);
        ok = 0;
    }
    return ok;
}

static int generator_collect(arena *storage,
                             const char *const *paths,
                             int directories,
                             const char *macro,
                             generator_declaration **result,
                             size_t *result_count) {
    arena_result file_memory =
        arena_alloc(storage, GENERATOR_MAX_FILES * sizeof(char *), _Alignof(char *));
    arena_result declaration_memory =
        arena_alloc(storage,
                    GENERATOR_MAX_DECLARATIONS * sizeof(generator_declaration),
                    _Alignof(generator_declaration));
    const char **files = file_memory.ptr;
    generator_declaration *declarations = declaration_memory.ptr;
    size_t file_count = 0;
    size_t declaration_count = 0;
    size_t i;
    if (file_memory.error != arena_error_ok || declaration_memory.error != arena_error_ok) {
        fprintf(stderr, "generator storage exhausted\n");
        return 1;
    }
    for (i = 0; paths[i] != NULL; i++) {
        if (i == GENERATOR_MAX_FILES) {
            fprintf(stderr, "generator input limit exceeded\n");
            return 1;
        }
        if (directories) {
            if (!generator_collect_dir(storage, paths[i], files, &file_count))
                return 1;
        } else {
            if (file_count == GENERATOR_MAX_FILES || !generator_path_is_regular(paths[i])) {
                fprintf(stderr, "invalid, linked, or excess generator file: %s\n", paths[i]);
                return 1;
            }
            files[file_count++] = paths[i];
        }
    }
    for (i = 0; i < file_count; i++) {
        if (!generator_read_declarations(storage,
                                         files[i],
                                         macro,
                                         declarations,
                                         &declaration_count))
            return 1;
    }
    *result = declarations;
    *result_count = declaration_count;
    return 0;
}

#endif
