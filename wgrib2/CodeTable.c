#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * CodeTable.c
 *
 * returns values of various code tables
 *  use this routine instead of .h files
 *
 * 12/2006 Public Domain Wesley Ebisuzaki
 * 1/2007 M. Schwarb
 * 1/2008 S. Varlamov fixed to code_table_5.5
 * 5/2013 W. Ebisuzaki fixed code table 0.0 in response to error report M. Foster
 * 7/2013 W. Ebisuzaki added more *_location() functions, needed by set_pdt()
 * 5/2013 G. Schnee add case for DRT 5.42
 */

/*
 * HEADER:-1:code_table_0.0:inv:0:code table 0.0 discipline
 */

int f_code_table_0_0(ARG0) {
    int val;
    const char *string;

    if (mode >= 0) {
        val = code_table_0_0(sec);
        string = NULL;
        switch(val) {
#include "CodeTable_0.0.dat"
        }
        if (string) sprintf(inv_out,"code table 0.0=%d %s", val, string);
        else sprintf(inv_out,"code table 0.0=%d", val);
    }
    return 0;
}
int code_table_0_0(unsigned char **sec) {
    return  (int) sec[0][6];
}

/*
 * HEADER:-1:code_table_1.0:inv:0:code table 1.0 master table version
 */

int f_code_table_1_0(ARG0) {

    if (mode >= 0) {
        sprintf(inv_out,"code table 1.0=%d", code_table_1_0(sec));
    }
    return 0;
}
int code_table_1_0(unsigned char **sec) {
    return  (int) sec[1][9];
}

/*
 * HEADER:-1:code_table_1.1:inv:0:code table 1.1 local table version
 */

int f_code_table_1_1(ARG0) {

    if (mode >= 0) {
        sprintf(inv_out,"code table 1.1=%d", code_table_1_1(sec));
    }
    return 0;
}
int code_table_1_1(unsigned char **sec) {
    unsigned char *p;
    p = code_table_1_1_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_1_1_location(unsigned char **sec) {
   return sec[1] + 10;
}


/*
 * HEADER:-1:code_table_1.2:inv:0:code table 1.2 significance of reference time
 */

int f_code_table_1_2(ARG0) {
    const char *string;
    int val;
    if (mode >= 0) {
        val = code_table_1_2(sec);
        string = NULL;
        switch(val) {
#include "CodeTable_1.2.dat"
        }
	if (string == NULL) sprintf(inv_out,"code table 1.2=%d", val);
        else sprintf(inv_out,"code table 1.2=%d %s", val, string);
    }
    return 0;
}

int code_table_1_2(unsigned char **sec) {
    unsigned char *p;
    p = code_table_1_2_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_1_2_location(unsigned char **sec) {
   return sec[1] + 11;
}

/*
 * HEADER:-1:code_table_1.3:inv:0:code table 1.3 production status of processed data
 */

int f_code_table_1_3(ARG0) {
    const char *string;
    int val;

    if (mode >= 0) {
        val = code_table_1_3(sec);
        string = NULL;
	switch(val) {
#include "CodeTable_1.3.dat"
        }
        if (string == NULL) sprintf(inv_out,"code table 1.3=%d", val);
        else sprintf(inv_out,"code table 1.3=%d %s", val, string);
    }
    return 0;
}
int code_table_1_3(unsigned char **sec) {
    unsigned char *p;
    p = code_table_1_3_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}
unsigned char *code_table_1_3_location(unsigned char **sec) {
   return sec[1] + 19;
}


/*
 * HEADER:-1:code_table_1.4:inv:0:code table 1.4 type of processed data
 */

int f_code_table_1_4(ARG0) {
    const char *string;
    int val;
    if (mode >= 0) {
        val = code_table_1_4(sec);
	string = NULL;
	switch(val) {
#include "CodeTable_1.4.dat"
	}
	if (string == NULL) sprintf(inv_out,"code table 1.4=%d", val);
	else sprintf(inv_out,"code table 1.4=%d %s", val, string);
    }
    return 0;
}
int code_table_1_4(unsigned char **sec) {
    unsigned char *p;
    p = code_table_1_4_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}
unsigned char *code_table_1_4_location(unsigned char **sec) {
   return sec[1] + 20;
}


/*
 * HEADER:-1:code_table_1.5:inv:0:Identification template number
 */

int f_code_table_1_5(ARG0) {
    int val;
    if (mode >= 0) {
        val = code_table_1_5(sec);
        if (val >= 0) sprintf(inv_out,"code table 1.5=%d", val);
    }
    return 0;
}

int code_table_1_5(unsigned char **sec) {
    unsigned char *p;
    p = code_table_1_5_location(sec);
    if (p == NULL) return -1;
    return (int) uint2(p);
}

unsigned char *code_table_1_5_location(unsigned char **sec) {
    return  (GB2_Sec1_size(sec) >= 23) ?  sec[1] + 21 : NULL;
}

/*
 * HEADER:-1:code_table_1.6:inv:0:calendar
 */

int f_code_table_1_6(ARG0) {
    int val;
    if (mode >= 0) {
        val = code_table_1_6(sec);
        if (val >= 0) sprintf(inv_out,"code table 1.6=%d", val);
    }
    return 0;
}

int code_table_1_6(unsigned char **sec) {
    unsigned char *p;
    p = code_table_1_6_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}
unsigned char *code_table_1_6_location(unsigned char **sec) {
    return  (GB2_Sec1_size(sec) >= 24 && sec[1][21] == 0 && sec[1][22] == 0) 
	?  sec[1] + 23 : NULL;
}





/*
 * HEADER:-1:code_table_3.0:inv:0:code table 3.0 Source of grid definition
 */

int f_code_table_3_0(ARG0) {

    if (mode >= 0) {
        sprintf(inv_out,"code table 3.0=%d", code_table_3_0(sec));
    }
    return 0;
}
int code_table_3_0(unsigned char **sec) {
    return  (int) sec[3][5];
}

/*
 * HEADER:-1:code_table_3.1:inv:0:code table 3.1 Grid definition template number
 */

int f_code_table_3_1(ARG0) {
    const char *string;
    int val;
    if (mode >= 0) {
        val = code_table_3_1(sec);
	string = NULL;
	switch(val) {
#include "CodeTable_3.1.dat"
	}
	if (string == NULL) sprintf(inv_out,"code table 3.1=%d", val);
	else sprintf(inv_out,"code table 3.1=%d %s", val, string);
    }
    return 0;
}
int code_table_3_1(unsigned char **sec) {
    return  (int) uint2(sec[3]+12);
}

/*
 * HEADER:-1:code_table_3.2:inv:0:code table 3.2 Size (radius) and Shape of Earth
 */

int f_code_table_3_2(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
	val = code_table_3_2(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_3.2.dat"
	    }
	    if (string == NULL) sprintf(inv_out,"code table 3.2=%d", val);
	    else sprintf(inv_out,"code table 3.2=%d %s", val, string);
        }
	if (mode > 0) {
	    inv_out += strlen(inv_out);
	    sprintf(inv_out,", ave radius of earth=%lf km",radius_earth(sec)/1000.0);
	}
    }
    return 0;
}

