#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * FlagTable.c
 *
 * routines to return the value of the various flags
 *   policy: use this instead of .h files
 *
 * 12/2006: Public Domain Wesley Ebisuzaki
 * 1/2007 cleanup M. Schwarb
 */

extern char *scan_order[];
extern char *stagger_description[];

/*
 * HEADER:-1:flag_table_3.3:inv:0:flag table 3.3, resolution and component flags
 */

int f_flag_table_3_3(ARG0) {
    int res;
    if (mode >= 0) {
        res = flag_table_3_3(sec);
	if (res >= 0) {
            sprintf(inv_out,"flag table 3.3=%d", res);
	    inv_out += strlen(inv_out);
        }
    }
    return 0;
}
int flag_table_3_3(unsigned char **sec) {
    unsigned char *p;
    p = flag_table_3_3_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}
int set_flag_table_3_3(unsigned char **sec, unsigned int flag) {
    unsigned char *p;
    p = flag_table_3_3_location(sec);
    if (p == NULL) return 1;
    *p = flag;
    return 0;
}

unsigned char *flag_table_3_3_location(unsigned char **sec) {
    int grid_template, center;
    unsigned char *gds;

    grid_template = code_table_3_1(sec);
    center = GB2_Center(sec);
    gds = sec[3];
    switch (grid_template) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 40:
        case 41:
        case 42:
        case 43:
	case 140:
        case 204:
              return gds+54; break;
	case 4:
        case 5:
        case 10:
        case 12:
        case 20:
        case 30:
        case 31:
        case 90:
        case 110:
              return gds+46; break;
#ifdef WMO_VALIDATION
	case 60:
              return gds+71; break;
#endif
	case 32768:
		if (center == NCEP) return gds+54;
		return NULL;
	case 32769:
		if (center == NCEP) return gds+54;
		return NULL;
	case 40110:
		if ((center == JMA1) || (center == JMA2)) return gds+46; 
		return NULL;
		break;
        default: break;
    }
    return NULL;
}



/*
 * HEADER:1:vector_dir:inv:0:grid or earth relative winds
 */

int f_vector_dir(ARG0) {
    int res;
    if (mode >= 0) {
        res = flag_table_3_3(sec);
	if (res >= 0) {
	    sprintf(inv_out, res & 8 ? "winds(grid)" : "winds(N/S)");
	    inv_out += strlen(inv_out);
        }
    }
    return 0;
}

/*
 * HEADER:-1:flag_table_3.4:inv:0:flag table 3.4, scanning mode
 */

int f_flag_table_3_4(ARG0) {
    int scan;
    if (mode >= 0) {
        scan = flag_table_3_4(sec);
	if (scan >= 0) {
            sprintf(inv_out,"flag table 3.4=%d %s", scan, scan_order[scan >> 4]);
	}
    }
    return 0;
}
int flag_table_3_4(unsigned char **sec) {
    unsigned char *p;
    p = flag_table_3_4_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

int set_flag_table_3_4(unsigned char **sec, unsigned int flag) {
    unsigned char *p;
    p = flag_table_3_4_location(sec);
    if (p == NULL) return 1;
    *p = flag;
    return 0;
}

unsigned char *flag_table_3_4_location(unsigned char **sec) {
    int grid_template, center;
    unsigned char *gds;

    gds = sec[3];
    grid_template = code_table_3_1(sec);
    center = GB2_Center(sec);

    switch (grid_template) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 40:
        case 41:
        case 42:
        case 43:
                 return gds+71; break;
	case 4:
	case 5:
                 return gds+47; break;
        case 10: 
        case 12: 
                 return gds+59; break;
        case 20: return gds+64; break;
        case 30:
        case 31: return gds+64; break;
        case 50:
        case 51:
        case 52:
        case 53:
                 /* spectral modes don't have scan order */
                 return NULL; break;
        case 90: 
        case 140: 
		  return gds+63; break;
        case 110: return gds+56; break;
        case 190: 
	case 120: return gds+38; break;
	case 204: return gds+71; break;
        case 1000: return gds+50; break;
#ifdef WMO_VALIDATION
	case 60:
              return gds+72; break;
#endif
	case 32768:
		if (center == NCEP) return gds+71;
		return NULL;
		break;
	case 32769:
		if (center == NCEP) return gds+71;
		return NULL;
		break;
	case 40110:
		if ((center == JMA1) || (center == JMA2)) return gds+56; 
		return NULL;
		break;
        default: break;
    }
    return NULL;
}

/*
 * HEADER:-1:flag_table_3.5:inv:0:flag table 3.5 projection center
 */
int f_flag_table_3_5(ARG0) {
    int p;
    if (mode >= 0) {
        p = flag_table_3_5(sec);
	if (p >= 0) {
            sprintf(inv_out,"flag table 3.5=%d", p);
	    inv_out += strlen(inv_out);
        }
    }
    return 0;
}
int flag_table_3_5(unsigned char **sec) {
    unsigned char *p;
    p = flag_table_3_5_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *flag_table_3_5_location(unsigned char **sec) {
    unsigned char *gds;
    int center;

    gds = sec[3];
    center = GB2_Center(sec);

    switch (code_table_3_1(sec)) {
        case 20:
        case 30:
        case 31: return gds+63; break;
        case 110: return gds+55; break;
	case 40110:
	    if ((center == JMA1) || (center == JMA2)) return gds+55;
	    break;
	default: break;
    }
    return NULL;
}

/*
 * HEADER:-1:flag_table_3.9:inv:0:flag table 3.9 numbering order of diamonds seen from corresponding pole
 */

int f_flag_table_3_9(ARG0) {
    int p;
    if (mode >= 0) {
        p = flag_table_3_9(sec);
        if (p >= 0) {
            sprintf(inv_out,"flag table 3.9=%d", p);
            inv_out += strlen(inv_out);
        }
    }
    return 0;
}
int flag_table_3_9(unsigned char **sec) {

    unsigned char *gds;

    gds = sec[3];
    switch (code_table_3_1(sec)) {
        case 100: return gds[32];
    }
    return -1;
}
/*
 * HEADER:-1:flag_table_3.10:inv:0:flag table 3.10 scanning mode for one diamond
 */
int f_flag_table_3_10(ARG0) {
    int p;
    if (mode >= 0) {
        p = flag_table_3_10(sec);
        if (p >= 0) {
            sprintf(inv_out,"flag table 3.10=%d", p);
            inv_out += strlen(inv_out);
        }
    }
    return 0;
}
int flag_table_3_10(unsigned char **sec) {
    unsigned char *gds;
    gds = sec[3];
    switch (code_table_3_1(sec)) {
        case 100: return gds[33];
    }
    return -1;
}

