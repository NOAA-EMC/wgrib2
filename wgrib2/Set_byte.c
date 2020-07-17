#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * set_byte
 *
 * routine to modify grib fields
 *
 * 3/2008 Public Domain by Wesley Ebisuzaki
 *
 */

/*
 * HEADER:100:set_byte:misc:3:set bytes in Section X, Octet Y, bytes Z (a|a:b:c)
 */

int f_set_byte(ARG3) {

    int i, j, k, m, seclen;
    unsigned int val;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("set_byte: bad section number %s", arg1);
    if (j <= 0) fatal_error("set_byte: octet number must >= 1","");

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    k = sscanf(arg3, "%u%n", &val, &m);
    while (k == 1) {
	if (j > seclen) fatal_error("set_byte out of bounds section %s",arg1);
	sec[i][j++ -1] = val;
	arg3 += m;
        k = sscanf(arg3, ":%u%n", &val, &m);
    }
    return 0;
}

/*
 * HEADER:100:set_hex:misc:3:set bytes in Section X, Octet Y, bytes Z (a|a:b:c|abc) in hexadecimal
 */

int f_set_hex(ARG3) {

    int i, j, k, m, seclen, has_colon;
    unsigned int val;
    const char *str;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("set_byte: bad section number %s", arg1);
    if (j <= 0) fatal_error("set_hex: octet number must >= 1","");

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    /* see if there is a colon */
    str = arg3;
    has_colon = 0;
    while (*str) {
	if (*str++ == ':') {
	    has_colon = 1;
	    break;
	}
    }
    if (has_colon) {
        k = sscanf(arg3, "%x%n", &val, &m);
        while (k == 1) {
            if (j > seclen) fatal_error("set_byte out of bounds section %s",arg1);
            sec[i][j++ -1] = val;
            arg3 += m;
            k = sscanf(arg3, ":%x%n", &val, &m);
        }
    }
    else {
        k = sscanf(arg3, "%2x%n", &val, &m);
	while (k == 1) {
            if (j > seclen) fatal_error("set_byte out of bounds section %s",arg1);
            sec[i][j++ -1] = val;
            arg3 += m;
            k = sscanf(arg3, "%2x%n", &val, &m);
        }
    }
    return 0;
}


/*
 * HEADER:100:set_int:misc:3:set 4-byte ints in Section X, Octet Y, signed integers Z (a|a:b:c)
 */

int f_set_int(ARG3) {

    int i, j, k, m, val, seclen;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("set_int: bad section number %s", arg1);
    if (j <= 0) fatal_error("set_hex: octet number must >= 1","");
    
    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    k = sscanf(arg3, "%d%n", &val, &m);
    while (k == 1) {
        if (j+3 > seclen) fatal_error("set_int out of bounds section %s",arg1);
	int_char(val, sec[i]+j-1);
	j += 4;
        arg3 += m;
        k = sscanf(arg3, ":%d%n", &val, &m);
    }
    return 0;
}

/*
 * HEADER:100:set_int2:misc:3:set 2-byte ints in Section X, Octet Y, signed integers Z (a|a:b:c)
 */

int f_set_int2(ARG3) {

    int i, j, k, m, val, seclen;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("set_int2: bad section number %s", arg1);
    if (j <= 0) fatal_error("set_int2: octet number must >= 1","");

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    k = sscanf(arg3, "%d%n", &val, &m);
    while (k == 1) {
        if (j+1 > seclen) fatal_error("set_int2 out of bounds section %s",arg1);
        int2_char(val, sec[i]+j-1);
        j += 2;
        arg3 += m;
        k = sscanf(arg3, ":%d%n", &val, &m);
    }
    return 0;
}

/*
 * HEADER:100:set_ieee:misc:3:set ieee float in Section X, Octet Y, floats Z (a|a:b:c)
 */

int f_set_ieee(ARG3) {

    int i, j, k, m, seclen;
    float val;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("set_ieee - bad section number %s", arg1);
    if (j <= 0) fatal_error("set_ieee: octet number must >= 1","");

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    k = sscanf(arg3, "%f%n", &val, &m);
    while (k == 1) {
        if (j+3 > seclen) fatal_error("set_int out of bounds section %s",arg1);
        flt2ieee(val, sec[i]+j-1);
        j += 4;
        arg3 += m;
        k = sscanf(arg3, ":%f%n", &val, &m);
    }
    return 0;
}


/*
 * get_byte
 *
 * routine to view bytes in decimal format
 *
 * 5/2009 Public Domain by Wesley Ebisuzaki
 *
 */


/*
 * HEADER:100:get_byte:inv:3:get bytes in Section X, Octet Y, number of bytes Z (decimal format)
 */