int code_table_3_2(unsigned char **sec) {
    unsigned char *p;
    p = code_table_3_2_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_3_2_location(unsigned char **sec) {
    int grid_def, center;
    grid_def = code_table_3_1(sec);

    if (grid_def < 50) return sec[3]+14;

    switch (grid_def) {
        case 90:
        case 101:
        case 110:
        case 130:
        case 140:
        case 204:
        case 1000:
        case 1100:
#ifdef WMO_VALIDATION
        case 60:
#endif
            return sec[3]+14; break;
        default: break;
    }

    center = GB2_Center(sec);
    if (center == NCEP) {
        if (grid_def == 32768 || (grid_def == 32769)) return sec[3]+14;
    }
    if ((center == JMA1) || (center == JMA2)) {
        if (grid_def == 40110) return sec[3]+14;
    }
    return  NULL;
}


/*
 * HEADER:-1:code_table_3.6:inv:0:code table 3.6 Spectral data representation type
 */

int f_code_table_3_6(ARG0) {
    int val;
    if (mode >= 0) {
	val = code_table_3_6(sec);
	if (val >= 0) sprintf(inv_out,"code table 3.6=%d", val);
    }
    return 0;
}
int code_table_3_6(unsigned char **sec) {
    int grid_def;
    grid_def = code_table_3_1(sec);
    if (grid_def >= 50 && grid_def <= 53)  return GDS_Harmonic_code_3_6(sec[3]);
    return -1;
}

/*
 * HEADER:-1:code_table_3.7:inv:0:code table 3.7 Spectral data representation mode
 */

int f_code_table_3_7(ARG0) {
    int val;
    if (mode >= 0) {
        val = code_table_3_7(sec);
        if (val >= 0) sprintf(inv_out,"code table 3.7=%d", val);
    }
    return 0;
}
int code_table_3_7(unsigned char **sec) {
    int grid_def;
    grid_def = code_table_3_1(sec);
    if (grid_def >= 50 && grid_def <= 53)  return GDS_Harmonic_code_3_7(sec[3]);
    return -1;
}

/*
 * HEADER:-1:code_table_3.8:inv:0:code table 3.8 Grid point position
 */

int f_code_table_3_8(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_3_8(sec);
	if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_3.8.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 3.8=%d", val);
            else sprintf(inv_out,"code table 3.8=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_3_8(unsigned char **sec) {
    int grid_def;
    grid_def = code_table_3_1(sec);
    if (grid_def == 100)  return sec[3][31];
    return -1;
}

/*
 * HEADER:-1:code_table_3.11:inv:0:code table 3.11 regional/global thinned/reduced grid

 */
int f_code_table_3_11(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        if ((val = code_table_3_11(sec)) >= 0) {
            string = NULL;
            switch(val) {
		case 0: string = "not used";
			break;
		case 1: string = "thinned global grid, nx specified";
			break;
		case 2: string = "thinned regional grid, nx specified";
			break;
		case 3: string = "latitudes of rows are specified";
			break;
            }
            if (string == NULL) sprintf(inv_out,"code table 3.11=%d", val);
            else sprintf(inv_out,"code table 3.11=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_3_11(unsigned char **sec) {
    return sec[3][11];
}

/*
 * HEADER:-1:code_table_3.15:inv:0:code table 3.15 Physical meaning of vertical coordinate
 */
int f_code_table_3_15(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        if ((val = code_table_3_15(sec)) >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_3.15.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 3.15=%d", val);
            else sprintf(inv_out,"code table 3.15=%d %s", val, string);
        }
    }
    return 0;
}
int code_table_3_15(unsigned char **sec) {
    int grid_def;
    grid_def = code_table_3_1(sec);
    if (grid_def == 1000)  return sec[3][62];
    if (grid_def == 1200)  return sec[3][38];
    return -1;
}

/*
 * HEADER:-1:code_table_3.20:inv:0:code table 3.20 Type of Horizontal line
 */
int f_code_table_3_20(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_3_20(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_3.20.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 3.20=%d", val);
            else sprintf(inv_out,"code table 3.20=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_3_20(unsigned char **sec) {
    unsigned char *p;
    p = code_table_3_20_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_3_20_location(unsigned char **sec) {
    int grid_def;
    grid_def = code_table_3_1(sec);
    if (grid_def == 1000) return sec[3]+59;
    if (grid_def == 1100) return sec[3]+59;
    return NULL;
}

/*
 * HEADER:-1:code_table_3.21:inv:0:code table 3.21 Vertical Dimension coordinate values defn
 */
int f_code_table_3_21(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_3_21(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_3.21.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 3.21=%d", val);
            else sprintf(inv_out,"code table 3.21=%d %s", val, string);
        }
    }
    return 0;
}
int code_table_3_21(unsigned char **sec) {
    int grid_def;
    grid_def = code_table_3_1(sec);
    if (grid_def == 1000)  return sec[3][63];
    if (grid_def == 1200)  return sec[3][39];
    return -1;
}

/*
 * HEADER:-1:code_table_4.0:inv:0:code table 4.0 Product Definition Template Number
 */

int f_code_table_4_0(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_0(sec);
        string = NULL;
        switch(val) {
#include "CodeTable_4.0.dat"
        }
        if (string == NULL) sprintf(inv_out,"code table 4.0=%d", val);
        else sprintf(inv_out,"code table 4.0=%d %s", val, string);
    }
    return 0;
}
int code_table_4_0(unsigned char **sec) {
    return  GB2_ProdDefTemplateNo(sec);
}

/*
 * HEADER:-1:code_table_4.1:inv:0:code table 4.1
 */

int f_code_table_4_1(ARG0) {
    int p;
    if (mode >= 0) {
	p = code_table_4_1(sec);
	if (p >= 0) sprintf(inv_out,"code table 4.1=%d", p);
    }
    return 0;
}
int code_table_4_1(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_1_location(sec);
    return (p != NULL) ? (int) *p : -1;
}

unsigned char *code_table_4_1_location(unsigned char **sec) {
    int p, center;

    p = GB2_ProdDefTemplateNo(sec);
    if (p <= 15 || p == 20 || (p >= 30 && p <= 35) || (p >= 40 && p <= 49) || p == 51 || 
	   (p >= 53 && p <= 61) || p == 67 || p == 68 || (p >= 70 && p <= 74) || p == 91 || 
           p == 254 || (p >= 1000 && p <= 1002) || p == 1100 || p == 1101) return sec[4]+ 9;
    center = GB2_Center(sec);
    if ((center == JMA1) || (center == JMA2)) {
        if (p == 50000 || p == 50008 || p == 50009 || p == 50010 || p == 50011 || p == 50020 
	    || p == 51020 || p == 51021 || p == 51022 || p == 52020)
		return sec[4] + 9;
    }
    return NULL;
}

/*
 * HEADER:-1:code_table_4.2:inv:0:code table 4.2
 */

int f_code_table_4_2(ARG0) {
    int p;
    if (mode >= 0) {
	p = code_table_4_2(sec);
	if (p >= 0) sprintf(inv_out,"code table 4.2=%d", p);
    }
    return 0;
}
int code_table_4_2(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_2_location(sec);
    return (p != NULL) ? (int) *p : -1;
}

unsigned char *code_table_4_2_location(unsigned char **sec) {
    int p, center;

    p = GB2_ProdDefTemplateNo(sec);
    if (p <= 15 || p == 20 || (p >= 30 && p <= 35) || (p >= 40 && p <= 49) || p == 51 || 
	   (p >= 53 && p <= 61) || p == 67 || p == 68 || (p >= 70 && p <= 74) || p == 91 || 
           p == 254 || (p >= 1000 && p <= 1002) || p == 1100 || p == 1101) return sec[4] + 10;

    center = GB2_Center(sec);
    if (p == 50008 && ((center == JMA1) || (center == JMA2))) return sec[4] + 10;
    if ((center == JMA1) || (center == JMA2)) {
        if (p == 50000 || p == 50008 || p == 50009 || p == 50010 || p == 50011 || p == 50020 
	    || p == 51020 || p == 51021 || p == 51022 || p == 52020)
		return sec[4] + 10;
    }

    return NULL;
}


/*
 * HEADER:-1:code_table_4.3:inv:0:code table 4.3 Type of Generating Process
 */
int f_code_table_4_3(ARG0) {
    int val, center;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_3(sec);
	center = GB2_Center(sec);
        string = NULL;
        switch(val) {
#include "CodeTable_4.3.dat"
        }
        if (string == NULL) sprintf(inv_out,"code table 4.3=%d", val);
        else sprintf(inv_out,"code table 4.3=%d %s", val, string);
    }
    return 0;
}

int code_table_4_3(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_3_location(sec);
    return (p != NULL) ? (int) *p : -1;
}

unsigned char *code_table_4_3_location(unsigned char **sec) {
    int pdt, center, n;

    pdt =  GB2_ProdDefTemplateNo(sec);
    center = GB2_Center(sec);
    switch(pdt) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 20:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 47:
    case 60:
    case 61:
    case 1000:
    case 1001:
    case 1002:
    case 1100:
    case 1101:
        return sec[4]+11; break;
    case 40:
    case 41:
    case 42:
    case 43:
        return sec[4]+13; break;
    case 44:
    case 45:
    case 46:
        return sec[4]+24; break;
    case 48:
        return sec[4]+35; break;
    case 52:
        return sec[4]+13; break;
    case 57:
	n = number_of_mode(sec);
	if (n <= 0 || n == 65535) fatal_error_i("PDT 4.57 bad number of mode %d", n);
	return sec[4]+20+5*n; break;
    case 70:
    case 71:
    case 72:
    case 73:
	return sec[4]+16; break;
    case 50000:
    case 50008:
    case 50009:
    case 50010:
    case 50011:
    case 50020:
    case 51020:
    case 51021:
    case 51022:
    case 52020:
	if ((center == JMA1) || (center == JMA2)) return sec[4]+11;
	return NULL;
	break;
    default:
        return NULL;
	break;
    }
    return NULL;
}

/*
 * HEADER:-1:code_table_4.4:inv:0:code table 4.4  (first)
 */
int f_code_table_4_4(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
	val = code_table_4_4(sec);
        string = NULL;
        switch(val) {
#include "CodeTable_4.4.dat"
        }
        if (string == NULL) sprintf(inv_out,"code table 4.4=%d", val);
        else sprintf(inv_out,"code table 4.4=%d (%s)", val, string);
    }
    return 0;
}
int code_table_4_4(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_4_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}
unsigned char *code_table_4_4_location(unsigned char **sec) {
    int pdt, center, n;
    pdt = GB2_ProdDefTemplateNo(sec);
    center = GB2_Center(sec);

    switch(pdt) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 32:
    case 60:
    case 61:
    case 51:
    case 91:
    case 1000:
    case 1001:
    case 1002:
	return sec[4] + 17;
	break;
    case 40:
    case 41:
    case 42:
    case 43:
	return sec[4] + 19;
	break;
    case 44:
    case 45:
    case 46:
    case 47:
	return sec[4] + 30;
	break;
    case 48:
	return sec[4] + 41;
	break;
    case 52:
	return sec[4] + 20;
	break;
    case 57:
        n = number_of_mode(sec);
        if (n <= 0 || n == 65535) fatal_error_i("PDT 4.57 bad number of mode %d", n);
        return sec[4]+26+5*n; break;
    case 70:
    case 71:
    case 72:
    case 73:
	return sec[4]+22; break;
    case 50008:
	if (center == JMA1 || center == JMA2) return sec[4]+17;
	return NULL;
	break;
    case 50009:
	if (center == JMA1 || center == JMA2) return sec[4]+17;
	return NULL;
	break;
    case 50011:
	if (center == JMA1 || center == JMA2) return sec[4]+17;
	return NULL;
	break;
   }
   return NULL;
}

