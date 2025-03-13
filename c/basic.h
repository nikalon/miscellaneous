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

// ####################################################################################################################
// Buffer
typedef struct {
    u8  *data;
    i64 length;

    // We store a capacity for writing operations, otherwise we wouldn't need it at all. This value can be ignored in
    // most use cases. Read length instead.
    i64 capacity;
} Buffer;

#define BUFFER_ARRAY(array) Buffer{ (u8*)array, ARRAY_LENGTH(array), ARRAY_LENGTH(array) }

Buffer buffer_slice(Buffer buf, i64 start, i64 end);

// Read
bool buffer_read    (Buffer *buffer, void *out, i64 count);
bool buffer_read_try(Buffer *buffer, void *out, i64 count);
#define buffer_read_type(buffer, out)     buffer_read(buffer, out, sizeof(*out))
#define buffer_read_type_try(buffer, out) buffer_read_try(buffer, out, sizeof(*out))

// Write
//bool buffer_write        (Buffer *buffer, void *value, i64 count);
//bool buffer_write_or_zero(Buffer *buffer, void *value, i64 count);
//#define buffer_write_type(buffer, value)         buffer_write(buffer, sizeof(value), value)
//#define buffer_write_type_or_zero(buffer, value) buffer_write_or_zero(buffer, sizeof(value), value)


// ####################################################################################################################
// String
typedef struct {
    u8  *data;
    i64 length;
} String;

#define S(string) String{ (u8*)string, ARRAY_LENGTH(string)-1 }
#define CSTR(string) String{ (u8*)string, strlen(string) }

// Comparison
bool string_equals(String a, String b);

// Search
//bool string_starts_with(String str, String search);
//bool string_ends_with(String str, String search);
//i64  string_find(String str, String search);

// Slices
String string_slice(String str, i64 start, i64 end);
//String string_trim(String str);
//String string_trim_left(String str);
//String string_trim_right(String str);

//String *string_concat(String a, String var...);
//String *string_join(String a, String separator, String var...);

#endif



#ifdef BASIC_IMPLEMENTATION
#undef BASIC_IMPLEMENTATION
// ####################################################################################################################
// Buffer
bool buffer_read(Buffer *buffer, void *out, i64 count) {
    i64 read_bytes = CLAMP(count, 0, buffer->length);
    memcpy(out, buffer->data, read_bytes);

    buffer->data += read_bytes;
    buffer->length -= read_bytes;
    buffer->capacity -= read_bytes;

    return read_bytes == count;
}

bool buffer_read_try(Buffer *buffer, void *out, i64 count) {
    bool success = false;

    i64 read_bytes = CLAMP(count, 0, buffer->length);
    if (read_bytes == count) {
        memcpy(out, buffer->data, read_bytes);
        success = true;
    } else {
        memset(out, 0, read_bytes);
    }

    buffer->data += read_bytes;
    buffer->length -= read_bytes;
    buffer->capacity -= read_bytes;

    return success;
}

Buffer buffer_slice(Buffer buf, i64 start, i64 end) {
    i64 actual_start = CLAMP(start, 0, buf.length);
    i64 actual_end = CLAMP(end, actual_start, buf.length);
    i64 length = actual_end - actual_start;
    Buffer ret = {
        buf.data,
        length,
        length
    };
    return ret;
}

// ####################################################################################################################
// String
bool string_equals(String a, String b) {
    bool equals = a.length == b.length && memcmp(a.data, b.data, a.length) == 0;
    return equals;
}

String string_slice(String str, i64 start, i64 end) {
    i64 actual_start = CLAMP(start, 0, str.length);
    i64 actual_end = CLAMP(end, actual_start, str.length);
    i64 length = actual_end - actual_start;
    //printf("len = %llu, start = %llu, end = %llu\n", str.length, actual_start, actual_end);
    String ret = {
        str.data + actual_start,
        length
    };
    return ret;
}
#endif
