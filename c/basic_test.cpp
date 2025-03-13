#include <stdio.h>
#include <assert.h>

#define BASIC_IMPLEMENTATION
#include "basic.h"

typedef const char* (*TestFunction)(void);

static const char *test_buffer_read_u8() {
    u8 b[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    Buffer buffer = BUFFER_ARRAY(b);

    u8 counter = 0;
    while (buffer.length > 0) {
        u8 num;
        bool success = buffer_read_type(&buffer, &num);

        assert(success);
        assert(num == counter);

        counter++;
    }

    assert(buffer.length == 0);
    assert(buffer.capacity == 0);

    return __func__;
}

static const char *test_buffer_read_u32() {
    u32 b[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    Buffer buffer = BUFFER_ARRAY(b);

    u32 counter = 0;
    while (buffer.length > 0) {
        u32 num;
        bool success = buffer_read_type(&buffer, &num);

        assert(success);
        assert(num == counter);

        counter++;
    }

    assert(buffer.length == 0);
    assert(buffer.capacity == 0);

    return __func__;
}

static const char *test_buffer_read_with_less_data_left() {
    u8 b[1] = { 42 };
    Buffer buffer = BUFFER_ARRAY(b);

    u64 num = 0;
    bool success = buffer_read_type(&buffer, &num);

    assert(!success);
    assert(num == 42);
    assert(buffer.length == 0);
    assert(buffer.capacity == 0);

    return __func__;
}

static const char *test_buffer_read_try_with_less_data_left() {
    u8 b[1] = { 42 };
    Buffer buffer = BUFFER_ARRAY(b);

    u64 num = 0;
    bool success = buffer_read_type_try(&buffer, &num);

    assert(!success);
    assert(num == 0);
    assert(buffer.length == 0);
    assert(buffer.capacity == 0);

    return __func__;
}

static const char *test_buffer_read_invalid_length() {
    u8 b[1] = { 42 };
    Buffer buffer = BUFFER_ARRAY(b);

    u8 num = 2;
    bool success = buffer_read(&buffer, &num, -1);

    assert(! success);
    assert(num == 2);
    assert(buffer.length == 1);
    assert(buffer.capacity == 1);

    return __func__;
}

static const char *test_string_equality() {
    String a = S("Foo");
    String b = S("Bar");
    String c = S("Fooo");

    assert(! string_equals(a, b));
    assert(! string_equals(a, c));
    assert(string_equals(a, a));

    return __func__;
}

static const char *test_string_slice() {
    String sentence = S("The quick brown fox jumps over the lazy dog");


    // Empty slices with in-bound indices
    String slice = string_slice(sentence, 0, 0);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, 1, 1);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, 5, 5);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, 42, 42);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    // Empty slices with out of bounds indices
    slice = string_slice(sentence, -1, 0);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, -1, -1);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, -2, -1);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, 43, 43);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, 500, 500);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));


    // Slices with length > 0 in-bounds
    slice = string_slice(sentence, 0, 3);
    assert(string_equals(slice, S("The")));

    slice = string_slice(sentence, 1, 3);
    assert(string_equals(slice, S("he")));

    slice = string_slice(sentence, 10, 15);
    assert(string_equals(slice, S("brown")));

    slice = string_slice(sentence, 40, 43);
    assert(string_equals(slice, S("dog")));


    // Slices with length > 0 out of bounds
    slice = string_slice(sentence, -2, 3);
    assert(string_equals(slice, S("The")));

    slice = string_slice(sentence, 40, 88);
    assert(string_equals(slice, S("dog")));


    // Slices with invalid indices
    slice = string_slice(sentence, 40, 10);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, 3, -3);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));
    return __func__;
}

static TestFunction tests[] = {
    test_buffer_read_u8,
    test_buffer_read_u32,
    test_buffer_read_with_less_data_left,
    test_buffer_read_try_with_less_data_left,
    test_buffer_read_invalid_length,
    test_string_equality,
    test_string_slice,
};

int main(void) {
    printf("=== %s ===\n", __FILE__);
    for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
        const char *test_name = tests[i]();
        printf("[OK] %s\n", test_name);
    }

    printf("\n%llu tests passed\n", ARRAY_LENGTH(tests));
    return 0;
}