/*
 * code table 4.4 can be
 *   1. defined
 *   2. to be defined
 *   3. not used (example radar or satellite images)
 *
 *   returns 1 if code table 4.4 is not used
 */ 

int code_table_4_4_not_used(unsigned char **sec) {
    int pdt;
    pdt = GB2_ProdDefTemplateNo(sec);
    if (pdt == 20 || pdt == 30 || pdt == 31) return 1;
    return 0;
}

/*
 * HEADER:-1:code_table_4.5a:inv:0:code table 4.5 (1st value)
 */
int f_code_table_4_5a(ARG0) {
    int p;
    if (mode >= 0) {
	p = code_table_4_5a(sec);
	if (p >= 0) sprintf(inv_out,"code table 4.5a=%d", p);
    }
    return 0;
}
int code_table_4_5a(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_5a_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}
unsigned char *code_table_4_5a_location(unsigned char **sec) {
    int pdt, center, n;
    pdt = GB2_ProdDefTemplateNo(sec);
    center = GB2_Center(sec);

    if (center == JMA1 || center == JMA2) {
        switch (pdt) {
    	case 50008: return sec[4] + 22; break;
    	case 50009: return sec[4] + 22; break;
    	case 51020: 
    	case 51021: 
    	case 51022: 
    	case 51122: 
    	case 52020: 
		return NULL; break;
	case 50010:
	case 50011:
         	return sec[4]+22; break;
        default: break;
	}
    }

    switch (pdt) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 51:
    case 60:
    case 61:
    case 91:
    case 1100:
    case 1101:
         return sec[4]+22; break;
    case 40:
    case 41:
    case 42:
    case 43:
        return sec[4]+24; break;
    case 44:
        return sec[4]+33; break;
    case 45:
    case 46:
    case 47:
        return sec[4]+35; break;
    case 48:
        return sec[4]+46; break;
    case 52: // validation
        return sec[4]+25; break;
    case 57:
        n = number_of_mode(sec);
        if (n <= 0 || n == 65535) fatal_error_i("PDT 4.57 bad number of mode %d", n);
        return sec[4]+31+5*n; break;
    case 70:
    case 71:
    case 72:
    case 73:
	return sec[4] + 27; break;
    case 20:
    case 30:
    case 31:
    case 32:
    case 1000:
    case 1001:
    case 1002:
    case 254:
        return NULL; break;
    default:
        fprintf(stderr,"code_table_4.5a: product definition template #%d not supported\n", pdt);
        return NULL;
        break;
    }
    return NULL;
}

