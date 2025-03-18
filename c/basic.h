/*
 * Common utilities that can be used in any project. This is a single file that contains definitions and implementation
 * at the style of STB libraries.
 *
 * Here's how you're supposed to use this file:
 * - Copy this file into your project
 * - In your main file include this file in the following way. Only do this once in your project or you will get linking errors:
 *      #define BASIC_IMPLEMENTATION
 *      #include "basic.h"
 *  - In any other project file include this file as normal:
 *      #include "basic.h"
 *
 * Tests are defined in `basic_test.cpp` file. To run the tests compile that file and run `basic_test`. It's a single file
 * which only depends on the C standard library.
 * */

#ifndef __BASIC_HEADER_H__
#define __BASIC_HEADER_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ####################################################################################################################
// Primitive types
//
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
    i64 length;
} Buffer;

#define BUFFER_ARRAY(array) Buffer{ (u8*)array, ARRAY_LENGTH(array) }

Buffer buffer_slice(Buffer buf, i64 start, i64 end);

// Read
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

bool buffer_read(Buffer *buffer, void *out, i64 count);
#define buffer_read_struct(buffer, out) buffer_read(buffer, out, sizeof(*out))
bool buffer_read_nocopy(Buffer *inBuffer, Buffer *outBuffer, i64 count);

// If there's not enough bytes to read it reads zero first and then tries to read as much bytes as possible before consuming the buffer.

// ####################################################################################################################
// String
typedef Buffer String;

#define S(string) String{ (u8*)string, ARRAY_LENGTH(string)-1 }
String string_cstring(const char *str);

bool string_equals(String a, String b);
bool string_starts_with(String str, String search);
bool string_ends_with(String str, String search);
String string_slice(String str, i64 start, i64 end);

#endif



#ifdef BASIC_IMPLEMENTATION
#undef BASIC_IMPLEMENTATION
// ####################################################################################################################
// Buffer
#define X(type) \
    bool buffer_read_##type(Buffer *buffer, type *out) { \
        bool ok = false; \
        i64 read_bytes = MIN((i64)sizeof(type), buffer->length); \
        if (read_bytes == sizeof(type)) { \
            *out = *((type*)buffer->data); \
            ok = true; \
        } else { \
            *out = 0; \
        } \
        buffer->data += read_bytes; \
        buffer->length -= read_bytes; \
        return ok; \
    }
BUFFER_READ_FUNCTIONS
#undef X

bool buffer_read(Buffer *buffer, void *out, i64 count) {
    bool ok = false;

    i64 read_bytes = CLAMP(count, 0, buffer->length);
    if (read_bytes == count) {
        memcpy(out, buffer->data, read_bytes);
        ok = true;
    } else {
        memset(out, 0, read_bytes);
    }

    buffer->data += read_bytes;
    buffer->length -= read_bytes;

    return ok;
}

bool buffer_read_nocopy(Buffer *inBuffer, Buffer *outBuffer, i64 count) {
    bool ok = false;
    outBuffer->data = inBuffer->data;

    if (inBuffer->length >= count) {
        outBuffer->length = count;

        inBuffer->data += count;
        inBuffer->length -= count;

        ok = true;
    } else {
        // Not enough bytes to read. Consume input buffer anyway.
        inBuffer->length = 0;
        outBuffer->length = 0;
    }

    return ok;
}

Buffer buffer_slice(Buffer buf, i64 start, i64 end) {
    i64 actual_start = CLAMP(start, 0, buf.length);
    i64 actual_end = CLAMP(end, actual_start, buf.length);
    i64 length = actual_end - actual_start;
    Buffer ret = {
        buf.data + actual_start,
        length
    };
    return ret;
}

// ####################################################################################################################
// String
String string_cstring(const char *str) {
    String ret = {
        (u8*)str,
        (i64)strlen(str)
    };
    return ret;
}

bool string_equals(String a, String b) {
    bool equals = a.length == b.length && memcmp(a.data, b.data, a.length) == 0;
    return equals;
}

String string_slice(String str, i64 start, i64 end) {
    String ret = (String)buffer_slice((Buffer)str, start, end);
    return ret;
}

bool string_starts_with(String str, String search) {
    bool ok = str.length >= search.length && memcmp(str.data, search.data, search.length) == 0;
    return ok;
}

bool string_ends_with(String str, String search) {
    bool ok = false;
    if (str.length >= search.length) {
        i64 displacement = str.length - search.length;
        u8 *astr = str.data + displacement;
        ok = memcmp(astr, search.data, search.length) == 0;
    }
    return ok;
}
#endif
