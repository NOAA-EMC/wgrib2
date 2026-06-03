/**
 * This is a test for the wgrib2 project. This test is for itoshort_a.c.
 * 
 * Alyson Stahl, 3/2026
 */

#include <stdio.h>
#include <string.h>
#include "wgrib2.h"

#define NUM_TESTS 5
int
main()
{
    printf("Testing itoshort_a().\n");
    {
        int vals[NUM_TESTS] = {
            0, 123, -123, 2000, -1230000
        };
        char exp_str[NUM_TESTS][20] = {
            "0", "123", "n123", "2e3", "n123e4"
        };

        for (int i = 0; i < NUM_TESTS; i++) {
            char str[20];
            itoshort_a(str, vals[i]);
            if (strcmp(str, exp_str[i]) != 0) {
                printf("Test %d failed: itoshort_a(%d) returned '%s', expected '%s'.\n", 
                        i, vals[i], str, exp_str[i]);
                return 10;
            }
        }
    }
    printf("SUCCESS!\n");
    return 0;
}