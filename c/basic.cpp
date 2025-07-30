#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#    define _WIN32_LEAN_AND_MEAN
#    include <windows.h>
#elif __linux__
#   include <fcntl.h>
#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

#include "basic.h"

// ####################################################################################################################
// Arena
static u8* vm_reserve(uint64_t size) {
#ifdef _WIN32
    u8* memory = (u8*)VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    if (!memory) {
        fprintf(stderr, "Failed to reserve memory for Arena\n");
        abort();
    }
    return memory;
#elif __linux__
    u8* memory = (u8*)mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        fprintf(stderr, "Failed to reserve memory for Arena\n");
        abort();
    }
    return memory;
#else
    #error "Not implemented for your platform"
#endif
}

static void vm_commit_pages(u8 *start, uint64_t size) {
#ifdef _WIN32
    if (!VirtualAlloc(start, size, MEM_COMMIT, PAGE_READWRITE)) {
        fprintf(stderr, "Failed to commit memory pages for Arena\n");
        abort();
    }
#elif __linux__
    if (mprotect(start, size, PROT_READ|PROT_WRITE) == -1) {
        fprintf(stderr, "Failed to commit memory pages for Arena\n");
        abort();
    }
#else
    #error "Not implemented for your platform"
#endif
}

static void vm_free_pages(u8 *start, uint64_t size) {
    // Deallocate memory
#ifdef _WIN32
    (void)size;
    VirtualFree(start, 0, MEM_RELEASE);
#elif __linux__
    munmap(start, size);
#else
    #error "Not implemented for your platform"
#endif
}

static uint64_t get_page_size() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (uint64_t)sysinfo.dwPageSize;
#elif __linux__
    // @NOTE: "Portable applications should employ sysconf(_SC_PAGESIZE) instead of getpagesize()". Source: man 2 getpagesize
    uint64_t page_size = (uint64_t)sysconf(_SC_PAGESIZE);
    return page_size;
#else
    #error "Not implemented for your platform"
#endif
}

Arena arena_alloc(u64 capacity_hint) {
    Arena arena = {};
    arena._page_size = get_page_size();
    arena._capacity = (capacity_hint + (arena._page_size - 1)) & -arena._page_size;
    arena._memory_start = vm_reserve(arena._capacity);
    arena._position = arena._memory_start;
    arena._next_reserved_page = arena._memory_start;

    return arena;
}

void arena_free(Arena *arena) {
    if (arena->_memory_start != 0) {
        vm_free_pages(arena->_memory_start, arena->_capacity);
    }

    Arena zero = {};
    *arena = zero;
}

void *arena_push_data(Arena *arena, u64 type_size, u64 count, u64 alignment, b32 zero_data) {
    assert(arena->_position <= arena->_memory_start + arena->_capacity);

    u64 size = type_size*count;
    assert(size > 0);

    u8 *pos_start = arena->_position;
    u8 *pos_aligned = (u8*)(((u64)pos_start + (alignment - 1)) & -alignment);
    u8 *pos_end = pos_aligned + size;

    u8 arena_ran_out_of_memory = pos_end > arena->_memory_start + arena->_capacity;
    if (arena_ran_out_of_memory) {
        uint64_t total_size = pos_end - pos_start;
        uint64_t padding_size = pos_aligned - pos_start;
        uint64_t memory_left = arena->_capacity - (arena->_position - arena->_memory_start);
        fprintf(
            stderr,
            "Arena ran out of memory. Requested %zu bytes to reserve (%zu bytes for padding + %zu bytes for the data), but arena has only %zu bytes left.\n",
            total_size,
            padding_size,
            size,
            memory_left
        );
        abort();
    }

    // If memory reservation crosses a page boundary it needs to commit as many memory pages as needed
    u8 *next_reserved_page = (u8*)(((u64)pos_end + (arena->_page_size - 1)) & -arena->_page_size);
    if (next_reserved_page > arena->_next_reserved_page) {
        uint64_t size_from_last_page = pos_end - arena->_next_reserved_page;
        vm_commit_pages(arena->_next_reserved_page, size_from_last_page);
        arena->_next_reserved_page = next_reserved_page;
    }

    if (zero_data) {
        memset(pos_aligned, 0, size);
    }

    arena->_position = pos_end;
    return pos_aligned;
}

void *arena_grow_or_realloc(Arena *arena, void *prev_memory, u64 prev_size, u64 new_size) {
    // @TODO:
    UNUSED(arena);
    UNUSED(prev_memory);
    UNUSED(prev_size);
    UNUSED(new_size);
    abort();
}

uint64_t  arena_get_capacity(Arena *arena) {
    return arena->_capacity;
}

uint64_t arena_get_pos(Arena *arena) {
    return arena->_position - arena->_memory_start;
}

void arena_set_pos(Arena *arena, uint64_t pos) {
    assert(pos <= arena->_capacity);
    arena->_position = arena->_memory_start + pos;
}

void arena_clear(Arena *arena) {
    // @TODO: Set a deallocation strategy
    arena->_position = arena->_memory_start;
}

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
    // @NOTE: reference function that is copy-pasted in string_slice()
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
        (const u8*)str,
        (u64)strlen(str)
    };
    return ret;
}

const char *string_to_cstring(Arena *arena, String str) {
    char *ret = arena_push(arena, char, str.length + 1);
    memcpy(ret, str.data, str.length);
    ret[str.length] = 0;
    return ret;
}

bool string_equals(String a, String b) {
    bool equals = a.length == b.length && (a.length == 0 || memcmp(a.data, b.data, a.length) == 0);
    return equals;
}

String string_slice(String str, u64 start, u64 end) {
    // @NOTE: Copy-pasted from buffer_slice()
    u64 actual_start = MIN(start, str.length);
    u64 actual_end = CLAMP(end, actual_start, str.length);
    u64 length = actual_end - actual_start;
    String ret = {
        str.data + actual_start,
        length
    };
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
        data = arena_push(arena, u8, length);
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
        file_buffer = arena_push(arena, u8, file_size);

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
        file_buffer = arena_push(arena, u8, file_size);

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
        file_buffer = arena_push(arena, u8, file_size);
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
