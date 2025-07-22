// @TODO: Print failed tests instead of aborting the program

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "basic.h"
#include "arena.h"

typedef const char* (*TestFunction)(void);

#define FILL_ARRAY_WITH_GARBAGE(array, length) memset(array, 255, length);

static inline size_t ceil_div(size_t a, size_t b) {
    // Only defined for positive numbers
    return (a + (b - 1)) / b;
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
    Arena arena = arena_alloc();
    size_t max_pages_to_commit = ceil_div(arena.capacity, arena.page_size);

    for (size_t count = 0; count < 10; count++) {
        for (size_t i = 0; i < max_pages_to_commit; i++) {
            u8 *array = arena_push_array(&arena, u8, arena.page_size);
            FILL_ARRAY_WITH_GARBAGE(array, arena.page_size);
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
        one_gigabyte = arena_push_array(&arena, u8, arena.capacity);
        FILL_ARRAY_WITH_GARBAGE(one_gigabyte, arena.capacity);
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

static TestFunction tests[] = {
    test_push_for_primitive_types,
    test_push_and_clear_for_primitive_types,
    test_push_must_be_zero,
    test_reserve_arrays,
    test_reserve_primitive_types_and_arrays,
    test_arena_capacity_limit,
    test_reserve_arrays_of_page_size_until_maximum_capacity,
};

int main(void) {
    printf("=== %s ===\n", __FILE__);
    for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
        const char *test_name = tests[i]();
        printf("[OK] %s\n", test_name);
    }

    printf("\n%llu tests passed\n\n", ARRAY_LENGTH(tests));
    return 0;
}
