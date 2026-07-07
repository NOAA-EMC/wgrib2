/** @file
 * @brief RPN - reverse polish notation calculator.
 * 
 * operations:
 *
 *     + - * /
 *    == != < <= > >=
 *
 *    sqrt, sq, abs, 1/x, floor, ceil, pow (x^y), exp, ln
 *    min, max, merge, mask
 *    sin, cos, tan, asin, acos, atan, atan2
 *
 *    pi = 3.14159...
 *    days_in_ref_month = number of days in the reference month
 *    days_in_verf_month = number of days in the verification month
 *
 *    registers: sto_N, rcl_N, clr_N
 *               rcl_lat, rcl_lon, sto_lat, sto_lon
 *               rcl (data)
 *    stack: exc (swap), pop, dup, clr
 *
 *    yrev - swap grids, north <-> south
 *    raw2 - convert from input scan mode to output scan mode
 *    2raw - convert from output scan mode to input scan mode
 *    alt_x_scan - for Glahn packing
 *    xave, xdev
 *
 *    print_(X):  X=max, min, rms, corr, ave, diff
 *
 * At the end of rpn, the top of the stack is saved to data unless clr done first.
 *
 * @author Public Domain: Wesley Ebisuzaki @date 4/2009
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "wmath.h"
#include "wgrib2.h"
#include "fnlist.h"

// #define N_RPN_REGS 10 moved to wgrib2.h

/** Stack size for RPN calculator. */
#define STACK_SIZE 10

/** Decode grib file flag. */
extern int decode;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Flag to indicate whether to save translation information. */
extern int save_translation;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Run flag. */
extern int run_flag;

/** Item delimiter string. */
extern const char *item_deliminator;

/** Use scaling flag. */
extern int use_scale;

/** Pointer to array of translation values. */
extern unsigned int *translation;

/** Current geolocation type. */
extern enum geolocation_type geolocation;

/* note: rpn_n[N_RPN_REGS], and rpn_data[N_RPN_REG] */

/** Size of RPN registers.*/
size_t rpn_n[N_RPN_REGS] = { 0 };

/** Pointer to array of RPN register data. */
float *rpn_data[N_RPN_REGS] = { NULL };

/** Stack for RPN calculator. */
static float *stack[STACK_SIZE];

#define SCALAR 0 /**< Scalar type code value. */
#define VECTOR 1 /**< Vector type code value. */
#define DBL_VEC 2 /**< Double vector type code value. */

int push(int top, unsigned int ndata, int type, float f, float *ff, double *d);
static void gbl_wt(double *val, double *wt, float *data, int i, int j, int nx, int ny, double wt0);
static void reg_wt(double *val, double *wt, float *data, int i, int j, int nx, int ny, double wt0);

/*
 * HEADER:100:rpn:misc:1:reverse polish notation calculator
 */

