/** @file
 * @brief Routine to set bytes in GRIB sections.
 * @author Public Domain: Wesley Ebisuzaki @date 3/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:set_byte:misc:3:set bytes in Section X, Octet Y, bytes Z (a|a:b:c)
 */

/**
 * Sets 1 octet (byte) to a a specified integer value. The integer value ranges from 0 to 255. 
 * 
 * -set_byte I J K
 * 
 * I = section number = 1 to 7
 * J = location in the section = 1 to (section length)
 * K = 0 to 255
 *
 * would set Section I, Octet J to value K.
 * 
 * Multiple octets can be set by making the third argument a colon seperated list. 
 * 
 * ## Usage
 * -set_byte  SECTION STARTING_OCTET_LOCATION I-1:I-2:..:I-N
 * 
 * SECTION = 0 to 7
 * STARTING_OCTET_LOCATION >= 1
 * I-M = Mth octet
 *
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;


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

/**
 * Sets 1 octet (byte) to a specified hexadecimal value.  The hex value ranges from 0 to ff. 
 * 
 * -set_hex I J K
 * 
 * I = section number = 1 to 7
 * J = location in the section = 1 to (section length)
 * K = 00 to ff
 *
 * would set Section I, Octet J to hex value K. 
 * 
 * -set_hex I J K1:K2:K3:..KN
 * 
 * Ki = 00 .. ff
 *
 * Sets Section I, Octet J to hex value K1.
 * Sets Section I, Octet J+1 to hex value K2.
 * etc.
 * 
 * -set_hex I J K1K2K3..KN
 *
 * Ki = 00 .. ff
 * 
 * Sets Section I, Octet J to hex value K1.
 * Sets Section I, Octet J+1 to hex value K2.
 * etc.
 * 
 * ## Usage
 * -set_hex  SECTION STARTING_OCTET_LOCATION (I-1):(I-2):..:(I-N)
 * 
 * -set_hex  SECTION STARTING_OCTET_LOCATION (I-1)(I-2)..(I-N)
 * 
 * Section = 0 to 7
 * STARTING_OCTET_LOCATION = 1 to N
 * I-M = Mth octet as a 2 digit hex number
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * The file, template_512.grb, was converted from grib1, and the delta-lon, and the extreme latitudes were 
 * only to the nearest millidegree (grib1 precision). 
 * 
 * @code{.sh}
 * $ wgrib2 template_512.grb -grid
 * 1:0:grid_template=40:winds(N/S):
 *	    Gaussian grid: (512 x 256) units 1e-06 input WE:NS output WE:SN
 *	    number of latitudes between pole-equator=128 #points=131072
 *	    lat 89.463000 to -89.463000
 *	    lon 0.000000 to 359.233000 by 0.703000
 * @endcode
 * 
 * The grid with full precision can be obtained by using -new_grid, and the contents of sec3 (grid definition) 
 * can be obtained by -0xSec 3. 
 * 
 * @code{.sh}
 * $ wgrib2 template_512.grb -new_grid_winds earth -new_grid ncep grid 170 junk
 * 1:0:d=2016010109:PRES:mean sea level:2390 hour fcst:
 * $ wgrib2 junk -0xSec 3
 * 1:0:Sec3(1..72)=0x0000004803000002000000000028060000000000000000000000000000000000020000000
 * 10000000000ffffffff05551826000000003085551826156a6f6b000aba950000008000
 * @endcode
 * 
 * The script to change the precision of the Gaussian grid is given by:
 * @code{.sh}
 * $ sec3='000000480300000200000000002806000000000000000000000000000000000002000000010000000000ffffffff05551826000000003085551826156a6f6b000aba950000008000'
 * $ wgrib2 $1 -set_hex 3 1 "$sec3" -grid -grib $1.new
 * @endcode
 * 
 * The above script assumes that the size of the original section 3 is greater or equal to the size of the new 
 * section 3. If it isn't, you have to use the option -set_sec_size 3 72. Of course, you cannot make arbitrary 
 * changes to the grid definition because the number of grid points has to match the grid points in the data 
 * section. 
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;

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

/**
 * Sets 4 octets to a signed integer value as commonly use by grib for signed integers. The 
 * integer must range from -2147483647 to 2147483647.
 * 
 * -set_int I J K
 *
 * I = section number = 1 to 7
 * J = 1 to (section length - 3)
 * K = -(2**23-1) to (2**23-1)
 * 
 * would set:
 * 
 * @code{}
 * if (K >= 0) (K >> 24) & 255 // Section I, Octet J+0
 * if (K < 0) ((abs(K) >> 24) && 255) | 128 // Section I, Octet J+0
 * (abs(K) >> 16) & 255 // Section I, Octet J+1
 * (abs(K) >> 8) & 255 // Section I, Octet J+2
 * abs(K) & 255 // Section I, Octet J+3
 * @endcode
 * 
 * Multiple integers can be set by making the third argument a colon seperated list. 
 * 
 * ## Usage
 * -set_int  SECTION STARTING_OCTET_LOCATION I-1:I-2:..:I-N
 *
 * SECTION = 0 to 7
 * STARTING_OCTET_LOCATION = 1 to N
 * I-M = Mth integer
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;

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

/**
 * Sets 2 octets to a signed integer value as commonly use by grib for signed integers. The 
 * integer must range from -32767 to 32767. 
 * 
 * -set_int2 I J K
 *
 * I = section number = 1 to 7
 * J = 1 to (section length - 1)
 * K = -(2**15-1) to (2**15-1)
 * 
 * would set:
 * 
 * @code{}
 * if (K >= 0) (K >> 8) & 255 // Section I, Octet J+0
 * if (K < 0) ((abs(K) >> 8 ) && 255) | 128 // Section I, Octet J+0
 * abs(K) & 255 // Section I, Octet J+3
 * @endcode
 * 
 * Multiple integers can be set by making the third argument a colon seperated list. 
 * 
 * ## Usage
 * -set_int2  SECTION STARTING_OCTET_LOCATION I-1:I-2:..:I-N
 *
 * SECTION = 0 to 7
 * STARTING_OCTET_LOCATION = 1 to N
 * I-M = Mth integer
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;

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

/**
 * Sets 4 octets to a single precision IEEE floating point value. The IEEE floating point standard 
 * is often used grib2. 
 * 
 * -set_ieee I J X
 *
 * I = section number = 1 to 7
 * J = 1 to (section length - 3)
 * X = single precision floating point number
 * 
 * Multiple floats can be set by making the third argument a colon seperated list. 
 * 
 * ## Usage
 * -set_ieee SECTION STARTING_OCTET_LOCATION I-1:I-2:..:I-N
 *
 * SECTION = 0 to 7
 * STARTING_OCTET_LOCATION = 1 to N
 * I-M = Mth floating point number
 *
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 on success. Throws fatal_error() on failure.
 *
 * @author Wesley Ebisuzaki @date 3/2008
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;


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
 * HEADER:100:get_byte:inv:3:get bytes in Section X, Octet Y, number of bytes Z (decimal format)
 */

