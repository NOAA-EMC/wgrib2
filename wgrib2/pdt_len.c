#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* 2/2022 Public Domain Wesley Ebisuzaki
 *
 * replaces: check_pdt_size(sec), prod_def_temp_size(sec), and int smallest_pdt_len(int pdt)
 *
 * old  1) return expected size of pdt == size of pdt
 *              (bug, doesn't include vertical coordinates)
 *      2) return expected size of pdt minus vertical coordinates
 *      3) return minimum size of pdt for generating a new PDT
 *
 * new  1) return expected size of pdt (including vertical coordinates)
 *      2) return minimum size of pdt for generating a new PDT 
 *
 * input: char **sec, if not NULL, return expected size of PDT
 *                    if NULL, return minimum size of PDT
 *
 * output:  0  illegal value
 *         -1  not recognized pdt
 *          N  expected size of PDT
 */

int pdt_len(unsigned char **sec, int pdt) {

    int center;
    int vert_coor = 0, nb = 0, np = 0, i;
    int nc = 0, n=1;	/* n == 0 is illegal, n=1 is minimum value of n */
    int Nr, Fp, Ft;	/* JMA fields */

    if (sec != NULL) {
    	pdt = code_table_4_0(sec);
	if (pdt == -1) return -1;
	vert_coor = 4 * number_of_coordinate_values_after_template(sec);
	nb = number_of_contributing_spectral_bands(sec);
	if (nb == -1) nb = 0;

	i = stat_proc_n_time_ranges_index(sec);
	if (i != -1) {
	    n = sec[4][i];
	    if (n == 0) {
		fprintf(stderr,"pdt_len: bad stat_proc ranges = 0 set to to 1\n");
		n = 1;
	    }
	}
    }

    if (pdt < 32768) {
        switch(pdt) {
        case 0: return 34 + vert_coor;
        case 1: return 37 + vert_coor;
        case 2: return 36 + vert_coor;
	case 3: if (sec) nc = sec[4][57];
	        return 68 + nc + vert_coor;
	case 4: if (sec) nc = sec[4][53];
	        return 64 + nc + vert_coor;
        case 5: return 47 + vert_coor;
        case 6: return 35 + vert_coor;
        case 7: return 34 + vert_coor;
        case 8: return 46 + 12*n + vert_coor;
        case 9: return 59 + 12*n + vert_coor;
        case 10: return 47 + 12*n + vert_coor;
        case 11: return 49 + 12*n + vert_coor;
        case 12: return 48 + 12*n + vert_coor;
        case 13: if (sec) nc = sec[4][57];
                return 80 + 12*n + nc + vert_coor;
	case 14: if (sec) nc = sec[4][53];
                return 76 + 12*n + nc + vert_coor;
        case 15: return 37 + vert_coor;
        case 20: return 43 + vert_coor;
	case 30: return 14 + 10*nb + vert_coor;
	case 31: return 14 + 11*nb + vert_coor;
	case 32: return 23 + 11*nb + vert_coor;
	case 33: return 24 + 11*nb + 2 + vert_coor;
	case 34: return 50 + 11*nb + 12*(n-1) + vert_coor;
	case 35: return 26 + 11*(nb-1) + vert_coor;
	case 40: return 36 + vert_coor;
	case 41: return 39 + vert_coor;
	case 42: return 48 + 12*n + vert_coor;
	case 43: return 51 + 12*n + vert_coor;
	case 44: return 45 + vert_coor;
	case 45: return 50 + vert_coor;
	case 46: return 59 + 12*n + vert_coor;
	case 47: return 62 + 12*n + vert_coor;
	case 48: return 58 + vert_coor;
	case 49: return 61 + vert_coor;
	case 51: if (sec) nc = sec[4][34];
		return 35 + 12*nc + vert_coor;
	case 53: if (sec) np = sec[4][12];
		return 38 + 2*np + vert_coor;
	case 54: if (sec) np = sec[4][12];
		return 41 + 2*np + vert_coor;
	case 55: return 40 + vert_coor;
	case 56: return 42 + vert_coor;
	case 57: if (sec) np = sec[4][19];
		return 43 + 5*np + vert_coor;
	case 58: if (sec) np = sec[4][19];
                return 46 + 5*np + vert_coor;
	case 59: return 43 + vert_coor;
	case 60: return 44 + vert_coor;
	case 61: return 56 + 12*n + vert_coor;
	case 62: return 55 + 12*n + vert_coor;
	case 63: return 55 + 12*n + vert_coor;
	case 67:
	case 68:
		if (sec) np = sec[4][19];
		if (sec) n = sec[4][50+5*np];
                return 55 + 5*np + 12*n + vert_coor;
	case 70: return 39 + vert_coor;
	case 71: return 42 + vert_coor;
	case 72: return 51 + 12*n + vert_coor;
	case 73: return 54 + 12*n + vert_coor;
	case 91: if (sec) nc = sec[4][34];
		return 71 + 12*(n-1) + 12*(nc-1) + vert_coor;
		// 2/2023 if (n == 1) return 71 + 12*(nc-1) + vert_coor;
		// MS:    return 72 + 12*(n-1) + 12*(nc-1) + vert_coor;
	case 254: return 15 + vert_coor;
	case 1000: return 22 + vert_coor;
	case 1001: return 38 + vert_coor;
	case 1002: return 35 + vert_coor;
	case 1100: return 34 + vert_coor;
	case 1101: return 50 + vert_coor;
        }
    }
    center = GB2_Center(sec);
    if (center == JMA1 || center == JMA2) {
        switch(pdt) {
	case 50000:
        	return 42 + vert_coor;
	case 50008:
	case 50010:
        case 50011: return 82 + vert_coor;

	case 50009: n = sec != NULL ? uint2(sec[4]+82) : 0;
		return 85+2*n + vert_coor;

        case 50012: return 66 + vert_coor;
        case 50020: return 36 + vert_coor;
	case 51020:
	case 51021: return 44 + vert_coor;
	case 51022:
	case 51122: Nr = JMA_Nr(sec);
		    if (Nr == -1)  Nr = 1;
		    return 60 + 4*Nr + vert_coor;

	case 51123: Nr = JMA_Nr(sec);
		    if (Nr == -1)  Nr = 1;
		    Fp = sec[4][55];
		    Ft = sec[4][56];
		    return 61 + 2 * Nr * (Fp + Ft) + vert_coor;   /* thanks R. Hanai */

        case 52020: return 45 + vert_coor;
        }
    }
    return -1;
}
