#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * RPN reversed polish notation
 *
 * 4/2009 Public Domain by Wesley Ebisuzaki
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
 *               rcl_lat, rcl_lon
 *               rcl (data)
 *    stack: exc (swap), pop, dup, clr
 *
 *    yrev - swap grids, north <-> south
 *    alt_x_scan - for Glahn packing
 *    xave, xdev
 *
 *    print_(X):  X=max, min, rms, corr, ave, diff
 *
 * at the end of rpn, the top of the stack is saved to data unless clr done first
 */

// #define N_RPN_REGS 10 moved to wgrib2.h
#define STACK_SIZE 10

extern int decode, latlon;
extern double *lat, *lon;
extern int match_flag;
extern const char *item_deliminator;
extern int use_scale;

/* note: rpn_n[N_RPN_REGS], and rpn_data[N_RPN_REG] */
size_t rpn_n[N_RPN_REGS] = { 0 };
float *rpn_data[N_RPN_REGS] = { NULL };
static float *stack[STACK_SIZE];

#define SCALAR 0
#define VECTOR 1
#define DBL_VEC 2

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

int push(int top, unsigned int ndata, int type, float f, float *ff, double *d);
static void gbl_wt(double *val, double *wt, float *data, int i, int j, int nx, int ny, double wt0);
static void reg_wt(double *val, double *wt, float *data, int i, int j, int nx, int ny, double wt0);

/*
 * HEADER:100:rpn:misc:1:reverse polish notation calculator
 */

int f_rpn(ARG1) {
    char string[100];
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
	    for (i = 0; i < ndata; i++) {
		if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) { stack[j][i] = stack[j][i] + stack[top][i];
	        }
		else stack[j][i] = UNDEFINED;
	    }
	    top--;
	}
	else if (strcmp(string,"-") == 0) {
	    if (mode == 98) fprintf(stderr," minus");
	    if (top <= 0) fatal_error("rpn: bad - expression","");
	    j = top-1;
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
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i]) && stack[top][i] > 0.0) {
                    stack[top][i] = logf(stack[top][i]);
                }
                else stack[top][i] = UNDEFINED;
            }
        }

	// exp
	else if (strcmp(string,"exp") == 0) {
	    if (top < 0) fatal_error("rpn: bad exp expression","");
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
	    for (i = 0; i < ndata; i++) {
		if (DEFINED_VAL(stack[top][i])) {
		    stack[top][i] = sinf(stack[top][i]);
		}
	    }
	}
	else if (strcmp(string,"cos") == 0) {
	    if (top < 0) fatal_error("rpn: bad cos","");
	    for (i = 0; i < ndata; i++) {
		if (DEFINED_VAL(stack[top][i])) {
		    stack[top][i] = cosf(stack[top][i]);
		}
	    }
	}
	else if (strcmp(string,"tan") == 0) {
	    if (top < 0) fatal_error("rpn: bad tan","");
	    for (i = 0; i < ndata; i++) {
		if (DEFINED_VAL(stack[top][i])) {
		    stack[top][i] = tanf(stack[top][i]);
		}
	    }
	}
	else if (strcmp(string,"asin") == 0) {
	    if (top < 0) fatal_error("rpn: bad asin","");
	    for (i = 0; i < ndata; i++) {
		if (DEFINED_VAL(stack[top][i])) {
		    if (fabsf(stack[top][i]) > 1.0) stack[top][i] = UNDEFINED;
		    else stack[top][i] = asinf(stack[top][i]);
		}
	    }
	}
        else if (strcmp(string,"acos") == 0) {
            if (top < 0) fatal_error("rpn: bad acos","");
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    if (fabsf(stack[top][i]) > 1.0) stack[top][i] = UNDEFINED;
                    else stack[top][i] = acosf(stack[top][i]);
                }
            }
        }
	else if (strcmp(string,"atan") == 0) {
            if (top < 0) fatal_error("rpn: bad atan","");
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(stack[top][i])) {
                    stack[top][i] = atanf(stack[top][i]);
                }
            }
        }
        else if (strcmp(string,"atan2") == 0) {
            if (top <= 0) fatal_error("rpn: bad atan2 expression","");
            j = top-1;
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

	// change to rcl-data, rcl-lat, rcl-lon

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
#pragma omp parallel for private(i) reduction(+:wt,sum1)
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
#pragma omp parallel for private(i) reduction(+:wt,sum1)
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
#pragma omp parallel for private(i) reduction(+:wt,sum1)
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
#pragma omp parallel for private(i) reduction(+:wt,sum1,sum2)
                for (i = 0; i < ndata; i++) {
                    if (DEFINED_VAL(stack[top][i]) && DEFINED_VAL(stack[j][i])) {
		        sum1 += stack[top][i];
		        sum2 += stack[j][i];
		        wt += 1.0;
                    }
	        }
	        sum1 = sum1 / wt;
	        sum2 = sum2 / wt;
#pragma omp parallel for private(i) reduction(+:sq1,sq2,sq12)
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
	    f = atof(string);
	    top = push(top,ndata,SCALAR,f,NULL,NULL);
	    if (mode == 98) fprintf(stderr," constant=%f", f);
	}
	else fatal_error("rpn: unidentified symbol %s", string);
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
 * HEADER:100:if_reg:misc:1:if rpn registers defined, X = A, A:B, A:B:C, etc A = register number
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
	match_flag = 0;
	for (j=1; j <= i; j++) {
	    if (rpn_n[list[j]] == 0) match_flag = 1;
	}
    }
    return 0;
}

/*
 * HEADER:100:rpn_rcl:misc:1:data = register X .. same as -rpn rcl_X .. no geolocation calc needed
 */
int f_rpn_rcl(ARG1) {
    int reg;

    if (mode == -1) {
	decode = 1;
    }
    else if (mode >= 0) {
	reg = atoi(arg1);
	if (reg < 0 || reg >= N_RPN_REGS) fatal_error_i("rpn_rcl: bad register %d", reg);
	if (ndata != rpn_n[reg]) fatal_error("rpn_rcl: size mismatch","");
        use_scale = 0;
	memcpy(data, rpn_data[reg], ndata * sizeof(float));
    }
    return 0;
}

/*
 * HEADER:100:rpn_sto:misc:1:register X = data.. same as -rpn sto_X .. no geolocation calc needed
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
size_t wgrib2_get_reg_size(int reg) {
    if (reg < 0 ||reg >= N_RPN_REGS) return 0;
    return rpn_n[reg];
}

int wgrib2_get_reg_data(float *data, size_t size, int reg) {

    if (reg < 0 || reg >= N_RPN_REGS) return 1;
    if (rpn_n[reg] != size) return 2;

    memcpy(data, rpn_data[reg], sizeof(float) * (size_t) size);
    return 0;
}

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

