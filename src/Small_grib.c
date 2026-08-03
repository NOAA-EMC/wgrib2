/** @file
 * @brief Functions for creating small GRIB files.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 5/2008 | W. Ebisuzaki | Initial
 * 8/2011 | W. Ebisuzaki | added mercator, rotated lat-lon, redundant test for we:sn order
 * 1/2012 | W. Ebisuzaki | added Gaussian grid
 * @author Public Domain: Wesley Ebisuzaki @date 5/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
// #include <omp.h>

/** Decode grib file flag. */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Flush of output flag. */
extern int flush_mode;

/** Append grib file flag. */
extern int file_append;

/** Current output order type. */
extern enum output_order_type output_order;

/** Use scaling flag. */
extern int use_scale;

/** Decimal scaling. */
extern int dec_scale;

/** Binary scaling. */
extern int bin_scale;

/** Number of bits wanted. */
extern int wanted_bits;

/** Maximum number of bits. */
extern int max_bits;

/** Current output GRIB type. */
extern enum output_grib_type grib_type;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Number of points in the grid. */
extern int npts;

/** Number of grid points in the x-direction. */
extern int nx;

/** Number of grid points in the y-direction. */
extern int ny;

/** Scan mode of the grid. */
extern int scan;

/** Number of grid points in the x-direction (unsigned). */
extern unsigned int nx_;

/** Number of grid points in the y-direction (unsigned). */
extern unsigned int ny_;

static unsigned int idx(int ix, int iy, int nx, int ny, int cyclic_grid);

/*
 * HEADER:100:ijsmall_grib:output:3:make small domain grib file X=ix0:ix1 Y=iy0:iy1 Z=file
 */

/**
 * Writes the grid values to a grib2 file with the same grid spacing but a smaller domain. 
 * It is similar to the -small_grib option except it uses i,j values rather than lat-lon values. 
 * The grid point locations are unchanged. This option is used to make a regional subset and 
 * only works for certain grids such as the lat-lon, rotated lat-lon, Mercator and Lambert 
 * conformal. 
 * 
 * ## Limitations
 * 1. rotated lat-lon grid: must use WMO grid template and not the NCEP locally defined grid 
 * template
 * 2. staggered grids are not supported 
 * 
 * ## Usage
 * -ijsmall_grib ix0:ix1 iy0:iy1 file_name
 * 
 * Where:
 * file_name == output grib2 file
 * 1 <= ix0 < ix1 < nx
 * 1 <= iy0 < iy1 < ny
 * 
 * By default, (i,j) is the South-West corner.
 *
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 *
 * @author Wesley Ebisuzaki @date 5/2008
 */
