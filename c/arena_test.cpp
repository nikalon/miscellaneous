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
#include "test_suite.cpp"

#define ARENA_CAPACITY 1*GiB
#define FILL_ARRAY_WITH_GARBAGE(array, length) memset(array, 255, length);

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

static void test_push_for_primitive_types(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);
    u8  *a = arena_push(&arena, u8);
    *a = 255;

    u16 *b = arena_push(&arena, u16);
    *b = 1111;

    u32 *c = arena_push(&arena, u32);
    *c = 123456;

    u64 *d = arena_push(&arena, u64);
    *d = 1234567890;

    EXPECT(*a == 255);
    EXPECT(*b == 1111);
    EXPECT(*c == 123456);
    EXPECT(*d == 1234567890);

    arena_free(&arena);
}

static void test_push_and_clear_for_primitive_types(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);
    u8  *a = arena_push(&arena, u8);
    *a = 255;

    u16 *b = arena_push(&arena, u16);
    *b = 1111;

    u32 *c = arena_push(&arena, u32);
    *c = 123456;

    u64 *d = arena_push(&arena, u64);
    *d = 1234567890;

    EXPECT(*a == 255);
    EXPECT(*b == 1111);
    EXPECT(*c == 123456);
    EXPECT(*d == 1234567890);

    // Clear all memory and starts pushing new values into the arena
    arena_clear(&arena);

    u8  *e = arena_push(&arena, u8);
    *a = 55;

    u16 *f = arena_push(&arena, u16);
    *b = 3412;

    u32 *g = arena_push(&arena, u32);
    *c = 37489;

    u64 *h = arena_push(&arena, u64);
    *d = 4739204;

    EXPECT(*e == 55);
    EXPECT(*f == 3412);
    EXPECT(*g == 37489);
    EXPECT(*h == 4739204);

    arena_free(&arena);
}