/**
 * Runs a reverse polish notation (RPN) calculator. 
 * 
 * Having a built-in calculator is quite handy. We use it to convert units (ex. geopotential to geopotential meters, accumulations to rates), compute simple quantities (net flux from downward and upward fluxes), and even compute the plant hardiness index from the 2 m temperatures. The goal of the calculator is to reduce the need to write simple grib programs that do simple calculations. 
 * 
 * The "hardware" of the rpn calculator consists of 20 registers and a stack (10 entries deep). (Wgrib2 prior to 2.0.6 has 10 registers.) Stack entries and registers are arrays rather floating point numbers on your store-bought calculator. 
 * 
 * The conceptual model of the >-rpn calculator is the grid values array is the top of the stack. The calculator has a statck that is 10 entries deep and 20 registers. When ever you enter rpn mode, the stack is cleared except for the top of the stack. The registers are only clears when you "turn on" the calculator; that is start the wgrib2 utility or the first call to wgrib2 if you are using calling wgrib2 through wgrib2api or pywgrib2. 
 * 
 * To save the calculations, you can save them in a register or write them out by -grib_out, -bin, -ieee, -text, etc. 
 * 
 * ## wgrib2api
 * The wgrib2api uses the -rpn registers to transfer gridded data between the calling program and the wgrib2 subroutine. A Fortran or C program can read and write the wgrib2's RPN registers. For example, if a program wants to write a grib2 file, it would first place the grid values into a register. The calling program would then call the wgrib2 subroutine with instructions to read a template, replace the grid values with the register values, and then write a grib message. 
 * 
 * ## Implementation Details
 * The size of a registers may differ as the size of grids can also vary in a grib file. However the size of the register has to match the size of the stack entry in order "recall" the register. 
 * 
 * Wgrib2 always reads a grib message before processing it using commmands like -rpn. This sets the size of the data array. Thus the size of the stack entries is always the size of the grib message that is being processed. 
 * 
 * The conceptual moddel is the data array (grid values) is the top of the stack. The implmentation is that the data array is copied to the top of the stack when -rpn is run, and the top of the stack is copied to the data array when -rpn is finished. An error message will occur if the top of the stack is empty when -rpn finishes. 
 * 
 * ## Limitations of -rpn
 * The -rpn option was designed for simple calculations. For more complicated calculations, you 
 * should use a real programming language. You can do the calculations in another step and then 
 * import the results using one of the various -import_* options. You can use wgrib2api (Fortran 
 * and C) to read the data into a Fortran or C program, do the calculation and then write the data 
 * out using wgrib2. Finally you can use python and one of the pywgrib2 packages to do the calculation 
 * using Python, numpy and [pywgrib2](https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/pywgrib2.html). 
 * 
 * ## Uses
 * - change of units when importing data (gribifying data)
 * - computations: ex, U,V -> wind speed, wind direction, potential temperature
 * - merging data
 * - complex masking of data
 * - changing units before writing text/ieee files
 * - removing extreme data values
 * - finding min and max values
 * - finding the globally averaged precipitation
 * - comparing fields
 * 
 * ## Usage
 * -rpn  "A:B:C:..."
 * 
 * A,B,C,.. = number, rpn function, or rpn operator
 * 
 * ### Operators and Functions
 * Pop X, Push Fn(X)
 * 
 * - abs: absolute value
 * - acos: arc cos, [0, pi] radians
 * - alt_x_scan: changes alternate x scanning to regular x scanning and vice versa
 * - asin: arc sin, result is [-pi/2, pi/2] radians
 * - atan: arc tan, result is [-pi/2, pi/2] radians, see atan2
 * - abs: absolute value
 * - ceil: smallest integer >= X
 * - cos: cosine, argument in radians
 * - exp: e^X
 * - floor: largest integer <= X
 * - ln: natural logorithm
 * - raw2: [convert from input scan order to output scan order]
 * (https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/rpn_raw2_2raw.html)
 * - sin: sine, argument in radians
 * - smth9g: smth9 for global fields, 9 point smoothing, undefined values adjacent to defined values are defined
 * - smth9r: snth9 for regional fields, 9 point smoothing, undefined values adjacent to defined values are defined
 * - sq: X*X
 * - sqrt: square root
 * - tan: tangent, argument in radians
 * - xave: for nx-ny grids, averages in the x direction (normally zonal mean)
 * - xdev: for nx-ny grids, remove x average (normally deviation from zonal mean)
 * - yrev: for nx-ny grids, changes we:sn to we:ns and vice versa
 * - 1/x: 1/X
 * - 2raw: [convert from output scan order to input scan order]
 * (https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/rpn_raw2_2raw.html)
 * 
 * Pop Y, Pop X, push Fn(X,Y)
 * 
 * - +: push X+Y
 * - -: push X-Y
 * - *: push X*Y
 * - /: push X/Y
 * - <: push X < Y    (1/0 if true/false)
 * - <=: push X <= Y  (1/0 if true/false)
 * - ==: push X == Y  (1/0 if true/false)
 * - !=: push X != Y  (1/0 if true/false)
 * - >=: push X >= Y  (1/0 if true/false)
 * - >: push X > Y    (1/0 if true/false)
 * - atan2: push arctan(X/Y), result is [-pi, pi] radians, see atan
 * - pow: push X**Y (X^Y)
 * - mask: if (Y != 0) push(X) else push(UNDEFINED)
 * - max: push max(X,Y), for logical values, max is the same as OR
 * - merge: if (Y != UNDEFINED) push(Y) else push(X)
 * - min: push min(X,Y), for logical values, min is the same as AND
 * 
 * Note: an operation involving an UNDEFINED is UNDEFINED
 * 
 * ### Stack Operators
 *
 * - clr, clear the stack (stack is emptied)
 * - dup, duplicate the top of the stack
 * - pop, remove the top of the stack
 * - exc/swap, exchange the top 2 stack entries
 *
 * ### Register Operators: (note: CW2 v2.0.6+ uses registers 7,8,9 prior versions 0,1,2) 
 * - clr_I, clear register I,  I=0,1..,9   (19 for v2.0.6+)
 * - rcl_I, push register I on top of stack,  I=0,1..,9  (19 for v2.0.6+)
 * - sto_I, save top of stack in register I,  I=0,1..,9  (19 for v2.0.6+)
 * - rcl_lat, push latitudes onto the top of the stack (degrees)
 * - rcl_lon, push longitudes onto the top of the stack (degrees)
 * - sto_lon, save top of stack as longitudes (degrees) (wgrib2 v3.0.0+)
 * - sto_lat, save top of stack as latitudes (degrees) (wgrib2 v3.0.0+)
 * - note: latitudes and longitudes are double precision values, the stack is single precision
 * 
 * ### Variables and Constants: put on the top of the stack 
 * - number               number = floating point or integer number like 0, 10.1, -1.23e-4
 * - days_in_ref_month    number of days in the month for the reference date (conversion between monthly acc. and rates)
 * - days_in_verf_month   number of days in the month for the verification time (conversion between monthly acc. and rates)
 * - pi                   3.1415....
 * - rand                 random number uniformly distributed between 0 and 1, each grid point has a different random number
 * 
 * ### Printing Operators
 * - print_corr, write cosine weighted spatial correlation, data[TOP] data[TOP-1]
 * - print_max, print_min, data[TOP]
 * - print_rms, write cosine weighted RMS, data[TOP]-data[TOP-1]
 * - print_diff, write cosine weighted difference, data[TOP-1]-data[TOP]
 * - print_ave, write cosine weighted average,  grid_ave(data[TOP]*cos(grid))/grid_ave(cos(grid))
 * - print_wt_ave, write weighted average grid_ave, data[TOP] is the weighting
 *      print grid_ave(data[TOP]*data[TOP-1])/grid_ave(data[TOP])
 * 
 * ## Comments
 * Warning: Reverse Polish notation can cause headaches if you try something too complicated. 
 * An infix -> postfix calculator is the suggested remedy. Another approach is to use 
 * pywgrib2_s, pywgrib2_xy, or pywgrib2_lite. 
 * 
 * The -rpn option is a piece of easy to understand and modify code (RPN.c). If you want to add 
 * a specialized function (ex. wind chill calculation), you many consider adding it to the RPN 
 * calculator. The another method is to code your calculation in python and use pywgrib2_s, pywgrib2_xy 
 * or pywgrib2_lite. 
 * 
 * Why an RPN calculator? Well, wgrib2 is heavily influenced by the stack language Forth. It's only 
 * natural that the calculator would be based on reverse Polish notation. 
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int f_rpn(ARG1) {
    char string[100], *endptr;
    const char *p;
    int j, n;
    unsigned int i, k, m;
    float f;
    float tmp;
    int top, flag;
    double cos_lat, last_lat;
    double sum1, sum2, wt, sq1, sq2, sq12;
    int nx, ny, res, scan;
    unsigned int npnts;
    float *p1, *p2;

    int year, month, day, hour, minute, second;

    static int state=0;

    if (mode == -1) {
        decode = latlon = 1;
        save_translation = 1;
        if (state == 0) {
            /* check compile-time configuration */
            if (sizeof(rpn_n)/sizeof(size_t) != N_RPN_REGS) 
                fatal_error("RPN: configure N_RPN_REGS and rpn_n[]","");
            if (sizeof(rpn_data)/sizeof(float *) != N_RPN_REGS) 
                fatal_error("RPN: configure N_RPN_REGS and rpn_data[]","");
            state = 1;
        }
        return 0;
    }
    if (mode == -2) {
/*  5/2015 no cleanup for callable wgrib2, preserve registers
	if (state == 1) {
            for (i = 0; i < N_RPN_REGS; i++) {
	        if (rpn_data[i]) {
		    free (rpn_data[i]);
		    rpn_data[i] = NULL;
	            rpn_n[i] = 0;
	        }
	    }
	}
	state = 0;
 */
        return 0;
    }

    // initialize stack

    if (data == NULL) fatal_error("rpn: decode failed","");
    use_scale = 0;

    for (i = 0; i < STACK_SIZE; i++) stack[i] = NULL;
    top = push(-1, ndata, VECTOR, 0.0, data, NULL);

    if (mode == 98) fprintf(stderr,"RPN: arg=%s\n",arg1);

    // scan parameters

    p = arg1;
    while (sscanf(p,"%[^:]%n", string, &n) == 1) {
        if (mode == 98) fprintf(stderr, "RPN: top=%d (%s)", top, string);
        p = p + n;
        if (*p == ':') p++;

        // binary operators + - * /
        if (strcmp(string,"+") == 0) {
            if (mode == 98) fprintf(stderr," plus");
            if (top <= 0) fatal_error("rpn: bad + expression","");
            j = top-1;
#ifdef USE_OPENMP
#pragma omp parallel for private(i) schedule(static)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] = stack[j][i] + stack[top][i];
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"-") == 0) {
            if (mode == 98) fprintf(stderr," minus");
            if (top <= 0) fatal_error("rpn: bad - expression","");
            j = top-1;
#ifdef USE_OPENMP
#pragma omp parallel for private(i) schedule(static)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] = stack[j][i] - stack[top][i];
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"*") == 0) {
            if (mode == 98) fprintf(stderr," times");
            if (top <= 0) fatal_error("rpn: bad * expression","");
            j = top-1;
#ifdef USE_OPENMP
#pragma omp parallel for private(i) schedule(static)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] = stack[j][i] * stack[top][i];
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"/") == 0) {
            if (mode == 98) fprintf(stderr," div");
            if (top <= 0) fatal_error("rpn: bad / expression","");
            j = top-1;
#ifdef USE_OPENMP
#pragma omp parallel for private(i) schedule(static)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i]) && (stack[top][i] != 0.0)) {
                    stack[j][i] = stack[j][i] / stack[top][i];
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }

        // merge:  stack(top-1) = stack(top) (if defined) ; top--;

        else if (strcmp(string,"merge") == 0) {
            if (mode == 98) fprintf(stderr," merge");
            if (top <= 0) fatal_error("rpn: bad merge expression","");
            j = top-1;
#ifdef USE_OPENMP
#pragma omp parallel for private(i) schedule(static)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[j][i] = stack[top][i];
                }
            }
            top--;
        }

        // exc (swap top and top-1 stack entries)

        else if (strcmp(string,"exc") == 0 || strcmp(string,"swap") == 0) {

            if (mode == 98) fprintf(stderr," exchange");
            if (top <= 0) fatal_error("rpn: bad exc/swap expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                f = stack[j][i];
                stack[j][i] = stack[top][i];
                stack[top][i] = f;
            }
        }

        // pop:  top--;

        else if (strcmp(string,"pop") == 0) {
            if (mode == 98) fprintf(stderr," pop");
            if (top < 0) fatal_error("rpn: bad pop","");
            top--;
        }

        // dup: top++; stack(top) = stack(top-1)

        else if (strcmp(string,"dup") == 0) {
            if (mode == 98) fprintf(stderr," dup");
            top = push(top,ndata,VECTOR,0.0,stack[top],NULL);
        }

        //  sqrt: stack(top) = sqrt(stack(top))

        else if (strcmp(string,"sqrt") == 0) {
            if (mode == 98) fprintf(stderr," sqrt");
            if (top < 0) fatal_error("rpn: bad sqrt expression","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i) schedule(static)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && stack[top][i] >= 0.0) {
                    stack[top][i] = sqrtf(stack[top][i]);
                    }
                else stack[top][i] = UNDEFINED;
            }
        }
        // sq: x*x
        else if (strcmp(string,"sq") == 0) {
            if (mode == 98) fprintf(stderr," sq");
            if (top < 0) fatal_error("rpn: bad sq expression","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i) schedule(static)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] *= stack[top][i];
                }
            }
        }
        // pow: x^y
        else if (strcmp(string,"pow") == 0) {
            if (top <= 0) fatal_error("rpn: bad pow expression","");
            j = top-1;
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] = powf(stack[j][i], stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        // ln - natural log
        else if (strcmp(string,"ln") == 0) {
            if (top < 0) fatal_error("rpn: bad log expression","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && stack[top][i] > 0.0) {
// glib bug                    stack[top][i] = logf(stack[top][i]);
//                    stack[top][i] = log(stack[top][i]);
                    stack[top][i] = logf(stack[top][i]);
                }
                else stack[top][i] = UNDEFINED;
            }
        }

        // exp
        else if (strcmp(string,"exp") == 0) {
            if (top < 0) fatal_error("rpn: bad exp expression","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = expf(stack[top][i]);
                }
            }
        }

        // abs
        else if (strcmp(string,"abs") == 0) {
            if (top < 0) fatal_error("rpn: bad abs expression","");
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    if (stack[top][i] < 0.0) stack[top][i] = -stack[top][i];
                }
            }
        }

        // 1/x

        else if (strcmp(string,"1/x") == 0) {
            if (mode == 98) fprintf(stderr," 1/x");
            if (top < 0) fatal_error("rpn: bad 1/x","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && stack[top][i] != 0.0) {
                    stack[top][i] = 1.0 / stack[top][i];
                }
                else stack[top][i] = UNDEFINED;
            }
        }

        // floor

        else if (strcmp(string,"floor") == 0) {
            if (top < 0) fatal_error("rpn: bad floor","");
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = floorf(stack[top][i]);
                }
            }
        }

        // ceil

        else if (strcmp(string,"ceil") == 0) {
            if (top < 0) fatal_error("rpn: bad ceil","");
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = ceilf(stack[top][i]);
                }
            }
        }

        // sin cos tan asin acos atan
        else if (strcmp(string,"sin") == 0) {
            if (top < 0) fatal_error("rpn: bad sin","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = sinf(stack[top][i]);
                }
            }
        }
        else if (strcmp(string,"cos") == 0) {
            if (top < 0) fatal_error("rpn: bad cos","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = cosf(stack[top][i]);
                }
            }
        }
        else if (strcmp(string,"tan") == 0) {
            if (top < 0) fatal_error("rpn: bad tan","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = tanf(stack[top][i]);
                }
            }
        }
        else if (strcmp(string,"asin") == 0) {
            if (top < 0) fatal_error("rpn: bad asin","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    if (fabsf(stack[top][i]) > 1.0) stack[top][i] = UNDEFINED;
                    else stack[top][i] = asinf(stack[top][i]);
                }
            }
        }
        else if (strcmp(string,"acos") == 0) {
            if (top < 0) fatal_error("rpn: bad acos","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    if (fabsf(stack[top][i]) > 1.0) stack[top][i] = UNDEFINED;
                    else stack[top][i] = acosf(stack[top][i]);
                }
            }
        }
        else if (strcmp(string,"atan") == 0) {
            if (top < 0) fatal_error("rpn: bad atan","");
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = atanf(stack[top][i]);
                }
            }
        }
        else if (strcmp(string,"atan2") == 0) {
            if (top <= 0) fatal_error("rpn: bad atan2 expression","");
            j = top-1;
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] = atan2f(stack[j][i], stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }

        // sto_N
        else if (string[0] == 's' && string[1] == 't' && string[2] == 'o' && string[3] == '_' 
                && isdigit((unsigned char) string[4]) && (string[5] == 0 || (isdigit((unsigned char) string[5]) && string[6] == 0) )) {
            if (top < 0) fatal_error("rpn: sto","");
            j = atoi(string+4);
            if (j >= N_RPN_REGS || j < 0) fatal_error("rpn: bad register number in %s", string);
            if (ndata != rpn_n[j]) {
                if (rpn_data[j]) free(rpn_data[j]);
                rpn_n[j] = ndata;
                rpn_data[j] = (float *) malloc(sizeof(float) * (size_t) ndata);
                if (rpn_data[j] == NULL) fatal_error("rpn: memory allocation failed in %s",string);
            }
            for (i=0; i < ndata; i++) {
                rpn_data[j][i] = stack[top][i];
            }
        }

        // rcl_N
        else if (string[0] == 'r' && string[1] == 'c' && string[2] == 'l' && string[3] == '_'
                && isdigit((unsigned char) string[4]) && (string[5] == 0 || (isdigit((unsigned char) string[5]) && string[6] == 0) )) {
            j = atoi(string+4);
            if (j >= N_RPN_REGS || j < 0) fatal_error("rpn: bad register number in %s", string);

            if (rpn_n[j] != 0 && rpn_n[j] != ndata) fatal_error("rpn: rcl size mismatch","");

            if (rpn_n[j] == 0) {	// unused register are zero
                top = push(top,ndata,SCALAR,0.0,rpn_data[j],NULL);
            }
            else {
                top = push(top,ndata,VECTOR,0.0,rpn_data[j],NULL);
            }
        }

        // clr_N
        else if (string[0] == 'c' && string[1] == 'l' && string[2] == 'r' && string[3] == '_'
                && isdigit((unsigned char) string[4]) && (string[5] == 0 || (isdigit((unsigned char) string[5]) && string[6] == 0) )) {
            j = atoi(string+4);
            if (j >= N_RPN_REGS || j < 0) fatal_error("rpn: bad register number in %s", string);
            if (rpn_data[j]) {
                free(rpn_data[j]);
                rpn_data[j] = NULL;
            }
            rpn_n[j] = 0;
        }

        // rcl_lat
        else if (strcmp(string,"rcl_lat") == 0) {
            if (lat == NULL) fatal_error("rpn: rcl_lat: lat not defined","");
            top = push(top,ndata,DBL_VEC,0.0,NULL,lat);
        }
        // rcl_lon
        else if (strcmp(string,"rcl_lon") == 0) {
            if (lon == NULL) fatal_error("rpn: rcl_lon: lon not defined","");
            top = push(top,ndata,DBL_VEC,0.0,NULL,lon);
        }
        else if (strcmp(string,"sto_lat") == 0) {
            if (lat == NULL) {
                lat = (double *) malloc(sizeof(double) * (size_t) ndata);
                if (lat == NULL) fatal_error("rpn: bad malloc for sto_lat","");
            }
            for (i = 0; i < ndata; i++) {
                lat[i] = stack[top][i];
            }
            geolocation = external;
        }
        else if (strcmp(string,"sto_lon") == 0) {
            if (lon == NULL) {
                lon = (double *) malloc(sizeof(double) * (size_t) ndata);
                if (lon == NULL) fatal_error("rpn: bad malloc for sto_lon","");
            }
            for (i = 0; i < ndata; i++) {
                lon[i] = stack[top][i];
            }
            geolocation = external;
        }

        // max and min

        else if (strcmp(string,"max") == 0) {
            if (top <= 0) fatal_error("rpn: bad max expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    if (stack[j][i] < stack[top][i]) stack[j][i] = stack[top][i];
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"min") == 0) {
            if (top <= 0) fatal_error("rpn: bad min expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    if (stack[j][i] > stack[top][i]) stack[j][i] = stack[top][i];
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }

        else if (strcmp(string,">") == 0) {
            if (top <= 0) fatal_error("rpn: bad > expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] =  (stack[j][i] > stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,">=") == 0) {
            if (top <= 0) fatal_error("rpn: bad >= expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] =  (stack[j][i] >= stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"!=") == 0) {
            if (top <= 0) fatal_error("rpn: bad != expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] =  (stack[j][i] != stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"==") == 0) {
            if (top <= 0) fatal_error("rpn: bad == expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] =  (stack[j][i] == stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"<") == 0) {
            if (top <= 0) fatal_error("rpn: bad < expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] =  (stack[j][i] < stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }
        else if (strcmp(string,"<=") == 0) {
            if (top <= 0) fatal_error("rpn: bad <= expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    stack[j][i] =  (stack[j][i] <= stack[top][i]);
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }

        else if (strcmp(string,"mask") == 0) {
            if (top <= 0) fatal_error("rpn: bad mask expression","");
            j = top-1;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    if (stack[top][i] == 0.0) stack[j][i] = UNDEFINED;
                }
                else stack[j][i] = UNDEFINED;
            }
            top--;
        }

        // yrev - like in GrADS : N <-> S
        else if (strcmp(string,"yrev") == 0) {
            if (top < 0) fatal_error("rpn: yrev needs field","");
                get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
            if (nx <= 0 || ny <= 0) fatal_error("rpn: yrev only on nx x ny grids","");
            if ((scan >> 4) != 0 && (scan >> 4) != 4) 
                fatal_error("rpn: yrev only appropriate for we:ns and we:sn grids","");
            for (k = 0; k < ny/2; k++) {
                p1 = stack[top] + nx*k;
                p2 = stack[top] + nx*(ny-k-1);
                for (m = 0; m < nx; m++) {
                    tmp = p1[m];
                    p1[m] = p2[m];
                    p2[m] = tmp;
                }
            }
        }

        // smth9 - like in GrADS  smth9g - global field
        else if (strcmp(string,"smth9g") == 0) {
            if (mode == 98) fprintf(stderr," smth9");
            if (top < 0) fatal_error("rpn: smth9 needs field","");

            get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
            if (nx <= 0 || ny <= 0) fatal_error("rpn: yrev only on nx x ny grids","");
            if ((scan >> 4) != 0 && (scan >> 4) != 4) 
                fatal_error("rpn: smth9 only appropriate for we:ns and we:sn grids","");

            top = push(top,ndata,VECTOR,0.0,stack[top],NULL);
            for (m = 0; m < ny; m++) {
                for (i = 0; i < nx; i++) {
                    wt = sum1 = 0.0;
                    gbl_wt(&sum1, &wt, stack[top], i-1, m-1, nx, ny,0.3);
                    gbl_wt(&sum1, &wt, stack[top], i  , m-1, nx, ny,0.5);
                    gbl_wt(&sum1, &wt, stack[top], i+1, m-1, nx, ny,0.3);
                    gbl_wt(&sum1, &wt, stack[top], i-1, m  , nx, ny,0.5);
                    gbl_wt(&sum1, &wt, stack[top], i  , m  , nx, ny,1.0);
                    gbl_wt(&sum1, &wt, stack[top], i+1, m  , nx, ny,0.5);
                    gbl_wt(&sum1, &wt, stack[top], i-1, m+1, nx, ny,0.3);
                    gbl_wt(&sum1, &wt, stack[top], i  , m+1, nx, ny,0.5);
                    gbl_wt(&sum1, &wt, stack[top], i+1, m+1, nx, ny,0.3);
                    stack[top-1][i + m*nx] = wt > 0.0 ? sum1/wt : UNDEFINED;
                }
            }
            top--;
        }

        // smth9r - like in GrADS  smth9g - regional field
        else if (strcmp(string,"smth9r") == 0) {
            if (mode == 98) fprintf(stderr," smth9");
            if (top < 0) fatal_error("rpn: smth9 needs field","");

            get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
            if (nx <= 0 || ny <= 0) fatal_error("rpn: yrev only on nx x ny grids","");
            if ((scan >> 4) != 0 && (scan >> 4) != 4)
                fatal_error("rpn: smth9 only appropriate for we:ns and we:sn grids","");

            top = push(top,ndata,VECTOR,0.0,stack[top],NULL);
            for (m = 0; m < ny; m++) {
                for (i = 0; i < nx; i++) {
                    wt = sum1 = 0.0;
                    reg_wt(&sum1, &wt, stack[top], i-1, m-1, nx, ny,0.3);
                    reg_wt(&sum1, &wt, stack[top], i  , m-1, nx, ny,0.5);
                    reg_wt(&sum1, &wt, stack[top], i+1, m-1, nx, ny,0.3);
                    reg_wt(&sum1, &wt, stack[top], i-1, m  , nx, ny,0.5);
                    reg_wt(&sum1, &wt, stack[top], i  , m  , nx, ny,1.0);
                    reg_wt(&sum1, &wt, stack[top], i+1, m  , nx, ny,0.5);
                    reg_wt(&sum1, &wt, stack[top], i-1, m+1, nx, ny,0.3);
                    reg_wt(&sum1, &wt, stack[top], i  , m+1, nx, ny,0.5);
                    reg_wt(&sum1, &wt, stack[top], i+1, m+1, nx, ny,0.3);
                    stack[top-1][i + m*nx] = wt > 0.0 ? sum1/wt : UNDEFINED;
                }
            }
            top--;
        }

        // raw2 .. convert from scan mode of input file to current mode

        else if (strcmp(string,"raw2") == 0) {
            if (mode == 98) fprintf(stderr," raw2");
            if (top < 0) fatal_error("rpn: raw2 needs field","");
            if (translation != NULL) {
                top = push(top,ndata,VECTOR,0.0,stack[top],NULL);
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                for (i = 0; i < ndata; i++) {
                    stack[top-1][i] =  stack[top][translation[i]];
                }
                top--;
            }
        }

        // 2raw .. convert from current scan mode to input file scan mode

        else if (strcmp(string,"2raw") == 0) {
            if (mode == 98) fprintf(stderr," raw2");
            if (top < 0) fatal_error("rpn: raw2 needs field","");
            if (translation != NULL) {
                top = push(top,ndata,VECTOR,0.0,stack[top],NULL);
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
                for (i = 0; i < ndata; i++) {
                    stack[top-1][translation[i]] = stack[top][i];
                }
                top--;
            }
        }

        else if (strcmp(string,"alt_x_scan") == 0) {
            if (top < 0) fatal_error("rpn: yrev needs field","");
            get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
            if (nx <= 0 || ny <= 0) fatal_error("rpn: alt_x_scan only works on nx x ny grids","");
            for (k = 1; k < ny; k += 2) {
            p1 = stack[top] + nx*k;
            p2 = p1 + nx - 1;
            for (m = 0; m < nx/2; m++) {
                tmp = *p1;
                *p1++ = *p2;
                *p2-- = tmp;
            }
            }
        }

        else if (strcmp(string,"xave") == 0) {			// x average the field
            if (top < 0) fatal_error("rpn: xave needs field","");
            get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
            if (nx <= 0 || ny <= 0) fatal_error("rpn: xave only works on nx x ny grids","");
            for (k = 0; k < ndata; k += nx) {
                sum1 = 0.0;
                i = 0;
                for (m = 0; m < nx; m++) {
                    if (DEFINED_VAL(stack[top][k+m])) {
                        sum1 += stack[top][k+m];
                        i++;
                    }
                }
                tmp = i ? sum1 / (double) i : 0.0;
                for (m = 0; m < nx; m++) {
                    if (DEFINED_VAL(stack[top][k+m])) {
                        stack[top][k+m] = tmp;
                    }
                }
            }
        }

        else if (strcmp(string,"xdev") == 0) {			// deviation from zonal mean
            if (top < 0) fatal_error("rpn: xave needs field","");
            get_nxny(sec, &nx, &ny, &npnts, &res, &scan);
            if (nx <= 0 || ny <= 0) fatal_error("rpn: xave only works on nx x ny grids","");
            for (k = 0; k < ndata; k += nx) {
                sum1 = 0.0;
                i = 0;
                for (m = 0; m < nx; m++) {
                    if (DEFINED_VAL(stack[top][k+m])) {
                        sum1 += stack[top][k+m];
                        i++;
                    }
                }
                tmp = i ? sum1 / (double) i : 0.0;
                for (m = 0; m < nx; m++) {
                    if (DEFINED_VAL(stack[top][k+m])) {
                        stack[top][k+m] -= tmp;
                    }
                }
            }
        }

        // change to rcl-data

        // rcl:  stack(++top) = data
        else if (strcmp(string,"rcl") == 0) {
            top = push(top,ndata,VECTOR,0.0,data,NULL);
        }

        // sto:  data = stack(top)
        else if (strcmp(string,"sto") == 0) {
            if (top < 0) fatal_error("rpn: bad sto","");
            for (i=0; i < ndata; i++) {
                data[i] = stack[top][i];
            }
        }

        // clr: emtpy stack
        else if (strcmp(string,"clr") == 0) {
            top = -1;
        }

        // pi: stack(++top) = pi
        else if (strcmp(string,"pi") == 0) {
            top = push(top,ndata,SCALAR,(float) M_PI,NULL,NULL);
        }

        // rand: stack(++top) = random number from 0..1
        // note: rand() is not thread safe, do not OpenMP
        // srand(seed) could be called first to set up seed
        // since srand is not called, seed is 1

        else if (strcmp(string,"rand") == 0) {
            if (mode == 98) fprintf(stderr," rand");
                top = push(top,ndata,SCALAR,(float) 0.0f,NULL,NULL);
            for (i = 0; i < ndata; i++) {
                stack[top][i] = (double) rand() / (double) RAND_MAX;
            }
        }

        else if (strcmp(string,"days_in_ref_month") == 0) {
            reftime(sec, &year, &month, &day, &hour, &minute, &second);
            i = num_days_in_month(year, month);
            top = push(top,ndata,SCALAR,(float) i,NULL,NULL);
        }
        else if (strcmp(string,"days_in_verf_month") == 0) {
            verftime(sec, &year, &month, &day, &hour, &minute, &second);
            i = num_days_in_month(year, month);
            top = push(top,ndata,SCALAR,(float) i,NULL,NULL);
        }

        // print operations .. doesnt affect the stack

        else if (strcmp(string,"print_max") == 0) {
            if (top < 0) fatal_error("rpn: bad print_max expression","");
            flag = 0;
            tmp = 0.0;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    if (flag) tmp = (tmp < stack[top][i]) ? stack[top][i] : tmp;
                    else {
                        flag = 1;
                        tmp = stack[top][i];
                    }
                }
            }
            sprintf(inv_out,"%srpn_max=%g",item_deliminator,tmp);
            inv_out += strlen(inv_out);
        }
        else if (strcmp(string,"print_min") == 0) {
            if (top < 0) fatal_error("rpn: bad print_min expression","");
            flag = 0;
            tmp = 0.0;
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    if (flag) tmp = (tmp > stack[top][i]) ? stack[top][i] : tmp;
                    else {
                        flag = 1;
                        tmp = stack[top][i];
                    }
                }
            }
            sprintf(inv_out,"%srpn_min=%g",item_deliminator,tmp);
            inv_out += strlen(inv_out);
        }

        // print_diff: prints out cosine weighted difference (push - top)

        else if (strcmp(string,"print_diff") == 0) {
            if (top <= 0) fatal_error("rpn: print_rms needs two fields","");
            j = top - 1;
            last_lat = 0;
            cos_lat = 1.0;
            sum1 = wt = 0.0;
            if (lat != NULL) {
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        if (last_lat != lat[i]) {
                            cos_lat = cos(lat[i]*M_PI/180.0);
                            last_lat = lat[i];
                        }
                        sum1 +=  (stack[j][i] - stack[top][i]) * cos_lat;
                        wt += cos_lat;
                    }
                }
            }
            else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(+:wt,sum1)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        sum1 +=  (stack[j][i] - stack[top][i]);
                        wt += 1.0;
                    }
                }
            }

            if (wt != 0.0)  sprintf(inv_out,"%srpn_diff=%g",item_deliminator,sum1/wt);
            else sprintf(inv_out,"%srpn_diff=undefined",item_deliminator);
            inv_out += strlen(inv_out);
        }


        // print_rms: prints out cosine weighted RMS

        else if (strcmp(string,"print_rms") == 0) {
            if (top <= 0) fatal_error("rpn: print_rms needs two fields","");
            j = top - 1;
            last_lat = 0;
            cos_lat = 1.0;
            sum1 = wt = 0.0;
            if (lat != NULL) {
#ifdef USE_OPENMP
#pragma omp parallel for private(i) firstprivate(cos_lat,last_lat) reduction(+:wt,sum1) schedule(static)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        if (last_lat != lat[i]) {
                            cos_lat = cos(lat[i]*M_PI/180.0);
                            last_lat = lat[i];
                        }
                        sum1 +=  (stack[top][i] - stack[j][i]) * (stack[top][i] - stack[j][i]) * cos_lat;
                        wt += cos_lat;
                    }
                }
            }
            else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(+:wt,sum1) schedule(static)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        sum1 +=  (stack[top][i] - stack[j][i]) * (stack[top][i] - stack[j][i]);
                        wt += 1.0;
                    }
                }
            }
            if (wt != 0.0)  sprintf(inv_out,"%srpn_rms=%g",item_deliminator,sqrt(sum1/wt));
            else sprintf(inv_out,"%srpn_rms=undefined",item_deliminator);
            inv_out += strlen(inv_out);
        }

        // print_ave: prints out cosine weighted ave

        else if (strcmp(string,"print_ave") == 0) {
            if (top < 0) fatal_error("rpn: bad print_ave expression","");
            // if (lat == NULL) fatal_error("rpn: print_ave .. no latitudes defined","");
            last_lat = 0;
            cos_lat = 1.0;
            sum1 = wt = 0.0;
            if (lat != NULL) {
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i])) {
                        if (last_lat != lat[i]) {
                            cos_lat = cos(lat[i]*M_PI/180.0);
                            last_lat = lat[i];
                        }
                        sum1 +=  stack[top][i] * cos_lat;
                        wt += cos_lat;
                    }
                }
            }
            else {
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(+:wt,sum1) schedule(static)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i])) {
                        sum1 +=  stack[top][i];
                        wt += 1.0;
                    }
                }
            }
            if (wt != 0.0)  sprintf(inv_out,"%srpn_ave=%g",item_deliminator,sum1/wt);
            else sprintf(inv_out,"%srpn_ave=undefined",item_deliminator);
            inv_out += strlen(inv_out);
        }

        // print_wt_ave: prints weighted ave, X=data, Y=weights

        else if (strcmp(string,"print_wt_ave") == 0) {
            if (top <= 0) fatal_error("rpn: print_wt_ave needs two fields","");
            j = top - 1;
            sum1 = sum2 = 0.0;

            // find mean values
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                    sum1 += stack[j][i]*stack[top][i];
                    sum2 += stack[top][i];
                }
            }
            if (sum2 != 0.0) sum1 = sum1 / sum2;
            sprintf(inv_out,"%srpn_wt_ave=%g",item_deliminator,sum1);
        }

        // print_corr: prints cosine(lat) weighted spatial correlation

        else if (strcmp(string,"print_corr") == 0) {
            if (top <= 0) fatal_error("rpn: print_corr needs two fields","");
            // if (lat == NULL) fatal_error("rpn: print_corr .. no latitudes defined","");
            j = top - 1;
            sum1 = sum2 = wt = 0.0;
            last_lat = 0;
            cos_lat = 1.0;
            sq1 = sq2 = sq12 = 0.0;
            if (lat != NULL) {
                // find mean values
#ifdef USE_OPENMP
#pragma omp parallel for private(i) firstprivate(last_lat, cos_lat) reduction(+:wt,sum1,sum2) schedule(static)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        if (last_lat != lat[i]) {
                            cos_lat = cos(lat[i]*M_PI/180.0);
                            last_lat = lat[i];
                        }
                        sum1 += stack[top][i] * cos_lat;
                        sum2 += stack[j][i] * cos_lat;
                        wt += cos_lat;
                    }
                }
                sum1 = sum1 / wt;
                sum2 = sum2 / wt;
                last_lat = 0;
                cos_lat = 1.0;
