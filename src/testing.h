#ifdef ENABLE_TEST_SUITE

/* We rolled out our own minimal test suite! */

#include <stdio.h>

#define TEST_SUITE(name, body) int run_tests_##name() { int TESTS_PASSED = 0; int TESTS_TOTAL = 0; (body); printf("%s: %d/%d passed.\n", #name, TESTS_PASSED, TESTS_TOTAL); return TESTS_PASSED == TESTS_TOTAL; }
#define TEST(description, body) { \
    __label__ error, end; \
    const char *MSG = 0; \
    (body); \
    TESTS_PASSED += 1; \
    goto end; \
error:\
    printf("ERROR in test: %s (" __FILE__ ")\n", description); \
    printf("%s\n", MSG); \
end:\
    TESTS_TOTAL += 1; \
}
#define TESTS(name, body) TEST_SUITE(name, TEST(#name, (body)))

#define ASSERT_EQ(actual, expected) { if ((actual) != (expected)) { MSG = "Expected " #actual " to equal " #expected "."; goto error; } }

#else

// The test suite is disabled

#define TEST_SUITE(name, body)
#define TEST(description, body)
#define TESTS(name, body)

#endif