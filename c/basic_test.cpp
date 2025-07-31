#include <string.h>

#include "basic.h"
#include "test_suite.cpp"

static void test_buffer_read_u8(void *context) {
    UNUSED(context);

    u8 b[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    Buffer buffer = BUFFER_FROM_ARRAY(b);

    u8 counter = 0;
    while (buffer.length > 0) {
        u8 num;
        bool success = buffer_read_u8(&buffer, &num);

        EXPECT(success);
        EXPECT(num == counter);

        counter++;
    }

    EXPECT(buffer.length == 0);
}

static void test_buffer_read_u32(void *context) {
    UNUSED(context);

    u32 b[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    Buffer buffer = BUFFER_FROM_ARRAY(b);

    u32 counter = 0;
    while (buffer.length > 0) {
        u32 num;
        bool success = buffer_read_u32(&buffer, &num);

        EXPECT(success);
        EXPECT(num == counter);

        counter++;
    }

    EXPECT(buffer.length == 0);
}

static void test_buffer_read_count_with_less_data_left(void *context) {
    UNUSED(context);

    u8 b[1] = { 42 };
    Buffer buffer = BUFFER_FROM_ARRAY(b);

    u64 num = 4;
    bool success = buffer_read_count(&buffer, &num, sizeof(num));
    //bool success = buffer_read_struct(&buffer, &num);

    EXPECT(!success);
    EXPECT(num == 0);
    EXPECT(buffer.length == 0);
}

static void test_buffer_read_nocopy(void *context) {
    UNUSED(context);

    u8 b[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    Buffer in = BUFFER_FROM_ARRAY(b);

    // Read zero bytes
    Buffer out;
    bool ok = buffer_read_nocopy(&in, &out, 0);
    EXPECT(ok);
    EXPECT(in.length == 8);
    EXPECT(out.length == 0);

    // Read one byte
    ok = buffer_read_nocopy(&in, &out, 1);
    EXPECT(ok);
    EXPECT(in.length == 7);
    EXPECT(out.length == 1);
    EXPECT(out.data[0] == 1);

    // Read 4 bytes
    ok = buffer_read_nocopy(&in, &out, 4);
    EXPECT(ok);
    EXPECT(in.length == 3);
    EXPECT(out.length == 4);
    EXPECT(out.data[0] == 2);
    EXPECT(out.data[1] == 3);
    EXPECT(out.data[2] == 4);
    EXPECT(out.data[3] == 5);

    // Read 8 bytes. It's much more data than what the input buffer has.
    EXPECT(in.length > 0);
    ok = buffer_read_nocopy(&in, &out, 8);
    EXPECT(!ok);
    EXPECT(in.length == 0);
    EXPECT(out.length == 0);
}

static void test_string_from_cstring(void *context) {
    UNUSED(context);

    const char *a = "foo"; // len = 3
    const char *b = "catto"; // len = 5

    String c = string_from_cstring(a);
    String d = string_from_cstring(b);

    EXPECT(c.length == 3);
    EXPECT(d.length == 5);
}

static void test_string_from_cstring_equality(void *context) {
    UNUSED(context);

    const char *a = "foo";
    const char *b = "bar";

    String c = string_from_cstring(a);
    String d = string_from_cstring(b);

    EXPECT(string_equals(c, S("foo")));
    EXPECT(string_equals(d, S("bar")));
}

static void test_string_to_cstring(void *context) {
    Arena *arena = (Arena*)context;

    String a = S("The quick brown fox");
    const char *ret = string_to_cstring(arena, a);
    const char *expected = "The quick brown fox";

    EXPECT(strlen(expected) == 19);
    EXPECT(strncmp(ret, expected, 19) == 0);
}

static void test_string_to_cstring_empty(void *context) {
    Arena *arena = (Arena*)context;

    String a = S("");
    const char *ret = string_to_cstring(arena, a);
    const char *expected = "";

    EXPECT(strlen(expected) == 0);
    EXPECT(strncmp(ret, expected, 0) == 0);
}

static void test_string_equality(void *context) {
    UNUSED(context);

    String a = S("Foo");
    String b = S("Bar");
    String c = S("Fooo");

    EXPECT(! string_equals(a, b));
    EXPECT(! string_equals(a, c));
    EXPECT(string_equals(a, a));
}

static void test_string_slice(void *context) {
    UNUSED(context);

    String sentence = S("The quick brown fox jumps over the lazy dog");


    // Empty slices with in-bound indices
    String slice = string_slice(sentence, 0, 0);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, 1, 1);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, 5, 5);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, 42, 42);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    // Empty slices with out of bounds indices
    slice = string_slice(sentence, -1, 0);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, -1, -1);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, -2, -1);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, 43, 43);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, 500, 500);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));


    // Slices with length > 0 in-bounds
    slice = string_slice(sentence, 0, 3);
    EXPECT(string_equals(slice, S("The")));

    slice = string_slice(sentence, 1, 3);
    EXPECT(string_equals(slice, S("he")));

    slice = string_slice(sentence, 10, 15);
    EXPECT(string_equals(slice, S("brown")));

    slice = string_slice(sentence, 40, 43);
    EXPECT(string_equals(slice, S("dog")));


    // Slices with length > 0 out of bounds
    slice = string_slice(sentence, 0, 3);
    EXPECT(string_equals(slice, S("The")));

    slice = string_slice(sentence, 40, 88);
    EXPECT(string_equals(slice, S("dog")));


    // Slices with invalid indices
    slice = string_slice(sentence, 40, 10);
    EXPECT(slice.length == 0);
    EXPECT(string_equals(slice, S("")));

    slice = string_slice(sentence, 3, 3);
    EXPECT(slice.length == 0);
}

