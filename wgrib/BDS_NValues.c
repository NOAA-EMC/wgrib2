#include <stdio.h>
#include "bds.h"
#include "grib.h"

/*
 * support for complex packing
 *  determine the number of data points in the BDS
 *  does not handle matrix values
 */

extern int ec_large_grib,  len_ec_bds;


int BDS_NValues(unsigned char *bds) {

    /* returns number of grid points as determined from the BDS */

    int i = 0;

    if (BDS_SimplePacking(bds) && BDS_Grid(bds)) {
	i = ((BDS_LEN(bds) - BDS_DataStart(bds))*8 -
		BDS_UnusedBits(bds)) / (BDS_NumBits(bds));
    }
    else if (BDS_ComplexPacking(bds) && BDS_Grid(bds)) {
	i = BDS_P2(bds);
    }
    return i;
}