/*
 * HEADER:-1:code_table_4.5b:inv:0:code table 4.5 (2nd value)
 */
int f_code_table_4_5b(ARG0) {
    int p;
    if (mode >= 0) {
	p = code_table_4_5b(sec);
	if (p >= 0) sprintf(inv_out,"code table 4.5b=%d", p);
    }
    return 0;
}
int code_table_4_5b(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_5b_location(sec);
    if (p == NULL) return -1;
    return *p;
}
unsigned char *code_table_4_5b_location(unsigned char **sec) {
    int pdt, center, n;
    pdt = GB2_ProdDefTemplateNo(sec);
    center = GB2_Center(sec);

    if (center == JMA1 || center == JMA2) {
        switch (pdt) {
        case 50008: return sec[4] + 28; break;
        case 50009: return sec[4] + 28; break;
    	case 51020: 
    	case 51021: 
    	case 51022: 
    	case 51122: 
		return NULL; break;
	case 50010:
	case 50011:
         	return sec[4]+28; break;
        default: break;
        }
    }

    switch (pdt) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 51:
    case 60:
    case 61:
    case 91:
    case 1100:
    case 1101:
        return sec[4]+28; break;
    case 40:
    case 41:
    case 42:
    case 43:
        return sec[4]+30; break;
    case 44:
        return sec[4]+39; break;
    case 45:
    case 46:
    case 47:
        return sec[4]+41; break;
    case 48:
        return sec[4]+52; break;
    case 52: 
        return NULL; break;
    case 57:
        n = number_of_mode(sec);
        if (n <= 0 || n == 65535) fatal_error_i("PDT 4.57 bad number of mode %d", n);
        return sec[4]+37+5*n; break;
    case 70:
    case 71:
    case 72:
    case 73:
	return sec[4]+33; break;
    case 20:
    case 30:
    case 31:
    case 32:
    case 1000:
    case 1001:
    case 1002:
    case 254:
        return NULL; break;
    default:
        fprintf(stderr,"code_table_4.5b: product definition template #%d not supported\n", pdt);
        return NULL;
        break;
    }
    return NULL;
}

