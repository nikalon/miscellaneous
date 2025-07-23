// @TODO: Print failed tests instead of aborting the program

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "basic.h"

typedef const char* (*TestFunction)(Arena*);

static const char *test_buffer_read_u8(Arena *arena) {
    UNUSED(arena);

    u8 b[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    Buffer buffer = BUFFER_FROM_ARRAY(b);

    u8 counter = 0;
    while (buffer.length > 0) {
        u8 num;
        bool success = buffer_read_u8(&buffer, &num);

        assert(success);
        assert(num == counter);

        counter++;
    }

    assert(buffer.length == 0);

    return __func__;
}

static const char *test_buffer_read_u32(Arena *arena) {
    UNUSED(arena);

    u32 b[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    Buffer buffer = BUFFER_FROM_ARRAY(b);

    u32 counter = 0;
    while (buffer.length > 0) {
        u32 num;
        bool success = buffer_read_u32(&buffer, &num);

        assert(success);
        assert(num == counter);

        counter++;
    }

    assert(buffer.length == 0);

    return __func__;
}

static const char *test_buffer_read_count_with_less_data_left(Arena *arena) {
    UNUSED(arena);

    u8 b[1] = { 42 };
    Buffer buffer = BUFFER_FROM_ARRAY(b);

    u64 num = 4;
    bool success = buffer_read_count(&buffer, &num, sizeof(num));
    //bool success = buffer_read_struct(&buffer, &num);

    assert(!success);
    assert(num == 0);
    assert(buffer.length == 0);

    return __func__;
}

static const char *test_buffer_read_nocopy(Arena *arena) {
    UNUSED(arena);

    u8 b[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    Buffer in = BUFFER_FROM_ARRAY(b);

    // Read zero bytes
    Buffer out;
    bool ok = buffer_read_nocopy(&in, &out, 0);
    assert(ok);
    assert(in.length == 8);
    assert(out.length == 0);

    // Read one byte
    ok = buffer_read_nocopy(&in, &out, 1);
    assert(ok);
    assert(in.length == 7);
    assert(out.length == 1);
    assert(out.data[0] == 1);

    // Read 4 bytes
    ok = buffer_read_nocopy(&in, &out, 4);
    assert(ok);
    assert(in.length == 3);
    assert(out.length == 4);
    assert(out.data[0] == 2);
    assert(out.data[1] == 3);
    assert(out.data[2] == 4);
    assert(out.data[3] == 5);

    // Read 8 bytes. It's much more data than what the input buffer has.
    assert(in.length > 0);
    ok = buffer_read_nocopy(&in, &out, 8);
    assert(!ok);
    assert(in.length == 0);
    assert(out.length == 0);

    return __func__;
}

static const char *test_string_from_cstring(Arena *arena) {
    UNUSED(arena);

    const char *a = "foo"; // len = 3
    const char *b = "catto"; // len = 5

    String c = string_from_cstring(a);
    String d = string_from_cstring(b);

    assert(c.length == 3);
    assert(d.length == 5);

    return __func__;
}

static const char *test_string_from_cstring_equality(Arena *arena) {
    UNUSED(arena);

    const char *a = "foo";
    const char *b = "bar";

    String c = string_from_cstring(a);
    String d = string_from_cstring(b);

    assert(string_equals(c, S("foo")));
    assert(string_equals(d, S("bar")));

    return __func__;
}

static const char *test_string_to_cstring(Arena *arena) {
    String a = S("The quick brown fox");
    const char *ret = string_to_cstring(arena, a);
    const char *expected = "The quick brown fox";

    assert(strlen(expected) == 19);
    assert(strncmp(ret, expected, 19) == 0);

    return __func__;
}

static const char *test_string_to_cstring_empty(Arena *arena) {
    String a = S("");
    const char *ret = string_to_cstring(arena, a);
    const char *expected = "";

    assert(strlen(expected) == 0);
    assert(strncmp(ret, expected, 0) == 0);

    return __func__;
}

static const char *test_string_equality(Arena *arena) {
    UNUSED(arena);

    String a = S("Foo");
    String b = S("Bar");
    String c = S("Fooo");

    assert(! string_equals(a, b));
    assert(! string_equals(a, c));
    assert(string_equals(a, a));

    return __func__;
}

static const char *test_string_slice(Arena *arena) {
    UNUSED(arena);

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
    slice = string_slice(sentence, 0, 3);
    assert(string_equals(slice, S("The")));

    slice = string_slice(sentence, 40, 88);
    assert(string_equals(slice, S("dog")));


    // Slices with invalid indices
    slice = string_slice(sentence, 40, 10);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));

    slice = string_slice(sentence, 3, 3);
    assert(slice.length == 0);
    assert(string_equals(slice, S("")));
    return __func__;
}

static const char *test_string_starts_with(Arena *arena) {
    UNUSED(arena);

    String sentence = S("The quick brown fox");
    String a = S("The");
    String b = S("The quick");
    String c = S("The quick brown fox");
    String d = S("The quick brown fox j");

    assert(string_starts_with(sentence, a));
    assert(string_starts_with(sentence, b));
    assert(string_starts_with(sentence, c));
    assert(! string_starts_with(sentence, d));

    return __func__;
}

static const char *test_string_ends_with(Arena *arena) {
    UNUSED(arena);

    String sentence = S("quick brown fox");
    String a = S("fox");
    String b = S("brown fox");
    String c = S("quick brown fox");
    String d = S("The quick brown fox");

    assert(string_ends_with(sentence, a));
    assert(string_ends_with(sentence, b));
    assert(string_ends_with(sentence, c));
    assert(! string_ends_with(sentence, d));

    return __func__;
}

static const char *test_string_concat(Arena *arena) {
    String a = S("The quick");
    String b = S(" brown fox");
    String ret = string_concat(arena, a, b);
    String expected = S("The quick brown fox");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 19);
    assert(string_equals(ret, expected));

    return __func__;
}