static void test_reserve_arrays(void *context) {
    UNUSED(context);
    size_t array_size = 4096;

    Arena arena = arena_alloc(ARENA_CAPACITY);
    u8 *a = arena_push(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(a, array_size);
    a[0] = 100;
    a[array_size - 1] = 250;

    u8 *b = arena_push(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(b, array_size);
    b[0] = 101;;
    b[array_size - 1] = 251;

    u8 *c = arena_push(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(c, array_size);
    c[0] = 102;
    c[array_size - 1] = 252;

    u8 *d = arena_push(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(d, array_size);
    d[0] = 103;
    d[array_size - 1] = 253;

    EXPECT(a[0] == 100);
    EXPECT(a[array_size - 1] == 250);
    EXPECT(b[0] == 101);
    EXPECT(b[array_size - 1] == 251);
    EXPECT(c[0] == 102);
    EXPECT(c[array_size - 1] == 252);
    EXPECT(d[0] == 103);
    EXPECT(d[array_size - 1] == 253);

    arena_free(&arena);
}

void test_reserve_primitive_types_and_arrays(void *context) {
    UNUSED(context);
    size_t array_size = 4096;

    Arena arena = arena_alloc(ARENA_CAPACITY);
    u8  *a = arena_push(&arena, u8);
    *a = 255;

    u64 *b = arena_push(&arena, u64, array_size);
    FILL_ARRAY_WITH_GARBAGE(b, array_size);
    b[0] = 100;
    b[array_size - 1] = 250;

    u16 *c = arena_push(&arena, u16);
    *c = 1111;

    u32 *d = arena_push(&arena, u32, array_size);
    FILL_ARRAY_WITH_GARBAGE(d, array_size);
    d[0] = 101;
    d[array_size - 1] = 251;

    u32 *e = arena_push(&arena, u32);
    *e = 123456;

    u16 *f = arena_push(&arena, u16, array_size);
    FILL_ARRAY_WITH_GARBAGE(f, array_size);
    f[0] = 102;
    f[array_size - 1] = 252;

    u64 *g = arena_push(&arena, u64);
    *g = 1234567890;

    u8 *h = arena_push(&arena, u8, array_size);
    FILL_ARRAY_WITH_GARBAGE(h, array_size);
    h[0] = 103;
    h[array_size - 1] = 253;

    EXPECT(*a == 255);

    EXPECT(b[0] == 100);
    EXPECT(b[array_size - 1] == 250);

    EXPECT(*c == 1111);

    EXPECT(d[0] == 101);
    EXPECT(d[array_size - 1] == 251);

    EXPECT(*e == 123456);

    EXPECT(f[0] == 102);
    EXPECT(f[array_size - 1] == 252);

    EXPECT(*g == 1234567890);

    EXPECT(h[0] == 103);
    EXPECT(h[array_size - 1] == 253);


    arena_free(&arena);
}

static void  test_reserve_arrays_of_page_size_until_maximum_capacity(void *context) {
    UNUSED(context);
    u64 page_size = get_page_size();
    Arena arena = arena_alloc(ARENA_CAPACITY);

    assert(arena_get_capacity(&arena) % page_size == 0);
    size_t max_pages_to_commit = arena_get_capacity(&arena) / page_size;

    for (size_t count = 0; count < 10; count++) {
        for (size_t i = 0; i < max_pages_to_commit; i++) {
            u8 *array = arena_push(&arena, u8, page_size);
            FILL_ARRAY_WITH_GARBAGE(array, page_size);
        }
        arena_clear(&arena);
    }

    arena_free(&arena);
}

static void  test_arena_capacity_limit(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);

    u8 *one_gigabyte;
    for (size_t i = 0; i < 10; i++) {
        one_gigabyte = arena_push(&arena, u8, arena_get_capacity(&arena));
        FILL_ARRAY_WITH_GARBAGE(one_gigabyte, arena_get_capacity(&arena));
        arena_clear(&arena);
    }

    arena_free(&arena);
}

static void test_push_must_be_zero(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);

    for (u64 i = 0; i < 50; i++) {
        u8 *a = arena_push(&arena, u8);
        EXPECT(*a == 0);

        u16 *b = arena_push(&arena, u16);
        EXPECT(*b == 0);

        u32 *c = arena_push(&arena, u32);
        EXPECT(*c == 0);

        u64 *d = arena_push(&arena, u64);
        EXPECT(*d == 0);

        size_t array_size = 5000;
        u8 *array = arena_push(&arena, u8, array_size);

        // Write garbage values
        *a = 102;
        *b = 10343;
        *c = 3478243;
        *d = 6738914983;
        for (size_t j = 0; j < array_size; j++) {
            array[i] = (u8)(i*3428 + 12);
        }

        arena_clear(&arena);
    }

    arena_clear(&arena);

    for (u64 i = 0; i < 50; i++) {
        u8 *a = arena_push(&arena, u8);
        EXPECT(*a == 0);

        u16 *b = arena_push(&arena, u16);
        EXPECT(*b == 0);

        u32 *c = arena_push(&arena, u32);
        EXPECT(*c == 0);

        u64 *d = arena_push(&arena, u64);
        EXPECT(*d == 0);

        size_t array_size = 5000;
        u8 *array = arena_push(&arena, u8, array_size);
        for (size_t j = 0; j < array_size; j++) {
            EXPECT(array[i] == 0);
        }

        arena_clear(&arena);
    }

    arena_free(&arena);
}

static void  test_arena_free_invalidates_instance(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);
    arena_free(&arena);

    Arena arena_zero = {};

    // Check arena_free() zeroes out original instance
    EXPECT(memcmp(&arena, &arena_zero, sizeof(Arena)) == 0);
}

static void  test_get_set_position_and_clear(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);

    // Nothing pushed yet to the arena. We let the initial position be anything, not necessarily zero to provide some
    // flexibility in the implementation.
    u64 pos_start = arena_get_pos(&arena);

    arena_push(&arena, u8);
    u64 pos_after_initial_push = arena_get_pos(&arena);
    EXPECT(pos_after_initial_push > pos_start);

    arena_push_nozero(&arena, u32, 1024);
    u64 pos_after_pushing_array = arena_get_pos(&arena);
    EXPECT(pos_after_pushing_array > pos_start);
    EXPECT(pos_after_pushing_array > pos_after_initial_push);

    // Test set position to pos_after_initial_push
    arena_set_pos(&arena, pos_after_initial_push);
    EXPECT(arena_get_pos(&arena) == pos_after_initial_push);

    // When we clear the arena we want the exact same position as the recently-allocated arena
    arena_clear(&arena);
    u64 pos_last = arena_get_pos(&arena);
    EXPECT(pos_last == pos_start);

    arena_free(&arena);
}

