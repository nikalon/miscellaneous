#pragma once

/*
 * Common utilities that can be used in any project. It includes:
 *  - Primitive types
 *  - Buffers
 *  - Strings
 *  - Basic file I/O
 *
 * Tests are defined in `basic_test.cpp`.
 * */

#include <stdint.h>

#include "arena.h"

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

#define STRING_TO_BUFFER(string) Buffer{ .data = (u8*) string.data, .length = string.length }
#define BUFFER_TO_STRING(buffer) String{ .data = (const u8*) buffer.data, .length = buffer.length }

// ####################################################################################################################
// Buffer
typedef struct {
    u8  *data;
    u64 length;
} Buffer;

#define BUFFER_FROM_ARRAY(array) Buffer{ (u8*)array, sizeof(array) }

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

#define S(string) String{ (u8*)string, ARRAY_LENGTH(string)-1 }

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