#ifdef USE_OPENMP
#pragma omp parallel for private(i) firstprivate(last_lat, cos_lat) reduction(+:sq1,sq2,sq12) schedule(static)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        if (last_lat != lat[i]) {
                            cos_lat = cos(lat[i]*M_PI/180.0);
                            last_lat = lat[i];
                        }
                        sq1 += (stack[top][i]-sum1)*(stack[top][i]-sum1)*cos_lat;
                        sq2 += (stack[j][i]-sum2)*(stack[j][i]-sum2)*cos_lat;
                        sq12 += (stack[top][i]-sum1)*(stack[j][i]-sum2)*cos_lat;
                    }
                }
            }
            else {
                // find mean values
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(+:wt,sum1,sum2) schedule(static)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        sum1 += stack[top][i];
                        sum2 += stack[j][i];
                        wt += 1.0;
                    }
                }
                sum1 = sum1 / wt;
                sum2 = sum2 / wt;
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(+:sq1,sq2,sq12) schedule(static)
#endif
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
                        sq1 += (stack[top][i]-sum1)*(stack[top][i]-sum1);
                        sq2 += (stack[j][i]-sum2)*(stack[j][i]-sum2);
                        sq12 += (stack[top][i]-sum1)*(stack[j][i]-sum2);
                    }
                }
            }
            sq1 = sq1 / wt;
            sq2 = sq2 / wt;
            sq12 = sq12 / wt;

            if (sq1*sq2 == 0.0) sprintf(inv_out,"%srpn_corr=%g",item_deliminator,1.0);
            else sprintf(inv_out,"%srpn_corr=%g",item_deliminator, sq12/sqrt(sq1*sq2));
            inv_out += strlen(inv_out);
        }

        // number:  stack(++top) = number
        else if (string[0] == '+' || string[0] == '-' || string[0] == '.' || isdigit((unsigned char) string[0])) {
	        f = strtof(string, &endptr);
            if (endptr == NULL || endptr[0] == 0) {		/* no characters after conversion */
                top = push(top,ndata,SCALAR,f,NULL,NULL);
                if (mode == 98) fprintf(stderr," constant=%f", f);
            }
            else {
    //	       fprintf(stderr,">>(%d) (%c)\n" , (int) endptr[0], *endptr);
                fatal_error("rpn: bad number (%s)", string);
            }
        }
        else {
    //	       fprintf(stderr,">>(%d) (%c)\n" , (int) endptr[0], *endptr);
            fatal_error("rpn: unidentified symbol %s", string);
        }
        if (mode == 98) fprintf(stderr," top=%d\n", top);
    }
    if (*p != 0) fatal_error("-rpn didn't find operatore or value before %s",p);
    if (top >= 0) {
        for (i = 0; i < ndata; i++) {
            data[i] = stack[top][i];
        }
    }
    else fatal_error("rpn: stack empty","");

    // free stack
    for (i = 0; i < STACK_SIZE; i++) free(stack[i]);

    return 0;
}

