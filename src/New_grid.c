/** @files
 * @brief Interpolation routines using ipolates library.
 * 
 * input = winds are N/S or grid
 * output = winds are N/S or grid  depending on wind_rotation
 *
 * To add new grids:
 *      input grids: add to mk_kdgs.c
 *      output grids: add to sec3_grids
 *                    add code to mode == -1 to parse grid specifications
 *
 * To add types to vector fields definition:
 *      modify source code: vectors[]
 *
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 6/2010 | W. Ebisuzaki | Initial
 * 5/2018 | W. Ebisuzaki | USE_IPOLATES == 0   disable
 *                         USE_IPOLATES == 1   grib1 version of ipolates
 *                         USE_IPOLATES == 3   grib2 version of ipolates (double)
 *                          in ip2 code, use  dlon = mod( (angle + 3600 - 1), 360) + 1
 *                              this makes dlon only xxx.xx precision whic is less
 *                              than grib1 (xxx.xxx) and grib2 (xxx.xxxxxx) expectations
 *                              should not use float version of ip2.
 * 4/2024 | W. Ebisuzaki | changed to current ip lib
 *                         removed calls to old ip lib (1)
 *
 * @author Public Domain: Wesley Ebisuzaki @date 6/2010
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Vector fields. */
const char **vectors;

/** Default vector fields. */
const char *default_vectors[] = {"UGRD", "VGRD", "VUCSH", "VVCSH","UFLX", "VFLX",
    "UGUST","VGUST","USTM","VSTM","VDFUA", "VDFVA", "MAXUW", "MAXVW",
    "UOGRD","VOGRD", "UICE", "VICE", "U-GWD", "V-GWD", "USSD", "VSSD", NULL };

/** Desired grid format. */
enum new_grid_format_type new_grid_format;

/* new_grid_vectors can be called submsg_uv, should not require ipolates to be installed */

/** No vector fields. */
static const char *no_vectors[] = { NULL };

/** UV vector fields. */
static const char *UV_vectors[] = { "UGRD", "VGRD", NULL };

#ifdef USE_IPOLATES

/*
 * HEADER:111:new_grid_vectors:misc:1:change fields to vector interpolate: X=none,default,UGRD:VGRD,(U:V list)
 */

/** 
 * Selects the wind rotation/orientation for the -new_grid option.
 * 
 * This orientation should apply to all vector quantities. However, the grib format doesn't have 
 * a flag specifying which fields are vector fields. Sometimes you like to treat vector quanties 
 * as scalars because it makes interpolation easier (for example a time series of tropical U fields). 
 * Another complication is that one often wants to duplicate the default action of copygb which is 
 * to only treat U and V as vectors. 
 * 
 * The -new_grid_vectors option allows you to select which fields will be interpolated as vectors. 
 * The options are, 
 * 
 * 1. none 
 *      All fields are interpolated as scalars
 * 2. UGRD:VGRD 
 *      Only UGRD and VGRD are interpolated as vectors (like default copygbfault behavior)
 * 3. default 
 *      UGRD,VGRD,UVCSH,VVCSH,UFLX,VFLX,UGUST, VGUST,USTM,VSTM,VDFUA,VDFVA,UOGRD,VOGRD,MAXUW,MAXVW,UICE,
 *      VICE,U-GWD,V-GWD,USSD,VSSD are interpolated as vectors
 * 
 * ## Usage
 * -new_grid_vectors X
 * 
 * X = none, default, UGRD:VGRD, UV list
 * 
 * Example UV list: "UGRD:VGRD:UICE:VICE"
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 6/2010
 */
int f_new_grid_vectors(ARG1) {

    int i, n;
    const char *from;
    char *to;

    struct local_struct {
        char *buff;
        const char **uv_vectors;
    };
    struct local_struct *save;

    if (mode == -1) {
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("new_grid_vectors: memory allocation","");
        save->buff = NULL;
        save->uv_vectors = NULL;

        if (strcmp(arg1,"none") == 0)  {
            save->uv_vectors = vectors = (const char **) no_vectors;
            return 0;
        }
        if (strcmp(arg1,"default") == 0)  {
            save->uv_vectors = vectors = (const char **) default_vectors;
            return 0;
        }
        if (strcmp(arg1,"UGRD:VGRD") == 0)  {
            save->uv_vectors = (const char **) UV_vectors;
            return 0;
        }

        from = arg1;
        n = 0;
        while (*from) {
        if (*from++ == ':') n++;
        }
        if (n % 2 == 0) fatal_error("new_grid_vectors: bad definition: %s", arg1);

        i = strlen(arg1);
        save->buff = (char *) malloc(i + 1);
        if (save->buff == NULL) fatal_error("new_grid_vectors: memory allocation","");
        save->uv_vectors = (const char **) malloc((n+2) * sizeof(char *));
        if (save->uv_vectors == NULL) fatal_error("new_grid_vectors: memory allocation","");

        from = arg1;
        to = save->buff;
	
        for (i = 0; i <= n; i++) {
            save->uv_vectors[i] = to;
            while (*from != '\0' && *from != ':') {
                *to++ = *from++;
            }
            if (*from == ':') from++;
            *to++ = '\0';
        }
        save->uv_vectors[n+1] = NULL;
        return 0;
    }
    save = *local;
    if (mode == -2) {
        if (save->buff != NULL) {
            free(save->buff);
            free(save->uv_vectors);
        }
        free(save);
        return 0;
    }
    vectors = save->uv_vectors;
    return 0;
}

/*
 * this is the double precision float and 4-byte integer version of grib2 version of ipolates
 * this version will not handle 2G+ grids (4 byte integers)
 * some optimizations relative to grib1 version and the single precision grib2 versions
 * of the wrappers
 *
 * spectral restored
 */

/** Decode grib file flag. */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Append grib file flag. */
extern int file_append;

/** Flush of output flag. */
extern int flush_mode;

/** File header flag. */
extern int header;

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

/** Scan mode flag. */
extern int scan;

/** Flag to indicate whether to save translation information. */
extern int save_translation;

/** Number of grid points in x-direction. */
extern unsigned int nx_;

/** Number of grid points in y-direction. */
extern unsigned int ny_;

/** Desired output order type. */
extern enum output_order_type output_order_wanted;

/** Current output order type. */
extern enum output_order_type output_order;

/** Wind rotation type. */
enum wind_rotation_type wind_rotation;

/*
 * HEADER:111:new_grid_winds:misc:1:new_grid wind orientation: X = grid, earth (no default)
 */

/** 
 * Selects the wind rotation/orientation for the -new_grid option.
 * 
 * Most users will want the winds to be oriented to the earth's north and south directions. 
 * This is done by using the -new_grid_winds earth option. Many of NCEP grids have the winds 
 * being rotated so that north direction is relative to the grid; i.e., grid point (i,j) to 
 * (i,j+1). For lat-lon, Mercator and Gaussian grids, the grid and earth relative directions 
 * are the same. For the Lambert conformal, polar stereographic and various rotated grids, the 
 * directions are different. 
 * 
 * To make the interpolated wind fields have a earth (grid) orientation, you have to use the 
 * -new_grid_winds earth or to use the -new_grid_winds grid option before doing the interpolation. 
 * The exception is the "-new_grid grib". In this case, the grid definition and default wind 
 * rotation is read from a grib file. In this case, a -new_grid_winds option will override the 
 * wind rotation read from the file. 
 * 
 * The wind orientation applies to all the identified vector fields: "UGRD", "VGRD", "VUCSH", 
 * "VVCSH","UFLX", "VFLX", "UGUST","VGUST","USTM","VSTM","VDFUA", "VDFVA", "UOGRD","VOGRD". 
 * 
 * In the future, the wind orientation will be a part of the -new_grid option. However, the current 
 * systax will still work. 
 * 
 * ## Usage
 * -new_grid_winds X
 * 
 * X = earth, grid
 * 
 * earth:
 * U wind goes eastward, V goes northward
 * 
 * grid:
 * U wind goes from grid (i,j) to (i+1,j) which is not eastward in a Lambert-conformal or polar 
 * stereographic grids
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 6/2010
*/
int f_new_grid_winds(ARG1) {
    int *save;
    if (mode == -2) {
        free(*local);
        return 0;
    }
    if (mode == -1) {
        if ((*local = save = (int *) malloc(sizeof(int))) == NULL) fatal_error("new_grid_winds: malloc","");
        if (strcmp(arg1,"grid") == 0) *save = 0;
        else if (strcmp(arg1,"earth") == 0) *save = 1;
        else fatal_error("new_grid_winds: bad arg %s", arg1);
    }
    save = (int *) *local;
    wind_rotation = (*save) ? earth : grid;
    return 0;
}

