#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "c_wgrib2api.h"

int main() {
        char *grb, *inv;
        float *data, *lat, *lon;
        int i, ndata;
        float new_data[1679];

        grb = "../examples/merc.g2";
        inv = "merc.inv";
        i = grb2_mk_inv(grb,inv);
        printf("mk_inv=  i=%i\n",i);
        ndata = grb2_inq(grb,inv,DATA|LATLON,"TMP");
        printf("ndata=%i\n",ndata);

        data = (float *) malloc(ndata*4);
        lat = (float *) malloc(ndata*4);
        lon = (float *) malloc(ndata*4);
        i= grb2_get_data(data,ndata);
        printf("err=%d data[0[=%f\n", i, data[0]);
        i= grb2_get_lonlat(lon,lat,ndata);
        printf("err=%d lon/lat[0[=%f %f\n", i, lon[0],lat[0]);

        ndata=1679;
        for (i = 0; i < 1679; i++) data[i] = 10.0;
        data[0] = 0.0;
        fprintf(stderr,"now the grb2_wrt section\n");

        grb2_wrt("new.grb", grb, 2, data, (unsigned int) ndata,"lev", "201 mb","grib_type","s"
              ,"ftime", "0-1 hour ave fcst","var","UFLX","bin_prec",7,"date",20880112010259LL);

        data[0] = 2.0;
        grb2_wrt("new.grb", grb, 2, data, (unsigned int) ndata,"lev", "2 m above ground","set","center",255,
              "grib_type","a","ftime", "10 sec fcst","var","VFLX","bin_prec",7,"percentile",50);


        return 0;
}