/**
 * Push a value onto the RPN stack.
 * 
 * @param top The current top index of the stack.
 * @param ndata The number of data points.
 * @param type The type of data (SCALAR, VECTOR, DBL_VEC).
 * @param f The scalar value to push.
 * @param ff The vector values to push.
 * @param d The double vector values to push.
 * 
 * @return The new top index of the stack.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int push(int top, unsigned int ndata, int type, float f, float *ff, double *d) {

    unsigned int i;

    if (++top == STACK_SIZE) fatal_error_i("rpn: push: stack overflow %d",top);
    if (stack[top] == NULL) {
        stack[top] = (float *) malloc(sizeof(float) * (size_t) ndata);
        if (stack[top] == NULL) fatal_error("rpn: push: memory allocation","");
    }
    if (type == SCALAR) {
        for (i = 0; i < ndata; i++) stack[top][i] = f;
    }
    else if (type == VECTOR) {
        for (i = 0; i < ndata; i++) stack[top][i] = ff[i];
    }
    else if (type == DBL_VEC) {
        for (i = 0; i < ndata; i++) stack[top][i] = (float) d[i];
    }

    return top;
}

/*
 * HEADER:100:if_reg:If:1:if rpn registers defined, X = A, A:B, A:B:C, etc A = register number
 */

