/** @file
 * @brief Handles arguments for wgrib2.
 * @author Public Domain: Wesley Ebisuzaki @date 3/2018
 */
#include <stdio.h>
#include <string.h>
#include "c_wgrib2api.h"

int wgrib2(int argc, const char **argv);

static int n_cmds = -1;     /**< Number of commands in command list. */
char cmd[N_CMDS][CMD_LEN]; /** List of command strings. */
char *cmds[N_CMDS+1]; /**< List of formatted command-line options. */

/**
 * Initialize the list of command-line options for wgrib2.
 *
 * @author Wesley Ebisuzaki @date 3/2018
 */
void wgrib2_init_cmds(void) {
    cmds[0] = "wgrib2 C_api";
    n_cmds = 1;
    return;
}

/**
 * Add a command to the list for wgrib2.
 *
 * @param string The command string to add. Cannot be longer than CMD_LEN.
 * 
 * @return 0 on success, error code otherwise.
 *
 * @author Wesley Ebisuzaki @date 3/2018
 */
int wgrib2_add_cmd(const char *string) {
    int j;

    j = strlen(string);
    if (j >= CMD_LEN) fatal_error("add_cmd: string too long %s", string);
    if (n_cmds >= N_CMDS) fatal_error("add_cmd: too many options %s", string);
    strncpy(&(cmd[n_cmds][0]), string, j+1);
    cmds[n_cmds] = &(cmd[n_cmds][0]);
    n_cmds++;
    return 0;
}

/**
 * Execute the command-line options for wgrib2.
 *
 * @return 0 on success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 3/2018
 */
int wgrib2_cmd(void) {
    return wgrib2(n_cmds, (const char **) cmds);
}

/**
 * List all command-line options used.
 *
 * @author Wesley Ebisuzaki @date 3/2018
 */
int wgrib2_list_cmd(void) {
    int i;
    if (n_cmds < 0) {
        fprintf(stderr,"no wgrib2 cmds\n");
        return 0;
    }
    fprintf(stderr,"wgrib2  ");
    for (i = 1; i < n_cmds; i++) {
        fprintf(stderr,"%s ", cmds[i]);
    }
    fprintf(stderr,"\n");
    return 0;
}
