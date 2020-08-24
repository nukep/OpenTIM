#if ENABLE_TEST_SUITE

/* We rolled out our own minimal test suite! */

#include <stdio.h>

#define TEST_ERROR_FMT(v) _Generic(v, byte: "%d", sbyte: "%d", u16: "%d", s16: "%d", u32: "%d", s32: "%d")

#define ASSERT_EQ(actual_expr, expected_expr) { \
    typeof(actual_expr) actual = actual_expr; \
    typeof(expected_expr) expected = expected_expr; \
    if (actual != expected) { \
        char expected_str[64]; \
        char actual_str[64]; \
        char msg[256]; \
        snprintf(expected_str, sizeof(expected_str), TEST_ERROR_FMT(expected), expected); \
        snprintf(actual_str, sizeof(actual_str), TEST_ERROR_FMT(expected), actual); \
        snprintf(msg, sizeof(msg), __FILE__ ":%d: Got %s, expected %s (ASSERT_EQ(" #actual_expr ", " #expected_expr "))\n", __LINE__, actual_str, expected_str); \
        report_error(msg); \
        return; \
    } \
}

#define TEST_SUITE(name) \
    void run_tests_##name(void (*report_error)(const char *msg))

#define TEST(description) \
    TEST_TO_RUN -= 1; \
    TEST_STATE_RUN_THIS_ONE = TEST_TO_RUN == 0; \
    if (TEST_STATE_RUN_THIS_ONE) { \
        TEST_FOUND = 1; \
        TEST_CURRENT_DESCRIPTION = description; \
    } \
    if (TEST_STATE_RUN_THIS_ONE)

extern int TEST_TO_RUN;
extern int TEST_FOUND;
extern int TEST_ERROR;
extern int TEST_STATE_RUN_THIS_ONE;
extern const char *TEST_CURRENT_DESCRIPTION;

#else

// The test suite is disabled

#endif