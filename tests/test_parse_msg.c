/**
 * This is a test for the wgrib2 project. This test is for parse_msg.c.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include <setjmp.h>
#include "wgrib2.h"

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing standard calls of parse_1st_msg() and parse_next_msg()...\n");
    {
        /**
         * TODO: Ammend this test to call parse_next_msg() after parse_1st_msg()
         * and verify that it returns the expected result. This means 
         * checking that it returns 0 then checking that sec is updated as expected.
         * 
         * Create an array called exp_next_sec that contains the expected values of sec 
         * after calling parse_next_msg().
         */
        unsigned char msg[88] = { 0 };
        unsigned char *sec[10] = { 0 };
        unsigned char *exp_sec[10];
        unsigned char *exp_next_sec[10];
        int ret;

        /* total message length: 88 bytes */
        msg[15] = 88;

        /* section 1 */
        msg[19] = 6;
        msg[20] = 1;

        /* section 3 */
        msg[25] = 6;
        msg[26] = 3;

        /* section 4 */
        msg[31] = 6;
        msg[32] = 4;

        /* section 5 */
        msg[37] = 6;
        msg[38] = 5;

        /* section 6, bitmap indicator 0 so sec[9] = p */
        msg[43] = 6;
        msg[44] = 6;

        /* section 7 (last section) */
        msg[49] = 6;
        msg[50] = 7;

        /* second field: sections 1, 3, 4, 5, 6, 7 */
        msg[55] = 6;
        msg[56] = 1;

        msg[61] = 6;
        msg[62] = 3;

        msg[67] = 6;
        msg[68] = 4;

        msg[73] = 6;
        msg[74] = 5;

        msg[79] = 6;
        msg[80] = 6;

        msg[85] = 6;
        msg[86] = 7;

        sec[0] = msg;
        sec[8] = msg + 88;

        exp_sec[0] = msg;
        exp_sec[1] = msg + 16;
        exp_sec[2] = NULL;
        exp_sec[3] = msg + 22;
        exp_sec[4] = msg + 28;
        exp_sec[5] = msg + 34;
        exp_sec[6] = msg + 40;
        exp_sec[7] = msg + 46;
        exp_sec[8] = msg + 88;
        exp_sec[9] = msg + 40;

        if ((parse_1st_msg(sec) != 0)) {
            return 10;
        }

        for (int i = 0; i < 10; i++) {
            if (sec[i] != exp_sec[i]) {
                return 11;
            }
        }

        exp_next_sec[0] = msg;
        exp_next_sec[1] = msg + 52;
        exp_next_sec[2] = NULL;
        exp_next_sec[3] = msg + 58;
        exp_next_sec[4] = msg + 64;
        exp_next_sec[5] = msg + 70;
        exp_next_sec[6] = msg + 76;
        exp_next_sec[7] = msg + 82;
        exp_next_sec[8] = msg + 88;
        exp_next_sec[9] = msg + 76;

        ret = parse_next_msg(sec);
        if (ret != 0) {
            return 12;
        }

        for (int i = 0; i < 10; i++) {
            if (sec[i] != exp_next_sec[i]) {
                return 13;
            }
        }


    }
    printf("Testing parse_1st_msg() with sec[0] == NULL. Should throw fatal_error()...\n");
    {
        unsigned char *sec[10] = { 0 };

        if (setjmp(fatal_err) == 0) {
            sec[0] = NULL;
            (void) parse_1st_msg(sec);
            return 14;
        }
    }
    printf("Testing parse_1st_msg() with an illegal section. Should throw fatal_error()...\n");
    {
        unsigned char msg[22] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[19] = 6;
        msg[20] = 9;

        sec[0] = msg;
        sec[8] = msg + 22;

        if (setjmp(fatal_err) == 0) {
            (void) parse_1st_msg(sec);
            return 15;
        }
    }
    printf("Testing parse_1st_msg() with unhandled predefined bitmaps. Should throw fatal_error()...\n");
    {
        unsigned char msg[22] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[19] = 6;
        msg[20] = 6;
        msg[21] = 1;

        sec[0] = msg;
        sec[8] = msg + 22;

        if (setjmp(fatal_err) == 0) {
            (void) parse_1st_msg(sec);
            return 16;
        }
    }
    printf("Testing parse_1st_msg() with undefined bitmaps. Should throw fatal_error()...\n");
    {
        unsigned char msg[22] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[19] = 6;
        msg[20] = 6;
        msg[21] = 254;

        sec[0] = msg;
        sec[8] = msg + 22;

        if (setjmp(fatal_err) == 0) {
            (void) parse_1st_msg(sec);
            return 17;
        }
    }
    printf("Testing parse_1st_msg() with illegally formatted GRIB message. Should throw fatal_error()...\n");
    {
        unsigned char msg[16] = { 0 };
        unsigned char *sec[10] = { 0 };

        sec[0] = msg;
        sec[8] = msg + 16;

        if (setjmp(fatal_err) == 0) {
            (void) parse_1st_msg(sec);
            return 18;
        }
    }
    printf("Testing parse_next_msg() with parsing error. Should throw fatal_error()...\n");
    {
        unsigned char msg[16] = { 0 };
        unsigned char *sec[10] = { 0 };

        sec[0] = msg;
        sec[7] = msg;
        msg[4] = 6;

        if (setjmp(fatal_err) == 0) {
            (void) parse_next_msg(sec);
            return 19;
        }
    }
    printf("Testing parse_next_msg() when sec 7 length runs past message end. Should throw fatal_error()...\n");
    {
        unsigned char msg[16] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[15] = 16;
        msg[3] = 20;
        msg[4] = 7;

        sec[0] = msg;
        sec[7] = msg;
        sec[8] = msg + 16;

        if (setjmp(fatal_err) == 0) {
            (void) parse_next_msg(sec);
            return 20;
        }
    }
    printf("Testing parse_next_msg() with later section length running past message end. Should throw fatal_error()...\n");
    {
        unsigned char msg[32] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[15] = 32;
        msg[19] = 8;
        msg[20] = 7;
        msg[27] = 16;
        msg[28] = 8; /* section 7 length is 16, which runs past the end of the message */

        sec[0] = msg;
        sec[7] = msg + 16;
        sec[8] = msg + 32;

        if (setjmp(fatal_err) == 0) {
            (void) parse_next_msg(sec);
            return 21;
        }
    }
    printf("Testing parse_next_msg() with unhandled predefined bitmaps. Should throw fatal_error()...\n");
    {
        unsigned char msg[32] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[15] = 32;
        msg[3] = 8;
        msg[4] = 7;

        msg[11] = 8;
        msg[12] = 6;
        msg[13] = 1;

        sec[0] = msg;
        sec[7] = msg;
        sec[8] = msg + 32;

        if (setjmp(fatal_err) == 0) {
            (void) parse_next_msg(sec);
            return 22;
        }
    }
    printf("Testing parse_next_msg() with undefined bitmaps. Should throw fatal_error()...\n");
    {
        unsigned char msg[32] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[15] = 32;
        msg[3] = 8;
        msg[4] = 7;

        msg[11] = 8;
        msg[12] = 6;
        msg[13] = 254;

        sec[0] = msg;
        sec[7] = msg;
        sec[8] = msg + 32;

        if (setjmp(fatal_err) == 0) {
            (void) parse_next_msg(sec);
            return 23;
        }
    }
    printf("Testing parse_next_msg() with incomplete message. Should return 1...\n");
    {
        unsigned char msg[32] = { 0 };
        unsigned char *sec[10] = { 0 };

        msg[15] = 32;

        msg[16] = 0;
        msg[17] = 0;
        msg[18] = 0;
        msg[19] = 8;
        msg[20] = 7;

        msg[24] = 0;
        msg[25] = 0;
        msg[26] = 0;
        msg[27] = 8;
        msg[28] = 3;

        sec[0] = msg;
        sec[7] = msg + 16;
        sec[8] = msg + 32;

        if (parse_next_msg(sec) != 1) {
            return 24;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}