/**
 * Check if RPN registers are defined.
 * 
 * The reverse polish notation calculator (-rpn) allows you to manipulate the grid values and 
 * save the results in registers. The values in the registers are persistent until they are 
 * cleared or the program (wgrib2) ends. The persistent registers allow you to do calculations 
 * that require multiple fields. For example, to calculate the wind speed, you have to save the 
 * zonal wind in a register and the meridional wind in another register. When both registers have 
 * values (U and V), you can then proceed with the wind speed calculation. The -if_reg option 
 * tests to see if specified registers have been set. If the register have been set, then the 
 * options up to and including the next output option are executed like with an -if option. 
 * 
 * ## Usage
 * -if_reg X
 * 
 * X is a list of register names, ex. 1:2 or 2:4:7
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int f_if_reg(ARG1) {
    int i, j, *list;
    const char *p;

    if (mode == -1) {
        // figure out the number of arguments
        i = 1;
        p = arg1;
        while (*p) {
            if (*p++ == ':') i++;
        }
        *local = list = (int *) calloc(i+1, sizeof (int));
        if (list == NULL) fatal_error("if_reg: memory allocation failed","");
        list[0] = i;
        p = arg1;
        for (j = 1; j <= i; j++) {
            list[j] = atoi(p);
            if (list[j] >= N_RPN_REGS || list[j] < 0) fatal_error_i("if_reg: bad register %d", list[j]);
            while (isdigit((unsigned char) *p)) p++;
            if (*p == ':') p++;
        }
    }
    else if (mode == -2) {
        list = (int *) *local;
        free(list);
    }
    else if (mode >= 0) {
        list = (int *) *local;
        i = list[0];
        run_flag = 1;			/* should already be one */
        for (j=1; j <= i; j++) {
            if (rpn_n[list[j]] == 0) run_flag = 0;
        }
    }
    return 0;
}

