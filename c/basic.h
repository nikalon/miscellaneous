#pragma once

/*
 * Common utilities that can be used in any project. It includes:
 *  - Primitive types
 *  - Buffers
 *  - Strings
 *
 * Tests are defined in `basic_test.cpp`.
 * */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define CLAMP(val, min, max) (MIN(MAX(val, min), max))
#define ABS(val) (val >= 0 ? val : -val)

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
typedef Buffer String;

#define S(string) String{ (u8*)string, ARRAY_LENGTH(string)-1 }
String string_from_cstring(const char *str);

bool   string_equals     (String a, String b);
bool   string_starts_with(String str, String search);
bool   string_ends_with  (String str, String search);
String string_slice      (String str, u64 start, u64 end);