/*
 * HEADER:-1:code_table_4.6:inv:0:code table 4.6 ensemble type
 */
int f_code_table_4_6(ARG0) {
    int p;
    if (mode >= 0) {
	p = code_table_4_6(sec);
	if (p >= 0) sprintf(inv_out,"code table 4.6=%d", p);
    }
    return 0;
}
int code_table_4_6(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_6_location(sec);
    if (p == NULL) return -1;
    return *p;
}
unsigned char *code_table_4_6_location(unsigned char **sec) {
    int p, nb;
    p = GB2_ProdDefTemplateNo(sec);
    if (p == 1 || p == 11|| p == 60 || p == 61) return  sec[4]+34;
    // bad: if (p >= 40 && p <= 43) return sec[4]+36;
    if (p == 41 || p == 43) return sec[4]+36;
    if (p == 33 || p == 34) {		/* synthetic sat data */
	nb = sec[4][22];
	return sec[4]+23+11*nb;
    }
    if (p == 71 || p == 73) return sec[4]+39;
    return NULL;
}


/*
 * HEADER:-1:code_table_4.7:inv:0:code table 4.7 derived forecast
 */
int f_code_table_4_7(ARG0) {
    int val, center;
    const char *string;
    if (mode >= 0) {
	val = code_table_4_7(sec);
	center = GB2_Center(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.7.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.7=%d", val);
            else sprintf(inv_out,"code table 4.7=%d %s", val, string);
        }
    }
    return 0;
}
int code_table_4_7(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_7_location(sec);
    if (p)  return (int) *p;
    return -1;
}

unsigned char *code_table_4_7_location(unsigned char **sec) {
    int pdt;
    pdt = GB2_ProdDefTemplateNo(sec);
    switch (pdt) {
	case 2:
	case 3:
	case 4:
	case 12:
		return sec[4]+34; break;
    }
    return NULL;
}

/*
 * HEADER:-1:code_table_4.8:inv:0:code table 4.7 derived forecast
 */
