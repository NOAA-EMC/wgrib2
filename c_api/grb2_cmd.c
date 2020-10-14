#include <stdio.h>
#include <string.h>
#include "c_wgrib2api.h"

/*
 * handles arguments for wgrib2
 */


static int n_cmds = -1;
char cmd[N_CMDS][CMD_LEN];
char *cmds[N_CMDS+1];

void wgrib2_init_cmds() {

    cmds[0] = "wgrib2 C_api";
    n_cmds = 1;
    return;
}

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

int wgrib2_cmd() {
    return wgrib2(n_cmds, cmds);
}

int wgrib2_list_cmd() {
    int i;
    if (n_cmds < 0) {
        fprintf(stderr,"no wgrib2 cmds\n");
        return 0;
    }
    fprintf(stderr,"wgrib2  ", cmd[i]);
    for (i = 1; i < n_cmds; i++) {
        fprintf(stderr,"%s ", cmds[i]);
    }
    fprintf(stderr,"\n");
    return 0;
}
