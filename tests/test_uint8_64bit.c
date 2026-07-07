/**
 * This is a test for uint8() function in aux_progs. For 64-bit systems.
 * 
 * Alyson Stahl
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "aux_progs.h"

int
main()
{
    printf("Testing uint8() function for 64-bit systems...\n");
    printf("Verifying we are in 64-bit test mode.");
    {
        if (ULONG_MAX == 4294967295UL) {
            printf("FAIL: Expected 64-bit system (ULONG_MAX > 4294967295)\n");
            return 1;
        }
    }
    printf("ok!\n");
    printf("Test with all zeros. Should work."); 
    {
        unsigned char test_bytes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        unsigned long result;

        result = uint8(test_bytes);

        if (result != 0) {
            printf("FAIL: All zeros test - expected 0, got %lu\n", result);
            return 2;
        }
    }
    printf("ok!\n");
    printf("Test simple pattern in lower 4 bytes only. Should work.");
    {
        unsigned char test_bytes[8] = {0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78};
        unsigned long result;
        unsigned long expected = (0x12UL << 24) + (0x34UL << 16) + (0x56UL << 8) + 0x78UL;

        result = uint8(test_bytes);

        if (result != expected) {
            printf("FAIL: Lower 4 bytes pattern test - expected %lu, got %lu\n", expected, result);
            return 3;
        }
    }
    printf("ok!\n");
    printf("Test most significant bytes only. Should work.");
    {
        unsigned char test_bytes[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned long result;
        unsigned long expected = (0x01UL << 56);

        result = uint8(test_bytes);

        if (result != expected) {
            printf("FAIL: MSB only test - expected %lu, got %lu\n", expected, result);
            return 4;
        }
    }
    printf("ok!\n");
    printf("Test all 8 bytes pattern. Should work.");
    {
        unsigned char test_bytes[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        unsigned long result;
        unsigned long expected = 
            (0x01UL << 56) + (0x23UL << 48) + (0x45UL << 40) + (0x67UL << 32) +
            (0x89UL << 24) + (0xABUL << 16) + (0xCDUL << 8) + 0xEFUL;

        result = uint8(test_bytes);

        if (result != expected) {
            printf("FAIL: All bytes pattern test - expected %lu, got %lu\n", expected, result);
            return 5;
        }
    }
    printf("ok!\n");
    printf("Test max 64-bit value. Should work.");
    {
        unsigned char test_bytes[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        unsigned long result;
        unsigned long expected = 18446744073709551615UL; // 2^64 - 1
        
        result = uint8(test_bytes);
        if (result != expected) {
            printf("FAIL: Max 64-bit value test - expected %lu, got %lu\n", expected, result);
            return 6;
        }
    }
    printf("ok!\n");
    printf("Test individual byte position (verify bit shifting).");
    {
        unsigned char test_bytes[8] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned long result;
        unsigned long expected = (0x01UL << 48);

        result = uint8(test_bytes);
        if (result != expected) {
            printf("FAIL: Byte position test - expected %lu, got %lu\n", expected, result);
            return 7;
        }
    }
    printf("ok!\n");
    printf("Test values that would trigger the 32-bit overflow but are valid on 64-bit");
    {
        unsigned char test_bytes[8] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
        unsigned long result;
        unsigned long expected = (0x01UL << 32);

        result = uint8(test_bytes);
        if (result != expected) {
            printf("FAIL: 32-bit overflow test - expected %lu, got %lu\n", expected, result);
            return 8;
        }
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}