/*
 * HEADER:100:rpn_rcl:misc:1:data = register X .. same as -rpn rcl_X .. no geolocation calc needed
 */

/**
 * Replaces the memory-copy of the grid values with specified RPN register.
 * 
 * The -rpn_rcl N is option that behaves like -rpn "rcl_N". The only difference is the the latter 
 * will automatically compute the longitudes and latitudes of all the grid points. The -rpn_rcl and 
 * -rpn_sto do not initiate the geolocation calculations which make them ideal for CW2 applications. 
 * 
 * ## Usage
 * -rpn_rcl N
 * 
 * N = 0 to 9
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int f_rpn_rcl(ARG1) {
    int reg;
    unsigned int i;

    if (mode == -1) {
        decode = 1;
    }
    else if (mode >= 0) {
        reg = atoi(arg1);
        if (reg < 0 || reg >= N_RPN_REGS) fatal_error_i("rpn_rcl: bad register %d", reg);
        if (ndata != rpn_n[reg]) fatal_error("rpn_rcl: size mismatch","");
        use_scale = 0;
        if (rpn_data[reg] == NULL) {
            for (i=0; i < ndata; i++) {
                data[i] = 0.0;
            }
        }
        else {
            memcpy(data, rpn_data[reg], ndata * sizeof(float));
        }
    }
    return 0;
}

/*
 * HEADER:100:rpn_sto:misc:1:register X = data.. same as -rpn sto_X .. no geolocation calc needed
 */