int f_code_table_4_8(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_8(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.8.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.8=%d", val);
            else sprintf(inv_out,"code table 4.8=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_4_8(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_8_location(sec);
    if (p)  return (int) *p;
    return -1;
}
unsigned char *code_table_4_8_location(unsigned char **sec) {
    int pdt;
    pdt = GB2_ProdDefTemplateNo(sec);
    switch (pdt) {
	case 3:
	case 13:
		return sec[4]+34; break;
    }
    return NULL;
}


/*
 * HEADER:-1:code_table_4.9:inv:0:code table 4.9 Probability Type
 */
int f_code_table_4_9(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_9(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.9.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.9=%d", val);
            else sprintf(inv_out,"code table 4.9=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_4_9(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_9_location(sec);
    if (p == NULL) return -1;
    return *p;
}

unsigned char *code_table_4_9_location(unsigned char **sec) {
    int val;
    val = GB2_ProdDefTemplateNo(sec);
    switch(val) {
    case 5:
    case 9:
	return sec[4]+36;
	break;
    }
    return NULL;
}


/*
 * HEADER:-1:code_table_4.10:inv:0:code table 4.10 statistical processing .. first occurence
 */
int f_code_table_4_10(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
	val = code_table_4_10(sec);
	if (val >= 0) {
	    string = NULL;
	    switch(val) {
#include "CodeTable_4.10.dat"
	    }
	    if (string == NULL) sprintf(inv_out,"code table 4.10=%d", val);
	    else sprintf(inv_out,"code table 4.10=%d %s", val, string);
	}
    }
    return 0;
}

int code_table_4_10(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_10_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_4_10_location(unsigned char **sec) {
    int pdt, i, center;

    pdt = GB2_ProdDefTemplateNo(sec);
    if (pdt == -1) return NULL;

    switch(pdt) {
        case 8: i = 46; break;
        case 9: i = 59; break;
        case 10: i = 47; break;
        case 11: i = 49; break;
        case 12: i = 48; break;
        case 13: i = 80; break;
        case 14: i = 76; break;
        case 15: i = 34; break;
        case 42: i = 48; break;
        case 43: i = 51; break;
        case 1001: i = 26; break;
        case 1002: i = 24; break;
        case 1101: i = 38; break;
	case 50011:
    		center = GB2_Center(sec);
		if (center != JMA1 && center != JMA2) return NULL;
		i = 46; break;
        default: return NULL; break;
    }
    return sec[4] + i;
}



/*
 * HEADER:-1:code_table_4.11:inv:0:code table 4.11 (first) type of time intervals
 */
int f_code_table_4_11(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_11(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.11.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.11=%d", val);
            else sprintf(inv_out,"code table 4.11=%d %s", val, string);
        }
    }
    return 0;
}

/*
 * HEADER:-1:code_table_4.11s:inv:0:code table 4.11 (all) type of time intervals
 */

int f_code_table_4_11s(ARG0) {
    int n, i, val;
    unsigned char *p;
    if (mode < 0) return 0;
    n = (int) sec[4][stat_proc_n_time_ranges_index(sec)];
    if (n < 1) return 0;
    p = code_table_4_11_location(sec);
    if (p == NULL) return 0;
    for (i = 0; i < n; i++) {
	val = (int) p[i*12];
	if (i == 0) sprintf(inv_out,"code table 4.11=%d",val);
	else sprintf(inv_out,",%d", val);
	inv_out += strlen(inv_out);
    }
    return 0;
}


int code_table_4_11(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_11_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_4_11_location(unsigned char **sec) {
    int val, center;
    unsigned char *p;
    val = GB2_ProdDefTemplateNo(sec);
    switch (val) {
        case 8: p = sec[4] + 47; break;
        case 9: p = sec[4] + 60; break;
        case 10: p = sec[4] + 48; break;
        case 11: p = sec[4] + 50; break;
        case 12: p = sec[4] + 49; break;
        case 13: p = sec[4] + 81; break;
        case 14: p = sec[4] + 77; break;
        case 42: p = sec[4] + 49; break;
        case 43: p = sec[4] + 52; break;
        case 1001: p = sec[4] + 27; break;
        case 1101: p = sec[4] + 39; break;
	case 50011:
    		center = GB2_Center(sec);
		if (center == JMA1 || center == JMA2) p = sec[4] + 47;
		else p = NULL;
		break;
        default: p = NULL; break;
    }
    return p;
}



/*
 * HEADER:-1:code_table_4.15:inv:0:code table 4.15 type of areal statistical processing
 */
int f_code_table_4_15(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_15(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.15.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.15=%d", val);
            else sprintf(inv_out,"code table 4.15=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_4_15(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_15_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_4_15_location(unsigned char **sec) {
    int val;
    unsigned char *p;
    val = GB2_ProdDefTemplateNo(sec);
    switch (val) {
        case 15: p = sec[4]+35; break;
        default: p = NULL; break;
    }
    return p;
}


/*
 * HEADER:-1:code_table_4.91:inv:0:code table 4.91 type of interval
 */
int f_code_table_4_91(ARG0) {
    int val;
    const char *string;
    
    if (mode >= 0) {
        val = code_table_4_91(sec);
	if (val >= 0) {
	    string = NULL;
	    switch(val) {
#include "CodeTable_4.91.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.91=%d", val);
            else sprintf(inv_out,"code table 4.91=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_4_91(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_91_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_4_91_location(unsigned char **sec) {
    int val;
    unsigned char *p;
    val = GB2_ProdDefTemplateNo(sec);
    switch (val) {
        case 44:
        case 45:
        case 46:
        case 48: p = sec[4] + 13; break;
        case 47: p = sec[4] + 14; break;
        default: p = NULL; break;
    }
    return p;
}

/*
 * HEADER:-1:code_table_4.91b:inv:0:code table 4.91 type of interval (2nd copy)
 */
int f_code_table_4_91b(ARG0) {
    int val;
    const char *string;
   
    if (mode >= 0) {
        val = code_table_4_91b(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.91.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.91=%d", val);
            else sprintf(inv_out,"code table 4.91=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_4_91b(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_91b_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

unsigned char *code_table_4_91b_location(unsigned char **sec) {
    int val;
    unsigned char *p;
    val = GB2_ProdDefTemplateNo(sec);
    switch (val) {
        case 48: p = sec[4] + 24; break;
        default: p = NULL; break;
    }
    return p;
}



int prt_code_table_4_91(int type_of_interval, double val1, double val2, char *inv_out) {

    switch (type_of_interval) {
        case 0: sprintf(inv_out,"<%lg", val1); break;
        case 1: sprintf(inv_out,">%lg", val2); break;
//        case 2: sprintf(inv_out,"%lg<=x<%lg", val1, val2); break;
        case 2: sprintf(inv_out,">=%lg,<%lg", val1, val2); break;
        case 3: sprintf(inv_out,">%lg", val1); break;
        case 4: sprintf(inv_out,"<%lg", val2); break;
        case 5: sprintf(inv_out,"<=%lg", val1); break;
        case 6: sprintf(inv_out,">=%lg", val2); break;
//        case 7: sprintf(inv_out,"%lg<=x<=%lg", val1, val2); break;
        case 7: sprintf(inv_out,">=%lg,<=%lg", val1, val2); break;
        case 8: sprintf(inv_out,">=%lg", val1); break;
        case 9: sprintf(inv_out,"<=%lg", val2); break;
//        case 10: sprintf(inv_out,"%lg<x<=%lg", val1, val2); break;
        case 10: sprintf(inv_out,">%lg,<=%lg", val1, val2); break;
        case 11: sprintf(inv_out,"=%lg", val1); break;
        default: sprintf(inv_out,"??? code_table_4.91=%d",type_of_interval); break;
    }
    return 0;
}


/*
 * HEADER:-1:code_table_4.230:inv:0:code table 4.230 chemical constituent type
 */

extern struct codetable_4_230  codetable_4_230_table[];

int f_code_table_4_230(ARG0) {
    int val, i;
    const char *string;
    static int error_count = 0;

    if (mode >= 0) {
        val = code_table_4_230(sec);
	if (val < 0) return 0;

	i = 0;
	string = NULL;
	if (GB2_MasterTable(sec) <= 4) {
	    if (error_count++ <= 10) fprintf(stderr,
		"Warning: if file made with ECMWF API, may be using incompatible chemistry table\n");
	}
	while (codetable_4_230_table[i].no != 65535) {
	    if (codetable_4_230_table[i].no == val) {
		string = codetable_4_230_table[i].name;
		break;
	    }
	    i++;
	}
	if (GB2_MasterTable(sec) <= 4 && GB2_Center(sec) == ECMWF) string = NULL;
        if (string == NULL) sprintf(inv_out,"code table 4.230=%d", val);
        else sprintf(inv_out,"code table 4.230=%d %s", val, string);
	inv_out += strlen(inv_out);
    }
    return 0;
}

int code_table_4_230(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_230_location(sec);
    return  (p == NULL) ? -1 : uint2(p);
}

unsigned char *code_table_4_230_location(unsigned char **sec) {
    int pdt;
    pdt = GB2_ProdDefTemplateNo(sec);
    unsigned char *p;
    switch(pdt) {
        case 40: 
	case 41:
	case 42:
	case 43:
	case 57:
	case 67:
	case 68:
		p = sec[4]+11; break;
	default: p = NULL; break;
    }
    return  p;
}

/*
 * HEADER:-1:code_table_4.233:inv:0:code table 4.233 aerosol type
 */

int f_code_table_4_233(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_233(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.233.dat"
            }
            fprintf(stderr,">>>>code table 4.233=%d %s", val, string);
            if (string == NULL) sprintf(inv_out,"code table 4.233=%d", val);
            else sprintf(inv_out,"code table 4.233=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_4_233(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_233_location(sec);
    if (p == NULL) return -1;
    return (int) uint2(p);
}

unsigned char *code_table_4_233_location(unsigned char **sec) {
    int pdt;
    pdt = GB2_ProdDefTemplateNo(sec);
    if (pdt == 47) return sec[4]+12;
    if (pdt >= 44 && pdt <= 48) return sec[4]+11;
    return  NULL;
}

/*
 * HEADER:-1:code_table_4.235:inv:0:code table 4.235 Wind-generated wave spectral description
 */
int f_code_table_4_235(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
	val = code_table_4_235(sec);
	if (val >= 0) {
	    string = NULL;
	    switch(val) {
#include "CodeTable_4.235.dat"
	    }
            if (string == NULL) sprintf(inv_out,"code table 4.235=%d", val);
            else sprintf(inv_out,"code table 4.235=%d %s", val, string);
        }
    }
    return 0;
}
int code_table_4_235(unsigned char **sec) {
    int pdt;
    pdt = GB2_ProdDefTemplateNo(sec);
    if (pdt == 52) return sec[4][11];
    return -1;
}
/*
 * HEADER:-1:code_table_4.240:inv:0:code table 4.240 Type of distribution function
 */
int f_code_table_4_240(ARG0) {
    int val;
    const char *string;
    if (mode >= 0) {
        val = code_table_4_240(sec);
        if (val >= 0) {
            string = NULL;
            switch(val) {
#include "CodeTable_4.240.dat"
            }
            if (string == NULL) sprintf(inv_out,"code table 4.240=%d", val);
            else sprintf(inv_out,"code table 4.240=%d %s", val, string);
        }
    }
    return 0;
}

int code_table_4_240(unsigned char **sec) {
    unsigned char *p;
    p = code_table_4_240_location(sec);
    if (p == NULL) return -1;
    return (int) uint2(p);
}

unsigned char *code_table_4_240_location(unsigned char **sec) {
    int pdt;
    unsigned char *p;

    pdt = GB2_ProdDefTemplateNo(sec);
    switch(pdt) {
	case 57: 
	case 67: 
	case 68: 
	    p = sec[4]+17; break;
	default:
	    p = NULL; break;
    }
    return  p;
}

/*
 * HEADER:-1:code_table_5.0:inv:0:code table 5.0 data representation number
 */
int f_code_table_5_0(ARG0) {
    int p;
    const char *string;
    if (mode >= 0) {
	p = code_table_5_0(sec);
	if (p >= 0) {
	    string = NULL;
	    switch(p) {
#include "CodeTable_5.0.dat"
            }
	    if (string == NULL) sprintf(inv_out,"code table 5.0=%d", p);
	    else sprintf(inv_out,"code table 5.0=%d %s", p, string);
	}
    }
    return 0;
}

int code_table_5_0(unsigned char **sec) {
    return (int) uint2(sec[5]+9);
}

/*
 * HEADER:-1:code_table_5.1:inv:0:code table 5.1 type of original field values
 */
int f_code_table_5_1(ARG0) {
    int p;
    const char *string;

    if (mode >= 0) {
        p = code_table_5_1(sec);
        if (p >= 0) {
	    string = NULL;
	    switch(p) {
#include "CodeTable_5.1.dat"
            }
	    if (string == NULL) sprintf(inv_out,"code table 5.1=%d", p);
	    else sprintf(inv_out,"code table 5.1=%d %s", p, string);
	}
    }
    return 0;
}

int code_table_5_1(unsigned char **sec) {

    switch(code_table_5_0(sec)) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 40:
    case 41:
    case 42:
        return (int) (sec[5][20]);
        break;
    default:
	return -1;
	break;
    }
    return -1;
}

/*
 * HEADER:-1:code_table_5.4:inv:0:code table 5.4 group splitting method
 */

int f_code_table_5_4(ARG0) {
    int p;
    if (mode >= 0) {
        p = code_table_5_4(sec);
        if (p >= 0) sprintf(inv_out,"code table 5.4=%d", p);
    }
    return 0;
}
int code_table_5_4(unsigned char **sec) {
    if (code_table_5_0(sec) < 2 || code_table_5_0(sec) > 3) return -1;
    return (int) (sec[5][21]);
}


/*
 * HEADER:-1:code_table_5.5:inv:0:code table 5.5 missing value management for complex packing
 */
int f_code_table_5_5(ARG0) {
    int p;
    if (mode >= 0) {
        p = code_table_5_5(sec);
        if (p >= 0) sprintf(inv_out,"code table 5.5=%d", p);
    }
    return 0;
}
int code_table_5_5(unsigned char **sec) {
    if (code_table_5_0(sec) < 2 || code_table_5_0(sec) > 3) return -1;
    return (int) (sec[5][22]);
}

/*
 * HEADER:-1:code_table_5.6:inv:0:code table 5.5 complex packing spatial differencing
 */
int f_code_table_5_6(ARG0) {
    int p;
    if (mode >= 0) {
        p = code_table_5_6(sec);
        if (p >= 0) sprintf(inv_out,"code table 5.6=%d", p);
    }
    return 0;
}
int code_table_5_6(unsigned char **sec) {
    if (code_table_5_0(sec) != 3) return -1;
    return (int) (sec[5][47]);
}

/*
 * HEADER:-1:code_table_5.7:inv:0:code table 5.7 precision in IEEE packing
 */
int f_code_table_5_7(ARG0) {
    int p;
    if (mode >= 0) {
        p = code_table_5_7(sec);
        if (p >= 0) sprintf(inv_out,"code table 5.7=%d", p);
    }
    return 0;
}

int code_table_5_7(unsigned char **sec) {
    if (code_table_5_0(sec) ==  4) return (int) (sec[5][11]);
    return -1;
}




/*
 * HEADER:-1:code_table_6.0:inv:0:code table 6.0 Bitmap indicator
 */
int f_code_table_6_0(ARG0) {
    int p;
    const char *string;
    if (mode >= 0) {
        p = code_table_6_0(sec);
        if (p >= 0) {
	    if (mode == 0) sprintf(inv_out,"code table 6.0=%d", p);
	    else {
	        string = NULL;
	        switch(p) {
#include "CodeTable_6.0.dat"
                }
	        if (string == NULL) sprintf(inv_out,"code table 6.0=%d", p);
	        else sprintf(inv_out,"code table 6.0=%d %s", p, string);
	    }
	}
    }
    return 0;
}

int code_table_6_0(unsigned char **sec) {
    return (int) (sec[6][5]);
}
