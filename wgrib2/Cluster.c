#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Cluster.c    10/2024  Public Domain  Wesley Ebisuzaki
 *
 * options for showing cluster information
 *
 */

/*
 * HEADER:-1:cluster:inv:0:cluster identifier
 */

int f_cluster(ARG0) {
    int i;

    if (mode >= 0) {
        i = cluster_identifier(sec);
        if (i >= 0) sprintf(inv_out,"cluster=%d", i);
    }
    return 0;
}

/*
 * HEADER:-1:N_clusters:inv:0:number of clusters
 */


int f_N_clusters(ARG0) {
    int i;

    if (mode >= 0) {
        i = number_of_clusters(sec);
        if (i >= 0) sprintf(inv_out,"N_clusters=%d", i != 255 ? i : -1);
    }
    return 0;
}

/*
 * HEADER:-1:cluster_info:inv:0:cluster information
 */
extern char *nl;
int f_cluster_info(ARG0) {

    unsigned char *nc, *cluster;

    if (mode < 0) return 0;
    cluster = cluster_identifier_location(sec);
    if (cluster == NULL) return 0; // not a cluster
    nc = number_of_forecasts_in_the_cluster_location(sec);
    if (nc == NULL) return 0;

    sprintf(inv_out,"%scluster=%d",nl, cluster[0] != 255 ? cluster[0]: -1);
    inv_out += strlen(inv_out);

    if (cluster[1] != 255) sprintf(inv_out,"%scluster_with high-res_ctl=%d",nl,cluster[1]);
    inv_out += strlen(inv_out);

    if (cluster[2] != 255) sprintf(inv_out,"%scluster_with_low-res_ctl=%d",nl,cluster[2]);
    inv_out += strlen(inv_out);

    sprintf(inv_out,"%sN_forecasts_in_cluster=%d", nl, *nc != 255 ? *nc : -1);
    inv_out += strlen(inv_out);

    if (nc[1] != 255 || nc[2] != 255 || nc[3] != 255 || nc[4] != 255 || nc[5] != 255) {
	sprintf(inv_out, "%sstd_dev_cluster_members=%lf", nl, scaled2flt(INT1(nc[1]), int4(nc+2)));
        inv_out += strlen(inv_out);
    }
    if (nc[6] != 255 || nc[7] != 255 || nc[8] != 255 || nc[9] != 255 || nc[10] != 255) {
	sprintf(inv_out, "%sdist_cluster_to_e.m.=%lf", nl, scaled2flt(INT1(nc[6]), int4(nc+7)));
        inv_out += strlen(inv_out);
    }
    inv_out += strlen(inv_out);

    return 0;
}