static void test_string_starts_with(void *context) {
    UNUSED(context);

    String sentence = S("The quick brown fox");
    String a = S("The");
    String b = S("The quick");
    String c = S("The quick brown fox");
    String d = S("The quick brown fox j");

    EXPECT(string_starts_with(sentence, a));
    EXPECT(string_starts_with(sentence, b));
    EXPECT(string_starts_with(sentence, c));
    EXPECT(! string_starts_with(sentence, d));
}

static void test_string_ends_with(void *context) {
    UNUSED(context);

    String sentence = S("quick brown fox");
    String a = S("fox");
    String b = S("brown fox");
    String c = S("quick brown fox");
    String d = S("The quick brown fox");

    EXPECT(string_ends_with(sentence, a));
    EXPECT(string_ends_with(sentence, b));
    EXPECT(string_ends_with(sentence, c));
    EXPECT(! string_ends_with(sentence, d));
}

static void test_string_concat(void *context) {
    Arena *arena = (Arena*)context;

    {
        // Strings A and B aren't allocated in the arena
        String a = S("The quick");
        String b = S(" brown fox");

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("The quick brown fox");

        // Check new string was allocated in the arena with at least the necessary memory to store the string.
        EXPECT(arena_get_pos(arena) == arena_original_pos + a.length + b.length);

        // Check string correctness
        EXPECT(ret.length == 19);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A is allocated in the arena and string B isn't allocated in the arena
        const char a_cstr[] = "The quick";
        u8 *a_data = arena_push(arena, u8, ARRAY_LENGTH(a_cstr) - 1);
        memcpy(a_data, a_cstr, ARRAY_LENGTH(a_cstr) - 1);
        String a = { a_data, ARRAY_LENGTH(a_cstr) - 1 };

        String b = S(" brown fox");

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("The quick brown fox");

        // Check new string was allocated in the arena with at least the necessary memory to store the string.
        EXPECT(arena_get_pos(arena) == arena_original_pos + b.length);

        // Check string correctness
        EXPECT(ret.length == 19);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A isn't allocated in the arena and string B is allocated in the arena
        String a = S("The quick");

        const char b_cstr[] = " brown fox";
        u8 *b_data = arena_push(arena, u8, ARRAY_LENGTH(b_cstr) - 1);
        memcpy(b_data, b_cstr, ARRAY_LENGTH(b_cstr) - 1);
        String b = { b_data, ARRAY_LENGTH(b_cstr) - 1 };

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("The quick brown fox");

        // The returned string will be pushed entirely into the arena because string A isn't the last element pushed into
        // the arena, even though strings a and b are most likely to be sequential in memory.
        EXPECT(arena_get_pos(arena) >= arena_original_pos + a.length);

        // Check string correctness
        EXPECT(ret.length == 19);
        EXPECT(string_equals(ret, expected));
    }

    {
        // Strings A and B are both allocated in the arena
        const char a_cstr[] = "The quick";
        u8 *a_data = arena_push(arena, u8, ARRAY_LENGTH(a_cstr) - 1);
        memcpy(a_data, a_cstr, ARRAY_LENGTH(a_cstr) - 1);
        String a = { a_data, ARRAY_LENGTH(a_cstr) - 1 };

        const char b_cstr[] = " brown fox";
        u8 *b_data = arena_push(arena, u8, ARRAY_LENGTH(b_cstr) - 1);
        memcpy(b_data, b_cstr, ARRAY_LENGTH(b_cstr) - 1);
        String b = { b_data, ARRAY_LENGTH(b_cstr) - 1 };

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("The quick brown fox");

        // Check new string was allocated in the arena with at least the necessary memory to store the string.
        EXPECT(arena_get_pos(arena) == arena_original_pos + a.length + b.length);

        // Check string correctness
        EXPECT(ret.length == 19);
        EXPECT(string_equals(ret, expected));
    }
}

