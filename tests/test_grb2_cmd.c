/* This is a test of the functions in grb2_cmd.c, which is part of the C API for wgrib2.
 *
 * Note: This test uses setjmp/longjmp to catch fatal_error() calls. This may cause an 
 * Illegal Instruction error (or similar) when using the Intel Classic compiler. I haven't 
 * been able to find a workaround for this. CMakeLists.txt has been modified to skip this 
 * test when using the Intel Classic compiler.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <setjmp.h>

extern jmp_buf fatal_err;
extern char cmd[N_CMDS][CMD_LEN];
extern char *cmds[N_CMDS+1];

int
main()
{
    printf("Testing wgrib2_init_cmds()...\n");
    {
        /* cmds[0] should be NULL prior to initialization */
        if (cmds[0] != NULL) {
            printf("ERROR: cmds[0] should be NULL prior to initialization.\n");
            return 2;
        }

        /* Now checking AFTER initialization. */
        wgrib2_init_cmds();
        if (strcmp(cmds[0], "wgrib2 C_api") != 0) {
            printf("ERROR: cmds[0] not initialized properly by wgrib2_init_cmds().\n");
            return 3;
        }
    }
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