/**
 * Saves the memory-copy of the grid values in to specified RPN register.
 * 
 * The -rpn_sto N is option that behaves like -rpn "sto_N". The only difference is the the latter 
 * will automatically compute the longitudes and latitudes of all the grid points. The -rpn_rcl 
 * and -rpn_sto do not initiate the geolocation calculations which make them ideal for CW2 
 * applications. 
 * 
 * ## Usage -rpn_sto N
 * 
 * N = 0 to 9
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
int f_rpn_sto(ARG1) {
    int reg;
    if (mode == -1) {
        decode = 1;
    }
    else if (mode >= 0) {
        reg = atoi(arg1);
        if (reg < 0 || reg >= N_RPN_REGS) fatal_error_i("rpn_sto: bad register %d", reg);
        if (ndata != rpn_n[reg]) {
            if (rpn_n[reg] != 0) free(rpn_data[reg]);
            rpn_data[reg] = (float *) malloc(sizeof(float) * (size_t) ndata);
            if (rpn_data[reg] == NULL) {
                rpn_n[reg] = 0;
                fatal_error("rpn_sto: memory allocation","");
            }
            rpn_n[reg] = ndata;
        }
        memcpy(rpn_data[reg], data, ndata * sizeof(float));
    }
    return 0;
}


/**
 * Accumulate one neighbor’s contribution for global smoothing with cyclic longitude.
 * 
 * @param sum Pointer to the sum of weighted values.
 * @param wt Pointer to the weight of the current grid point.
 * @param data Pointer to the grid data.
 * @param i Column index of the current grid point.
 * @param j Row index of the current grid point.
 * @param nx Number of x coordinates in the grid.
 * @param ny Number of y coordinates in the grid.
 * @param wt0 Weight to apply to the current grid point.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
static void gbl_wt(double *sum, double *wt, float *data, int i, int j, int nx, int ny, double wt0) {
    float t;

    i = (i == -1) ? nx-1 : i;
    i = (i == nx) ? 0 : i;
    if (i < 0 || i >= nx || j < 0 || j >= ny) return;

    t = data[i + j*nx];
    if (UNDEFINED_VAL(t)) return;
    *wt = *wt + wt0;
    *sum = *sum + t*wt0;
    return;
}

/**
 * Accumulate one neighbor’s contribution for regional smoothing without cyclic longitude.
 *
 * @param sum Pointer to the sum of weighted values.
 * @param wt Pointer to the weight of the current grid point.
 * @param data Pointer to the grid data.
 * @param i Column index of the current grid point.
 * @param j Row index of the current grid point.
 * @param nx Number of x coordinates in the grid.
 * @param ny Number of y coordinates in the grid.
 * @param wt0 Weight to apply to the current grid point.
 * 
 * @author Wesley Ebisuzaki @date 4/2009
 */