/** Interpolation type. */
static int interpol_type = 0;

/** Interpolation options. */
static int ipopt[20] = {-1,-1,0, 0,0,0, 0,0,0, 0};

/*
 * HEADER:111:new_grid_interpolation:misc:1:new_grid interpolation X=bilinear,bicubic,neighbor,budget,neighbor-budget
 */

/**
 * Selects the type of interpolation used by the -new_grid option.
 * 
 * The possible values are bilinear, bicubic, nearest neighbor, spectral, neighbor-budget and 
 * budget. The -new_grid_interpolation option must appear before the -new_grid option. If the 
 * -new_grid_interpolation option is not used, the interpolation defaults to bilinear. 
 * 
 * 1. bilinear, interpolate linearly in X and then linearly in Y
 * 2. bicubic
 * 3. nearest neighbor, value of closest source grid point in X-Y space
 * 4. budget, make 5x5 grid, find the 25 bilinear values and average
 * 5. neighbor-budget, make 5x5 grid, find 25 nearest neighbor values and average
 * 6. spectral, convert to spectral space (user specified) and then to grid values
 *
 * Some fields may require different types of interpolation such as the soil type should be 
 * found using a nearest neighbor interpolation. (A fractional soil type is meaningless.) It 
 * is common practice to use -if and -fi options to control the setting of the the interpolation 
 * type as shown by the following command. 
 * 
 * @code{.sh}
 * wgrib2 IN.grb -new_grid_winds earth -new_grid_interpolation bilinear \
 *  -if ":(VGTYP|SOTYP):" -new_grid_interpolation neighbor -fi \
 *  -new_grid latlon 0:360:1 90:181:-1 OUT.grb
 * @endcode
 * 
 * Budget or neighbor-budget is often used for precipitation in order to roughly conserve global 
 * averages. The budget interpolations are slower because the output grid cell is covered with a 
 * 5x5 grid, and interpolations are done for each point of the 5x5 grid. So the budget interpolations 
 * do 25 times more interpolations. 
 * 
 * Spectral interpolation is specialized interpolation scheme. A global field is transformed into 
 * spherical harmonics (user specified truncation), and the grid point values are determined from 
 * the spherical harmonics. This interpolation is unlike other interpolation scheme. For example, 
 * the nearest neighbor is based on the 1 grid point, the bilinear internpolation is based on 4 
 * grid points. The spectral interpolation is based on all the grid points. So the interpolation 
 * scheme will act as noise reduction when projected to a fewer spherical harmonics than the 
 * number of grid points. 
 * 
 * The limiting zonal wave number is specified and should not be prime as it makes the Fast Fourier 
 * Transform into a slow Fourier Transform. The limiting zonal wave number should be a product of 
 * many 2s, 3s, 5s and other small primes. The limiting zonal wave number may need to be compatible 
 * with the Fast Fourier Transform (FFT) code used. 
 * 
 * ## Usage
 * -new_grid_interpolation X
 * 
 * X = bilinear, bicubic, neighbor, budget, neighbor-budget, spectral-(trun)(num)
 * 
 * ### For spectral interpolation:
 * trun = 't', 'T', 'r' or 'R' for triangular or rhomboidal trunction
 * (num) = max zonal wave number
 * 
 * The input has to be a global field and contain valid values for all grid points.
 * 
 * If undefined values are found, bilinear interpolation is used.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 6/2010
 */
int f_new_grid_interpolation(ARG1) {
   
// #ifdef USE_SPECTRAL
   char type;
   int max_wave;
// #endif

    if (mode >= -1) {
        if (strcmp(arg1,"bilinear") == 0) { interpol_type = 0; ipopt[0] = -1; }
        else if (strcmp(arg1,"bicubic") == 0) { interpol_type = 1; ipopt[0] = 0; }
        else if (strcmp(arg1,"neighbor") == 0) { interpol_type = 2; ipopt[0] = 1; }
        else if (strcmp(arg1,"budget") == 0) { interpol_type = 3; ipopt[0] = -1; }
// #ifdef USE_SPECTRAL
        else if (sscanf(arg1,"spectral-%c%d", &type, &max_wave) == 2) {
            if (type == 't' || type == 'T') ipopt[0] = 0;
            else if (type == 'r' || type == 'R') ipopt[0] = 1;
            else fatal_error("new_grid_interpolation: spectral-(T|R)NUM not %s", arg1);
            if (max_wave <= 1) fatal_error("new_grid_interpolation: spectral-(T|R)NUM, NUM > 1", "");
            ipopt[1] = max_wave;
            interpol_type = 4;
        }
// #endif
        else if (strcmp(arg1,"neighbor-budget") == 0) { interpol_type = 6; ipopt[0] = -1; }
        else fatal_error("new_grid_interpolation: unknown type %s", arg1);
    }
    return 0;
}

/*
 * HEADER:111:new_grid_format:misc:1:new_grid output format  X=bin,ieee,grib
 */

/**
 * Specified the output format for the -new_grid option.
 * 
 * The default is grib, with options for bin and ieee. The bin format is the machine's native 
 * single precision floating point format. The ieee format is single precision IEEE 754 format. 
 * The bin format can have optional f77-style header. The ieee format be in big endian or little 
 * endian with an optional f77-style header. The formats are the same as produced by -bin and 
 * -ieee. Headers are specified by -header and -no_header. The ieee format defaults to big endian 
 * (network order) and specified by -little_endian and -big_endian. The f77-style header is prepends 
 * and appends a 32-bit integer of the number of bytes in the data field. The integer is appropriate 
 * to the endian of the data and is stored in the native 32-bit integer format. Outputing in the 
 * ieee format is slower than in the native bin format because there is a conversion from the native 
 * format to ieee even if the native format happens to be ieee. 
 * 
 * The output order of the fields may differ from the input order of the fields. The problem occurs 
 * when -new_grid encounters a vector interpolation. To do a vector interpolation, both the U and V 
 * must be interpolated together. (Vector interpolation is needed so that the winds near the poles 
 * remain accurate.) In order for the input order to be the same as the output order, the V field 
 * must follow the corresponding U field. Wgrib2 allows a sequence of "U-any number of scalar fields-V" 
 * to work with -new_grid even though it is undocumented. To make sure the the input order is the 
 * same as the output order, make sure V directly follows U, and also make sure you know which 
 * fields are specified to be vector fields. (The default vectors are version specific.) 
 * 
 * The -new_grid_format option was introduced with wgrib2 v3.0.0, for support of the python 
 * interface.
 * 
 * ## Usage
 * -new_grid_format FORMAT
 *
 * FORMAT is the output format of -new_grid. The available formats are:
 *   - grib: GRIB format (default)
 *   - bin: native single precision format like -bin
 *   - ieee: IEEE single precision like -ieee
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Examples
 * 
 * The following creates a grib output file called "new.grb".
 * @code{.sh}
 * $ wgrib2 gep19.aec -for 1:5  -new_grid_winds earth -new_grid ncep grid 3 new.grb
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * 2:70707:d=2009060500:TMP:200 mb:180 hour fcst:ENS=+19
 * 3:96843:d=2009060500:RH:200 mb:180 hour fcst:ENS=+19
 * 4:125750:d=2009060500:UGRD:200 mb:180 hour fcst:ENS=+19
 * 5:166391:d=2009060500:VGRD:200 mb:180 hour fcst:ENS=+19
 * @endcode
 * 
 * The following creates a binary formatted output file "new.bin", the scan order is we:ns. U and V are paired, 
 * so the input order is the same as the output order.
 * @code{.sh}
 * $ wgrib2 gep19.aec -for 1:5  -new_grid_format bin -new_grid_winds earth -new_grid ncep grid 3 new.bin
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * 2:70707:d=2009060500:TMP:200 mb:180 hour fcst:ENS=+19
 * 3:96843:d=2009060500:RH:200 mb:180 hour fcst:ENS=+19
 * 4:125750:d=2009060500:UGRD:200 mb:180 hour fcst:ENS=+19
 * 5:166391:d=2009060500:VGRD:200 mb:180 hour fcst:ENS=+19
 * @endcode
 * 
 * The following compares the binary and grib output files.
 * @code{.sh}
 * $ wgrib2 new.grb  -rpn sto_0 -import_bin new.bin -rpn 'raw2:rcl_0:print_corr'
 * 1:0:rpn_corr=1:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * 2:130502:rpn_corr=1:d=2009060500:TMP:200 mb:180 hour fcst:ENS=+19
 * 3:203989:rpn_corr=1:d=2009060500:RH:200 mb:180 hour fcst:ENS=+19
 * 4:261186:rpn_corr=1:d=2009060500:UGRD:200 mb:180 hour fcst:ENS=+19
 * 5:350963:rpn_corr=1:d=2009060500:VGRD:200 mb:180 hour fcst:ENS=+19
 * @endcode
 * 
 * The binary file was written in we:ns, and -import_bin does not change the scan order. Therefore we have to change 
 * the order we:sn by raw2. rpn_corr=1 .. new.grb == new.bin upto a grib rounding error.
 * 
 * @author Wesley Ebisuzaki @date 6/2010
 */