static void test_string_concat_empty_strings(void *context) {
    Arena *arena = (Arena*)context;

    {
        // Strings A and B aren't allocated in the arena
        String a = S("");
        String b = S("");
        String ret = string_concat(arena, a, b);
        String expected = S("");
        u64 arena_start_pos = arena_get_pos(arena);

        // Check nothing was allocated in the arena, given that string A and string B are empty. The function should return
        // an empty String object
        EXPECT(arena_get_pos(arena) == arena_start_pos);

        // Check string correctness
        EXPECT(ret.length == 0);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A is allocated in the arena and string B isn't allocated in the arena
        String a = {};
        a.data = arena_push(arena, u8); // Gargabe data. Unused.
        a.length = 0;

        String b = S("");
        String ret = string_concat(arena, a, b);
        String expected = S("");
        u64 arena_start_pos = arena_get_pos(arena);

        // Check nothing was allocated in the arena, given that string A and string B are empty. The function should return
        // an empty String object
        EXPECT(arena_get_pos(arena) == arena_start_pos);

        // Check string correctness
        EXPECT(ret.length == 0);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A isn't allocated in the arena and string B is allocated in the arena
        String a = S("");

        String b = {};
        b.data = arena_push(arena, u8); // Gargabe data. Unused.
        b.length = 0;

        String ret = string_concat(arena, a, b);
        String expected = S("");
        u64 arena_start_pos = arena_get_pos(arena);

        // Check nothing was allocated in the arena, given that string A and string B are empty. The function should return
        // an empty String object
        EXPECT(arena_get_pos(arena) == arena_start_pos);

        // Check string correctness
        EXPECT(ret.length == 0);
        EXPECT(string_equals(ret, expected));
    }

    {
        // Strings A and B are both allocated in the arena
        String a = {};
        a.data = arena_push(arena, u8); // Gargabe data. Unused.
        a.length = 0;

        String b = {};
        b.data = arena_push(arena, u8); // Gargabe data. Unused.
        b.length = 0;

        String ret = string_concat(arena, a, b);
        String expected = S("");
        u64 arena_start_pos = arena_get_pos(arena);

        // Check nothing was allocated in the arena, given that string A and string B are empty. The function should return
        // an empty String object
        EXPECT(arena_get_pos(arena) == arena_start_pos);

        // Check string correctness
        EXPECT(ret.length == 0);
        EXPECT(string_equals(ret, expected));
    }
}

