/**
 * This is a test for the wgrib2 project. This test is for parse_loop.c.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include <limits.h>
#include <setjmp.h>
#include "wgrib2.h"

/* Number of valid input cases */
#define NUM_TESTS 4

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing parse_loop().\n");
    printf("Testing standard calls of parse_loop()...\n");
    {
        const char *inputs[NUM_TESTS] = {
            "2:10:3",  /* i == 3: start:end:step */
            "5:20",    /* i == 2: step defaults to 1 */
            "7::4",    /* i == 1 then 2: end=INT_MAX, step from second sscanf */
            "9"        /* i == 1 then !=2: end=INT_MAX, step=1 */
        };
        int exp_start[NUM_TESTS] = { 2, 5, 7, 9 };
        int exp_end[NUM_TESTS]   = { 10, 20, INT_MAX, INT_MAX };
        int exp_step[NUM_TESTS]  = { 3, 1, 4, 1 };

        int i;
        for (i = 0; i < NUM_TESTS; i++) {
            int start, end, step;

            if ((parse_loop(inputs[i], &start, &end, &step) != 0)) {
                printf("parse_loop() failed for test %d: '%s'\n",
                        i, inputs[i]);
                return 10;
            }

            if (start != exp_start[i] || end != exp_end[i] || step  != exp_step[i]) {
                printf("parse_loop() mismatch for test %d : '%s'. "
                        "Expected start=%d end=%d step=%d, got start=%d end=%d step=%d\n",
                        i, inputs[i], exp_start[i], exp_end[i], exp_step[i], start, end, step);
                return 11;
            }
        }
    }
    printf("Testing parse_loop() with invalid string. Should throw fatal_error()...\n");
    {
        const char *string = "invalid";
        int start, end, step;

        if (setjmp(fatal_err) == 0) {
            (void) parse_loop(string, &start, &end, &step);
            printf("parse_loop() did not fail for invalid input string.\n");
            return 12;
        }
    }
    printf("Testing parse_loop() with start <= 0. Should throw fatal_error()...\n");
    {
        const char *string = "-1:4:2";
        int start, end, step;

        if (setjmp(fatal_err) == 0) {
            (void) parse_loop(string, &start, &end, &step);
            printf("parse_loop() did not fail for start <= 0.\n");
            return 13;
        }
    }
    printf("Testing parse_loop() with end < start. Should throw fatal_error()...\n");
    {
        const char *string = "5:4:2";
        int start, end, step;

        if (setjmp(fatal_err) == 0) {
            (void) parse_loop(string, &start, &end, &step);
            printf("parse_loop() did not fail for end < start.\n");
            return 14;
        }
    }
    printf("Testing parse_loop() with step <= 0. Should throw fatal_error()...\n");
    {
        const char *string = "1:4:0";
        int start, end, step;

        if (setjmp(fatal_err) == 0) {
            (void) parse_loop(string, &start, &end, &step);
            printf("parse_loop() did not fail for step <= 0.\n");
            return 15;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}