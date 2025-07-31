#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#    define _WIN32_LEAN_AND_MEAN
#    include <io.h>
#endif

#define TEST_SUITE_MAX_TESTS 2048
#define TEST(suite_ptr, test_func_name) test_suite_add_test(suite_ptr, #test_func_name, test_func_name)

#define EXPECT(condition) if (!(condition)) raise(SIGABRT);
//#define EXPECT(condition) if (!(condition)) __builtin_trap();

static void test_suite_handle_sigabrt(int signal);
static void test_suite_handle_sigabrt(int signal);

// ====================================================================================================================
// Types
typedef void (*TestFunction)(void *context);

typedef struct {
    const char *name;
    TestFunction func;
} Test;

typedef struct {
    const char *suite_name;
    int has_been_run; // TestSuite can only be run once to avoid typical errors with dangling pointers in the context

    // Array of tests
    Test tests[TEST_SUITE_MAX_TESTS];
    uint64_t num_tests;

    // Used for catching traps produced by expects or asserts
    jmp_buf jump_buffer;

    // Context/Environment used for all tests. It serves as a way to pass global variables that will be shared by every
    // individual test. It can be null.
    void *context;

    // Handler that executes before every individual test. It can be null.
    TestFunction do_before_every_test_handler;
} TestSuite;

// ====================================================================================================================
// TestSuite functions
TestSuite test_suite_new(const char *suite_name) {
    assert(suite_name != 0);

    TestSuite suite = {};
    suite.suite_name = suite_name;

    // Catch SIGABRT raised by assertions or expectations
    // @TODO: Is it bad if this function gets called several times with the same arguments?
    signal(SIGABRT, test_suite_handle_sigabrt);

    return suite;
}

void test_suite_set_context(TestSuite *test_suite, void *context) {
    assert(test_suite != 0);
    assert(context != 0);
    test_suite->context = context;
}

// @NOTE: This global variable is a hack that is used to handle a SIGABRT from a single and specific TestSuite. I don't
// know how to do it without global variables, so this limits the ability of this test suite to run in multiple threads.
static TestSuite *active_test_suite = 0;

static int can_print_colors() {
    // @TODO: Implement complete logic defined in https://bixense.com/clicolors/
    const int standard_output = 1;

#ifdef _WIN32
    return _isatty(standard_output);
#elif __linux__
    return isatty(standard_output);
#else
    return false;
#endif
}

void test_suite_do_before_every_test(TestSuite *test_suite, TestFunction handler) {
    assert(test_suite != 0);
    assert(handler != 0);
    test_suite->do_before_every_test_handler = handler;
}

void test_suite_add_test(TestSuite *test_suite, const char *new_test_name, TestFunction new_test_function) {
    assert(test_suite != 0);
    assert(test_suite->num_tests < TEST_SUITE_MAX_TESTS && "Reached maximum number of tests in a suite");
    assert(new_test_name != 0);
    assert(new_test_function != 0);

    Test new_test = {};
    new_test.name = new_test_name;
    new_test.func = new_test_function;

    test_suite->tests[test_suite->num_tests++] = new_test;
}

int test_suite_run_all_and_print(TestSuite *test_suite) {
    assert(test_suite != 0);
    assert(!test_suite->has_been_run && "TestSuite instance can only run once!");

    // @NOTE: This assert doesn't work in concurrent scenarios because of race conditions. This test suite library isn't
    // designed to be used in multiple threads anyway. So no mutexes or other synchronization primitives are necessary.
    assert(active_test_suite == 0 && "Can't execute test suite while another is running!");

    // Mark this test suite as running. Sadly I need this hack to handle SIGABRT in a specific TestSuite. I don't know
    // any other way to do it without using global variables.
    active_test_suite = test_suite;

    printf("=== %s (%zu tests) ===\n", test_suite->suite_name, test_suite->num_tests);
    size_t tests_passed = 0;
    for (size_t i = 0; i < test_suite->num_tests; i++) {
        if (test_suite->do_before_every_test_handler) {
            test_suite->do_before_every_test_handler(test_suite->context);
        }

        const char *test_result = 0;
        const char *test_name = test_suite->tests[i].name;

        if (setjmp(test_suite->jump_buffer) == 0) {
            // @TODO: Pass context for every single test
            test_suite->tests[i].func(test_suite->context);

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
    }

    printf("\n%zu/%zu tests passed\n", tests_passed, test_suite->num_tests);
    test_suite->has_been_run = 1;

    // This is no longer the current active TestSuite. Let other TestSuites run.
    active_test_suite = 0;

    int errcode = 0;
    if (tests_passed != test_suite->num_tests) {
        errcode = 1;
    }
    return errcode;
}

static void test_suite_handle_sigabrt(int signal) {
    if (active_test_suite != 0 && signal == SIGABRT) {
        // We receive this signal when a condition hasn't been met in an assertion. This signal can also be sent from other
        // sources but for simplicity we consider every SIGABRT signal as sent from abort() when its condition fails.
        longjmp(active_test_suite->jump_buffer, 1);
    }
}
