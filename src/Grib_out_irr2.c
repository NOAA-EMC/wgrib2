/** @file
 * @brief Write irregular grid GRIB messages using grid definition template 101.
 * @author Public Domain: Wesley Ebisuzaki @date 7/2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Flush of output flag. */
extern int flush_mode;

/** Current output order type. */
extern enum output_order_type output_order;

/** Use scaling flag. */
extern int use_scale;

/** Decimal scaling. */
extern int dec_scale;

/** Binary scaling. */
extern int bin_scale;

/** Maximum number of bits. */
extern int max_bits;

/** Number of bits wanted. */
extern int wanted_bits;

/** Current output grib type. */
extern enum output_grib_type grib_type;

/*
 * HEADER:100:grib_out_irr2:output:5:writes irregular grid grib  GDT 101 X=npnts Y=grid_no Z=grid_ref A=UUID B=(output file)
 */

/**
 * The option -grib_out_irr2 replaces -grib_out_irr. The former uses Grid Definition Template (GDT) 
 * 101 which is part of the grib standard. The latter uses GDT 130 which was was not adopted. The 
 * -grib_out_irr2 allows you to define an unstructured grids which allows the locations of the grid 
 * points to be arbitrary. For example, you can define an unstructured grid to be the locations of 
 * the all the US weather observing stations. One neat feature of GDT 101 is the grid number is a 
 * UUID (universally unique identifier) which can be generated on the fly and is not "registered" 
 * with the center or WMO. This feature, for example, will allow you to generate a grid for all the 
 * ship observations at 00Z January 1, 2017 and a different grid for any other observation time. 
 * With the UUID feature of GDT 101, one can store observational data in grib. 
 * 
 * The locations of are not part of the metadata in grib message, and the locations have to be provided 
 * another means such as documentation at the center's web site or by including grib mesages with the 
 * latitudes (NLAT) ane longitudes (ELON). 
 * 
 * ## Is GDT 101 Useful?
 * GDT 101 is useful! I can define a UUID, and make a grib files with 3 grib messages, NLAT, ELON, TMP2m. 
 * Using wgrib2, I can interpolate TMP2m to a lat-lon grid. 
 * 
 * I have a netcdf file with latitude(x,y), longitude(x,y) and TMP2m(t,latitude,longitude). Using 
 * -import_netcdf, I can make grib message with ELON, NLAT and TMP2m. I can now interpolate TMP2m to a 
 * lat-lon grid. 
 * 
 * ## Usage
 * -grib_out_irr2 NPNTS CENTER_GRID_NUBER REF_GRID_TYPE UUID OUTFILE
 * 
 * NPNTS              = number of grid points, can be differ from the size of the input grid
 * CENTER_GRID_NUMBER = use -1 unless your center has defined an appropriate grid number
 * REF_GRID_TYPE      = use -1 unless your center has defined an appropriate reference grid type
 * UUID               = universally unique identifier, use uuidgen to create a new UUID (0 for no UUID)
 * OUTFILE            = output grib file
 * 
 * The option, -grib_out_irr2, can generate a grid with any number of grid points. The data for the new grid 
 * is taken from the DATA register which is usually the input data. If NPNTS is less than the size of the data 
 * register (NDATA), then the first NPNTS of DATA are written out. If NPNTS is greater or equal to NDATA, the 
 * DATA is written out and any extra points are set to undefined. For both cases, there is no attempt to remap 
 * the data by finding the nearest grid point, etc. (At this point, the latitudes and longitudes have not been 
 * specified.) 
 * 
 * @param ARG5 List of function arguments set by wgrib2's main() function (see @ref ARG5). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @note Data size of input and output do not have to match to allow people to write out GDT 101 unstructured grids.
 * 
 * @author Wesley Ebisuzaki @date 7/2017
 */
int f_grib_out_irr2(ARG5) {

    float *data_tmp;
    int table_3_2;
    unsigned int i, n;
    struct seq_file *save;
    unsigned char gdt101[35], *g, *old_gds;

    if (mode == -1) {
        decode = 1;
        *local = save = (struct seq_file *) malloc(sizeof(struct seq_file));
        if (save == NULL) fatal_error("grib_out_irr: memory allocation","");
        if (fopen_file(save, arg5, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("grib_out_irr2: Could not open %s", arg3);
        }
        return 0;
    }

    save = (struct seq_file *) *local;
    if (mode == -2) {
        fclose_file(save);
        free(save);
        return 0;
    }

    n = (unsigned int) strtoul(arg1,NULL,10);

    g = &(gdt101[0]);
    uint_char(35, g);				/* length of Section 3 */
    g += 4;
    *g++ = 3;					/* section 3 */
    *g++ = 0;					/* source of grid definition */
    uint_char(n, g);				/* number of data points */
    g += 4;
    *g++ = 0;
    *g++ = 0;					/* table 3.11 */
    uint2_char(101,g);				/* template number */
    g += 2;

    table_3_2 = code_table_3_2(sec);
    if (table_3_2 == 1 || table_3_2 == 3 || table_3_2 == 7) 
        fatal_error("grib_out_irr2: user-defined earth shape is not supported by GDT 101","");
    *g++ = (unsigned char) table_3_2;

    i = atoi(arg2);				/* grid number */
    *g++ = (i >> 16) & 255;
    *g++ = (i >> 8) & 255;
    *g++ = i & 255;

    i = atoi(arg3);
    *g++ = (unsigned char) i;			/* number of grid in reference */

    /* UUID */
    if (strcmp(arg4,"0") == 0) {		/* nill UUID */
        for (i = 0; i < 16; i++) *g++ = 0;
    }
    else {
        i = sscanf(arg4, "%2hhx%2hhx%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
            g+0, g+1, g+2, g+3, g+4, g+5, g+6, g+7, g+8, g+9, g+10, g+11, g+12, g+13, g+14, g+15);
        if (i != 16) fatal_error("grib_out_irr2: bad uuid %s",arg4);
        g += 16;
    }

    data_tmp = (float *) malloc(n  * sizeof(float));
    if (data_tmp == NULL) fatal_error("grib_out_irr2: memory allocation","");
    if (ndata != n) fprintf(stderr,"grib_out_irr2: Warning data size=%u, requested size=%u\n", ndata, n);
    if (ndata >= n) {
        for (i = 0; i < n; i++) data_tmp[i] = data[i];
    }
    else {
        for (i = 0; i < ndata; i++) data_tmp[i] = data[i];
        for (i = ndata; i < n; i++) data_tmp[i] = UNDEFINED;
    }

    old_gds = sec[3];
    sec[3] = &(gdt101[0]);
    grib_wrt(sec, data_tmp, n, n, 1, use_scale, dec_scale, 
            bin_scale, wanted_bits, max_bits, grib_type, save);
    sec[3] = old_gds;
    if (flush_mode) fflush_file(save);

    free(data_tmp);
    return 0;
}
