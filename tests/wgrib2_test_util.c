/*
 * These are utility functions for wgrib2 tests.
 * 
 * Alyson Stahl
 */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <ctype.h>

#include "wgrib2_test_util.h"

#define BUFFER_SIZE 1024 * 64

/**
 * Compare two text files, ignoring whitespace differences. 
 * 
 * Return 0 if they are identical, error code otherwise.
 * 
 * Alyson Stahl 12/2025
 */
int compare_files(const char *fname1, const char *fname2) {
    FILE *f1 = fopen(fname1, "r");
    FILE *f2 = fopen(fname2, "r");
    if (f1 == NULL || f2 == NULL) {
        printf("Error opening files for comparison.\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 101;
    }

    int c1, c2;

    for (;;) {
        // Advance fa to next non-whitespace char (or EOF)
        do { c1 = fgetc(f1); } while (c1 != EOF && isspace((unsigned char)c1));
        // Advance fb to next non-whitespace char (or EOF)
        do { c2 = fgetc(f2); } while (c2 != EOF && isspace((unsigned char)c2));

        if (c1 == EOF || c2 == EOF) {
            int same = (c1 == EOF && c2 == EOF);
            fclose(f1);
            fclose(f2);
            if (same) {
                return 0;   // Both files ended simultaneously
            } else {
                printf("Files differ in length.\n");
                return 102; 
            }
        }

        if (c1 != c2) {
            fclose(f1);
            fclose(f2);
            printf("Files differ.\n");
            return 103;
        }
        // else equal non-whitespace chars; continue
    }
}

/**
 * Compare two GRIB2 files. 
 * 
 * Return 0 if they are identical, error code otherwise.
 * 
 * Alyson Stahl 12/2025
 */
int compare_grib2_files(const char *fname1, const char *fname2) {
    FILE *f1 = fopen(fname1, "rb");
    FILE *f2 = fopen(fname2, "rb");
    if (f1 == NULL || f2 == NULL) return 101;

    unsigned char buf1[BUFFER_SIZE];
    unsigned char buf2[BUFFER_SIZE];

    for (;;) {
        size_t n1 = fread(buf1, 1, BUFFER_SIZE, f1);
        size_t n2 = fread(buf2, 1, BUFFER_SIZE, f2);

        if (n1 != n2) {
            fclose(f1);
            fclose(f2);
            printf("Files differ in size.\n");
            return 102; 
        }

        if (n1 == 0) {
            int err1 = ferror(f1);
            int err2 = ferror(f2);
            fclose(f1);
            fclose(f2);
            if (err1 || err2) {
                printf("Error reading files.\n");
                return 103;  
            } else {
                return 0;   // End of both files reached, they are identical
            }
        }

        if (memcmp(buf1, buf2, n1) != 0) {
            fclose(f1);
            fclose(f2);
            printf("Files differ in content.\n");
            return 104;  
        }
    }
}
