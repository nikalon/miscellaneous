#pragma once

/*
 * Common utilities that can be used in any project. It includes:
 *  - Primitive types
 *  - Buffers
 *  - Strings
 *  - Arena and scratch arena
 *  - Basic file I/O
 *
 * Tests are defined in `basic_test.cpp`.
 * */

#include <stdalign.h>
#include <stdint.h>

// ####################################################################################################################
// Primitive types

// Enable support for boolean values in C
#ifndef __cplusplus
typedef int   bool;
#define true  1
#define false 0
#endif

typedef int8_t  b8;
typedef int16_t b16;
typedef int32_t b32;
typedef int64_t b64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

// ####################################################################################################################
// Miscellaneous macros

#define UNUSED(value) ((void) value)

#define KiB (1024)
#define MiB (1024*KiB)
#define GiB (1024*MiB)
#define TiB (1024*GiB)

#define ARRAY_LENGTH(array) ((u64)(sizeof(array)/sizeof(array[0])))

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define CLAMP(val, min, max) (MIN(MAX(val, min), max))
#define ABS(val) (val >= 0 ? val : -val)

#define BUFFER_TO_STRING(buffer) (String){ .data = (const u8*) buffer.data, .length = buffer.length }

// ====================================================================================================================
// Arena
typedef struct {
    u8 *_memory_start;
    u8 *_position;
    u8 *_next_reserved_page;
    u64 _capacity;
    u64 _page_size;
} Arena;

Arena arena_alloc(u64 capacity_hint);
void  arena_free(Arena *arena);

// Push macros and functions. Use the following macros with some example arguments:
// - arena_push(&arena, b16)
// - arena_push(&arena, i32, 12)
// - arena_push_nozero(&arena, MyStruct)
// - arena_push_nozero(&arena, char, 30)
#define GET_ARENA_PUSH_MACRO(_1, _2, _3, MACRO, ...) MACRO
#define arena_push(...) GET_ARENA_PUSH_MACRO(__VA_ARGS__, arena_push_3, arena_push_2, arena_push_1)(__VA_ARGS__)
#define arena_push_2(arena, type) (type*)arena_push_data(arena, sizeof(type), 1, alignof(type), 1)
#define arena_push_3(arena, type, count) (type*)arena_push_data(arena, sizeof(type), count, alignof(type), 1)
#define arena_push_nozero(...) GET_ARENA_PUSH_MACRO(__VA_ARGS__, arena_push_nozero_3, arena_push_nozero_2, arena_push_nozero_1)(__VA_ARGS__)
#define arena_push_nozero_2(arena, type) (type*)arena_push_data(arena, sizeof(type), 1, alignof(type), 0)
#define arena_push_nozero_3(arena, type, count) (type*)arena_push_data(arena, sizeof(type), count, alignof(type), 0)
void *arena_push_data(Arena *arena, u64 type_size, u64 count, u64 alignment, b32 zero_data);

// Grow the element pushed in the arena or reallocate it if there's not space left. It's discouraged to use this function
// directly. Use any of the arena_push() or arena_push_nozero() macros instead.
void *arena_grow_or_realloc(Arena *arena, void *prev_memory, u64 prev_size, u64 new_size);

u64  arena_get_capacity(Arena *arena);
u64  arena_get_pos(Arena *arena);
void arena_set_pos(Arena *arena, u64 pos);
void arena_clear(Arena *arena);

// ####################################################################################################################
// Buffer
typedef struct {
    u8  *data;
    u64 length;
} Buffer;

#define BUFFER_FROM_ARRAY(array) (Buffer){ .data = (u8*)array, .length = sizeof(array) }

// Slice
Buffer buffer_slice(Buffer buf, u64 start, u64 end);

// Read. Any read operation consumes the buffer, even if there's not enough bytes to read.
bool buffer_read_nocopy(Buffer *inBuffer, Buffer *outBuffer, u64 count); // @TODO: Choose a better function name

// bool buffer_read_u8(Buffer *buffer, u8 *out);
// ...
// bool buffer_read_f64(Buffer *buffer, f64 *out);
#define BUFFER_READ_FUNCTIONS \
    X(b8)  \
    X(b16) \
    X(b32) \
    X(b64) \
    X(i8)  \
    X(i16) \
    X(i32) \
    X(i64) \
    X(u8)  \
    X(u16) \
    X(u32) \
    X(u64) \
    X(f32) \
    X(f64)

// If there's not enough bytes to read it reads zero and consumes the buffer.
#define X(type) bool buffer_read_##type(Buffer *buffer, type *out);
BUFFER_READ_FUNCTIONS
#undef X

bool    buffer_read_count (Buffer *buffer, void *out, u64 count);
#define buffer_read_struct(buffer, out) buffer_read_count(buffer, out, sizeof(*out))

// ####################################################################################################################
// String

// Fat pointer to a read-only string. It's not guaranteed that the string is null-terminated, so don't just assume you can
// cast String.data into const char* directly. If you want to convert between Strings and C-strings it's best to use the
// following functions:
// - string_from_cstring() -> Convert C-string to String. It doesn't allocate any memory because it's just a String view.
// - string_to_cstring()   -> Convert String to C-string. It allocates memory because we need to write the null character.
typedef struct {
    const u8 *data;
    u64 length;
} String;

#define S(string) (String){ .data = (const u8*)string, .length = ARRAY_LENGTH(string)-1 }

// Converts C-string into String
String      string_from_cstring(const char *str);

// Converts String into a C-string with a null terminator
const char *string_to_cstring(Arena *arena, String str);

bool   string_equals     (String a, String b);
bool   string_starts_with(String str, String search);
bool   string_ends_with  (String str, String search);
String string_slice      (String str, u64 start, u64 end);
String string_concat     (Arena *arena, String a, String b);

// ####################################################################################################################
// File I/O

// Read the entire content of the file requested in file_name and store it into out_file_buffer. Return value indicates success.
bool read_entire_file(Arena *arena, String file_name, Buffer *out_file_buffer);
