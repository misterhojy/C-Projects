#include <stdio.h>
#include "criterion/criterion.h"

#define TEST_TIMEOUT 3 
#define TEST_INPUT_DIR "tests.in"
#define TEST_OUTPUT_DIR "tests.out"

static char test_log_outfile[100];

int run_using_system(char *test_name, char *args) {
    sprintf(test_log_outfile, "%s/%s.log", TEST_OUTPUT_DIR, test_name);
    char cmd[500];
    sprintf(cmd, "mkdir -p %s", TEST_OUTPUT_DIR);
    system(cmd);
    sprintf(cmd, "./integers %s > %s 2>&1", args, test_log_outfile);
    return system(cmd);
}

void assert_outfile_matches(char *name) {
    char cmd[500];
    sprintf(cmd, "diff --strip-trailing-cr %s/%s.txt %s ", TEST_INPUT_DIR, name, test_log_outfile);
    int err = system(cmd);
    cr_assert_eq(err, 0, "The output was not what was expected (diff exited with status %d).\n", WEXITSTATUS(err));
}

TestSuite(base_suite, .timeout=TEST_TIMEOUT);

Test(base_suite, test_output01) {
    char *test_name = "test_output01";
    run_using_system(test_name, "1 2 0xfffffffa");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output02) {
    char *test_name = "test_output02";
    run_using_system(test_name, "2 1 0x00000020");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output03) {
    char *test_name = "test_output03";
    run_using_system(test_name, "S 2 0x80000001");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output04) {
    char *test_name = "test_output04";
    run_using_system(test_name, "1 S 0xfffffff0");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output05) {
    char *test_name = "test_output05";
    run_using_system(test_name, "S 2 0x80000000");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output06) {
    char *test_name = "test_output06";
    run_using_system(test_name, "1 1 0x59f2ca50");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output07) {
    char *test_name = "test_output07";
    run_using_system(test_name, "F 1 0x00394812");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output08) {
    char *test_name = "test_output08";
    run_using_system(test_name, "2 1 0x80000000");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output09) {
    char *test_name = "test_output09";
    run_using_system(test_name, "D 2 0x00411d01");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output10) {
    char *test_name = "test_output10";
    run_using_system(test_name, "D 2 0xa02837fa");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output11) {
    char *test_name = "test_output11";
    run_using_system(test_name, "1 D 0xfffff075");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output12) {
    char *test_name = "test_output12";
    run_using_system(test_name, "2 D 0x000000ff");
    assert_outfile_matches(test_name);
}

Test(base_suite, test_output13) {
    char *test_name = "test_output13";
    run_using_system(test_name, "S D 0x801252eb");
    assert_outfile_matches(test_name);
}