int f_new_grid_format(ARG1) {
    if (mode >= -1) {
        if (strcmp(arg1,"grib") == 0) new_grid_format = grib;
        else if (strcmp(arg1,"bin") == 0) new_grid_format = bin;
        else if (strcmp(arg1,"ieee") == 0) new_grid_format = ieee;
        else fatal_error("new_grid_format: unknown type (%s)", arg1);
    }
    return 0;
}

/*
 * HEADER:111:new_grid_ipopt:misc:1:new_grid ipopt values X=i1:i2..:iN N <= 20
 */

/**
 * Modifies the parameters used by the interpolation method set by the -new_grid_interpolation 
 * option.
 * 
 * The -new_grid_ipopt option MUST FOLLOW the -new_grid_interpolation option because the latter will overwrite the IPOPT values with the default values for that interpolation method. 
 * 
 * 1. bilinear: ipopt is not used
 * 2. bicubic:
 *    i. ipopt(1) = 0: straight bicubic
 *    ii. ipopt(1) = 1: constrained to range of 4 neighboring points
 *    iii. ipopt(2) = %data converage for output point, default=50%
 * 3. nearest neighbor: ipopt is not used
 * 4. budget: ipopt is not used
 * 5. spectral: (alpha) 
 *    i. ipopt(1) = 0: triangular truncation
 *    ii. ipopt(1) = 1: rhomboidal truncation
 *    iii. ipopt(2) = N: truncation number
 *    iv. ipopt(2) = -1: sensible truncation number, based on grid (default for now, will be changed)
 * 6. neighbor-budget: not enabled by wgrib2 
 * 
 * ## Note on spectral options
 * 
 * Should specify these values by -new_grid_interpolation.
 * 
 * when ipopt(2) == -1, sensible truncation number
 * @code{.f}
 * IROMB = ipopt(1)
 * IDRT=4 FOR GAUSSIAN GRID
 * IDRT=0 FOR EQUALLY-SPACED GRID INCLUDING POLES
 * IDRT=256 FOR EQUALLY-SPACED GRID EXCLUDING POLES
 * 
 * IF(IROMB.EQ.0.AND.IDRTI.EQ.4) MAXWV=(JMAXI-1)
 * IF(IROMB.EQ.1.AND.IDRTI.EQ.4) MAXWV=(JMAXI-1)/2
 * IF(IROMB.EQ.0.AND.IDRTI.EQ.0) MAXWV=(JMAXI-3)/2
 * IF(IROMB.EQ.1.AND.IDRTI.EQ.0) MAXWV=(JMAXI-3)/4
 * IF(IROMB.EQ.0.AND.IDRTI.EQ.256) MAXWV=(JMAXI-1)/2
 * IF(IROMB.EQ.1.AND.IDRTI.EQ.256) MAXWV=(JMAXI-1)/4
 * @endcode
 * 
 * The values are appropriate for the spectral GFS Gaussian grid because the grid values were 
 * derived from the spherical harmonics. The values are not appropriate for the FV3 GFS Gaussian 
 * grid  because the grid values were derived by a bilinear interpolation (mostly) from the 
 * cubed sphere grid.
 *
 * Since NCEP is transitioning to a FV3 model, the grid point values are not the exact 
 * representation of the model fields but rather a bilinear interpolation from the cubed sphere.  
 * So a high-wavenumber noise is being added to the grid values.  So the appropriate spectral 
 * representation will have filter applied to the spectral representation. The math hasn't been 
 * worked out.
 * 
 * ## Usage
 * 
 * -new_grid_ipopt X
 * 
 * X = integer or integer:integer
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 6/2010
 */
int f_new_grid_ipopt(ARG1) {
    int i, k, val, m;

    if (mode >= -1) {
        i = 0;
        k = sscanf(arg1, "%d%n", &val, &m);
        while (k == 1) {
            if (i > 19) fatal_error("new_grid_ipopt: too many ipopt values, 20 max","");
            ipopt[i++] = val;
            arg1 += m;
            k = sscanf(arg1, ":%d%n", &val, &m);
        }
    }
    return 0;
}

static void check_grid_size(int nx, int ny);

/**
 * ipolates is using a fortran library with integers up to 2GB
 *
 * This routine checks that the grid size is < 2GB.
 *
 * This function returns nothing, but if the grid size exceeds the limit, a fatal_error is raised.
 * 
 * @param nx number of grid points in the x direction
 * @param ny number of grid points in the y direction
 * 
 * @author Wesley Ebisuzaki @date 6/2010
 */
static void check_grid_size(int nx, int ny) {
    int err;

    err=0;
    if (nx <= 0) err = 1;
    if (ny <= 0) err = 1;
    if ((double) nx * (double) ny > INT_MAX) err = 1;
    if (err == 1) fatal_error_i("new_grid: grid size exceeds %d (INT_MAX)", INT_MAX);
}

/** Local structure for holding grid information. */
struct local_struct {
    // U data
    float *u_val; /**< Value of U component of wind. */
    int has_u; /**< Flag indicating presence of U component. */
    int nx; /**< Number of grid points in the x direction. */
    int ny; /**< Number of grid points in the y direction. */
    unsigned char *clone_sec[9]; /**< Clone of the original section. */
    char name[NAMELEN]; /**< Name of the grid. */

    // interpolation
    int npnts_out; /**< Number of grid points in the output grid. Must be an integer (Fortran call requires int). */
	int defined_lat_lon; /**< Flag indicating if latitude/longitude is defined. Most grids have lat-lon defined by iplib. */
    double *rlat; /**< Rotated latitude values. */
    double *rlon; /**< Rotated longitude values. */
    double *crot; /**< Cosine of rotation angle. */
    double *srot; /**< Sine of rotation angle. */
    double *data_out; /**< Output data values. */
	float *data_wrt; /**< Write data values. */
    unsigned char *bitmap_out; /**< Bitmap for output data. */
    unsigned char *sec3; /**< Section 3 data. */
    double radius_major; /**< Major radius of the grid. */
    double radius_minor; /**< Minor radius of the grid. */
    int gdtnum_out; /**< Grid definition number for output. */
    int gdt_out[200]; /**< Grid definition template for output. */
    int gdt_out_size; /**< Size of the output grid definition template. */
    int output_latlon; /**< Output lat-lon flag. */
    struct seq_file out; /**< Output file structure. */
};

/** Blank section 1 for GRIB messages. */
unsigned char blank_sec1[21] = { 0,0,0,21,1,
                255,255,255,255,        // center subcenter
                2,1,255,                // grib master table, local table, sig ref time
                255, 255,               // year
                255, 255, 255, 255, 255, // month .. second
                255, 255};
/*
 * HEADER:111:new_grid:output:4:bilinear interpolate: X=projection Y=x0:nx:dx Z=y0:ny:dy A=grib_file alpha
 */

