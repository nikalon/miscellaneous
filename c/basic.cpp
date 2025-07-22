#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"

// ####################################################################################################################
// Buffer
#define X(type) \
    bool buffer_read_##type(Buffer *buffer, type *out) { \
        bool ok = false; \
        u64 read_bytes = MIN((u64)sizeof(type), buffer->length); \
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

bool buffer_read_count(Buffer *buffer, void *out, u64 count) {
    bool ok = false;

    u64 read_bytes = MIN(count, buffer->length);
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

bool buffer_read_nocopy(Buffer *inBuffer, Buffer *outBuffer, u64 count) {
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

Buffer buffer_slice(Buffer buf, u64 start, u64 end) {
    u64 actual_start = MIN(start, buf.length);
    u64 actual_end = CLAMP(end, actual_start, buf.length);
    u64 length = actual_end - actual_start;
    Buffer ret = {
        buf.data + actual_start,
        length
    };
    return ret;
}

// ####################################################################################################################
// String
String string_from_cstring(const char *str) {
    String ret = {
        (u8*)str,
        (u64)strlen(str)
    };
    return ret;
}

const char *string_to_cstring(Arena *arena, String str) {
    char *ret = arena_push_array(arena, char, str.length + 1);
    memcpy(ret, str.data, str.length);
    ret[str.length] = 0;
    return ret;
}

bool string_equals(String a, String b) {
    bool equals = a.length == b.length && memcmp(a.data, b.data, a.length) == 0;
    return equals;
}

String string_slice(String str, u64 start, u64 end) {
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
        u64 displacement = str.length - search.length;
        u8 *astr = str.data + displacement;
        ok = memcmp(astr, search.data, search.length) == 0;
    }
    return ok;
}

String string_concat(Arena *arena, String a, String b) {
    assert(arena != 0);

    u64 length = a.length + b.length;
    u8 *data = arena_push_array(arena, u8, length);
    memcpy(data, a.data, a.length);
    memcpy(data + a.length, b.data, b.length);

    String ret = {
        .data = data,
        .length = length
    };
    return ret;
}

// ####################################################################################################################
// File I/O
bool read_entire_file(Arena *arena, String file_name, Buffer *out_file_buffer) {
    // @TODO: Use syscalls directly for Windows and POSIX systems. Use the C standard library as fallback.
    assert(arena != 0);
    assert(out_file_buffer != 0);

    bool ok = true;
    const char *file_name_cstr = string_to_cstring(arena, file_name);

    // Read in binary mode because some operating systems like Windows try to convert some characters and we don't want that
    FILE *file = fopen(file_name_cstr, "rb");
    if (file == 0) {
        ok = false;
    }

    u8 *file_buffer = 0;
    u64 file_size = 0;
    if (ok) {
        // Get file size
        fseek(file, 0, SEEK_END);
        file_size = (u64) ftell(file);
        fseek(file, 0, SEEK_SET);

        // Read the entire file into RAM
        file_buffer = arena_push_array(arena, u8, file_size);
        u64 total_read = 0;
        while (total_read < file_size) {
            u64 bytes_read = (u64) fread(file_buffer + total_read, 1, file_size - total_read, file);
            if (bytes_read == 0) {
                ok = false;
                break;
            }

            total_read += bytes_read;
        }
    }

    Buffer out = {
        .data = 0,
        .length = 0
    };
    if (ok) {
        out.data = file_buffer;
        out.length = file_size;
    }
    *out_file_buffer = out;

    if (file != 0) {
        fclose(file);
    }

    return ok;
}