static void test_string_concat_empty_with_something(void *context) {
    Arena *arena = (Arena*)context;

    {
        // Strings A and B aren't allocated in the arena
        String a = S("");
        String b = S("quick brown fox");

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("quick brown fox");

        // Check no memory was allocated, given that string A is zero, so we should get string B directly
        EXPECT(arena_get_pos(arena) == arena_original_pos);

        // Check string correctness
        EXPECT(ret.length == 15);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A is allocated in the arena and B isn't allocated in the arena
        String a = {};
        a.data = (u8*)arena_push(arena, u8); // Garbage data. Unused.
        a.length = 0;

        String b = S("quick brown fox");

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("quick brown fox");

        // Check no memory was allocated, given that string A is zero, so we should get string B directly
        EXPECT(arena_get_pos(arena) == arena_original_pos);

        // Check string correctness
        EXPECT(ret.length == 15);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A isn't allocated in the arena and B is allocated in the arena
        String a = S("");

        const char b_cstr[] = "quick brown fox";
        u8 *b_data = arena_push(arena, u8, ARRAY_LENGTH(b_cstr) - 1);
        memcpy(b_data, b_cstr, ARRAY_LENGTH(b_cstr) - 1);
        String b = { b_data, ARRAY_LENGTH(b_cstr) - 1 };

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("quick brown fox");

        // Check no memory was allocated, given that string A is zero, so we should get string b directly
        EXPECT(arena_get_pos(arena) == arena_original_pos);

        // Check string correctness
        EXPECT(ret.length == 15);
        EXPECT(string_equals(ret, expected));
    }

    {
        // Strings A and B are both allocated in the stack
        String a = {};
        a.data = (u8*)arena_push(arena, u8); // Garbage data. Unused.
        a.length = 0;

        const char b_cstr[] = "quick brown fox";
        u8 *b_data = arena_push(arena, u8, ARRAY_LENGTH(b_cstr) - 1);
        memcpy(b_data, b_cstr, ARRAY_LENGTH(b_cstr) - 1);
        String b = { b_data, ARRAY_LENGTH(b_cstr) - 1 };

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("quick brown fox");

        // Check no memory was allocated, given that string A is zero, so we should get string b directly
        EXPECT(arena_get_pos(arena) == arena_original_pos);

        // Check string correctness
        EXPECT(ret.length == 15);
        EXPECT(string_equals(ret, expected));
    }
}

static void test_string_concat_something_with_empty(void *context) {
    Arena *arena = (Arena*)context;

    {
        // Strings A and B aren't allocated in the arena
        String a = S("The quick");
        String b = S("");

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("The quick");

        // Check no memory was allocated, given that string B is zero, so we should get string A directly
        EXPECT(arena_get_pos(arena) == arena_original_pos);

        // Check string correctness
        EXPECT(ret.length == 9);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A is allocated in the stack and B isn't allocated in the arena
        const char a_cstr[] = "The quick";
        u8 *a_data = arena_push(arena, u8, ARRAY_LENGTH(a_cstr) - 1);
        memcpy(a_data, a_cstr, ARRAY_LENGTH(a_cstr) - 1);
        String a = { a_data, ARRAY_LENGTH(a_cstr) - 1 };

        String b = S("");

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("The quick");

        // Check no memory was allocated, given that string B is zero, so we should get string A directly
        EXPECT(arena_get_pos(arena) == arena_original_pos);

        // Check string correctness
        EXPECT(ret.length == 9);
        EXPECT(string_equals(ret, expected));
    }

    {
        // String A is allocated in the stack and B isn't allocated in the arena
        const char a_cstr[] = "The quick";
        u8 *a_data = arena_push(arena, u8, ARRAY_LENGTH(a_cstr) - 1);
        memcpy(a_data, a_cstr, ARRAY_LENGTH(a_cstr) - 1);
        String a = { a_data, ARRAY_LENGTH(a_cstr) - 1 };

        String b = {};
        b.data = arena_push(arena, u8); // Garbage data. Unused.
        b.length = 0;

        u64 arena_original_pos = arena_get_pos(arena);
        String ret = string_concat(arena, a, b);
        String expected = S("The quick");

        // Check no memory was allocated, given that string b is zero, so we should get string A directly
        EXPECT(arena_get_pos(arena) == arena_original_pos);

        // Check string correctness
        EXPECT(ret.length == 9);
        EXPECT(string_equals(ret, expected));
    }
}

