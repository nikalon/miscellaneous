#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #define _WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif __linux__
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

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
    String ret = BUFFER_TO_STRING(buffer_slice(STRING_TO_BUFFER(str), start, end));
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
        const u8 *astr = str.data + displacement;
        ok = memcmp(astr, search.data, search.length) == 0;
    }
    return ok;
}

String string_concat(Arena *arena, String a, String b) {
    assert(arena != 0);

    u64 length = a.length + b.length;
    u8 *data = 0;
    if (length > 0) {
        data = arena_push_array(arena, u8, length);
        memcpy(data, a.data, a.length);
        memcpy(data + a.length, b.data, b.length);
    }

    String ret = {
        .data = data,
        .length = length
    };
    return ret;
}

// ####################################################################################################################
// File I/O
bool read_entire_file(Arena *arena, String file_name, Buffer *out_file_buffer) {
    assert(arena != 0);
    assert(out_file_buffer != 0);

    u64 arena_original_pos = arena_get_pos(arena);
    const char *file_name_cstr = string_to_cstring(arena, file_name);

    bool ok = true;
    u8 *file_buffer = 0;
    u64 file_size = 0;

#if _WIN32
    // @TODO: Enable longer paths in Windows
    // @NOTE: According to the Microsoft documentation "[OpenFile] has limited capabilities and is not recommended. For
    // new application development, use the CreateFile function."
    // Source (24/07/2025): https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-openfile?source=recommendations
    //
    // With CreateFile() we have also the benefit of supporting paths longer than MAX_PATH characters imposed by older
    // Windows editions.
    HANDLE fd = CreateFileA(
        file_name_cstr,
        GENERIC_READ,           // Desired access
        FILE_SHARE_READ,        // Allow other processes to read the file while we have the file opened
        NULL,                   // The file descriptor cannot be shared with child processes
        OPEN_EXISTING,          // Open existing file only if it exists
        FILE_ATTRIBUTE_NORMAL,  // Normal file attributes and I/O operations will be synchronous
        NULL                    // No attribute template
    );
    if (fd == INVALID_HANDLE_VALUE) {
        ok = false;
    }

    // Get file size
    LARGE_INTEGER li_file_size = {};
    if (ok) {
        // @NOTE: Microsoft documentation recommends to use GetFileSizeEx() instead of GetFileSize()
        // Source (24/07/2025): https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize
        if (GetFileSizeEx(fd, &li_file_size) == 0) {
            ok = false;
        }
    }

    // Read the entire file into RAM
    if (ok) {
        file_size = (u64)li_file_size.QuadPart;
        file_buffer = arena_push_array(arena, u8, file_size);

        u64 total_read = 0;
        while (total_read < file_size) {
            DWORD bytes_read = 0;
            BOOL read_ok = ReadFile(
                fd,                         // File descriptor
                file_buffer + total_read,   // Output buffer
                file_size - total_read,     // Bytes to read
                &bytes_read,                // Pointer to number of bytes read
                NULL
            );
            if (!read_ok) {
                ok = false;
                break;
            }

            total_read += bytes_read;
        }
    }

    if (fd != INVALID_HANDLE_VALUE) {
        CloseHandle(fd);
    }

#elif __linux__
    int fd = open(file_name_cstr, O_RDONLY);
    if (fd == -1) {
        ok = false;
    }

    // Get file size
    struct stat st = {};
    if (ok) {
        if (fstat(fd, &st) == -1) {
            ok = false;
        }
    }

    // Read the entire file into RAM
    if (ok) {
        file_size = (u64)st.st_size;
        file_buffer = arena_push_array(arena, u8, file_size);

        u64 total_read = 0;
        while (total_read < file_size) {
            u64 bytes_read = (u64)read(fd, file_buffer + total_read, file_size - total_read);
            if (bytes_read == 0) {
                ok = false;
                break;
            }

            total_read += bytes_read;
        }
    }

    if (fd != -1) {
        close(fd);
    }

#else
    // Fallback implementation with C-standard library

    // Read in binary mode because some operating systems like Windows try to convert some characters and we don't want that
    FILE *file = fopen(file_name_cstr, "rb");
    if (file == 0) {
        ok = false;
    }

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

    if (file != 0) {
        fclose(file);
    }

#endif

    if (ok) {
        out_file_buffer->data = file_buffer;
        out_file_buffer->length = file_size;
    } else {
        // Failed to read file. Deallocate any memory used.
        arena_set_pos(arena, arena_original_pos);
    }

    return ok;
}