int f_get_byte(ARG3) {

    int i, j, k, m, seclen;
    double tot;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("get_byte: bad section number %s", arg1);
    if (j <= 0) fatal_error("get_byte: octet number must >= 1","");

    k = atoi(arg3);

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    if (k+j-1 > seclen) fatal_error("get_byte - query out of range","");

    // check output size first
    tot = 0;
    for (m = 1; m < k; m++) {
	if (sec[i][j+m-1] > 99) tot +=4.0;
	else if (sec[i][j+m-1] > 9) tot +=3.0;
	else tot += 2.0;
    }
    if (tot > INV_BUFFER - 1000) {
	sprintf(inv_out,"%d-%d=too many numbers",i,j);
	return 0;
    }

    sprintf(inv_out,"%d-%d=%d",i,j,sec[i][j-1]);
    for (m = 1; m < k; m++) {
	inv_out += strlen(inv_out);
	sprintf(inv_out,",%d",sec[i][j+m-1]);
    }
    return 0;
}

/*
 * HEADER:100:get_hex:inv:3:get bytes in Section X, Octet Y, number of bytes Z (bytes in hexadecimal format)
 */

int f_get_hex(ARG3) {

    int i, j, k, m, seclen;
    double tot;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("get_hex: bad section number %s", arg1);
    if (j <= 0) fatal_error("get_hex: octet number must >= 1","");

    k = atoi(arg3);

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    if (k+j-1 > seclen) fatal_error("get_hex - query out of range","");

    // check output size first
    tot = 3*k;

    if (tot > INV_BUFFER - 1000) {
        sprintf(inv_out,"%d-%d=too many numbers",i,j);
        return 0;
    }

    sprintf(inv_out,"%d-%d=%d",i,j,sec[i][j-1]);
    for (m = 1; m < k; m++) {
        inv_out += strlen(inv_out);
        sprintf(inv_out,",%.2x",sec[i][j+m-1]);
    }
    return 0;
}

/*
 * HEADER:100:get_int:inv:3:get 4-byte ints in Section X, Octet Y, number of ints Z
 */

int f_get_int(ARG3) {
    int i, j, k, m, seclen, len;
    double tot;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("get_int: bad section number %s", arg1);
    if (j <= 0) fatal_error("get_int: octet number must >= 1","");
    k = atoi(arg3);

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    if (4*k+j-1 > seclen) fatal_error("get_int - query out of range","");
    sprintf(inv_out,"%d-%d=%d",i,j,int4(sec[i]+j-1));
    tot = (len = strlen(inv_out));
    inv_out += len;

    for (m = 1; m < k; m++) {
        sprintf(inv_out,",%d", int4(sec[i]+j-1+4*m));
        tot += (len = strlen(inv_out));
        inv_out += len;
	if (tot > INV_BUFFER - 1000) {
            sprintf(inv_out,"... too many numbers");
 	    return 0;
	}
    }
    return 0;
}

/*
 * HEADER:100:get_int2:inv:3:get 2-byte ints in Section X, Octet Y, number of ints Z
 */

int f_get_int2(ARG3) {
    int i, j, k, m, seclen, len;
    double tot;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("get_int2: bad section number %s", arg1);
    if (j <= 0) fatal_error("get_int2: octet number must >= 1","");
    k = atoi(arg3);

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    if (2*k+j-1 > seclen) fatal_error("get_int2 - query out of range","");
    sprintf(inv_out,"%d-%d=%d",i,j,int2(sec[i]+j-1));
    tot = (len = strlen(inv_out));
    inv_out += len;

    for (m = 1; m < k; m++) {
        sprintf(inv_out,",%d", int2(sec[i]+j-1+2*m));
        tot += (len = strlen(inv_out));
        inv_out += len;
        if (tot > INV_BUFFER - 1000) {
            sprintf(inv_out,"... too many numbers");
            return 0;
        }
    }
    return 0;
}

/*
 * HEADER:100:get_ieee:inv:3:get ieee float in Section X, Octet Y, number of floats Z
 */

int f_get_ieee(ARG3) {
    int i, j, k, m, seclen, len;
    double tot;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("get_ieee - bad section number %s", arg1);
    if (j <= 0) fatal_error("get_ieee: octet number must >= 1","");
    k = atoi(arg3);

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = uint4(sec[i]);

    if (4*k+j-1 > seclen) fatal_error("get_int - query out of range","");
    sprintf(inv_out,"%d-%d=%lf",i,j,(double) ieee2flt(sec[i]+j-1));
    tot = (len = strlen(inv_out));
    inv_out += len;

    for (m = 1; m < k; m++) {
        sprintf(inv_out,",%lf", (double) ieee2flt(sec[i]+j-1+4*m));
        tot += (len = strlen(inv_out));
        inv_out += len;
        if (tot > INV_BUFFER - 1000) {
            sprintf(inv_out,"... too many numbers");
            return 0;
        }
    }
    return 0;
}
