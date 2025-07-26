#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #define _WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif __linux__
    #include <sys/mman.h>
    #include <unistd.h>
#endif

#include "arena.h"

static uint8_t* vm_reserve(uint64_t size) {
#ifdef _WIN32
    uint8_t* memory = (uint8_t*)VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    if (!memory) {
        fprintf(stderr, "Failed to reserve memory for Arena\n");
        abort();
    }
    return memory;
#elif __linux__
    uint8_t* memory = (uint8_t*)mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        fprintf(stderr, "Failed to reserve memory for Arena\n");
        abort();
    }
    return memory;
#else
    #error "Not implemented for your platform"
#endif
}

static void vm_commit_pages(uint8_t *start, uint64_t size) {
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

static void vm_free_pages(uint8_t *start, uint64_t size) {
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

inline static uint8_t *round_up_mult_pow2(uint8_t *n, uint64_t pow2) {
    // Round up to the next multiple of a power of 2. Formula taken from the book Hacker's Delight (2nd edition, chapter 3-1)
    // @NOTE: the following formulas always return 0 if n == 0. That could cause some unexpected bugs if used incorrectly.
    uint64_t x = (uint64_t)n;
    return (uint8_t*)((x + (pow2 - 1)) & -pow2);

    // Alternative formula:
    //return (uint8_t*)(x + (-x & (pow2 - 1)));
}

Arena arena_alloc() {
    Arena arena = {};
    arena._capacity = 1073741824; // 1 GiB
    arena._memory_start = vm_reserve(arena._capacity);
    arena._position = arena._memory_start;
    arena._next_reserved_page = arena._memory_start;
    arena._page_size = get_page_size();

    return arena;
}

void arena_free(Arena *arena) {
    if (arena->_memory_start != 0) {
        vm_free_pages(arena->_memory_start, arena->_capacity);
    }

    Arena zero = {};
    *arena = zero;
}

void *arena_push(Arena *arena, uint64_t size) {
    void *memory = arena_push_nozero(arena, size);
    memset(memory, 0, size);
    return memory;
}

void *arena_push_nozero(Arena *arena, uint64_t size) {
    assert(size > 0);
    assert(arena->_position <= arena->_memory_start + arena->_capacity);

    uint8_t *pos_start = arena->_position;
    uint8_t *pos_aligned = pos_start; // @TODO: align memory
    uint8_t *pos_end = pos_aligned + size;

    uint8_t arena_ran_out_of_memory = pos_end > arena->_memory_start + arena->_capacity;
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
    uint8_t *next_reserved_page = round_up_mult_pow2(pos_end, arena->_page_size);
    if (next_reserved_page > arena->_next_reserved_page) {
        uint64_t size = pos_end - arena->_next_reserved_page;
        vm_commit_pages(arena->_next_reserved_page, size);
        arena->_next_reserved_page = next_reserved_page;
    }

    arena->_position = pos_end;
    return pos_aligned;
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
