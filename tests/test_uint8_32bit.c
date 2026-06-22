/**
 * This is a test for uint8() function in aux_progs. For 32-bit systems.
 * 
 * Alyson Stahl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <setjmp.h>

#include "aux_progs.h"

#ifdef FORCE_32BIT_TEST
#undef ULONG_MAX
#define ULONG_MAX 4294967295UL
#endif

// Jump buffer for catching exit() calls
static jmp_buf exit_jmp;
static int exit_code = 0;

// Override exit() to catch it during testing
void exit(int code) {
    exit_code = code;
    longjmp(exit_jmp, 1);
}

int
main()
{
    printf("Testing uint8() function for 32-bit systems...\n");
    printf("Verifying we are in 32-bit test mode.");
    {
        if (ULONG_MAX != 4294967295UL) {
            printf("FAIL: ULONG_MAX should be 4294967295 for 32-bit test\n");
            return 1;
        }
    }
    printf("ok!\n");
    printf("Testing error case: upper byte set (should trigger exit)\n");
    {
        unsigned char test_bytes[8];

        memset(test_bytes, 0, 8);
        test_bytes[0] = 0x01;  // Set upper byte - this should cause exit(8)

        // Set up jump point to catch the exit()
        if (setjmp(exit_jmp) == 0) {
            // First time through - call the function that should exit
            uint8(test_bytes);
            // If we get here, the function didn't exit as expected
            printf("FAIL: Error case test - function should have called exit() but didn't\n");
            return 2;
        } else {
            // We jumped back here from exit() - check the exit code
            if (exit_code != 8) {
                printf("FAIL: Error case test - called exit(%d), expected exit(8)\n", exit_code);
                return 3;
            }
        }
    }
    printf("ok!\n");
    printf("Test with all zeros. Should work.\n");
    {
        unsigned char test_bytes[8];
        unsigned long result;

        memset(test_bytes, 0, 8);
        result = uint8(test_bytes);
        if (result != 0) {
            printf("FAIL: All zeros test - expected 0, got %lu\n", result);
            return 4;
        }
    }
    printf("ok!\n");
    printf("Test simple pattern in lower 4 bytes. Should work.\n");
    {
        unsigned char test_bytes[8];
        unsigned long result;

        memset(test_bytes, 0, 8);
        test_bytes[4] = 0x12; test_bytes[5] = 0x34; test_bytes[6] = 0x56; test_bytes[7] = 0x78;
        result = uint8(test_bytes);

        unsigned long expected = (0x12UL << 24) + (0x34UL << 16) + (0x56UL << 8) + 0x78UL;
        if (result != expected) {
            printf("FAIL: Simple pattern test - expected %lu, got %lu\n", expected, result);
            return 5;
        }
    }
    printf("ok!\n");
    printf("Test max 32-bit value. Should work.\n");
    {
        unsigned char test_bytes[8];
        unsigned long result;

        memset(test_bytes, 0, 4);  // Upper 4 bytes zero
        memset(test_bytes + 4, 0xFF, 4);  // Lower 4 bytes all 0xFF
        result = uint8(test_bytes);
        if (result != 4294967295UL) {
            printf("FAIL: Max 32-bit value test - expected 4294967295, got %lu\n", result);
            return 6;
        }
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}
