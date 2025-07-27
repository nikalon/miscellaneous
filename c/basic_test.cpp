#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#elif __linux__
#include <unistd.h>
#endif

#include "basic.h"

#define TEST(function_name) (Test){ #function_name, function_name }

typedef struct {
    const char *test_name;
    void (*test_function)(Arena*);
} Test;

static void test_buffer_read_u8(Arena *arena) {
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
}

static void test_buffer_read_u32(Arena *arena) {
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
}

static void test_buffer_read_count_with_less_data_left(Arena *arena) {
    UNUSED(arena);

    u8 b[1] = { 42 };
    Buffer buffer = BUFFER_FROM_ARRAY(b);

    u64 num = 4;
    bool success = buffer_read_count(&buffer, &num, sizeof(num));
    //bool success = buffer_read_struct(&buffer, &num);

    assert(!success);
    assert(num == 0);
    assert(buffer.length == 0);
}

static void test_buffer_read_nocopy(Arena *arena) {
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
}

static void test_string_from_cstring(Arena *arena) {
    UNUSED(arena);

    const char *a = "foo"; // len = 3
    const char *b = "catto"; // len = 5

    String c = string_from_cstring(a);
    String d = string_from_cstring(b);

    assert(c.length == 3);
    assert(d.length == 5);
}

static void test_string_from_cstring_equality(Arena *arena) {
    UNUSED(arena);

    const char *a = "foo";
    const char *b = "bar";

    String c = string_from_cstring(a);
    String d = string_from_cstring(b);

    assert(string_equals(c, S("foo")));
    assert(string_equals(d, S("bar")));
}

static void test_string_to_cstring(Arena *arena) {
    String a = S("The quick brown fox");
    const char *ret = string_to_cstring(arena, a);
    const char *expected = "The quick brown fox";

    assert(strlen(expected) == 19);
    assert(strncmp(ret, expected, 19) == 0);
}

static void test_string_to_cstring_empty(Arena *arena) {
    String a = S("");
    const char *ret = string_to_cstring(arena, a);
    const char *expected = "";

    assert(strlen(expected) == 0);
    assert(strncmp(ret, expected, 0) == 0);
}

static void test_string_equality(Arena *arena) {
    UNUSED(arena);

    String a = S("Foo");
    String b = S("Bar");
    String c = S("Fooo");

    assert(! string_equals(a, b));
    assert(! string_equals(a, c));
    assert(string_equals(a, a));
}

static void test_string_slice(Arena *arena) {
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
}

static void test_string_starts_with(Arena *arena) {
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
}

static void test_string_ends_with(Arena *arena) {
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
}

static void test_string_concat(Arena *arena) {
    String a = S("The quick");
    String b = S(" brown fox");
    String ret = string_concat(arena, a, b);
    String expected = S("The quick brown fox");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 19);
    assert(string_equals(ret, expected));
}

static void test_string_concat_empty_strings(Arena *arena) {
    String a = S("");
    String b = S("");
    String ret = string_concat(arena, a, b);
    String expected = S("");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 0);
    assert(string_equals(ret, expected));
}

static void test_string_concat_empty_with_something(Arena *arena) {
    String a = S("");
    String b = S("quick brown fox");
    String ret = string_concat(arena, a, b);
    String expected = S("quick brown fox");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 15);
    assert(string_equals(ret, expected));
}

static void test_string_concat_something_with_empty(Arena *arena) {
    String a = S("The quick");
    String b = S("");
    String ret = string_concat(arena, a, b);
    String expected = S("The quick");

    // Check new string was allocated with enough memory
    assert(arena_get_pos(arena) == expected.length);

    // Check string correctness
    assert(ret.length == 9);
    assert(string_equals(ret, expected));
}