static void test_grow_buffer_in_place(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);

    {
        // Buffer is the first thing pused into the arena
        u8 *buffer = arena_push(&arena, u8, 100);
        u8 *buffer_original = buffer;

        buffer = arena_grow_in_place_or_realloc(&arena, u8, buffer, 100, 200);
        EXPECT(buffer == buffer_original);
    }

    {
        // Push other random stuff in the arena
        arena_push(&arena, u64, 20);
        arena_push(&arena, b32, 43);
        arena_push(&arena, i8, 541);
    }

    {
        // Buffer is the last thing pused into the arena
        u8 *buffer = arena_push(&arena, u8, 100);
        u8 *buffer_original = buffer;

        buffer = arena_grow_in_place_or_realloc(&arena, u8, buffer, 100, 200);
        EXPECT(buffer == buffer_original);
    }

    arena_free(&arena);
}

static void test_reallocate_buffer_instead_of_growing(void *context) {
    UNUSED(context);
    Arena arena = arena_alloc(ARENA_CAPACITY);

    // First push an array that we will try to grow later and fill it with something that will be checked later
    u64 *buffer = arena_push(&arena, u64, 100);
    u64 *buffer_original = buffer;
    for (u64 i = 0; i < 100; i++) {
        buffer[i] = 0xBABA;
    }

    // Push random stuff in the arena, so that the previous buffer is no longer the last element pushed into the arena
    u16 expected_canary = 0xCACA;
    u16 *canary = arena_push(&arena, u16);
    *canary = expected_canary;

    // Now if we try to grow the buffer it should reallocate it entirely because it hasn't enough space left to grow
    buffer = arena_grow_in_place_or_realloc(&arena, u64, buffer, 100, 200);
    EXPECT(buffer > buffer_original);

    // Check original data has been cloned
    for (u64 i = 0; i < 100; i++) {
        EXPECT(buffer[i] == 0xBABA);
    }

    // Write random data into the buffer to check that we are really sure it has reallocated the buffer
    for (u64 i = 0; i < 200; i++) {
        buffer[i] = 0xFF;
    }

    EXPECT(*canary == expected_canary);
    arena_free(&arena);
}

int main(void) {
    TestSuite suite = test_suite_new(__FILE__);
    TEST(&suite, test_push_for_primitive_types);
    TEST(&suite, test_push_and_clear_for_primitive_types);
    TEST(&suite, test_push_must_be_zero);
    TEST(&suite, test_reserve_arrays);
    TEST(&suite, test_reserve_primitive_types_and_arrays);
    TEST(&suite, test_arena_capacity_limit);
    TEST(&suite, test_reserve_arrays_of_page_size_until_maximum_capacity);
    TEST(&suite, test_arena_free_invalidates_instance);
    TEST(&suite, test_get_set_position_and_clear);
    TEST(&suite, test_grow_buffer_in_place);
    TEST(&suite, test_reallocate_buffer_instead_of_growing);

    int errcode = test_suite_run_all_and_print(&suite);
    return errcode;
}
