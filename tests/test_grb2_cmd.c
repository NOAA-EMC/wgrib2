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
#include <unistd.h>

#define STDERR_FILE_NAME "test_stderr.txt"

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
    printf("ok!\n");
    printf("Testing wgrib2_add_cmd() with overly long command string...\n");
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
    printf("Testing wgrib2_add_cmd() with too many options...\n");
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
    printf("Testing wgrib2_add_cmd() with normal command string...\n");
    {
        int ret;

        wgrib2_init_cmds();

        if ((ret = wgrib2_add_cmd("-test"))) {
            return 4;
        }
    }
    printf("ok!\n");
    printf("Testing wgrib2_list_cmd() without initialization...\n");
    {
        int original_stderr, tmp;
        char buffer[128];

        fflush(stderr);
        original_stderr = dup(STDERR_FILENO);
        tmp = open(STDERR_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (tmp < 0) {
            printf("ERROR: could not open temporary stderr file.\n");
            return 12;
        }

        dup2(tmp, STDERR_FILENO);
        close(tmp);

        wgrib2_list_cmd();

        fflush(stderr);
        dup2(original_stderr, STDERR_FILENO);
        close(original_stderr);

        FILE* f = fopen(STDERR_FILE_NAME, "r");
        if (f == NULL) {
            printf("ERROR: could not open temporary stderr file for reading.\n");
            return 13;
        }

        fgets(buffer, sizeof(buffer), f);
        fclose(f);
        remove(STDERR_FILE_NAME);

        if (strcmp(buffer, "no wgrib2 cmds\n") != 0) {
            printf("ERROR: wrong stderr output: %s\n", buffer);
            return 14;
        }

    }
    printf("SUCCESS!\n");
    return 0;
}