static const char *test_string_concat_empty_strings(Arena *arena) {
    String a = S("");
    String b = S("");
    String ret = string_concat(arena, a, b);
    String expected = S("");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 0);
    assert(string_equals(ret, expected));

    return __func__;
}

static const char *test_string_concat_empty_with_something(Arena *arena) {
    String a = S("");
    String b = S("quick brown fox");
    String ret = string_concat(arena, a, b);
    String expected = S("quick brown fox");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 15);
    assert(string_equals(ret, expected));

    return __func__;
}

static const char *test_string_concat_something_with_empty(Arena *arena) {
    String a = S("The quick");
    String b = S("");
    String ret = string_concat(arena, a, b);
    String expected = S("The quick");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 9);
    assert(string_equals(ret, expected));

    return __func__;
}

static const char *test_read_entire_file(Arena *arena) {
    u64 file_size = 44;
    // @NOTE: The file test_file.txt must exist and must contain exactly the following sentence:
    String expected = S("The quick brown fox jumps over the lazy dog.");

    Buffer file_buffer = {};
    bool ret = read_entire_file(arena, S("test_file.txt"), &file_buffer);

    String file_buffer_str = BUFFER_TO_STRING(file_buffer);
    assert(ret);
    assert(file_buffer.length == file_size);
    assert(string_equals(file_buffer_str, expected));

    return __func__;
}

static const char *test_read_entire_file_does_not_exist(Arena *arena) {
    // @NOTE: The file test_file_does_not_exist.txt must not exist in the workspace!
    u64 file_size = 44;

    Buffer file_buffer = {};
    bool ret = read_entire_file(arena, S("test_file_does_not_exist.txt"), &file_buffer);

    assert(ret == false);
    assert(file_buffer.length == 0);

    return __func__;
}

static TestFunction tests[] = {
    test_buffer_read_u8,
    test_buffer_read_u32,
    test_buffer_read_count_with_less_data_left,
    test_buffer_read_nocopy,
    test_string_from_cstring,
    test_string_from_cstring_equality,
    test_string_to_cstring,
    test_string_to_cstring_empty,
    test_string_equality,
    test_string_slice,
    test_string_starts_with,
    test_string_ends_with,
    test_string_concat,
    test_string_concat_empty_strings,
    test_string_concat_empty_with_something,
    test_string_concat_something_with_empty,
    test_read_entire_file,
    test_read_entire_file_does_not_exist,
};

int main(void) {
    // Arena used for every individual test
    Arena arena_test = arena_alloc();

    printf("=== %s ===\n", __FILE__);
    for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
        assert(arena_get_pos(&arena_test) == 0 && "arena_test must be completely empty before starting a new test");

        const char *test_name = tests[i](&arena_test);
        printf("[OK] %s\n", test_name);
        arena_clear(&arena_test);
    }

    arena_free(&arena_test);
    printf("\n%llu tests passed\n", ARRAY_LENGTH(tests));
    return 0;
}
