// @TODO: Print failed tests instead of aborting the program

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #define _WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif __linux__
    #include <sys/mman.h>
    #include <unistd.h>
#endif

#include "basic.h"
#include "arena.h"

typedef const char* (*TestFunction)(void);

#define FILL_ARRAY_WITH_GARBAGE(array, length) memset(array, 255, length);

static inline size_t ceil_div(size_t a, size_t b) {
    // Only defined for positive numbers
    return (a + (b - 1)) / b;
}

static u64 get_page_size() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (u64)sysinfo.dwAllocationGranularity;
#elif __linux__
    // @NOTE: "Portable applications should employ sysconf(_SC_PAGESIZE) instead of getpagesize()". Source: man 2 getpagesize
    u64 page_size = (u64)sysconf(_SC_PAGESIZE);
    return page_size;
#else
    #error "Not implemented for your platform"
#endif
}

static const char *test_push_for_primitive_types() {
    Arena arena = arena_alloc();
    u8  *a = arena_push_type(&arena, u8);
    *a = 255;

    u16 *b = arena_push_type(&arena, u16);
    *b = 1111;

    u32 *c = arena_push_type(&arena, u32);
    *c = 123456;

    u64 *d = arena_push_type(&arena, u64);
    *d = 1234567890;

    assert(*a == 255);
    assert(*b == 1111);
    assert(*c == 123456);
    assert(*d == 1234567890);

    arena_free(&arena);
    return __func__;
}

static const char *test_push_and_clear_for_primitive_types() {
    Arena arena = arena_alloc();
    u8  *a = arena_push_type(&arena, u8);
    *a = 255;

    u16 *b = arena_push_type(&arena, u16);
    *b = 1111;

    u32 *c = arena_push_type(&arena, u32);
    *c = 123456;

    u64 *d = arena_push_type(&arena, u64);
    *d = 1234567890;

    assert(*a == 255);
    assert(*b == 1111);
    assert(*c == 123456);
    assert(*d == 1234567890);

    // Clear all memory and starts pushing new values into the arena
    arena_clear(&arena);

    u8  *e = arena_push_type(&arena, u8);
    *a = 55;

    u16 *f = arena_push_type(&arena, u16);
    *b = 3412;

    u32 *g = arena_push_type(&arena, u32);
    *c = 37489;

    u64 *h = arena_push_type(&arena, u64);
    *d = 4739204;

    assert(*e == 55);
    assert(*f == 3412);
    assert(*g == 37489);
    assert(*h == 4739204);

    arena_free(&arena);
    return __func__;
}

