#ifdef ENABLE_TEST_SUITE

#define RUNTEST(name) int run_tests_##name(); passed_overall = passed_overall && run_tests_##name();

int main(int argc, char *argv[]) {
    int passed_overall = 1;

    RUNTEST(arctan);
    RUNTEST(draw_rope);
    RUNTEST(acceleration_and_terminal_velocity);

    if (passed_overall) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}

#else
int main(int argc, char *argv[]) {
    printf("nope!\n");
}
#endif