int f_ijsmall_grib(ARG3) {

    struct local_struct {
        struct seq_file out;
        int ix0, iy0, ix1, iy1;
    };  
    struct local_struct *save; 

    if (mode == -1) {
        decode = latlon = 1;

        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("ijsmall_grib  memory allocation ","");

        if (sscanf(arg1,"%d:%d", &(save->ix0), &(save->ix1)) != 2) 
            fatal_error("ijsmall_grib: ix0:ix1 = %s?", arg1);
        if (sscanf(arg2,"%d:%d", &(save->iy0), &(save->iy1)) != 2) 
            fatal_error("ijsmall_grib: iy0:iy1 = %s?", arg2);
        if (fopen_file(&(save->out), arg3, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }
        if (save->iy0 <= 0) fatal_error_i("ijsmall_grib: iy0=%d <= 0", save->iy0);
        if (save->iy0 > save->iy1) fatal_error("ijsmall_grib: iy0 > iy1","");
        if (save->ix0 > save->ix1) fatal_error("ijsmall_grib: ix0 > ix1","");
    }
    else if (mode == -2) {
        save = (struct local_struct *) *local;
        fclose_file(&(save->out));
        free(save);
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
        if (output_order != wesn) fatal_error("ijsmall_grib: data must be in we:sn order","");
        if (GDS_Scan_staggered(scan)) fatal_error("ijsmall_grib: does not work for staggered grids","");
        if (nx_ == 0 || ny_ == 0) fatal_error("small_grib: does not work for thinned grids","");
        small_grib(sec,mode,data,lon,lat, ndata,save->ix0,save->ix1,save->iy0,save->iy1,&(save->out));
    }
    return 0;
}

/**
 * Index into a we:sn array(1:nx,1:ny)
 *
 * @param ix Index in x-direction (1 to nx).
 * @param iy Index in y-direction (1 to ny).
 * @param nx Number of grid points in x-direction.
 * @param ny Number of grid points in y-direction.
 * @param cyclic_grid Set to 1 for an array that is cyclic in longitude.
 * 
 * @return Index into the array (0 to nx*ny-1). Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2008
 */
static unsigned int idx(int ix, int iy, int nx, int ny, int cyclic_grid) {
    int i;
    
    if (iy <= 0) fatal_error("idx(..) iy <= 0","");
    if (iy > ny) fatal_error_i("idx(..) iy = %d",iy);

    i = ix-1;
    if (cyclic_grid) {
	if (i < 0) i = nx - ( (-i) % nx );
	i = i % nx;
    }
    else {
       if (ix <= 0) fatal_error_ii("idx(..) ix=%d <= 0, iy=%d",ix,iy);
       if (i >= nx) fatal_error_ii("idx(..) ix = %d, iy=%d",ix,iy);
    }

    return (unsigned int) (i + (iy-1)*nx);
}

/**
 * Makes a subset of certain grids. Must be in we:sn order.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 5/2008 | W. Ebisuzaki | Initial
 * 8/2011 | W. Ebisuzaki | added mercator and rotated lat-lon grid
 * 1/2012 | W. Ebisuzaki | added Gaussian grid
 * 
 * @param sec Pointer to the GRIB sections.
 * @param mode Processing mode (e.g., -1 for initialization, -2 for cleanup, 0 for processing).
 * @param data Pointer to array of values to encode into GRIB2.
 * @param lon Pointer to array of longitude values.
 * @param lat Pointer to array of latitude values.
 * @param ndata Number of data values.
 * @param ix0 Starting index in x-direction.
 * @param ix1 Ending index in x-direction.
 * @param iy0 Starting index in y-direction.
 * @param iy1 Ending index in y-direction.
 * @param out Pointer to the output file struct.
 *
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2008
 */
int small_grib(unsigned char **sec, int mode, float *data, double *lon, double *lat, unsigned int ndata,
	int ix0, int ix1, int iy0, int iy1, struct seq_file *out) {

    int can_subset, grid_template;
    int nx, ny, res, scan, new_nx, new_ny, i, j;
    unsigned int sec3_len, new_ndata, k, npnts;
    unsigned char *sec3, *new_sec[9];
    double units;
    int basic_ang, sub_ang, cyclic_grid;
    float *new_data;
    double *tmp_lat, *tmp_lon;

    get_nxny(sec, &nx, &ny, &npnts, &res, &scan);        /* get nx, ny, and scan mode of grid */
    grid_template = code_table_3_1(sec);

    // make a copy of the gds (sec3)
    sec3_len = GB2_Sec3_size(sec);
    sec3 = (unsigned char *) malloc(sec3_len);
    for (k = 0; k < sec3_len; k++) sec3[k] = sec[3][k];

    // make a copy of the sec[] with new sec3
    new_sec[0] = sec[0];
    new_sec[1] = sec[1];
    new_sec[2] = sec[2];
    new_sec[3] = sec3;
    new_sec[4] = sec[4];
    new_sec[5] = sec[5];
    new_sec[6] = sec[6];
    new_sec[7] = sec[7];
//    new_sec[8] = sec[8];  not needed by writing routines

    can_subset = 1;
    if (lat == NULL || lon == NULL) can_subset = 0;
    new_nx = ix1-ix0+1;
    new_ny = iy1-iy0+1;
    if (new_nx <= 0) fatal_error("small_grib, new_nx is <= 0","");
    if (new_ny <= 0) fatal_error("small_grib, new_ny is <= 0","");
    new_ndata = new_nx * new_ny;
    cyclic_grid = 0;

    if (can_subset) {
        cyclic_grid = cyclic(sec);

        // lat-lon grid - no thinning
        if ((grid_template == 0 && sec3_len == 72) || (grid_template == 1 && sec3_len == 84)) {
            if (grid_template == 1) {
                /* need *lat, and *lon in rotated coordinates */
                i =  regular2ll(sec, &tmp_lat, &tmp_lon);
                if (i != 0) fatal_error("small_grib: regulat2ll failed");
            }
            uint_char(new_nx,sec3+30);		// nx
            uint_char(new_ny,sec3+34);		// ny

            basic_ang = GDS_LatLon_basic_ang(sec3);
            sub_ang = GDS_LatLon_sub_ang(sec3);
            if (basic_ang != 0) {
                units = (double) basic_ang / (double) sub_ang;
            }
            else {
                units = 0.000001;
            }
            if (grid_template == 1) {
                i = tmp_lat[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;		// lat1
                j = tmp_lon[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;		// lon1
            } else {
                i = lat[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;		// lat1
                j = lon[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;		// lon1
            }
            int_char(i,sec3+46);
            int_char(j,sec3+50);

            if (grid_template == 1) {
                i = tmp_lat[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;		// lat2
                j = tmp_lon[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;		// lon2
            } else {
                i = lat[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;		// lat2
                j = lon[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;		// lon2
            }
            int_char(i,sec3+55);
            int_char(j,sec3+59);
            if (grid_template == 1) {
                free(tmp_lon);
                free(tmp_lat);
            }
        }

        else if ((grid_template == 40 && sec3_len == 72)) { // full Gaussian grid
            uint_char(new_nx,sec3+30);		// nx
            uint_char(new_ny,sec3+34);		// ny

            basic_ang = GDS_Gaussian_basic_ang(sec3);
            sub_ang = GDS_Gaussian_sub_ang(sec3);
            if (basic_ang != 0) {
                units = (double) basic_ang / (double) sub_ang;
            }
            else {
                units = 0.000001;
            }

            i = lat[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;          // lat1
            int_char(i,sec3+46);
            i = lon[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;          // lon1
            int_char(i,sec3+50);
            i = lat[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;          // lat2
            int_char(i,sec3+55);
            i = lon[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;          // lon2
            int_char(i,sec3+59);
        }

	// polar-stereo graphic, lambert conformal , no thinning
        else if ((grid_template == 20 && sec3_len == 65) || 		// polar stereographic
                    (grid_template == 30 && sec3_len == 81)) {	// lambert conformal
            uint_char(new_nx,sec3+30);		// nx
            uint_char(new_ny,sec3+34);		// ny

            i = (int) (lat[ idx(ix0,iy0,nx,ny,cyclic_grid) ] * 1000000.0);		// lat1
            int_char(i,sec3+38);
            i = (int) (lon[ idx(ix0,iy0,nx,ny,cyclic_grid) ] * 1000000.0);		// lon1
            int_char(i,sec3+42);
        }

        // mercator, no thinning
        else if (grid_template == 10 && sec3_len == 72) { 		// mercator

            uint_char(new_nx,sec3+30);		// nx
            uint_char(new_ny,sec3+34);		// ny

            units = 0.000001;
            i = lat[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;		// lat1
            int_char(i,sec3+38);
            i = lon[ idx(ix0,iy0,nx,ny,cyclic_grid) ] / units;		// lon1
            int_char(i,sec3+42);
            i = lat[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;		// lat2
            int_char(i,sec3+51);
            i = lon[ idx(ix1,iy1,nx,ny,cyclic_grid) ] / units;		// lon2
            int_char(i,sec3+55);
        }

        else {
            can_subset = 0;
        }
    }

    // copy data to a new array

    if (can_subset) {
        uint_char(new_ndata, sec3+6);
        new_data = (float *) malloc(sizeof(float) * (size_t) new_ndata);

#ifdef USE_OPENMP
#pragma omp parallel for private(i,j,k)
#endif
        for(j = iy0; j <= iy1; j++) {
            k = (j-iy0) * (size_t) (ix1-ix0+1);
            for(i = ix0; i <= ix1; i++) {
                new_data[(i-ix0) + k ] = data[ idx(i,j,nx,ny,cyclic_grid) ];
            }
        }
    }
    else {
        new_ndata = ndata;
        new_data = (float *) malloc(sizeof(float) * (size_t) new_ndata);
        for (k = 0; k < ndata; k++) new_data[k] = data[k];
        new_nx = nx;
        new_ny = ny;
    }

    set_order(new_sec, output_order);

    grib_wrt(new_sec, new_data, new_ndata, new_nx, new_ny, use_scale, dec_scale, 
            bin_scale, wanted_bits, max_bits, grib_type, out);

    if (flush_mode) fflush_file(out);

    free(new_data);
    free(sec3);
    return 0;
}

/*
 * HEADER:100:small_grib:output:3:make small domain grib file X=lonW:lonE Y=latS:latN Z=file
 */

/** GDS change number. */
extern int GDS_change_no;

/**
 * Writes the grid values to a grib2 file with the same grid spacing but a smaller domain. 
 * 
 * The grid point locations are unchanged. This option is used to make a regional subset and 
 * only works for certain grids such as the lat-lon, rotated lat-lon, Mercator, Lambert 
 * conformal, and polar stereographic. 
 * 
 * When -small_grib option has problems, the grid is not subsetted and the original grid is 
 * written. Some reasons for problems are:
 * 1. Unsupported grid type
 * 2. Thinned grid
 * 3. The bounding box is outside of the grid domain
 * 4. The bounding box is too small and does not include a grid point
 * 5. The bounding box definition is not right (ex. LatN < LatS) 
 * 
 * The -ijsmall_grib option is similar to the -small_grib option except it uses the grid coordinates. 
 * The former is faster as it doesn't have to compute the latitudes and longitudes of the grid points 
 * and find a bounding box. 
 * 
 * There are other ways of making a grib file which only includes the data for a subregion. You can 
 * set the points outside of your region of interest to UNDEFINED using the -undefine or -ijundefine 
 * options. Once the grid points are set to UNDEFINED, many of the packing methods will reduce the 
 * size of the new grib file. (Complex packing without bitmaps is very good.) One can also interpolate 
 * the field to a new grid using the -new_grid option. 
 * 
 * 
 * ## Limitations
 * 1. rotated lat-lon grids: must use WMO grid template and not the NCEP locally defined template
 * 2. rotated lat-lon grids: lat and lon values must be in the rotated coordinates
 * 3. staggered grids are not supported
 * 
 * ## Usage
 * -small_grib LonW:LonE LatS:LatN file_name
 * 
 * For west longitudes and south latitudes, you can use negative values. The file_name is the output file. 
 * LonE must have a numerical value greater than LonW. For example for left boundary=20W and the right 
 * boundary=60E, you can use LonE=340 and LonW=420. You can also use LonE=-20 and LonW=60. 
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 fcst.grb2 -small_grib 10:20 -20:20 small.grb
 * 1:0:d=2007032600:HGT:1000 mb:anl:
 * @endcode
 * 
 * This writes a small grib file from 10E-20E and 20S-20N. Often you want to preserve the grib compression. 
 * In this case, you would add the option `-set_grib_type same` to the wgrib2 command line. 
 * 
 * @code{.sh}
 * $ wgrib2 small.grb -grid
 * 1:0:grid_template=0:
 *       lat-lon grid:(21 x 81) units 1e-06 input WE:SN output WE:SN res 48
 *       lat -20.000000 to 20.000000 by 0.500000
 *       lon 10.000000 to 20.000000 by 0.500000 #points=1701
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 5/2008
 */
int f_small_grib(ARG3) {

    struct local_struct {
        struct seq_file out;
        double lonE, lonW, latS, latN;
        int GDS_change_no;
        int ix0, ix1, iy0, iy1;
    };  
    struct local_struct *save; 

    if (mode == -1) {
        decode = latlon = 1;

        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("small_grib  memory allocation ","");

        save->GDS_change_no = 0;
        save->ix0 = save->ix1 = save->iy0 = save->iy1 = 0;

        if (sscanf(arg1,"%lf:%lf", &(save->lonW), &(save->lonE)) != 2) 
            fatal_error("small_grib: lonW:lonE = %s?", arg1);
        if (sscanf(arg2,"%lf:%lf", &(save->latS), &(save->latN)) != 2) 
            fatal_error("small_grib: latS:latN = %s?", arg2);
        if (fopen_file(&(save->out), arg3, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }
        if (save->latS > save->latN) fatal_error("small_grib: latS > latN","");
        if (save->lonW > save->lonE) fatal_error("small_grib: lonW > lonE","");

    }
    else if (mode == -2) {
        save = (struct local_struct *) *local;
        fclose_file(&(save->out));
        free(save);
        return 0;
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
        if (GDS_Scan_staggered(scan)) fatal_error("small_grib: does not work for staggered grids","");

        if (GDS_change_no != save->GDS_change_no) {
            small_domain(sec, save->lonW,save->lonE,save->latS,save->latN,
            &(save->ix0), &(save->ix1), &(save->iy0), &(save->iy1));
            save->GDS_change_no = GDS_change_no;
        }

        if (output_order != wesn) fatal_error("small_grib: data must be in we:sn order","");
        if (nx_ == 0 || ny_ == 0) fatal_error("small_grib: does not work for thinned grids","");
        small_grib(sec,mode,data,lon,lat, ndata,save->ix0,save->ix1,save->iy0,save->iy1,&(save->out));
    }
    return 0;
}

/*
 * finds smallest rectangular domain for a set of lat-lon grid points
 *
 * 5/2017: for lat-lon and mercator grids: this code finds the bigest grid(i0:i1,j0:j1) that
 *         will fit within the lat/lon specifications
 *
 *         for any other grid: it will find a grid(i0:i1,j0:j1) that is smaller
 *         the code is has a mistake in the the selection
 *         one could fix the code but that would cause problems for the users as
 *         the output grid would change.
 *
 * assumes that thinned grids are not passed to small_domain(..)
 */

/**
 * Finds smallest rectangular domain for a set of lat-lon grid points.
 * 
 * For lat-lon and mercator grids: this code finds the biggest grid(i0:i1,j0:j1) that will fit 
 * within the lat/lon specifications.
 *
 * For any other grid: it will find a grid(i0:i1,j0:j1) that is smaller.
 *
 * This routine assumes that thinned grids are not passed to small_domain(..).
 *
 * @param sec Pointer to GRIB sections.
 * @param lonW Western longitude boundary.
 * @param lonE Eastern longitude boundary.
 * @param latS Southern latitude boundary.
 * @param latN Northern latitude boundary.
 * @param ix0 Pointer to the variable for the x0 index.
 * @param ix1 Pointer to the variable for the x1 index.
 * @param iy0 Pointer to the variable for the y0 index.
 * @param iy1 Pointer to the variable for the y1 index.
 *
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 5/2008
 */
int small_domain(unsigned char **sec, double lonW, double lonE, double latS, double latN,
       int *ix0, int *ix1, int *iy0, int *iy1) {

    int i, j, k, flag, x0, x1, y0, y1;
    int X0, X1, Y0, Y1, flag0;
    int gdt, flag1;
    double e,w,n,s;
    double lat_pt, lon_pt;
//     double time0, time1;

#ifdef DEBUG
printf("\n>> small_domain: lon lat %f:%f %f:%f\n", lonW, lonE, latS, latN);
#endif

    if (GDS_Scan_staggered(scan)) fatal_error("small_domain: does not work for staggered grids","");

    if (lat == NULL || lon == NULL) {		// no lat-lon information return full grid
        *ix0 = 1;
        *ix1 = nx;
        *iy0 = 1;
        *iy1 = ny;
        return 1;
    }

    if (lonE < lonW) lonE += 360.0;
    if (lonE-lonW > 360.0) fatal_error("small_domain: longitude range is greater than 360 degrees","");

    if (lonW < 0.0) { lonW += 360.0; lonE += 360.0; }

#ifdef DEBUG
printf("\n>> small_domain: new lon lat %f:%f %f:%f\n", lonW, lonE, latS, latN);
printf(">> small_domain: nx %d ny %d\n", nx, ny);
#endif

    /* for latlon, mercator grid, only need to scan axis for X0,X1,Y0,Y1 */
    gdt = code_table_3_1(sec);
//    fprintf(stderr,"gdt= %d\n", gdt);

    if ( (gdt == 0 || gdt == 10) && nx > 1 && ny > 1) {		// already checked for thinned grids
        flag0 = flag1 = 0;
        X0 = 1;
        X1 = nx;
        Y0 = 1;
        Y1 = ny;
        w = e = s = n = -1;
//        time0 = omp_get_wtime();
        for (i = 1; i <= nx; i++) {
            lon_pt = lon[i-1];
            if (lon_pt < lonW) lon_pt += 360.0;
            if (lon_pt < lonW) lon_pt += 360.0;
            // lon_pt  > lonW
            if (lon_pt <= lonE) {
                if (flag0 == 0) {
                    X0 = X1 = i;
                    w = e = lon_pt;
                    flag0 = 1;
                }
                if (lon_pt > e) {
                    e = lon_pt;
                    X1 = i;
                }
                if (lon_pt < w) {
                    w = lon_pt;
                    X0 = i;
                }
            }
        }
        for (j = 1; j <= ny; j++) {
            lat_pt = lat[(j-1)*nx];
            if ((lat_pt >= latS) && (lat_pt <= latN)) {
                if (flag1 == 0) {
                    Y0 = Y1 = j;
                    n = s = lat_pt;
                    flag1 = 1;
                }
                if (lat_pt < s) {
                    s = lat_pt;
                    Y0 = j;
                }
                if (lat_pt > n) {
                    n = lat_pt;
                    Y1 = j;
                }
            }
        }
        if (X1 < X0 && cyclic(sec)) X1 += nx;

//        time1 = omp_get_wtime();
//        fprintf(stderr,"small_domain fast time=%lf %d %d %d %d flags=%d %d\n", time1-time0, X0, X1, Y0, Y1, flag0, flag1);

        if (flag0 == 1 && flag1 == 1) {
            *ix0 = X0;
            *ix1 = X1;
            *iy0 = Y0;
            *iy1 = Y1;
            return 0;
        }
        else {
            *ix0 = 1;
            *ix1 = nx;
            *iy0 = 1;
            *iy1 = ny;
            return 1;
        }
    }

    flag0 = 0;					// initial point on grid
    X0 = 1;
    X1 = nx;
    Y0 = 1;
    Y1 = ny;

//    time0 = omp_get_wtime();
#ifdef USE_OPENMP
#pragma omp parallel for private (i,j,k,flag,x0,x1,y0,y1,w,e,n,s,lat_pt,lon_pt)
#endif
    for (j = 1; j <= ny; j++) {
        x0 = x1 = y0 = y1 = w = e = s = n = -1;
        flag = 0;				// initial point on latitude
        for (i = 1; i <= nx; i++) {
            k = (i-1) + (j-1)*nx;
            lon_pt = lon[k];
            lat_pt = lat[k];
            if (lon_pt < lonW) lon_pt += 360.0;
            if (lon_pt < lonW) lon_pt += 360.0;

            // lon_pt  > lonW
            if ( (lon_pt <= lonE) && (lat_pt >= latS) && (lat_pt <= latN)) {
                if (flag == 0) {
                    x0 = x1 = i;
                    y0 = y1 = j;
                    w = e = lon_pt;
                    n = s = lat_pt;
                    flag = 1;
                }
                if (lat_pt < s) {
                    s = lat_pt;
                    y0 = j;
                }
                else if (lat_pt > n) {
                    n = lat_pt;
                    y1 = j;
                }
                if (lon_pt > e) {
                    e = lon_pt;
                    x1 = i;
                }
                if (lon_pt < w) {
                    w = lon_pt;
                    x0 = i;
                }
            }
        }
        if (flag) {		// found points
            if (x1 < x0 && cyclic(sec)) x1 += nx;
#ifdef USE_OPENMP
#pragma omp critical
#endif
            {
                if (flag0 ==  0) {
                    X0 = x0;
                    X1 = x1;
                    Y0 = y0;
                    Y1 = y1;
                    flag0 = 1;
                }
                else {
                    X0 = (x0 < X0) ? x0 : X0;
                    X1 = (x1 > X1) ? x1 : X1;
                    Y0 = (y0 < Y0) ? y0 : Y0;
                    Y1 = (y1 > Y1) ? y1 : Y1;
                }
            }
        }
    }
//    time1 = omp_get_wtime();
//    fprintf(stderr,"small_domain slow time=%lf %d %d %d %d flag0 %d\n", time1-time0, X0, X1, Y0, Y1, flag0);

#ifdef DEBUG
printf(">> small domain: flag0 %d flag %d\n", flag0, flag);
#endif
    if (flag0 && X1 < X0)  flag0 = 0;
    if (flag0 == 0) {
        *ix0 = 1;
        *ix1 = nx;
        *iy0 = 1;
        *iy1 = ny;
        return 1;
    }
#ifdef DEBUG
printf(">> small domain: ix %d:%d iy %d:%d\n", X0, X1, Y0, Y1);
#endif
    *ix0 = X0;
    *ix1 = X1;
    *iy0 = Y0;
    *iy1 = Y1;
    return 0;
}
