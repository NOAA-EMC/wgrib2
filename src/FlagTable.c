/** @file
 * @brief Routines to return the value of the various flags. Use this instead of .h 
 * files.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 12/2006 | W. Ebisuzaki | Initial
 * 1/2007 | M. Schwarb | Cleanup
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 12/2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Defines scan order of the grid. */
extern char *scan_order[];

/** Defines stagger of the grid. */
extern char *stagger_description[];

/*
 * HEADER:-1:flag_table_3.3:inv:0:flag table 3.3, resolution and component flags
 */

/**
 * Prints the resolution and component flags (Flag Table 3.3) from the GRIB2 message.
 * 
 * ## Usage
 * -flag_table_3.3
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 12/2006
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

/**
 * Return the value of the resolution and component flags (Flag Table 3.3) from the 
 * GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return The value of the resolution and component flags, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int flag_table_3_3(unsigned char **sec) {
    unsigned char *p;
    p = flag_table_3_3_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

/**
 * Set the value of the resolution and component flags (Flag Table 3.3) in the GRIB2 
 * message.
 * 
 * @param sec Pointer to the GRIB section.
 * @param flag The value to set.
 * 
 * @return 0 on success, or 1 if the flag could not be found.
 */
int set_flag_table_3_3(unsigned char **sec, unsigned int flag) {
    unsigned char *p;
    p = flag_table_3_3_location(sec);
    if (p == NULL) return 1;
    *p = flag;
    return 0;
}

/**
 * Returns the location of the resolution and component flags (Flag Table 3.3) in
 * the GRIB2 message.
 *
 * @param sec Pointer to the GRIB section.
 * 
 * @return Pointer to the location of the flags, or NULL if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
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
              return gds+54;
        case 4:
        case 5:
        case 10:
        case 12:
        case 20:
        case 30:
        case 31:
        case 90:
        case 110:
              return gds+46;
        case 60:
              return gds+71;
        case 32768:
            if (center == NCEP) return gds+54;
            return NULL;
        case 32769:
            if (center == NCEP) return gds+54;
            return NULL;
        case 40110:
            if ((center == JMA1) || (center == JMA2)) return gds+46; 
            return NULL;
            default: break;
    }
    return NULL;
}


/*
 * HEADER:1:vector_dir:inv:0:grid or earth relative winds
 */

/**
 * Bit 5 of the flag 3.3 indicates whether vector quantities are relative to the grid 
 * or the North/South poles. 
 * 
 * This option prints out the "winds(N/S)" or "winds(grids)" depending on the value of the flag.
 * 
 * Note there is no flag that indicates whether the quantity is a U/V component of a vector. 
 * 
 * ## Usage
 * -vector_dir
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * ## Example
 * 
 * @code{.sh}
 * $wgrib2 png.grb2 -vector_dir
 * 1:4:winds(N/S)
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 12/2006
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

/**
 * Prints out the scanning mode (Flag Table 3.4) from the GRIB2 message.
 * 
 * ## Usage
 * -flag_table_3.4
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 12/2006
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

/**
 * Returns the scanning mode (Flag Table 3.4) from the GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return The value of the scanning mode, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int flag_table_3_4(unsigned char **sec) {
    unsigned char *p;
    p = flag_table_3_4_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

/**
 * Sets the scanning mode (Flag Table 3.4) in the GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * @param flag The value to set for the scanning mode.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int set_flag_table_3_4(unsigned char **sec, unsigned int flag) {
    unsigned char *p;
    p = flag_table_3_4_location(sec);
    if (p == NULL) return 1;
    *p = flag;
    return 0;
}

/**
 * Returns the location of the scanning mode (Flag Table 3.4) in the GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return Pointer to the location of the scanning mode, or NULL if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
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
        case 20: 
            return gds+64; break;
        case 30:
        case 31: 
            return gds+64; break;
        case 50:
        case 51:
        case 52:
        case 53:
            /* spectral modes don't have scan order */
            return NULL; break;
        case 90: 
        case 140: 
            return gds+63; break;
        case 110: 
            return gds+56; break;
        case 190: 
        case 120: 
            return gds+38; break;
        case 204: 
            return gds+71; break;
        case 1000: 
            return gds+50; break;
        case 60:
            return gds+72; break;
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

/**
 * Prints out the projection center (Flag Table 3.5) from the GRIB2 message.
 * 
 * ## Usage
 * -flag_table_3.5
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 12/2006
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

/**
 * Returns the projection center (Flag Table 3.5) from the GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return The value of the projection center, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int flag_table_3_5(unsigned char **sec) {
    unsigned char *p;
    p = flag_table_3_5_location(sec);
    if (p == NULL) return -1;
    return (int) *p;
}

/**
 * Returns the location of the projection center (Flag Table 3.5) in the GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return Pointer to the location of the projection center, or NULL if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
unsigned char *flag_table_3_5_location(unsigned char **sec) {
    unsigned char *gds;
    int center;

    gds = sec[3];
    center = GB2_Center(sec);

    switch (code_table_3_1(sec)) {
        case 20:
        case 30:
        case 31: 
            return gds+63; break;
        case 110: 
            return gds+55; break;
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

/**
 * Prints the numbering order of diamonds seen from the corresponding pole (Flag Table 3.9) 
 * from the GRIB2 message.
 * 
 * ## Usage
 * -flag_table_3.9
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 12/2006
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

/**
 * Returns the numbering order of diamonds seen from the corresponding pole (Flag Table 3.9)
 * from the GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return The value of the numbering order of diamonds, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
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

/**
 * Prints the scanning mode for one diamond (Flag Table 3.10) from the GRIB2 message.
 * 
 * ## Usage
 * -flag_table_3.10
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @author Wesley Ebisuzaki @date 12/2006
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

/**
 * Returns the scanning mode for one diamond (Flag Table 3.10) from the GRIB2 message.
 * 
 * @param sec Pointer to the GRIB section.
 * 
 * @return The value of the scanning mode, or -1 if not found.
 * 
 * @author Wesley Ebisuzaki @date 12/2006
 */
int flag_table_3_10(unsigned char **sec) {
    unsigned char *gds;
    gds = sec[3];
    switch (code_table_3_1(sec)) {
        case 100: return gds[33];
    }
    return -1;
}