static const char *test_reserve_arrays() {
    size_t array_size = 4096;

    Arena arena = arena_alloc();
    u8 *a = arena_push_array(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(a, array_size);
    a[0] = 100;
    a[array_size - 1] = 250;

    u8 *b = arena_push_array(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(b, array_size);
    b[0] = 101;;
    b[array_size - 1] = 251;

    u8 *c = arena_push_array(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(c, array_size);
    c[0] = 102;
    c[array_size - 1] = 252;

    u8 *d = arena_push_array(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(d, array_size);
    d[0] = 103;
    d[array_size - 1] = 253;

    assert(a[0] == 100);
    assert(a[array_size - 1] == 250);
    assert(b[0] == 101);
    assert(b[array_size - 1] == 251);
    assert(c[0] == 102);
    assert(c[array_size - 1] == 252);
    assert(d[0] == 103);
    assert(d[array_size - 1] == 253);

    arena_free(&arena);
    return __func__;
}

const char* test_reserve_primitive_types_and_arrays() {
    size_t array_size = 4096;

    Arena arena = arena_alloc();
    u8  *a = arena_push_type(&arena, u8);
    *a = 255;

    u64 *b = arena_push_array(&arena, u64, array_size);
    FILL_ARRAY_WITH_GARBAGE(b, array_size);
    b[0] = 100;
    b[array_size - 1] = 250;

    u16 *c = arena_push_type(&arena, u16);
    *c = 1111;

    u32 *d = arena_push_array(&arena, u32, array_size);
    FILL_ARRAY_WITH_GARBAGE(d, array_size);
    d[0] = 101;
    d[array_size - 1] = 251;

    u32 *e = arena_push_type(&arena, u32);
    *e = 123456;

    u16 *f = arena_push_array(&arena, u16, array_size);
    FILL_ARRAY_WITH_GARBAGE(f, array_size);
    f[0] = 102;
    f[array_size - 1] = 252;

    u64 *g = arena_push_type(&arena, u64);
    *g = 1234567890;

    u8 *h = arena_push_array(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(h, array_size);
    h[0] = 103;
    h[array_size - 1] = 253;

    assert(*a == 255);

    assert(b[0] == 100);
    assert(b[array_size - 1] == 250);

    assert(*c == 1111);

    assert(d[0] == 101);
    assert(d[array_size - 1] == 251);

    assert(*e == 123456);

    assert(f[0] == 102);
    assert(f[array_size - 1] == 252);

    assert(*g == 1234567890);

    assert(h[0] == 103);
    assert(h[array_size - 1] == 253);


    arena_free(&arena);
    return __func__;
}

static const char* test_reserve_arrays_of_page_size_until_maximum_capacity() {
    u64 page_size = get_page_size();
    Arena arena = arena_alloc();
    size_t max_pages_to_commit = ceil_div(arena_get_capacity(&arena), page_size);

    for (size_t count = 0; count < 10; count++) {
        for (size_t i = 0; i < max_pages_to_commit; i++) {
            u8 *array = arena_push_array(&arena, u8, page_size);
            FILL_ARRAY_WITH_GARBAGE(array, page_size);
        }
        arena_clear(&arena);
    }

    arena_free(&arena);
    return __func__;
}

static const char* test_arena_capacity_limit() {
    Arena arena = arena_alloc();

    u8 *one_gigabyte;
    for (size_t i = 0; i < 10; i++) {
        one_gigabyte = arena_push_array(&arena, u8, arena_get_capacity(&arena));
        FILL_ARRAY_WITH_GARBAGE(one_gigabyte, arena_get_capacity(&arena));
        arena_clear(&arena);
    }

    arena_free(&arena);
    return __func__;
}

static const char* test_push_must_be_zero() {
    Arena arena = arena_alloc();

    {
        u8 *a = arena_push_type(&arena, u8);
        assert(*a == 0);

        u16 *b = arena_push_type(&arena, u16);
        assert(*b == 0);

        u32 *c = arena_push_type(&arena, u32);
        assert(*c == 0);

        u64 *d = arena_push_type(&arena, u64);
        assert(*d == 0);

        size_t array_size = 5000;
        u8 *array = arena_push_array(&arena, u8, array_size);

        // Write garbage values
        *a = 102;
        *b = 10343;
        *c = 3478243;
        *d = 6738914983;
        for (size_t i = 0; i < array_size; i++) {
            array[i] = (u8)(i*3428 + 12);
        }
    }

    arena_clear(&arena);

    {
        u8 *a = arena_push_type(&arena, u8);
        assert(*a == 0);

        u16 *b = arena_push_type(&arena, u16);
        assert(*b == 0);

        u32 *c = arena_push_type(&arena, u32);
        assert(*c == 0);

        u64 *d = arena_push_type(&arena, u64);
        assert(*d == 0);

        size_t array_size = 5000;
        u8 *array = arena_push_array(&arena, u8, array_size);
        for (size_t i = 0; i < array_size; i++) {
            assert(array[i] == 0);
        }
    }

    arena_free(&arena);
    return __func__;
}

static const char* test_arena_free_invalidates_instance() {
    Arena arena = arena_alloc();
    arena_free(&arena);

    Arena arena_zero = {};

    // Check arena_free() zeroes out original instance
    assert(memcmp(&arena, &arena_zero, sizeof(Arena)) == 0);

    return __func__;
}

static const char* test_get_set_position_and_clear() {
    Arena arena = arena_alloc();

    // Nothing pushed yet to the arena. We let the initial position be anything, not necessarily zero to provide some
    // flexibility in the implementation.
    u64 pos_start = arena_get_pos(&arena);

    arena_push_type(&arena, u8);
    u64 pos_after_initial_push = arena_get_pos(&arena);
    assert(pos_after_initial_push > pos_start);

    arena_push_array_nozero(&arena, u32, 1024);
    u64 pos_after_pushing_array = arena_get_pos(&arena);
    assert(pos_after_pushing_array > pos_start);
    assert(pos_after_pushing_array > pos_after_initial_push);

    // Test set position to pos_after_initial_push
    arena_set_pos(&arena, pos_after_initial_push);
    assert(arena_get_pos(&arena) == pos_after_initial_push);

    // When we clear the arena we want the exact same position as the recently-allocated arena
    arena_clear(&arena);
    u64 pos_last = arena_get_pos(&arena);
    assert(pos_last == pos_start);

    arena_free(&arena);
    return __func__;
}

static TestFunction tests[] = {
    test_push_for_primitive_types,
    test_push_and_clear_for_primitive_types,
    test_push_must_be_zero,
    test_reserve_arrays,
    test_reserve_primitive_types_and_arrays,
    test_arena_capacity_limit,
    test_reserve_arrays_of_page_size_until_maximum_capacity,
    test_arena_free_invalidates_instance,
    test_get_set_position_and_clear,
};

int main(void) {
    printf("=== %s ===\n", __FILE__);
    for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
        const char *test_name = tests[i]();
        printf("[OK] %s\n", test_name);
    }

    printf("\n%lu tests passed\n\n", ARRAY_LENGTH(tests));
    return 0;
}
