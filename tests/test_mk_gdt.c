/**
 * This is a test for the wgrib2 project. This test is for mk_gdt.c.
 * 
 * NOTE: The function mk_gdt() contains some unreachable code. The switch
 * statement handles cases that are not present in the current version of 
 * gdt_table. The default case is also unreachable.
 * 
 * Alyson Stahl, 2/2026
 */

#include <stdio.h>
#include <setjmp.h>
#include "wgrib2.h"

#define NUM_GDT_TEMPLATES 10

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing mk_gdt().\n");
    printf("Testing standard call of mk_gdt() with all gdt templates...\n");
    {
        unsigned char sec1[7] = { 0 };
        unsigned char sec3[128] = { 0 };
        unsigned char *sec[4] = { 0 };
        int igdtnum;
        int igdttmpl[22];
        int igdtleni;
        int i, j, ret;
        int template_nums[NUM_GDT_TEMPLATES] = { 
            0, 1, 10, 20, 30, 40, 90, 101, 32768, 32769
        };
        int exp_igdtnum[NUM_GDT_TEMPLATES] = {
            0, 1, 10, 20, 30, 40, 90, 101, 32768, 32769
        };
        int exp_igdtleni[NUM_GDT_TEMPLATES] = {
            19, 22, 19, 18, 22, 19, 21, 7, 19, 21
        };
        /**
         * TODO: Replace the ??? with the expected igdttmpl values for each template number in template_nums.
         * 
         * These expected values should be based on the values you assign to sec 3.
         * 
         * However you choose to assign values to sec 3, make sure that the expected igdttmpl values are
         * not just all zero.
         */
        int exp_igdtmpl[NUM_GDT_TEMPLATES][22] = { 
            /* template 0 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 16843009,
                16843009, 1, 16843009, 16843009, 16843009, 16843009, 1 },
            /* template 1 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 16843009,
                16843009, 1, 16843009, 16843009, 16843009, 16843009, 1,
                16843009, 16843009, 16843009 },
            /* template 10 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 1,
                16843009, 16843009, 16843009, 1, 16843009, 16843009, 16843009 },
            /* template 20 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 1,
                16843009, 16843009, 16843009, 16843009, 1, 1 },
            /* template 30 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 1,
                16843009, 16843009, 16843009, 16843009, 1, 1,
                16843009, 16843009, 16843009, 16843009 },
            /* template 40 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 16843009,
                16843009, 1, 16843009, 16843009, 16843009, 16843009, 1 },
            /* template 90 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 1,
                16843009, 16843009, 16843009, 16843009, 1,
                16843009, 16843009, 16843009, 16843009 },
            /* template 101 */
            { 1, 65793, 1, 16843009, 16843009, 16843009, 16843009 },
            /* template 32768 (same as 0) */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 16843009,
                16843009, 1, 16843009, 16843009, 16843009, 16843009, 1 },
            /* template 32769 */
            { 1, 1, 16843009, 1, 16843009, 1, 16843009,
                16843009, 16843009, 16843009, 16843009, 16843009,
                16843009, 1, 16843009, 16843009, 16843009, 16843009, 1,
                16843009, 16843009 }
        };


        for (int k = 14; k < 128; k++) {
            sec3[k] = 1;
        }
        
        sec1[5] = 0;
        sec1[6] = 7;

        sec[1] = sec1;
        sec[3] = sec3;

        for (i = 0; i < NUM_GDT_TEMPLATES; i++) {
            sec3[12] = (unsigned char) (template_nums[i] >> 8);
            sec3[13] = (unsigned char) (template_nums[i] & 0xff);
            igdtleni = 22;

            ret = mk_gdt(sec, &igdtnum, igdttmpl, &igdtleni);
            if (ret != 0) {
                printf("mk_gdt() failed for template %d with return code %d\n", template_nums[i], ret);
                return 10;
            }
            if (igdtnum != exp_igdtnum[i]) {
                printf("Unexpected igdtnum for template %d: got %d expected %d\n", template_nums[i], igdtnum, exp_igdtnum[i]);
                return 11;
            }
            if (igdtleni != exp_igdtleni[i]) {
                printf("Unexpected igdtleni for template %d: got %d expected %d\n", template_nums[i], igdtleni, exp_igdtleni[i]);
                return 12;
            }
            for (j = 0; j < igdtleni; j++) {
                if (igdttmpl[j] != exp_igdtmpl[i][j]) {
                    printf("Unexpected igdttmpl[%d] for template %d: got %d expected %d\n", j, template_nums[i], igdttmpl[j], exp_igdtmpl[i][j]);
                    return 13;
                }
            }
        }
    }
    printf("Testing with invalid center for gdt >= 32768. Should return 1...\n");
    {
        unsigned char sec1[7] = { 0 };
        unsigned char sec3[14] = { 0 };
        unsigned char *sec[4] = { 0 };
        int igdtnum;
        int igdttmpl[1];
        int igdtleni = 0;

        sec1[5] = 0;
        sec1[6] = 1;

        sec3[12] = 0x80;
        sec3[13] = 0x00;

        sec[1] = sec1;
        sec[3] = sec3;

        if (mk_gdt(sec, &igdtnum, igdttmpl, &igdtleni) != 1) {
            return 14;
        }
    }
    printf("Testing with invalid template number. Should return 2...\n");
    {
        unsigned char sec3[14] = { 0 };
        unsigned char *sec[4] = { 0 };
        int igdtnum;
        int igdttmpl[1];
        int igdtleni = 0;

        sec3[12] = 0x00;
        sec3[13] = 0x02;

        sec[3] = sec3;

        if (mk_gdt(sec, &igdtnum, igdttmpl, &igdtleni) != 2) {
            return 15;
        }
    }
    printf("Testing with bad gdt buffer size. Should throw a fatal_error()...\n");
    {
        unsigned char sec3[32] = { 0 };
        unsigned char *sec[4] = { 0 };
        int igdtnum;
        int igdttmpl[1];
        int igdtleni = 0;

        sec3[12] = 0x00;
        sec3[13] = 0x00;

        sec[3] = sec3;

        if (setjmp(fatal_err) == 0) {
            (void) mk_gdt(sec, &igdtnum, igdttmpl, &igdtleni);
            return 16;
        }
    }
    printf("SUCCESS!\n");
    return 0;
}