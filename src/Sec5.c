/** @file
 * @brief Some routines that involve Section 5 (Data Representation Section).
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 */
#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:400:Sec5:inv:0:Sec 5 values (Data representation section)
 */

/** Prints a short summary of Section 5, the Data Representation section. 
 * 
 * ## Usage
 * -Sec5
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 png.grb2 -Sec5
 * 1:4:Sec5 len=21 #defined data points=65160 Data Repr. Template=5.41
 * @endcode
 * 
 * <pre>
 * len=21                       Section 5 is 21 octets/bytes in length
 * #defined data points=65160   Number of data points with values present in Section 7
 * Data Repr. Template=5.41     Data Represenation Template is 5.41
 * </pre>
 * @author Wesley Ebisuzaki @date 2006
 */
int f_Sec5(ARG0) {
    if (mode >= 0) {
        sprintf(inv_out,"Sec5 len=%u #defined data points=%u Data Repr. Template=5.%u",
                uint4(&(sec[5][0])), uint4(&(sec[5][5])), uint2(&(sec[5][9])));
    }
    return 0;
}

/*
 * HEADER:500:npts:inv:0:number of grid points
 */

/** Prints the number of grid points in the grid. The total include both defined and undefined 
 * grid points. 
 * 
 * ## Usage
 * -npts
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 test.grb2 -s -npts -d 1
 * 1:0:d=2005090200:HGT:1000 mb:60 hour fcst:npts=259920
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_npts(ARG0) {
    if (mode >= 0) {
        sprintf(inv_out,"npts=%u", GB2_Sec3_npts(sec));
    }
    return 0;
}

/*
 * HEADER:510:packing:inv:0:shows the packing mode (use -v for more details)
 */

/**
 * Prints the packing mode (use -v for more details).
 * 
 * The grib format is for storing gridded data. Usually gridded data is stored as an scaled 
 * integer that has been packed or compressed. Exceptions are spectral data and tabular data 
 * (data are entries to a table). 
 * 
 * The -packing option shows how the gridded data was packed. The most common packing methods 
 * are jpeg2000, one of the various complex packing schemes. If you increase the verbosity by 
 * -v -packing, you see the packing method, scaling factors and range of possible integers. 
 * 
 * ## Usage
 * -packing
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 png.grb2 -packing
 * 1:4:packing=grid point data - png compression,_
 * $ wgrib2 png.grb2 -v -packing
 * 1:4:packing=grid point data - png compression,_ val=(0+i*2^0)*10^-1, i=0..65535 (#bits=16)
 * @endcode
 * 
 * The file, png.grb2, was packed using the obsolete png compression. The grid point values can have the 
 * values (0+i*2^0)*10^-1 where i is an integer that ranges from 0..65535.
 * 
 * @code{.sh}
 * $ wgrib2 small.grb2 -packing
 * 1:0:packing=grid point data - simple packing,s
 * $ wgrib2 small.grb2 -v -packing
 * 1:0:packing=grid point data - simple packing,s val=(1.22666e+06+i*2^2)*10^-2, i=0..4095 (#bits=12)
 * @endcode
 * 
 * The file small.grb2 is using simple packing, integers are stored using 12 bits (#bits=12). The grid points 
 * can have values of (1.22666e+06+i*2^2)*10^-2 where i is an integer that ranges from 0..4095.
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_packing(ARG0) {

    unsigned char *p;
    const char *string;
    int pack, i;
    float missing1, missing2;

    if (mode >= 0) {
        p = sec[5];
        pack = code_table_5_0(sec);

        if (mode >= 0) {
            string = NULL;
            switch(pack) {
#include "CodeTable_5.0.dat"
            }
            if (string == NULL) {
                if (pack == 40000) string = "grid point data - JPEG2000";
                if (pack == 40010) string = "grid point data - PNG";
                if (pack == 255) string = "missing";
            }
            if (string == NULL) string="unknown packing";
            sprintf(inv_out,"packing=%s", string);
            inv_out += strlen(inv_out);

            if (pack == 0) sprintf(inv_out,",s");
            else if (pack == 2 || pack == 3) {
                if (pack == 2) sprintf(inv_out,",c1");
                if (pack == 3) sprintf(inv_out,",c%d", code_table_5_6(sec)+1);
                if (GB2_Sec6_size(sec) > 6) {
                    inv_out += strlen(inv_out);
                    sprintf(inv_out,"b");
                }
            }
            else if (pack == 40) sprintf(inv_out,",j");
            else if (pack == 42) sprintf(inv_out,",a");
            else sprintf(inv_out,",_");
            inv_out += strlen(inv_out);
        }
        if (mode > 0) {
            if (pack == 0 || pack == 1 || pack == 2 || pack == 3 || pack == 40 || pack == 40000 || 
                    pack == 41 || pack == 50 || pack == 40010) {
                if (pack != 2 && pack != 3) {
                    sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d)", 
                            ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19]);
                    inv_out += strlen(inv_out);
                }
                if (pack == 2 || pack == 3) {
                    sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, ref=0..%d (#bits=%d) group width bits=%d", 
                            ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19],p[36]);
                    inv_out += strlen(inv_out);
                    sprintf(inv_out," #groups=%d", uint4(p+31));
                    inv_out += strlen(inv_out);
                    i = sub_missing_values(sec, &missing1, &missing2);
                    if (i > 0) {
                        sprintf(inv_out," missing1=%g", missing1);
                        inv_out += strlen(inv_out);
                        if (i == 2) {
                            sprintf(inv_out," missing2=%g", missing2);
                            inv_out += strlen(inv_out);
                        }
                    }
                }
            }
            else if (pack == 4) {
                sprintf(inv_out," precision code=%u", p[11]);
            }
            else if (pack == 42) {
                sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d)",
                        ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19]);
                inv_out += strlen(inv_out);
                sprintf(inv_out," compression options mask=%d samples/block=%d reference sample interval=%u",
                        (int) p[21], (int) p[22], uint2(p+23) );
                inv_out += strlen(inv_out);
            }
            else if (pack == 51) {
                sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d)", 
                        ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19]);
                inv_out += strlen(inv_out);
                sprintf(inv_out," P-Laplacian scaling factor*10^-6=%d",int4(p+20));
                inv_out += strlen(inv_out);
                sprintf(inv_out," Js=%u Ks=%u Ms=%u Ts=%d", uint2(p+24), uint2(p+26), uint2(p+28), 
                        int4(p+30));
                inv_out += strlen(inv_out);
                sprintf(inv_out," code_table_5.7=%d", (int) p[34]);
/*		sprintf(inv_out," mean?=%lg", ieee2flt(sec[7]+5)); */
            }
            else if (pack == 61) {
                sprintf(inv_out," val=(%lg+i*2^%d)*10^%d, i=0..%d (#bits=%d) pre-processing parameter=%lg", 
                ieee2flt(p+11), int2(p+15), -int2(p+17), (1 << p[19])-1, p[19], ieee2flt(p+20));
            }
        }
    }
    return 0;
}