static void test_read_entire_file(void *context) {
    Arena *arena = (Arena*)context;

    u64 file_size = 44;
    u64 arena_pos_before_reading_file = arena_get_pos(arena);

    // @NOTE: The file test_file.txt must exist and must contain exactly the following sentence:
    String expected = S("The quick brown fox jumps over the lazy dog.");

    Buffer file_buffer = {};
    bool ret = read_entire_file(arena, S("test_file.txt"), &file_buffer);

    u64 areana_pos_after_reading_file = arena_get_pos(arena);
    String file_buffer_str = BUFFER_TO_STRING(file_buffer);
    EXPECT(ret);
    EXPECT(file_buffer.length == file_size);
    EXPECT(string_equals(file_buffer_str, expected));

    // read_entire_file() should've allocated at least the necessary memory to store the file in memory
    EXPECT(areana_pos_after_reading_file > arena_pos_before_reading_file);
    EXPECT(areana_pos_after_reading_file - arena_pos_before_reading_file >= file_size);
}

static void test_read_entire_file_does_not_exist(void *context) {
    Arena *arena = (Arena*)context;

    // @NOTE: The file test_file_does_not_exist.txt must not exist in the workspace!
    u64 arena_pos_before_reading_file = arena_get_pos(arena);

    Buffer file_buffer = {};
    bool ret = read_entire_file(arena, S("test_file_does_not_exist.txt"), &file_buffer);

    EXPECT(ret == false);
    EXPECT(file_buffer.length == 0);

    // If read_entire_file() fails it should deallocate any memory used
    u64 arena_pos_after_reading_file = arena_get_pos(arena);
    EXPECT(arena_pos_after_reading_file == arena_pos_before_reading_file);
}

static void test_dynamic_array_add_elements_of_one_byte(void *context) {
    Arena *arena = (Arena*)context;

    DynamicArray da = da_new<b8>(arena);
    EXPECT(da.length == 0);
    EXPECT(da.capacity >= da.length);

    // Fill dynamic array with a increment counter starting at 0
    for (u64 i = 0; i < 120; i++) {
        da_add(&da, (b8)i);
        EXPECT(da.length == i + 1);
        EXPECT(da.capacity >= da.length);
    }

    // Check dynamic array actually stored all elements correctly
    for (u64 i = 0; i < 120; i++) {
        EXPECT(da[i] == (b8)i);
    }
}

static void test_dynamic_array_add_elements_of_four_bytes(void *context) {
    Arena *arena = (Arena*)context;

    DynamicArray da = da_new<u32>(arena);
    EXPECT(da.length == 0);
    EXPECT(da.capacity >= da.length);

    // Fill dynamic array with a increment counter starting at 0
    for (u64 i = 0; i < 1024; i++) {
        da_add(&da, (u32)i);
        EXPECT(da.length == i + 1);
        EXPECT(da.capacity >= da.length);
    }

    // Check dynamic array actually stored all elements correctly
    for (u64 i = 0; i < 1024; i++) {
        EXPECT(da[i] == (u32)i);
    }
}

