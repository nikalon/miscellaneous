#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "arena.h"

#define ARENA_MIN_COMMITTED_PAGES 2

static void arena_commit_num_pages(Arena *arena, i64 num_pages);
static inline i64 ceil_div(i64 a, i64 b);
static u8*  vm_reserve(i64 size);
static void vm_free(u8* memory, i64 size);
static void vm_commit_pages(u8* start_memory, i64 length);
static int  get_page_size();

Arena arena_alloc() {
    Arena arena = {};
    arena.capacity = 1*GiB;
    arena.memory = vm_reserve(arena.capacity);
    arena.page_size = get_page_size();

    // Start by reserving this amount of memory pages
    arena_commit_num_pages(&arena, ARENA_MIN_COMMITTED_PAGES);

    return arena;
}

void arena_free(Arena *arena) {
    if (arena->memory != 0) vm_free(arena->memory, arena->capacity);

    memset(arena, 0, sizeof(*arena));
}

void *arena_push(Arena *arena, i64 size) {
    void *memory = arena_push_nozero(arena, size);
    memset(memory, 0, size);
    return memory;
}

void *arena_push_nozero(Arena *arena, i64 size) {
    assert(arena->num_committed_pages >= ARENA_MIN_COMMITTED_PAGES);
    assert(arena->position <= arena->capacity);

    // Check we have enough space to reserve
    i64 memory_left = arena->capacity - arena->position;
    if (size > memory_left) {
        fprintf(stderr, "Arena ran out of memory. Requested %llu bytes to reserve, but arena has only %llu bytes left.\n", size, memory_left);
        abort();
    }

    // Commit as many memory pages as needed
    i64 arena_pos_after_push = arena->position + size;
    i64 required_pages_for_push = ceil_div(arena_pos_after_push, arena->page_size);
    if (required_pages_for_push > arena->num_committed_pages) {
        i64 pages_to_commit = required_pages_for_push - arena->num_committed_pages;
        arena_commit_num_pages(arena, pages_to_commit);
    }

    // @TODO: Align memory
    void *addr = arena->memory + arena->position;
    arena->position += size;

    return addr;
}

i64 arena_get_pos(Arena *arena) {
    return arena->position;
}

void arena_set_pos(Arena *arena, i64 pos) {
    assert(pos <= arena->capacity);
    arena->position = pos;
}

void arena_pop(Arena *arena, i64 size) {
    // @TODO: Set a deallocation strategy
    assert(size <= arena->position);
    arena->position -= size;
}

void arena_clear(Arena *arena) {
    // Uncommit all pages except the reserved ones
    /*i64 reserved_pages_size = ARENA_MIN_COMMITTED_PAGES * arena->page_size;
    u8 *start_memory = arena->memory + reserved_pages_size;
    i64 len = arena->capacity - reserved_pages_size;
    if (mprotect(start_memory, len, PROT_NONE) == -1) {
        fprintf(stderr, "Failed to uncommit memory pages for Arena\n");
        abort();
    }*/

    // @TODO: Set a deallocation strategy
    arena->position = 0;
}

static void arena_commit_num_pages(Arena *arena, i64 num_pages) {
    i64 total_pages = ceil_div(arena->capacity, arena->page_size);
    i64 unmapped_pages = total_pages - arena->num_committed_pages;
    i64 pages_to_commit = MIN(num_pages, unmapped_pages);

    // Commit requested pages
    u8 *start_memory = arena->memory + (arena->num_committed_pages * arena->page_size);
    i64 len = pages_to_commit * arena->page_size;
    vm_commit_pages(start_memory, len);

    arena->num_committed_pages += pages_to_commit;
    assert(arena->num_committed_pages <= total_pages);
}

static inline i64 ceil_div(i64 a, i64 b) {
    // Only defined for positive numbers
    return (a + (b - 1)) / b;
}

static u8* vm_reserve(i64 size) {
    // @TODO: Print extended information if memory reservation fails

#ifdef _WIN32
    u8* memory = (u8*) VirtualAlloc(
        0,
        size,
        MEM_RESERVE,
        PAGE_READWRITE
    );
    if (!memory) {
        fprintf(stderr, "Failed to reserve memory for Arena\n");
        abort();
    }
    return memory;
#else
    u8* memory = (u8*) mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        fprintf(stderr, "Failed to reserve memory for Arena\n");
        abort();
    }
    return memory;
#endif
}

static void vm_free(u8* memory, i64 size) {
#ifdef _WIN32
    UNUSED(size);
    VirtualFree(memory, 0, MEM_RELEASE);
#else
    munmap(memory, size);
#endif
}

static void vm_commit_pages(u8* start_memory, i64 length) {
#ifdef _WIN32
    if (!VirtualAlloc(start_memory, length, MEM_COMMIT, PAGE_READWRITE)) {
        fprintf(stderr, "Failed to commit memory pages for Arena\n");
        abort();
    }
#else
    if (mprotect(start_memory, length, PROT_READ|PROT_WRITE) == -1) {
        fprintf(stderr, "Failed to commit memory pages for Arena\n");
        abort();
    }
#endif
}

static int get_page_size() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwAllocationGranularity;
#else
    int page_size = getpagesize();
    return page_size;
#endif
}