/**
 * Prints the values of selected bytes in decimal format.
 * 
 * For example if you want to see the 20 and 21 octet (byte in WMO speak) of section 4, you would 
 * use -get_byte 4 20 2. The first argument is the section number. The second is the byte (octet) 
 * number starting from 1 (consistent with WMO documentation) and the third is the number of octets 
 * to display. 
 * 
 * ## Usage
 * -get_byte SECTION OCTET NUMBER
 * 
 * SECTION = section of the grib message to print
 * OCTET = starting octet to start printing, OCTET ≥ 1
 * NUMBER = number of bytes to print
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 f.grb2 -get_byte 0 1 16
 * 1:0:0-1=71,82,73,66,0,0,0,2,0,0,0,0,0,0,18,178
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 5/2009
 */
int f_get_byte(ARG3) {

    int i, j, k, m;
    double tot;
    unsigned int seclen;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("get_byte: bad section number %s", arg1);
    if (j <= 0) fatal_error("get_byte: octet number must >= 1","");

    k = atoi(arg3);

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = sec[i] ? uint4(sec[i]) : 0;

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

/**
 * Prints the values of selected bytes in hexadecimal format.
 * 
 * See the 20 and 21 octet (byte in WMO speak) of section 4, you would use -get_byte 4 20 2. The 
 * first argument is the section number. The second is the byte (octet) number starting from 1 
 * (consistent with WMO documentation) and the third is the number of octets to display. 
 * 
 * The input arguments to the option are in decimal rather than in hexidecimal because all the 
 * grib2 documentation uses decimal numbers to specify the byte locations. 
 * 
 * ## Usage
 * -get_hex SECTION OCTET NUMBER
 * 
 * SECTION = section to print
 * OCTET = starting octet to print
 * NUMBER = number of octets/bytes to print
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 f.grb2 -get_hex 0 1 12
 * 1:0:0-1=71,52,49,42,00,00,00,02,00,00,00,00
 * 2:46042:0-1=71,52,49,42,00,00,00,02,00,00,00,00
 * 3:63079:0-1=71,52,49,42,00,00,00,02,00,00,00,00
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 5/2009
 */
int f_get_hex(ARG3) {

    int i, j, k, m;
    double tot;
    unsigned int seclen;

    if (mode < 0) return 0;
    i = atoi(arg1);
    j = atoi(arg2);
    if (i < 0 || i > 8) fatal_error("get_hex: bad section number %s", arg1);
    if (j <= 0) fatal_error("get_hex: octet number must >= 1","");

    k = atoi(arg3);

    if (i == 0) seclen = GB2_Sec0_size;
    else if (i == 8)  seclen = GB2_Sec8_size;
    else seclen = sec[i] ? uint4(sec[i]) : 0;

    if (k+j-1 > seclen) fatal_error("get_hex - query out of range","");

    // check output size first
    tot = 3*k;

    if (tot > INV_BUFFER - 1000) {
        sprintf(inv_out,"%d-%d=too many numbers",i,j);
        return 0;
    }

    // 5/2022 sprintf(inv_out,"%d-%d=%d",i,j,sec[i][j-1]);
    sprintf(inv_out,"%d-%d=%.2x",i,j,sec[i][j-1]);
    for (m = 1; m < k; m++) {
        inv_out += strlen(inv_out);
        sprintf(inv_out,",%.2x",sec[i][j+m-1]);
    }
    return 0;
}

/*
 * HEADER:100:get_int:inv:3:get 4-byte ints in Section X, Octet Y, number of ints Z
 */

/**
 * Prints the values of selected 4 byte integers.
 * 
 * For example if you want to see the number of data points (section 3, octets 7-10), you could 
 * use -get_int 3 7 1. The first argument is the section number. The second is the octet number 
 * starting from 1 (consistent with WMO grib documentation) and the third is the number of integers 
 * to display. The -get_int option uses the grib format for signed 4-octet integers. 
 * 
 * ## Usage
 * -get_int SECTION OCTET COUNT
 * 
 * SECTION = section number of the grib message to print
 * OCTET = the octet number to print
 * N = number of integers to print
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 f.grb -get_int 3 7 1  
 * 1:0:3-7=65160
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 5/2009
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;

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

/**
 * Prints the values of selected 2 byte integers.
 * 
 * For example if you wanted to see octet N and N+1 is section M as a signed two byte integer, 
 * you could use -get_int2 M N 1. The first argument is the section number. The second is the 
 * octet number starting from 1 (consistent with WMO grib documentation) and the third is the 
 * number of integers to display. The -get_int2 option uses the grib format for signed 2-octet 
 * integers. 
 * 
 * ## Usage
 * -get_int2 SECTION OCTET COUNT
 * 
 * SECTION = section number of the grib message to print
 * OCTET = the octet number to print
 * N = number of integers to print
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 f.grb -get_int2 1 6 1  
 * 1:0:1-6=7
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 5/2009
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;

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

/**
 * Prints the values of selected bytes as an IEEE floating point number.
 * 
 * ## Usage
 * -get_ieee SECTION OCTET N
 * 
 * SECTION = section number of the grib message to print
 * OCTET = the octet number to print
 * N = number of IEEE float to print (4 octets per IEEE number)
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 rtgssthr_grb_0.083_awips.grib2
 * 1:0:d=2009062900:TMP:surface:anl:
 * : field is surface temperature, SST over water
 * $ wgrib2 rtgssthr_grb_0.083_awips.grib2 -packing -v
 * 1:0:packing=grid point data - jpeg2000 compression,j val=(27133+i*2^0)*10^-2, i=0..8191 (#bits=13)
 *  : jpeg2000 packing, reference value is 27133
 * $ wgrib2 rtgssthr_grb_0.083_awips.grib2 -get_ieee 5 12 1
 * 1:0:5-12=27133.000000
 *  : for jpeg2000 packing, the reference value is stored in Section 5, octet 12-15
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 5/2009
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
    else seclen = sec[i] ? uint4(sec[i]) : 0;

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
