#ifdef ENABLE_TEST_SUITE

#include <stdio.h>

int TEST_TO_RUN = 0;
int TEST_FOUND = 0;
int TEST_ERROR = 0;
int TEST_STATE_RUN_THIS_ONE = 0;
const char *TEST_CURRENT_DESCRIPTION = 0;

void my_report_error(const char *msg) {
    TEST_ERROR = 1;
    printf("  ERROR: %s\n    %s", TEST_CURRENT_DESCRIPTION, msg);
}

typedef void (*test_func_t)(void (*report_error)(const char *msg));

void run_test(test_func_t f, const char *name, int *tests_total, int *tests_passed) {
    printf("%s\n", name);
    for (int i = 1; ; i++) {
        TEST_TO_RUN = i;
        TEST_FOUND = 0;
        TEST_ERROR = 0;
        f(my_report_error);
        if (TEST_FOUND) {
            *tests_total += 1;
            if (!TEST_ERROR) {
                printf("  Passed: %s\n", TEST_CURRENT_DESCRIPTION);
                *tests_passed += 1;
            }
        } else {
            // No more tests
            break;
        }
    }
    printf("\n");
}

#define RUNTEST(name) void run_tests_##name(void (*report_error)(const char *msg)); run_test(run_tests_##name, #name, &tests_passed, &tests_total)

int main(int argc, char *argv[]) {
    int tests_passed = 0;
    int tests_total = 0;

    RUNTEST(arctan);
    RUNTEST(draw_rope);
    RUNTEST(acceleration_and_terminal_velocity);
    RUNTEST(sine);
    RUNTEST(helpers);
    RUNTEST(lzw_decode);

    printf("%d/%d tests passed.\n", tests_passed, tests_total);

    if (tests_passed == tests_total) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}

#endif