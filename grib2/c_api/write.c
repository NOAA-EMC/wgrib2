#include <stdio.h>
#include <stdlib.h>
#include "c_wgrib2api.h"
#define GRIDSIZE 1679

int main() {			/* simple program write grib2 file */
	float data[GRIDSIZE];
	char *template;
	int i;

	/* define data[0..npnts-1] */
        for (i = 0; i < GRIDSIZE; i++) data[i] = 10.0;
        data[0] = 0.0;
	template="merc.g2";

        i = grb2_wrt("new.grb", template, 2, data, GRIDSIZE,"lev", "201 mb","grib_type","s",
              "ftime", "0-1 hour ave fcst","var","UFLX","bin_prec",7,"date",20880112010259LL);

	printf("writing new.grb err=%d\n", i);
        return i;
}