/**
 * Interpolates the fields to a new gird.
 * 
 * Beginners are encouraged to read the [new grid introduction]
 * (https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/new_grid_intro.html).
 * 
 * The default interpolation is bilinear but that can be changed using the -new_grid_interpolation 
 * option. This option uses scalar and vector interpolation as appropriate. Vector interpolation 
 * is needed for interpolating zonal and meridional winds near the poles. 
 * 
 * In order for the vector interpolation to work, the vector quantities must be in a (U,V) order. 
 * For example: Z200, U200, V200, Z500, U500, V500 is good. If the data are not in (U,V) order, 
 * you can either convert the file to the right order by the -new_grid_order option. The other 
 * method is to sort the inventory into (U,V) order, and interpolate the file using the sorted 
 * inventory. (See "Example: Sorting the Inventory" to use this technique.) If the vector quanties 
 * are not in (U,V) order, the vector quantites will not be interpolated. 
 * 
 * The option is not part of the default configuration but it included with many distributions. 
 * The interpolation code is written in fortran and combining fortran and C code can require some 
 * work. (gcc/gfortran and clang/gfortran are already handled by the makefile.) Getting the C and 
 * Fortran code to cooperate requires some system-specific knowledge and may not be possible in all 
 * cases. Consequently you are on your own in getting the -new_grid option installed. 
 * 
 * Operations often has to use the -new_grid option to produce a large number of user grids. 
 * Fortunately the interpolation can be made embarrassingly parallel. A portable single-node solution 
 * is described in here. A multi-node solution is possible using MPI and the wgrib2 library. 
 * 
 * ## Caution
 * The -new_grid option works in raw scan mode, so data are not converted to sn:we order. Conequently 
 * options that only work in sn:we order cannot work at the same time as the -new_grid option. Any 
 * option that uses geolocation (ex. -rpn, -lon) is incompatible with -new_grid. 
 * 
 * ## Winds
 * Before you do an interpolation, you need to define the wind directions. Most people want the the V 
 * winds to be in the direction of the North Pole. With a verbose wgrib2 inventory, you will seee 
 * winds(N/S). However, some meteorologists want the V winds to go from grid point (i,j) to (i,j+1). 
 * The corresponding wgrib2 notation is "winds(grid)". See the -new_grid_winds option for more details. 
 * 
 * ## Usage
 * -new_grid_winds W -new_grid A B C outfile
 * 
 * W = earth or grid
 * earth: U wind goes eastward, V goes northward
 * grid: U wind goes from grid (i,j) to (i+1,j) which is not eastward in a Lambert-conformal or polar 
 * stereographic grids
 * 
 * A, B, C are the output grid description
 * 
 * outfile is an output file. The grib2 interpolated records are written in outfile.
 * 
 * ## Grid Description Format
 * The nx and ny parameters are integers such that nx*ny < 2147483648. Angles and delta-angles 
 * are in degrees, and are rounded to micro-degrees unless otherwise specified. Dx and Dy are 
 * in meters and are usually rounded to the nearest mm. 
 * 
 * The interpolation library does not handle non-spherical grids. The library also has problems 
 * with grid smaller than 1 degree in longitudinal width. It assumes that the calculations are 
 * not precise, and the grid is global. 
 * 
 * Note that lambert, nps, sps, mercator only support we:sn ordering and latlon, gaussian only 
 * support we:sn and we:ns ordering.
 * 
 * ### General Format
 * -new_grid A B C outfile
 * 
 * A = grid name with parameters
 * B = X or longitude description (ex lon0:nlon:dlon)
 * C = Y or latitude description (ex lat0:nlat:dlat)
 * 
 * ### Selected NCEP Grids (not in general format)
 * -new_grid ncep grid I outfile
 * 
 * I = grid definition 
 * 
 * NCEP grid defintions: 2,3,4,45,98,126-129,170,173,184,194,221,230,242,249 
 * 
 * NCEP Gaussian grid definitions: t62,t126,t170,t190,t254,t382,t574,t1148,t1534 
 * 
 * ### Latitude-Longitude Grid
 * -new_grid latlon lon0:nlon:dlon lat0:nlat:dlat outfile 
 * 
 * lat0, lon0 = degrees of lat/lon for 1st grid point 
 * nlon = number of longitudes
 * nlat = number of latitudes
 * dlon = grid cell size in degrees of longitude
 * dlat = grid cell size in degrees of latitude
 * 
 * ### Rotated Latitude-Longitude Grid
 * -new_grid rot-ll:sp_lon:sp_lat:sp_rot lon0:nlon:dlon lat0:nlat:dlat outfile
 * 
 * sp_lon = longitude of the South pole (for rotation)
 * sp_lat = latitude of the South pole (for rotation)
 * sp_rot = angle of rotation (degrees)
 *      The grid is defined in the rotated coordinates.
 * eat0, lon0 = degrees of lat/lon for 1st grid point 
 * nlat = number of longitudes
 * nlon = number of latitudes
 * dlon = grid cell size in degrees of longitude
 * dlat = grid cell size in degrees of latitude
 * 
 * ### Lambert Conic Grid
 * 
 * Lambert conic conformal
 * -new_grid lambert:lov:latin1:latin2:lad lon0:nx:dx lat0:ny:dy outfile	
 * 
 * lad = latin2
 * -new_grid lambert:lov:latin1:latin2 lon0:nx:dx lat0:ny:dy outfile 
 * 
 * latin2 = latin1 = lad
 * -new_grid lambert:lov:latin1 lon0:nx:dx lat0:ny:dy outfile
 * 
 * lov = longitude (degrees) where y axis is parallel to meridian
 * latin1 = first latitude from pole which cuts the secant cone
 * latin2 = second latitude from pole which cuts the secant cone
 * lad = latitude (degrees) where dx and dy are specified
 * lat0, lon0 = degrees of lat/lon for 1st grid point 
 * nx = number of grid points in X direction
 * ny = number of grid points in Y direction
 * dx = grid cell size in meters in x direction
 * dy = grid cell size in meters in y direction
 * 
 * Note: 
 * - if latin2 >= 0, the north pole is on proj plane
 * - if latin2 < 0, the south pole is on proj plane
 * 
 * ### Lambert Conic Grid with Centered Location
 * Lambert conic conformal with centered position
 * -new_grid lambertc:lov:latin1:latin2:lad lonc:nx:dx latc:ny:dy outfile
 * 
 * Like lambert except lonc and latc replace lon0 and lat0
 * -new_grid lambertc:lov:latin1:latin2 lonc:nx:dx latc:ny:dy outfile 
 * 
 * latc, lonc = degrees of lat/lon for center of the grid
 * -new_grid lambertc:lov:latin1 lonc:nx:dx latc:ny:dy outfile
 * 
 * ### Polar Stereographic Grid
 * North polar stereographic
 * -new_grid nps:lov:lad lon0:nx:dx lat0:ny:dy outfile  
 * 
 * South polar stereographic
 * -new_grid sps:lov:lad lon0:nx:dx lat0:ny:dy outfile	
 * 
 * lov = longitude (degrees) where y axis is parallel to meridian
 * lad = latitude (degrees) where dx and dy are specified
 * lat0, lon0 = degrees of lat/lon for 1st grid point 
 * nx = number of grid points in X direction
 * ny = number of grid points in Y direction
 * dx = grid cell distance meters in x direction at lad
 * dy = grid cell distance meters in y direction at lad
 * 
 * Note: grib1 uses lad = 60N (nps) or 60S (sps). lad must be 60 (nps) or -60 (sps) 
 * (library limitation)
 * 
 * ### Global Gaussian Grid
 * -new_grid gaussian lon0:nx:dlon lat0:ny outfile
 * 
 * lat0, lon0 = degrees of lat/lon for 1st grid point
 * nx = number of grid points in X direction
 * ny = number of grid points in Y direction (must be even)
 * dlon = degrees of longitude between adjacent grid points
 * 
 * Note 1:
 * 
 * @code{}
 * lon1 = -lon0;
 * lat1 = lat0 + (nx-1)*dlon;
 * @endcode
 * 
 * Note 2: wgrib2 supports regional Gaussian grids, but the interpolation library doesn't. 
 * 
 * ### Mercator Grid
 * -new_grid mercator:lad lon0:nx:dx:lonn lat0:ny:dy:latn outfile   
 * 
 * lad = latitude (degrees) where dx and dy are specified
 * lat0, lon0 = degrees of lat/lon for 1st grid point
 * latn, lonn = degrees of lat/lon for last grid point
 * nx = number of grid points in X direction
 * ny = number of grid points in Y direction
 * dx = grid cell distance in meters in x direction at lad (double precision)
 * dy = grid cell distance in meters in y direction at lad (double precision)
 * 
 * See [grib2 template](https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_temp3-10.shtml) 
 * for Mercator grid.
 * 
 * Note:
 * - dx and dy are in meters unlike the grib template which is in mm (integer).
 * - dx and dy depend on the radius of the earth as specified by the grib message.
 * - interpolation library assumes that earth will be spherical. 
 * - the mercator grid description is over specified. User must make sure (nx,dy) is consistent 
 * with lonn as well as (ny,dy) is consistent with latn
 * 
 * @param ARG4 List of function arguments set by wgrib2's main() function (see @ref ARG4). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * ## Changes from copygb
 * 
 * People may want to convert from copygb and copygb2 to wgrib2's -new_grid. Some differences to keep in mind.
 * 1. copygb default vectors: UGRD/VGRD
 * 2. wgrib2 default vectors: depends on version of wgrib2. See new_grid_vectors.
 * 3. copygb can have vectors in any order
 * 4. wgrib2 must have V follow U for vectors pairs
 * 5. copygb has bilinear, bicubic, nearest neighbor, budget, neighbor budget, and spectral interpolations.
 * 6. wgrib2 has bilinear, bicubic, nearest neighbor, budget, and spectral interpolations. 
 * 7. wgrib2 can select the interpolation type depending on the variable (ex soil type)
 * 8. copygb uses fixed Earth's radius
 * 9. wgrib2 uses Earth's radius based on grib message
 * 10. wgrib2 doesn't have merging, mapthreshold or map files
 * 11. copygb by default, ignores the binary scaling and preserves decimal scaling
 * 12. wgrib2 by default, preserves binary and decimal scaling
 * 13. copygb does grib1.
 * 14. copygb2 does grib2.
 * 15. wgrib2 does grib2.
 * 
 * ## Speed: Interpolation Weights
 * The first step of the -new_grid interpolation is to calculate the interpolation weights. (Each grid point on the 
 * new grid is a weighted average of a small set of the old grid points.) To save time for future interpolations, 
 * the last set of weights is saved. Consequently interpolation is fastest when the input and output grids don't change. 
 * While one can have multiple -new_grid options on the command line, it is not recommended because the caching of the 
 * weights wouldn't work and weights would have to be recalculated every time. 
 * 
 * ## Converting from WE:SN to WE:NS Grids
 * Many of wgrib2 grib2 writing options will write the grid in WE:SN order. This natural because geolocation is only enabled 
 * when the internal grids are in WE:SN order. However, some codes need the grid in WE:NS order. To convert a grib file from 
 * WE:SN order to WE:NS order, the simplest way is to use -new_grid. Lat0 and lon0 need to be lat/lon of the top left corner 
 * of the grid. Dlon will a positive number and dlat will be negative.
 * 
 * If you want to be tricky, you can do a variation of the "NDFD work arounds" technique. It will be faster and more generic.
 * 
 * ### NDFD work arounds
 * The -new_grid option will give the following error when trying to regrid a field that is written in (WE|EW):SN order.
 * @code{.sh}
 * *** FATAL ERROR: mk_kgds: unsupported scan mode 80
 * @endcode
 * The (WE|EW):SN order means that the odd rows are in WE order and the even rows are EW order. The rows go from south to north. 
 * This scan order is commonly used by NDFD in order to make newbies brain hurt. Try writting a program to get the lat-lon of the 
 * Nth grid point. 
 * 
 * The (WE|EW):SN order is not supported by the ipolates library which does the regridding for the -new_grid option. The simplest 
 * work around is to convert the grid to a WE:SN order. 
 * 
 * 1. Find the grid dimensions.
 * @code{.sh}
 * $ wgrib2 blend.grb -nxny 
 * 1:0:(2145 x 1597)
 * @endcode
 * 
 * 2. Use -ijsmall_grib to rewrite the entire grid. -ijsmall_grib will write the subgrid in WE:SN order.
 * @code{.sh}
 * $ wgrib2 blend.grb -ijsmall_grib 1:2145 1:1597 blend2.grb
 * @endcode
 * 
 * ## Thinned Gaussian Grids to other Grids
 * Converting from a thinned Gaussian grid is a two step process using wgrib2. First you convert from the thinned grid to full grid 
 * using -reduced_gaussian_grid. Then you can use -new_grid to interpolate to your desired grid. 
 * 
 * ## Quilting tiles - Merging files
 * Quilting is what we call combining (regional) forecasts for different domains together onto a single grid. For example, you may want 
 * to combine various regional oceanic forecasts with a global oceanic forecast to produce a forecast for a for the user who is on a 
 * route that covers different model domains. 
 * 
 * Yes, it has been done using -new_grid, -import_grib -rpn/merge. See Example 7 from @ref f_rpn.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Examples
 * 
 * The following examples interpolate from IN.grb to OUT.grb. The output file uses the same grib packing as 
 * the input file.
 * 
 * @code{.sh}
 * $ wgrib2 IN.grb -set_grib_type same -new_grid_winds earth -new_grid latlon 100:10:1 30:20:1 OUT.grb
 * @endcode
 * 
 * Makes a 10x20, 1x1 degree lat-lon grid, lower left corner: 100E 30N
 * 
 * @code{.sh}
 * $ wgrib2 IN.grb -set_grib_type same -new_grid_winds earth -new_grid ncep grid 221 OUT.grb
 * @endcode
 * 
 * Interpolates to NCEP grid 221.
 * 
 * @code{.sh}
 * $ wgrib2 IN.grb -set_grib_type same -new_grid_winds earth -new_grid `grid_defn.pl OLD.grb` OUT.grb
 * @endcode
 * 
 * Interpolates using the grid format of OLD.grb (1st record)
 * 
 * ### Example: Sorting the Inventory
 * In this example, U and V are not in the required order. This shows a sorting to the required order for 
 * -new_grid to work.
 * 
 * @code{.sh}
 * $ wgrib2 201201.A | sed -e 's/:UGRD:/:UGRDa:/' -e 's/:VGRD:/:UGRDb:/'  | \
 *      sort -t: -k3,3 -k5,8 -k4,4 | \
 *      wgrib2 201201.A -i -new_grid_winds earth -new_grid ncep grid 2 201201.A.grd2
 * @endcode
 * 
 * - The first line creates an inventory with new variable names: UGRD -> UGRDa and VGRD -> UGRDb
 * - The second line sorts the inventory so that UGRDb follows UGRDB.
 * - The third line regrids the file, with the order of processing controlled by the inventory.
 * 
 * ### Type of Interpolation
 * The IPOLATES library supports a number of interpolation schemes including bilinear (default), bicubic, 
 * neighbor, budget, and spectral. The interpolation method can be selected by using the -new_grid_interpolation 
 * option before the -new_grid option. Some of the interpolation options need numeric parameters which are set by 
 * the -new_grid_ipopt option. IPOPT is defined in the iplib library documentation. 
 * 
 * You can use different interpolations for different variables. For example, a bilinear interpolation of soil or 
 * vegetation type is meaningless. So nearest neighbor interpolation is used instead. 
 * 
 * @code{.sh}
 * $ wgrib2 IN.grb -new_grid_winds earth \
 *    -new_grid_interpolation bilinear \
 *    -if ":(VGTYP|SOTYP):" -new_grid_interpolation neighbor -fi \
 *    -new_grid latlon 0:360:1 90:181:-1 OUT.grb
 * @endcode
 * 
 * - line 2: set default interpolation to bilinear
 * - line 3: if VGTYP or SOTYP then set the interpolation to nearest neighbor
 * - line 4: do the interpolation
 * 
 * When you convert from a high resolution grid to a lower resolution grid, you have to be consider changing from 
 * the default interpolation (bilinear) to a budget interpolation. The budget gives a better estimate of the cell 
 * average (the default is 25 bilinear interpolations). 
 * 
 * ### Changing from grid-relative to Earth-relative winds and vice versa
 * Most NCEP grib files use grid-relative winds. If you want to convert to Earth-relative winds or grid-relative 
 * winds, you can use the -new_grid option. 
 * 
 * To Earth relative:
 * @code{.sh}
 * $ wgrib2 IN.grb -set_grib_type same -new_grid_winds earth -new_grid_interpolation neighbor \
 *      -new_grid `grid_defn.pl IN.grb` OUT.grb
 * @endcode
 * 
 * To Grid relative:
 * @code{.sh}
 * $ wgrib2 IN.grb -set_grib_type same -new_grid_winds grid -new_grid_interpolation neighbor \
 *      -new_grid `grid_defn.pl IN.grb` OUT.grb
 * @endcode
 * 
 * - "-set_grib_type same" preserves the grib packing or compression
 * - "-new_grid_interpolation neighbor" should be faster than the default bilinear
 * - `grid_defn.pl IN.grb` returns the grid definition of the first grib message in IN.grb
 * 
 * The limirations of the above command are:
 * - IN.grb can only have one grid type 
 * - OUT.grb will have any submessages converted into messages 
 * 
 * @author Wesley Ebisuzaki @date 6/2010
 */