static void test_read_entire_file(Arena *arena) {
    u64 file_size = 44;
    u64 arena_pos_before_reading_file = arena_get_pos(arena);

    // @NOTE: The file test_file.txt must exist and must contain exactly the following sentence:
    String expected = S("The quick brown fox jumps over the lazy dog.");

    Buffer file_buffer = {};
    bool ret = read_entire_file(arena, S("test_file.txt"), &file_buffer);

    u64 areana_pos_after_reading_file = arena_get_pos(arena);
    String file_buffer_str = BUFFER_TO_STRING(file_buffer);
    assert(ret);
    assert(file_buffer.length == file_size);
    assert(string_equals(file_buffer_str, expected));

    // read_entire_file() should've allocated at least the necessary memory to store the file in memory
    assert(areana_pos_after_reading_file > arena_pos_before_reading_file);
    assert(areana_pos_after_reading_file - arena_pos_before_reading_file >= file_size);
}

static void test_read_entire_file_does_not_exist(Arena *arena) {
    // @NOTE: The file test_file_does_not_exist.txt must not exist in the workspace!
    u64 arena_pos_before_reading_file = arena_get_pos(arena);

    Buffer file_buffer = {};
    bool ret = read_entire_file(arena, S("test_file_does_not_exist.txt"), &file_buffer);

    assert(ret == false);
    assert(file_buffer.length == 0);

    // If read_entire_file() fails it should deallocate any memory used
    u64 arena_pos_after_reading_file = arena_get_pos(arena);
    assert(arena_pos_after_reading_file == arena_pos_before_reading_file);
}

static Test tests[] = {
    TEST(test_buffer_read_u8),
    TEST(test_buffer_read_u32),
    TEST(test_buffer_read_count_with_less_data_left),
    TEST(test_buffer_read_nocopy),
    TEST(test_string_from_cstring),
    TEST(test_string_from_cstring_equality),
    TEST(test_string_to_cstring),
    TEST(test_string_to_cstring_empty),
    TEST(test_string_equality),
    TEST(test_string_slice),
    TEST(test_string_starts_with),
    TEST(test_string_ends_with),
    TEST(test_string_concat),
    TEST(test_string_concat_empty_strings),
    TEST(test_string_concat_empty_with_something),
    TEST(test_string_concat_something_with_empty),
    TEST(test_read_entire_file),
    TEST(test_read_entire_file_does_not_exist),
};

static jmp_buf jump_buffer;

void test_abort_handler(int signal) {
    if (signal == SIGABRT) {
        // We receive this signal when a condition hasn't been met in an assertion. This signal can also be sent from other
        // sources but for simplicity we consider every SIGABRT signal as sent from abort() when its condition fails.
        longjmp(jump_buffer, 1);
    }
}

b32 can_print_colors() {
    // @TODO: Implement complete logic defined in https://bixense.com/clicolors/
    const int __STDOUT = 1;

#ifdef _WIN32
    return _isatty(__STDOUT);
#elif __linux__
    return isatty(__STDOUT);
#else
    return false;
#endif
}

int main(void) {
    // Arena used for every individual test
    Arena arena_test = arena_alloc();

    u64 tests_passed = 0;
    signal(SIGABRT, test_abort_handler);
    printf("=== %s ===\n", __FILE__);

    for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
        assert(arena_get_pos(&arena_test) == 0 && "arena_test must be completely empty before starting a new test");
        const char *test_result = 0;
        const char *test_name = tests[i].test_name;

        if (setjmp(jump_buffer) == 0) {
            tests[i].test_function(&arena_test);

            // If execution reaches this point it means that no SIGABRT signal has been raised during the test, meaning
            // that all assertions passed. This test is considered as passed.
            tests_passed += 1;
            test_result = can_print_colors() ? "\x1b[1;32mOK\x1b[0m" : "OK";
        } else {
            // If execution reaches this point it means that a SIGABRT signal has been raised, most likely by an assertion.
            // This test is considered as failed.
            test_result = can_print_colors() ? "\x1b[1;31mKO\x1b[0m" : "KO";
        }

        printf("[%s] %s\n", test_result, test_name);
        arena_clear(&arena_test);
    }

    arena_free(&arena_test);
    printf("\n%zu/%zu tests passed\n", tests_passed, ARRAY_LENGTH(tests));

    int errcode = 0;
    if (tests_passed != ARRAY_LENGTH(tests)) {
        errcode = 1;
    }
    return errcode;
}
