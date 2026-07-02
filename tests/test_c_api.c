/* This is a test of the wgrib2 c api.
 *
 * Note: This test uses setjmp/longjmp to catch fatal_error() calls. This may cause an 
 * Illegal Instruction error (or similar) when using the Intel Classic compiler. I haven't 
 * been able to find a workaround for this. CMakeLists.txt has been modified to skip this 
 * test when using the Intel Classic compiler.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include "wgrib2_test_util.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <setjmp.h>

#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "junk_c_api.inv"
#define EXP_GRB_INV "data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv"

extern jmp_buf fatal_err;

int
main()
{
    printf("Testing grb2_mk_inv()...\n");
    {
        int ret;

        if ((ret = grb2_mk_inv(GRB_FILE, GRB_INV))) {
            printf("grb2_mk_inv() failed with return code %d\n", ret);
            return 2;
        }

        if ((ret = compare_files(GRB_INV, EXP_GRB_INV))) {
            printf("Inventory files differ.\n");
            return 3;
        }
    }
    printf("ok!\n");
    printf("Testing wgrib2_add_cmd()...\n");
    printf("Testing with overly long command string...\n");
    {
        char longopt[CMD_LEN + 1];
        wgrib2_init_cmds();
        memset(longopt, 'A', CMD_LEN);
        longopt[CMD_LEN] = '\0';

        if (setjmp(fatal_err) == 0) {
            wgrib2_add_cmd(longopt);
            printf("ERROR: expected fatal_error but call returned.\n");
            return 10;
        }
    }
    printf("ok!\n");
    printf("Testing with too many options...\n");
    {
        int i;
        wgrib2_init_cmds();

        if (setjmp(fatal_err) == 0) {
            for (i = 0; i < N_CMDS + 1; i++) {
                char opt[20];
                snprintf(opt, sizeof(opt), "-opt%d", i);
                wgrib2_add_cmd(opt);
            }
            printf("ERROR: expected fatal_error but call returned.\n");
            return 11;
        }
    }
    printf("ok!\n");
    printf("SUCCESS!\n");
    return 0;
}