int f_new_grid(ARG4) {
    struct local_struct *save;

    unsigned int i;
    int is_u, is_v, ftn_npnts, ftn_nout;
    int km, itmp;
    double *data_in;
    int gdtnum_in, gdt_in[200], gdt_in_size;
    double x0, y0, dx, dy, xn, yn;
    double lov, lad, latin1, latin2;
    int proj;                                   // projection: for LC 0 = NP, 128 = SP
    double sp_lat, sp_lon, sp_rot;
    char name[NAMELEN];

    /* grib2 grid definitions */
    struct seq_file input;
    unsigned char *input_sec[10];	/* sec[9] = last valid bitmap */
    unsigned char *msg;
    long int pos;
    unsigned long int len;
    int num_submsg;

    /* unstructured grid: location */
    unsigned char unstruct_grid_sec3[35];
    unsigned char sec4_latlon[34], *g;
    int discipline;
    unsigned char *p_discipline;

    int j, ibi, ibo, iret, nnx, nny, n_out, tmp_interpol_type;
    unsigned char *new_sec[8], *s, *bitmap, *p;

    /* for lambertc */
    double r_maj, r_min, ref_lon, ref_lat;

    if (mode == -1) {                   // initialization
        decode = 1;
        output_order_wanted = raw;      // in raw order
// ALEX
        use_ncep_post_arakawa();

        // allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("new_grid: memory allocation error","");

        if (fopen_file(&(save->out), arg4, file_append ? "ab" : "wb") != 0) {
            fatal_error("-new_grid: could not open file %s", arg4);
        }

        save->has_u = 0;
        save->output_latlon = 0;
        save->radius_major = save->radius_minor = 0.0;
        init_sec(save->clone_sec);
        s = NULL;
        save->defined_lat_lon = 0;
        // parse NCEP grids -- if ncep_grid, replace arg1, arg2, arg3
        ncep_grids(&arg1, &arg2, &arg3);

        // for each output grid
        if (strcmp(arg1,"latlon") == 0) {
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_lola(nnx, x0, dx, nny, y0, dy, sec);
        }
        else if (strncmp(arg1,"rot-ll:",7) == 0) {
            if (sscanf(arg1,"rot-ll:%lf:%lf:%lf",  &sp_lon, &sp_lat, &sp_rot) != 3)
                fatal_error("new_grid: bad %s",arg1);
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);
            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;
            s = sec3_rot_ll(nnx, x0, dx, nny, y0, dy, sp_lon, sp_lat, sp_rot, sec);
        }
        else if (strncmp(arg1,"mercator:",9) == 0) {
            if (sscanf(arg1,"mercator:%lf",  &lad) != 1)
                fatal_error("new_grid: LaD (latitude interesection) not specified","");
            if (sscanf(arg2,"%lf:%d:%lf:%lf", &x0, &nnx, &dx, &xn) != 4)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf:%lf", &y0, &nny, &dy, &yn) != 4)

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_mercator(lad, nnx, x0, dx, xn, nny, y0, dy, yn, sec);
        }
        else if (strcmp(arg1,"gaussian") == 0) {
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d", &y0, &nny) != 2)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_gaussian(nnx, x0, dx, nny, y0, sec);
        }
        else if (strncmp(arg1,"lambert:",8) == 0) {
            i = sscanf(arg1,"lambert:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
            if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            if (lov < 0.0)  lov += 360.0;
            if (i < 3) latin2 = latin1;
            if (i < 4) lad = latin2;
            proj = 0;
            if (latin2 < 0.0) proj = 128;

            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            check_grid_size(nnx, nny);
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);
        }

        /* for lambertc, input is the lon-lat of center point */
        /* can not calc grid until radius is given, so do lambert code to check args */

        else if (strncmp(arg1,"lambertc:",9) == 0) {
            i = sscanf(arg1,"lambertc:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
            if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            if (lov < 0.0)  lov += 360.0;
            if (i < 3) latin2 = latin1;
            if (i < 4) lad = latin2;
            proj = 0;
            if (latin2 < 0.0) proj = 128;

            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);
        }

        else if (strncmp(arg1,"nps:",4) == 0 || strncmp(arg1,"sps:",4) == 0)  {
            if (sscanf(arg1,"%*[ns]ps:%lf:%lf", &lov, &lad) != 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            // if (lad != 60.0) fatal_error("New_grid: only LatD = 60 is supported","");
            proj = 0;
            if (arg1[0] == 's') proj = 128;
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);
            if (lov < 0.0)  lov += 360.0;

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_polar_stereo(lov, lad, proj, nnx, x0, dx, nny, y0, dy, sec);
        }

        else if (strncmp(arg1,"grib",4) == 0) {

            /* read grib file, get grid definition */
            j = fopen_file(&input, arg2, "rb");
            if (j != 0) fatal_error("new_grid: %s could not be opened", arg2);
            msg = rd_grib2_msg_seq_file(input_sec, &input, &pos, &len, &num_submsg);
            if (msg == NULL) fatal_error("new_grid: grib record not found","");
            if (parse_1st_msg(input_sec) != 0) fatal_error("new_grid: grib not parsed correctly","");
            i = GB2_Sec3_size(input_sec);
            save->npnts_out = n_out = GB2_Sec3_npts(input_sec);
            save->nx = n_out;
            save->ny = 1;
            s = input_sec[3];
        }

        else if (strncmp(arg1,"location",4) == 0) {

            /* read lat-lon  values */
            n_out = read_latlon(arg2, &(save->rlon) , &(save->rlat)) ;
            if (n_out == 0) fatal_error("new_grid: bad set of locations","");

            save->npnts_out = save->nx = n_out;
            save->ny = 1;
            save->output_latlon = 1;
            /*i save->rlon and save->rlat memory allocated in read_latlon */
            save->crot = (double *) malloc(sizeof(double) * (size_t) n_out);
            save->srot = (double *) malloc(sizeof(double) * (size_t) n_out);
            for (i=0; i < n_out; i++) {
                save->crot[i] = 1.0;
                save->srot[i] = 0.0;
            }
            save->defined_lat_lon = 1;

            /* setup sec3 */
            
            int_char(35, unstruct_grid_sec3 + 0);	/* size of sec 3 */
            unstruct_grid_sec3[4] = 3;			/* this is sec 3 */
            unstruct_grid_sec3[5] = 0;
            int_char(n_out, unstruct_grid_sec3 + 6);	/* number of grid points */
            unstruct_grid_sec3[10] = 0;
            unstruct_grid_sec3[11] = 0;
            int2_char(101, unstruct_grid_sec3+12);
            unstruct_grid_sec3[14] = 0;
            int_char(0, unstruct_grid_sec3 + 15);

            /* UUID read from arg3 */
            for (i = 19; i <= 34; i++)  unstruct_grid_sec3[i] = 0;
            /* UUID */
            g = unstruct_grid_sec3 + 19;
            if (strcmp(arg3,"0") == 0) {                /* nill UUID */
                for (i = 19; i <= 34; i++)  unstruct_grid_sec3[i] = 0;
            }
            else {
                i = sscanf(arg3, "%2hhx%2hhx%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
                        g+0, g+1, g+2, g+3, g+4, g+5, g+6, g+7, g+8, g+9, g+10, g+11, g+12, g+13, g+14, g+15);
                if (i != 16) fatal_error("new_grid: bad uuid %s",arg3);
            }
            /* grid template has NO rotated wind */
            s = &(unstruct_grid_sec3[0]);

        }
        else fatal_error("new_grid: unsupported output grid %s", arg1);

        new_sec[1] = blank_sec1;        // add center info
        // save new section 3
        i = (int) uint4(s);         // size of section 3
        new_sec[3] = save->sec3 = (unsigned char *) malloc(i * sizeof(unsigned char));
        for (j = 0; j < i; j++) save->sec3[j] = s[j];

        /* s[*] points to buffer owned by input .. can close input now */
        if (strncmp(arg1,"grib",4) == 0) fclose_file(&input);

        // apply wind rotation .. change flag 3.3

        if (wind_rotation == undefined) {
            if (strncmp(arg1,"grib",4) != 0 && strncmp(arg1,"location",8) != 0) {
                fprintf(stderr,"Warning: -new_grid_winds set to earth\n");
                wind_rotation = earth;
            }
        }
        if (wind_rotation != undefined) {
            if ((p = flag_table_3_3_location(new_sec)) != NULL) {
                if (wind_rotation == grid) *p = *p | 8;
                else if (wind_rotation == earth) *p = *p & (255 - 8);
            }
        }

        save->gdt_out_size = sizeof(save->gdt_out) / sizeof(save->gdt_out[0]);
        if (mk_gdt(new_sec, &(save->gdtnum_out), &(save->gdt_out[0]), &(save->gdt_out_size) ))
            fatal_error("new_grid: encoding output gdt failed","");

        if (save->defined_lat_lon == 0) {		// use grib
            /* some vectors need by interpolation routines */
            save->rlat = (double *) malloc(sizeof(double) * (size_t) n_out);
            save->rlon = (double *) malloc(sizeof(double) * (size_t) n_out);
            save->crot = (double *) malloc(sizeof(double) * (size_t) n_out);
            save->srot = (double *) malloc(sizeof(double) * (size_t) n_out);
        }
        else {		// lat-lon are defined .. do not use grib output grid, rlat, rlon, crot, srot already defined
            save->gdtnum_out = -1;
        }

        save->data_out = (double *) malloc(2 * sizeof(double) * (size_t) n_out);
        save->data_wrt = (float *) malloc(sizeof(double) * (size_t) n_out);
        save->bitmap_out = (unsigned char *) malloc(sizeof(char) * (size_t) n_out);
        if (save->rlat == NULL || save->rlon == NULL || save->crot == NULL || save->srot == NULL || 
            save->data_out == NULL || save->data_wrt == NULL || save->bitmap_out == NULL) 
                fatal_error("new_grid memory allocation","");
        return 0;
    }

    save = (struct local_struct *) *local;

    if (mode == -2) {                   // cleanup
#ifdef G95
        if (g95_runstop == 1) { g95_runtime_stop(); g95_runstop = 0; }
#endif
        if (save->has_u > 0) {
            fprintf(stderr,"-new_grid: last field %s was not interpolated (missing V)\n", save->name);
            free(save->u_val);
            free_sec(save->clone_sec);
        }
        free(save->rlon);
        free(save->rlat);
        free(save->crot);
        free(save->srot);
        free(save->data_out);
        free(save->data_wrt);
        free(save->bitmap_out);
        free(save->sec3);
        fclose_file(&(save->out));
        free(save);

        return 0;
    }
    if (mode >= 0) { /* processing grid */

        if (output_order != raw) fatal_error("new_grid: must be in raw output order","");
        if (nx_ == 0 || ny_ == 0) {
            fprintf(stderr,"new_grid: does not work with thinned or non-grids (nx=%u, ny=%u)\n", nx_, ny_);
            return 0;
        }
        if (scan & 16) {
            fprintf(stderr,"new_grid: rows must scan in same direction\n");
            return 0;
        }
        if (scan & 31) {
            fprintf(stderr,"new_grid: staggered\n");
            return 0;
        }

        /* The kgds of some output grids will change depending on input grid */
        /* for example, radius of earth is not known until grib file is read, */
        /*   and mass vs wind fields */
        /* right now, only affects lambertc and location */

        if (strncmp(arg1,"lambertc:",8) == 0) {

            // lambertc depends on the radius of the earth which is
            // set by the input grib file

            /* read earth radius */
            i = axes_earth(sec, &r_maj, &r_min, NULL);
            if (i) fatal_error_i("axes_earth: error code %d", i);

            if (save->radius_major != r_maj || save->radius_minor != r_min) {

                // update sec3 and kgds

                i = sscanf(arg1,"lambertc:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
                if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
                if (lov < 0.0)  lov += 360.0;
                if (i < 3) latin2 = latin1;
                if (i < 4) lad = latin2;
                proj = 0;
                if (latin2 < 0.0) proj = 128;

                if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                    fatal_error("new_grid: XDEF wrong:%s",arg2);
                if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                    fatal_error("new_grid: YDEF wrong:%s",arg3);

                if (x0 < 0.0) x0 += 360.0;
                save->nx = nnx;
                save->ny = nny;
                check_grid_size(nnx, nny);
                save->npnts_out = n_out = nnx*nny;

                ref_lon = x0;
                ref_lat = y0;

                i = new_grid_lambertc(nnx, nny, ref_lon, ref_lat, latin1, latin2, lov, lad, r_maj, r_min, dx, dy, &x0, &y0);
                if (i) fatal_error_i("new_grid_lambertc: error code %d", i);

                // make a new section 3
                s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);

                // save new section 3
                i = (int) uint4(s);         // size of section 3
                for (j = 0; j < i; j++) save->sec3[j] = s[j];

                // make gdt
                new_sec[1] = blank_sec1;        // add center info
                new_sec[3] = save->sec3;

                if (mk_gdt(new_sec, &(save->gdtnum_out), &(save->gdt_out[0]), &(save->gdt_out_size) ))
                  fatal_error("new_grid: encoding output gdt","");

                // save radius of earth, to show sec3 and kgds has been done
                save->radius_major = r_maj;
                save->radius_minor = r_min;

                // apply wind rotation .. change flag 3.3

                if (save->defined_lat_lon == 1) wind_rotation = earth;
                if (wind_rotation == undefined && strncmp(arg1,"grib",4) != 0) {
                    fprintf(stderr,"Warning: -new_grid wind orientation undefined, "
                        "use \"-new_grid_winds (grid|earth)\", earth used\n");
                    wind_rotation = earth;
                }
                if ((p = flag_table_3_3_location(new_sec)) != NULL) {
                    if (wind_rotation == grid) *p = *p | 8;
                    else if (wind_rotation == earth) *p = *p & (255 - 8);
                }
            }
        }
        else if (strncmp(arg1,"location",8) == 0) {
            i = code_table_3_2(sec);
            if (i == 0 || i == 2 || i == 4 || i == 5 || i == 6 || i == 8 || i == 9) {
                save->sec3[14] = i;
            }
            else {
                save->sec3[14] = 255;
            }
            /* the gdt may change because of radius, but no need to update */
        }
        /* 3/2021: update shape of earth */
        p = code_table_3_2_location(sec);
        /* new_grid will only interpolate to grid with shape of earth in 16 bytes*/
        if (p) for (i = 0; i < 16; i++)  save->sec3[14+i] = p[i];
 
        i = getName(sec, mode, NULL, name, NULL, NULL);

        is_u = is_v = 0;
        for (j = 0; vectors[j] != NULL; j++) {
            if (strcmp(name,vectors[j]) == 0) {
                if (j % 2 == 0) is_u = 1;
                else is_v = 1;
                break;
            }
        }

        // check if V matches expectation
        // could cause a problem as record is ignored */

        if (is_v && (save->has_u == 0  || (same_sec0(sec,save->clone_sec) != 1 ||
            same_sec1(sec,save->clone_sec) != 1 ||
            same_sec3(sec,save->clone_sec) != 1 ||
            same_sec4(sec,save->clone_sec) != 1) )) {
            fprintf(stderr,"-new_grid: %s doesn't pair with previous vector field, field ignored\n", name);
            return 0;
        }

        // if U field - to sec 

        if (is_u) {
            if (save->has_u > 0) {
                fprintf(stderr,"-new_grid: missing V, %s not interpolated\n",save->name);
                free(save->u_val);
                free_sec(save->clone_sec);
            }
            copy_sec(sec, save->clone_sec);
            copy_data(data,ndata,&(save->u_val));
            GB2_ParmNum(save->clone_sec) = GB2_ParmNum(sec) + 1;
            save->has_u = 1;
            strncpy(save->name, name,NAMELEN-1);
            save->name[NAMELEN-2]=0;
            return 0;
        }


        /* if grib output and output_latlon == 1, write out grib lat/lon */
        
        if (save->output_latlon == 1 && new_grid_format == grib ) {
            n_out = save->npnts_out;
            nnx = save->nx;
            nny = save->ny;
            p_discipline = code_table_0_0_location(sec);
            discipline = *p_discipline;
            *p_discipline = 0;

            for (i = 0; i < 34; i++) sec4_latlon[i] = 0;
            sec4_latlon[3] = 34;
            sec4_latlon[4] = 4;
            sec4_latlon[9] = 191;	// Misc
            sec4_latlon[10] = 1;	// 1 = lat 2 = lon
            sec4_latlon[11] = 0;	// analysis
            for (i = 12; i <= 16; i++) sec4_latlon[i] = 255;
            sec4_latlon[17] = 0;
            sec4_latlon[18] = 0;
            sec4_latlon[19] = 0;
            sec4_latlon[20] = 0;
            sec4_latlon[22] = 1;	// surface
            for (i = 23; i <= 33; i++) sec4_latlon[i] = 255;

            for (i = 0; i < 8; i++) new_sec[i] = sec[i];
            new_sec[3] = save->sec3;
            new_sec[4] = sec4_latlon;
            new_sec[4][10] = 2;

            for (i = 0; i < n_out; i++) {
                itmp = floor(save->rlon[i] * 1000.0 + 0.5);
                save->data_wrt[i] = itmp * 0.001;
            }
            grib_wrt(new_sec, save->data_wrt, n_out, nnx, nny, 1, -3, 0,
                18, 18, complex1, &(save->out));
            for (i = 0; i < n_out; i++) {
                itmp = floor(save->rlat[i] * 1000.0 + 0.5);
                save->data_wrt[i] = itmp * 0.001;
            }
            new_sec[4][10] = 1;
            grib_wrt(new_sec, save->data_wrt, n_out, nnx, nny, 1, -3, 0,
                18, 18, complex1, &(save->out));
            *p_discipline = discipline;
            save->output_latlon = 0;
        }


        /* at this point will call polates with either a scalar or vector depending on is_v */

        n_out = save->npnts_out;
        nnx = save->nx;
        nny = save->ny;
        km = 1;                 // only one scalar or vector field


        gdt_in_size = sizeof(gdt_in) / sizeof(gdt_in[0]);
        if (mk_gdt(sec, &gdtnum_in, &(gdt_in[0]), &gdt_in_size ))
            fatal_error("new_grid: encoding input gdt","");

        data_in = (double *) malloc((1 + (is_v != 0)) * sizeof(double) * (size_t) ndata);
        bitmap = (unsigned char *) malloc(sizeof(unsigned char) * (size_t) ndata);

        if (data_in == NULL || bitmap == NULL)
            fatal_error("new_grid: memory allocation problem","");

        /* data format for ipolates, calling double precision ip2lib
           double precision data_in[0..npts-1] for scalar
           double precision data_in[0..2*npts-1] for vector 
           ibi = 0 if bitmap[] is not used (all defined)
                 1 if bitmap[] is used 
         */

        ibi = 0;
        if (is_v) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(|:ibi)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->u_val[i])) {
                    data_in[i] = save->u_val[i];
                    data_in[i+ndata] = data[i];
                    bitmap[i] = 1;
                }
                else {
                    data_in[i] = data_in[i + ndata] = 0.0;
                    ibi = ibi | 1;
                    bitmap[i] = 0;
                }
            }
            if (mode == 98) fprintf(stderr," UV interpolation %s , %s\n", save->name, name);
        }
        else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(|:ibi)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(data[i])) {
                    data_in[i] = data[i];
                    bitmap[i] = 1;
                }
                else {
                    data_in[i] = 0.0;
                    ibi = ibi | 1;
                    bitmap[i] = 0;
                }
            }
        }

        // interpolate

        tmp_interpol_type = interpol_type;
        if (interpol_type == 4 && ibi == 1) {
            fprintf(stderr,"new_grid: undef values, spectral interpolation changed to binlinear\n");
            tmp_interpol_type = 0;
        }

        ftn_npnts = (int) ndata;
        ftn_nout = (int) n_out;

        if (is_v) {
            ipolatev_grib2_single_field(&tmp_interpol_type, ipopt, &gdtnum_in, &(gdt_in[0]), 
                &gdt_in_size, &(save->gdtnum_out), 
                &(save->gdt_out[0]), &(save->gdt_out_size),
                &ftn_npnts, &ftn_nout, &km, &ibi, bitmap, data_in, data_in+ndata, &n_out,
                save->rlat,save->rlon,save->crot,save->srot, &ibo, save->bitmap_out, save->data_out, 
                save->data_out + n_out, &iret);
        }
        else {
            ipolates_grib2_single_field(&tmp_interpol_type, ipopt, &gdtnum_in, &(gdt_in[0]), 
                &gdt_in_size, &(save->gdtnum_out), 
                &(save->gdt_out[0]), &(save->gdt_out_size),
                &ftn_npnts, &ftn_nout, &km, &ibi, bitmap, data_in, &n_out,
                save->rlat,save->rlon, &ibo, save->bitmap_out, save->data_out, &iret);
        }
        free(data_in);
        free(bitmap);

        if (iret != 0) {
            if (iret == 2) fatal_error("IPOLATES failed, unrecognized input grid or no grid point of output grid is in input grid"
                 ,"");
            if (iret == 3) fatal_error("IPOLATES failed, unrecognized output grid","");
	    if (iret == 41 && tmp_interpol_type == 4) fatal_error("IPOLATES failed, non-global grid for spectral interpolation","");

            fatal_error_i("IPOLATES failed, error %d",iret);
        }

        // save data to data_wrt[]

        if (ibo == 1) {		// has a bitmap
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < n_out; i++) {
                save->data_wrt[i] = save->bitmap_out[i] == 0 ? UNDEFINED : save->data_out[i];
            }
        }
        else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < n_out; i++) {
                save->data_wrt[i] = save->data_out[i];
            }
        }

        if (new_grid_format == grib) {
            for (i = 0; i < 8; i++) new_sec[i] = sec[i];
            new_sec[3] = save->sec3;
            // write scalar or U

            if (is_v == 1) {		// change V -> U
                GB2_ParmNum(new_sec) = GB2_ParmNum(new_sec) - 1;
            }

            grib_wrt(new_sec, save->data_wrt, n_out, nnx, nny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));

            // write V if necessary

            if (is_v == 1) {		// vector
                GB2_ParmNum(new_sec) = GB2_ParmNum(new_sec) + 1;
                if (ibo == 1) {		// has a bitmap
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                    for (i = 0; i < n_out; i++) {
                        save->data_wrt[i] = save->bitmap_out[i] == 0 ? UNDEFINED : save->data_out[i+n_out];
                    }
                }
                else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                    for (i = 0; i < n_out; i++) {
                        save->data_wrt[i] = save->data_out[i+n_out];
                    }
                }
                grib_wrt(new_sec, save->data_wrt, n_out, nnx, nny, use_scale, dec_scale, bin_scale,
                    wanted_bits, max_bits, grib_type, &(save->out));
            }
        }
        else if (new_grid_format == ieee) {
            wrtieee(&(save->data_wrt[0]), n_out, header, &(save->out));
            if (is_v == 1) {		// vector
                if (ibo == 1) {		// has a bitmap
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                    for (i = 0; i < n_out; i++) {
                        save->data_wrt[i] = save->bitmap_out[i] == 0 ? UNDEFINED : save->data_out[i+n_out];
                    }
                }
                else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                    for (i = 0; i < n_out; i++) {
                        save->data_wrt[i] = save->data_out[i+n_out];
                    }
                }
                wrtieee(&(save->data_wrt[0]), n_out, header, &(save->out));
            }
        }
        else if (new_grid_format == bin) {
            if (header) {
                if (n_out > 4294967295U / sizeof(float))
                    fatal_error("new_grid: 4-byte header overflow bin header","");
            	i = n_out * sizeof(float);
                fwrite_file((void *) &i, sizeof(int), 1, &(save->out));
            }
            j = fwrite_file((void *) &(save->data_wrt[0]), sizeof(float), n_out, &(save->out));
            if (j != n_out) fatal_error("new_grid: error in write", "");
            if (header) fwrite_file((void *) &i, sizeof(int), 1, &(save->out));

            if (is_v == 1) {		// vector
                if (ibo == 1) {		// has a bitmap
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                    for (i = 0; i < n_out; i++) {
                        save->data_wrt[i] = save->bitmap_out[i] == 0 ? UNDEFINED : save->data_out[i+n_out];
                    }
                }
                else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                    for (i = 0; i < n_out; i++) {
	                    save->data_wrt[i] = save->data_out[i+n_out];
                    }
                }

            	i = n_out * sizeof(float);
                if (header) fwrite_file((void *) &i, sizeof(int), 1, &(save->out));
                j = fwrite_file((void *) &(save->data_wrt[0]), sizeof(float), n_out, &(save->out));
                if (j != n_out) fatal_error("new_grid: error in write", "");
                if (header) fwrite_file((void *) &i, sizeof(int), 1, &(save->out));
            }
        }

        if (flush_mode) fflush_file(&(save->out));
        /* cleanup for u/v case */    
        if (is_v != 0) {
            save->has_u = 0;
            free(save->u_val);
            free_sec(save->clone_sec);
        }
    }
    return 0;
}

#endif

#ifndef USE_IPOLATES
int f_new_grid_vectors(ARG1) {
    (void) UV_vectors;
    (void) no_vectors;
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid_interpolation(ARG1) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid_ipopt(ARG1) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid(ARG4) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid_winds(ARG1) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid_format(ARG1) {
    fprintf(stderr,"IPOLATES (ip2lib_d) package is not installed\n");
    return 1;
}
#endif
