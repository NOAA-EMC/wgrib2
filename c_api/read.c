#include <stdio.h>
#include <stdlib.h>
#include "c_wgrib2api.h"

/* 10/2024 Public Domain  Wesley Ebisuzaki */

int main() {                  /* simple program to read grib2 */
    long long int ndata;
    int ierr;
    float *data, *lat, *lon;
    char meta[200], gridmeta[1000];

    ierr = grb2_mk_inv("merc.g2", "merc.inv");     /* make inv/index file */
    if (ierr) exit(1);

    /* inquire - using 3 wgrib2 match strings, read data,lat,lon,metadata,gridmetadata */
    ndata = grb2_inq("merc.g2", "merc.inv", DATA|LATLON|META|GRIDMETA,
          ":TMP:",":d=2014050600:",":surface:");
    if (ndata <= 0) exit(2);

    /* allocate data */
    data = (float *) malloc(ndata * sizeof(float));
    lat = (float *) malloc(ndata * sizeof(float));
    lon = (float *) malloc(ndata * sizeof(float));
    if (data == NULL | lat == NULL || lon == NULL) exit(3);

    ierr = grb2_get_data(data,ndata);		/* get grid */
    if (ierr) exit(4);
    ierr = grb2_get_lonlat(lon,lat,ndata);	/* get lat-lon */
    if (ierr) exit(5);
    ierr = grb2_get_meta(meta, sizeof(meta));	/* get metadata */
    if (ierr) exit(6);
    ierr = grb2_get_gridmeta(gridmeta, sizeof(gridmeta));	/*get grid info */
    if (ierr) exit(7);

    printf("That was easy: data[0[=%lf, lon[0]=%lf lat[0]=%lf ndata=%d\n",
      data[0],lon[0],lat[0],ndata);
    printf("meta=%s\n", meta);
    printf("gridmeta=%s\n", gridmeta);
    exit(0);
}