static void test_dynamic_array_remove_elements_until_empty(void *context) {
    Arena *arena = (Arena*)context;

    // Fill dynamic array with data
    DynamicArray da = da_new<u32>(arena);
    da_add(&da, (u32)10);
    da_add(&da, (u32)20);
    da_add(&da, (u32)30);
    da_add(&da, (u32)40);
    EXPECT(da.length == 4);
    EXPECT(da.capacity >= da.length);

    // Remove a value in the middle of the array
    da_remove(&da, 1);
    EXPECT(da.length == 3);
    EXPECT(da.capacity >= da.length);
    EXPECT(da[0] == 10);
    EXPECT(da[1] == 30);
    EXPECT(da[2] == 40);

    // Remove last element
    da_remove(&da, 2);
    EXPECT(da.length == 2);
    EXPECT(da.capacity >= da.length);
    EXPECT(da[0] == 10);
    EXPECT(da[1] == 30);

    // Remove first element
    da_remove(&da, 0);
    EXPECT(da.length == 1);
    EXPECT(da.capacity >= da.length);
    EXPECT(da[1] == 30);

    // Remove remaining element
    da_remove(&da, 0);
    EXPECT(da.length == 0);
    EXPECT(da.capacity >= da.length);

    // Keep removing elements in the empty dynamic array
    for (u64 i = 0; i < 10000; i++) {
        da_remove(&da, 0);
    }
}

static void test_dynamic_array_try_remove_invalid_indices(void* context) {
    Arena *arena = (Arena*)context;

    // Fill dynamic array with data
    DynamicArray da = da_new<u32>(arena);
    da_add(&da, (u32)10);
    da_add(&da, (u32)20);
    da_add(&da, (u32)30);
    da_add(&da, (u32)40);
    EXPECT(da.length == 4);
    EXPECT(da.capacity >= da.length);

    // Try removing elements with invalid indices
    assert(da.length < 500);
    for (u64 i = 500; i < 10000; i++) {
        da_remove(&da, i);
    }

    // Dynamic array should remain the same
    EXPECT(da.length == 4)
    EXPECT(da.capacity >= da.length);
    EXPECT(da[0] == 10);
    EXPECT(da[1] == 20);
    EXPECT(da[2] == 30);
    EXPECT(da[3] == 40);
}

static void do_before_every_test_handler(void *context) {
    Arena *arena = (Arena*)context;
    arena_clear(arena);
}

int main(void) {
    TestSuite suite = test_suite_new(__FILE__);

    // Arena used for every individual test
    Arena arena_test = arena_alloc((u64)4*GiB);
    test_suite_set_context(&suite, &arena_test);
    test_suite_do_before_every_test(&suite, do_before_every_test_handler);

    TEST(&suite, test_buffer_read_u8);
    TEST(&suite, test_buffer_read_u32);
    TEST(&suite, test_buffer_read_count_with_less_data_left);
    TEST(&suite, test_buffer_read_nocopy);
    TEST(&suite, test_string_from_cstring);
    TEST(&suite, test_string_from_cstring_equality);
    TEST(&suite, test_string_to_cstring);
    TEST(&suite, test_string_to_cstring_empty);
    TEST(&suite, test_string_equality);
    TEST(&suite, test_string_slice);
    TEST(&suite, test_string_starts_with);
    TEST(&suite, test_string_ends_with);
    TEST(&suite, test_string_concat);
    TEST(&suite, test_string_concat_empty_strings);
    TEST(&suite, test_string_concat_empty_with_something);
    TEST(&suite, test_string_concat_something_with_empty);
    TEST(&suite, test_read_entire_file);
    TEST(&suite, test_read_entire_file_does_not_exist);
    TEST(&suite, test_dynamic_array_add_elements_of_one_byte);
    TEST(&suite, test_dynamic_array_add_elements_of_four_bytes);
    TEST(&suite, test_dynamic_array_remove_elements_until_empty);
    TEST(&suite, test_dynamic_array_try_remove_invalid_indices);

    int errcode = test_suite_run_all_and_print(&suite);
    arena_free(&arena_test);

    return errcode;
}