static void reg_wt(double *sum, double *wt, float *data, int i, int j, int nx, int ny, double wt0) {
    float t;

    if (i < 0 || i >= nx || j < 0 || j >= ny) return;
    t = data[i + j*nx];
    if (UNDEFINED_VAL(t)) return;
    *wt = *wt + wt0;
    *sum = *sum + t*wt0;
    return;
}

/*
 * routines to allow code to get or set various RPN registers
 *
 * for example: read field, save in reg_0
 *              allocate array to match read grid dimensions
 *              copy reg_0 to array
 */

/**
 * Get the size of the specified RPN register.
 *
 * @param reg The register number (0-9).
 * 
 * @return The size of the register in bytes, or 0 if the register is invalid.
 *
 * @author Wesley Ebisuzaki @date 4/2009
 */
size_t wgrib2_get_reg_size(int reg) {
    if (reg < 0 ||reg >= N_RPN_REGS) return 0;
    return rpn_n[reg];
}

/**
 * Get the data from the specified RPN register.
 * 
 * @param data Pointer to the buffer to store the register data.
 * @param size Size of the buffer in bytes.
 * @param reg The register number (0-9).
 * 
 * @return
 * - 0 :: success
 * - 1 :: invalid register
 * - 2 :: size mismatch
 *
 * @author Wesley Ebisuzaki @date 4/2009
 */
int wgrib2_get_reg_data(float *data, size_t size, int reg) {

    if (reg < 0 || reg >= N_RPN_REGS) return 1;
    if (rpn_n[reg] != size) return 2;

    memcpy(data, rpn_data[reg], sizeof(float) * (size_t) size);
    return 0;
}

/**
 * Set the data for the specified RPN register.
 *
 * @param data Pointer to the buffer containing the register data.
 * @param size Size of the buffer in bytes.
 * @param reg The register number (0-9).
 *
 * @return
 * - 0 :: success
 * - 1 :: invalid register
 * - 2 :: size mismatch
 *
 * @author Wesley Ebisuzaki @date 4/2009
 */
int wgrib2_set_reg(float *data, size_t size, int reg) {

    if (reg < 0 || reg >= N_RPN_REGS) return 1;

    if (rpn_n[reg] != size) {
        if (rpn_data[reg] != NULL) free(rpn_data[reg]);
        rpn_n[reg] = 0;
        rpn_data[reg] = (float *) malloc(sizeof(float) * (size_t) size);
        if (rpn_data[reg] == NULL) return 2;
        rpn_n[reg] = size;
    }
    memcpy(rpn_data[reg], data, size * sizeof(float));

    return 0;
}
