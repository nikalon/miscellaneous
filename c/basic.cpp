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
