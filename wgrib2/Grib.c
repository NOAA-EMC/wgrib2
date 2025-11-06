/** @file
 * @brief Routines to encode data into grib2.
 * @author Public Domain: Wesley Ebisuzaki @date 12/2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_G2CLIB_LOW
#include <grib2.h>
#endif

/*
 * Grib_out
 *
 * routines to encode data into grib2
 *
 * 12/2007 Public Domain by Wesley Ebisuzaki
 * 05/2016 Public domain DWD
 *
 */

/** Decode grib file flag. */
extern int decode;

/** Append grib file flag. */
extern int file_append;

/** Flush of output flag. */
extern int flush_mode;

/** Current output order type. */
extern enum output_order_type output_order;

/** Use scaling flag. */
extern int use_scale;

/** Input scaling flag. */
extern int input_scale;

/** Decimal scaling. */
extern int dec_scale;

/** Binary scaling. */
extern int bin_scale;

/** Maximum number of bits. */
extern int max_bits;

/** Number of bits wanted. */
extern int wanted_bits;

/** Current output grib type. */
extern enum output_grib_type grib_type;

/** Number of grid points in the x direction. */
extern int nx;

/** Number of grid points in the y direction. */
extern int ny;

/** Scan order mode. */
extern int scan;

/** Flag to indicate whether to save translation information. */
extern int save_translation;

/** Flag to use bitmap. */
extern int use_bitmap;

/*
 * HEADER:100:set_grib_type:misc:1:set grib type = jpeg, simple, ieee, complex(1|2|3), aec, same
 */

/**
 * Set the packing method for the output grib file.
 * 
 * wgrib2 can write grib files by either copying an existing packed grib message (-grib) or by 
 * taking floating point values and packing them into a new grib message. (For example, -grib_out, 
 * -small_grib, -ij_small_grib, and -wind_speed). For the latter, one has a choice of packing 
 * methods. Generally methods that are faster methods create larger files and slower methods make 
 * smaller files. For fields with no undefined values, the jpeg2000 produces the smallest files. 
 * With bitmaps, one of the complex packings is usually the best (of the supported packing). 
 * Complex3 is often the best trade off between speed and compression for smooth fields. For other 
 * fields complex1 is often the best trade off between speed and compression. The (-set_grib_type) 
 * option controls the packing method. 
 * 
 * Complex packing has the option of using special values or bitmap for undefined grid values. 
 * Special values produce significantly smaller files than using the bitmap. You should only use 
 * the bitmap and complex packing when trying to remain compatible with certain codes. (Bitmaps and 
 * special values were part of the original grib2 specification, so it is possible that some codes can 
 * handle special values and not bitmaps.) If you want to alter the default (special values) to bitmaps, 
 * you can use the option -set_bitmap. 
 * 
 * AEC is an implementation of CCSDS compression using libaec. This new compression has very fast 
 * compression speeds and good compression.
 * 
 * ## Usage
 * -set_grib_type X
 * 
 * X = ieee, simple, complex1, complex2, complex3, jpeg, aec, same
 *
 * You can also use short names s, c1, c1b, c2, c2b, c3, c3b, j, a for simple, complex1, complex1-bitmap, 
 * complex2, complex2-bitmap, complex3, complex3-bitmap, jpeg, and aec, respectively.
 *
 * ieee = data is ieee format (4 bytes per data point)
 * simple = no compression, packed scaled integers
 * complex1 = complex packing
 * complex1-bitmap = complex and using bitmap for undefined grid points
 * complex2 = complex packing, pack increments (deltas)
 * complex2-bitmap = complex packing, pack increments (deltas) and using bitmap for undefined
 * complex3 = complex packing, pack increments after linear extrapolation
 * complex3-bitmap = complex packing, pack increments after linear extrapolation and using bitmap for undefined
 * jpeg = jpeg2000 compression
 * aec = aec/CCSDS compression
 * same = try to keep same packing type as input
 *
 * If input is in an unsupported output packing, complex1 is used.
 * 
 * @note To use bitmap for undefined values in complex packing, you can use -set_bitmap 1 or you can use 
 * the complexN-bitmap.
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
 * @author Wesley Ebisuzaki @date 12/2007
 */
int f_set_grib_type(ARG1) {
    int pack;
    if (strcmp(arg1,"simple") == 0 || strcmp(arg1,"s") == 0) grib_type = simple;
#if G2_JPEG2000_ENABLED == 1
    else if(strcmp(arg1,"jpeg") == 0 || strcmp(arg1,"j") == 0) grib_type = jpeg;
#endif
    else if (strcmp(arg1,"ieee") == 0 || strcmp(arg1,"i") == 0) grib_type = ieee_packing;
    else if (strcmp(arg1,"complex1") == 0 || strcmp(arg1,"c1") == 0) grib_type = complex1;
    else if (strcmp(arg1,"complex2") == 0 || strcmp(arg1,"c2") == 0) grib_type = complex2;
    else if (strcmp(arg1,"complex3") == 0 || strcmp(arg1,"c3") == 0) grib_type = complex3;

    else if (strcmp(arg1,"complex1-bitmap") == 0 || strcmp(arg1,"c1b") == 0) {
        grib_type = complex1;
        use_bitmap = 1;
    }
    else if (strcmp(arg1,"complex2-bitmap") == 0 || strcmp(arg1,"c2b") == 0) {
        grib_type = complex2;
        use_bitmap = 1;
    }
    else if (strcmp(arg1,"complex3-bitmap") == 0 || strcmp(arg1,"c3b") == 0) {
        grib_type = complex3;
        use_bitmap = 1;
    }

#if G2_AEC_ENABLED == 1
    else if (strcmp(arg1,"aec") == 0 || strcmp(arg1,"a") == 0) grib_type = aec;
#endif
    else if (strcmp(arg1,"same") == 0) {
        if (mode >= 0) {
            pack = code_table_5_0(sec);
            if (pack == 0) grib_type = simple;
            else if (pack == 2) grib_type = complex1;
            else if (pack == 3) {
                if (code_table_5_6(sec) == 1) grib_type = complex2;
                else grib_type = complex3;
            }
            else if (pack == 4) grib_type = ieee_packing;
#if G2_JPEG2000_ENABLED == 1
            else if (pack == 40) grib_type = jpeg;
#endif
#if G2_AEC_ENABLED == 1
            else if (pack == 42) grib_type = aec;
#endif
            // cannot duplicate output grib type
            else grib_type = complex1;
        }
        else grib_type = complex1;
    }
    else fatal_error("set_grib_type: bad type %s", arg1);
    return 0;
}

/*
 * HEADER:100:set_bitmap:misc:1:use bitmap when creating complex packed files X=1/0
 */

/**
 * Use bitmap when creating complex packed files.
 * 
 * Complex packed grib files can have undefined grid values stored in the grib files as either a 
 * bitmap or a special value. You should use the special value method because it results in 
 * significantly smaller files. If software is a concern, you should use whatever is compatible 
 * with the decoding software. Note: only complex packing allows for using a special value for 
 * undefined grid values. 
 * 
 * Wgrib2's will store undefined grib values using a special value when using complex packing unless 
 * the -set_bitmap 1 option is used. The option -set_grib_type same will not enable the bitmap option. 
 * 
 * ## Software Concerns
 * NDFD (National Digital Forecast Database) which is produced by NOAA has used complex packing with 
 * a special value since the beginning (many years ago). Consequently many decoders will be able to 
 * handle special-value undefineds. Many codes will handle bitmaps because bitmaps were used in 
 * jpeg2000 packed grib2 files which have been used by NCEP for many years. At least one organization 
 * has codes that will not read special-value undefineds. 
 * 
 * ## Usage
 * -set_bitmap X   X=0, 1
 * 
 * 0 - do not use bitmap  (default)
 * 1 - use bitmap when packing undefined values with complex packing
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * wgrib2 in.grb -set_bitmap 1 -set_grib_type complex3 -grib_out out.grb 
 * @endcode{}
 * 
 * The above line rewrites a file using complex3 packing and a bitmap. A bitmap is not used if 
 * all the grid values are defined. 
 * 
 * @author Wesley Ebisuzaki @date 12/2007
 */
int f_set_bitmap(ARG1) {
    if (mode >= -1) {
        use_bitmap = atoi(arg1);
    }
    return 0;
}
 
/*
 * HEADER:100:grib_out:output:1:writes decoded/modified data in grib-2 format to file X
 */

/**
 * Writes decoded/modified data in grib2 format to a specified file.
 * 
 *  Normally you would use the -grib option as this option just copies the original grib 
 * (sub)message. The -grib_out option has to repack or compress the decoded grid which is much 
 * slower. You would only use the -grib_out option when you have modified the decoded grid by 
 * the -undefine, -import, -rpn, or some other option that modifies the decoded grid. Use the 
 * -set_grib_type option to specify the grib packing and -set_scaling to specify the scaling. 
 * When the -set_grib_type option is not used, the packing defaults to simple. When the 
 * -set_scaling option is not used, the scaling is retained from the input grib message unless 
 * a -rpn or -import option is executed. These two options reset the scaling to the default 
 * because they can alter the range of grid point values. 
 * 
 * ## Usage
 * -grib_out file_name
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
 * @author Wesley Ebisuzaki @date 12/2007
 */
int f_grib_out(ARG1) {

    float *data_tmp;
    struct seq_file *save;
    unsigned int new_nx, new_ny, new_npnts, i;
    int new_res, new_scan;

    if (mode == -1) {
        save_translation = decode = 1;

        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("memory allocation grib_out","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = *local;

        /* gds may have been modified, check for new npnts */
        get_nxny_(sec, &new_nx, &new_ny, &new_npnts, &new_res, &new_scan);
        if (ndata != new_npnts) 
            fprintf(stderr,"grib_out: gds modified, data size %u -> %u, data = undefined\n",
                    ndata, new_npnts);

        if ((data_tmp = (float *) malloc(((size_t) new_npnts) * sizeof(float))) == NULL)
            fatal_error("grib_out: memory allocation","");

        if (ndata == new_npnts) undo_output_order(data, data_tmp, ndata);
        else {
            for (i = 0; i < new_npnts; i++) data_tmp[i] = UNDEFINED;
        }

        grib_wrt(sec, data_tmp, new_npnts, new_nx, new_ny, use_scale, dec_scale, 
                bin_scale, wanted_bits, max_bits, grib_type, save);
        if (flush_mode) fflush_file(save);
        free(data_tmp);
    }
    return 0;
}

/*
 * grib_wrt - writes out grib file
 *
 * under some conditions, data will be changed
 */

/**
 * Write out GRIB file. Under some conditions, data will be changed.
 *
 * @param sec Pointer to grib section
 * @param data Pointer to data array
 * @param ndata Number of data points
 * @param nx Number of points in x direction
 * @param ny Number of points in y direction
 * @param use_scale Use scaling flag
 * @param dec_scale Decimal scaling
 * @param bin_scale Binary scaling
 * @param wanted_bits Number of bits wanted
 * @param max_bits Maximum number of bits
 * @param grib_type Current output grib type
 * @param out Pointer to seq_file struct
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 12/2007
 */
int grib_wrt(unsigned char **sec, float *data, unsigned int ndata, unsigned int nx, unsigned int ny, int use_scale, int dec_scale, 
            int bin_scale, int wanted_bits, int max_bits, enum output_grib_type grib_type, struct seq_file *out) {

    if (grib_type == simple) simple_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, max_bits, out); 
    else if (grib_type == ieee_packing) ieee_grib_out(sec, data, ndata, out);
#if G2_JPEG2000_ENABLED == 1
    else if (grib_type == jpeg) jpeg2000_grib_out(sec, data, ndata, nx, ny, use_scale, dec_scale, bin_scale, wanted_bits, max_bits, out);
#endif
    else if (grib_type == complex1) complex_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, 
        max_bits, 1, use_bitmap, out); 
    else if (grib_type == complex2) complex_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, 
        max_bits, 2, use_bitmap, out); 
    else if (grib_type == complex3) complex_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, 
        max_bits, 3, use_bitmap, out); 
#if G2_AEC_ENABLED == 1
    else if (grib_type == aec) aec_grib_out(sec, data, ndata, use_scale, dec_scale, bin_scale, wanted_bits, max_bits, out);
#endif
    return 0;
}

/*
 * HEADER:100:set_grib_max_bits:misc:1:sets scaling so number of bits does not exceed N in (new) grib output
 */

/**
 * Sets the maximum number of bits that the scaled integers can have when encoding data into 
 * grib.
 * 
 * With most grib packing methods, the grid values are usually stored as scaled integers. The 
 * grib format allows the scaled integers to have up to 254 bits (simple packing). This is an 
 * unreasonably high precision as wgrib2 converts the data to a single-precision floating point 
 * number which typically has only 25 bits of precision. To speed up the encoding and decoding 
 * processing, wgrib2 uses 32 bit integers which limits the maximum size of scaled integers to 
 * 25 after accounting for the limits of the packing and unpacking routines. 
 * 
 * The complex-2 and complex-3 compression schemes also store scaled integers but different. The 
 * complex-1 stores the scaled intger values, so it is limited to a 25 bit integers. Hewever, 
 * complex-2 stores the delta (the difference from previously defined grid point). The magnitude 
 * of delta is limited by the original series but the delta can be either positive or negative. 
 * So under worst case, the deltas need one more bit to store the scaled integers. So if you scale 
 * the grid point values to 24 bits, you will have a scaled deltas being 25 bits. The complex-3 
 * packing stores the deltas of the delta series. This is good for smooth fields, but for the worst 
 * case could cause two extra bits to store the original grid data. So at worst case, the data would 
 * need to be scaled at 23 bits. 
 * 
 * The extra bits needed for complex-2 and complex-3 are very rare. It was first encountered in 
 * 11/2024 which is about a decade after the complex packing was introduced. In 11/2024, code was added 
 * to the complex packing to detect the increase in bits needed and reduce the precision if it cause 
 * the precision to be greater than 25 bits. 
 * 
 * Grib fields are usually stored as a scaled integers, and usually you don't need 25 bits of 
 * precision. For example, temperature to the nearest 0.1 degree, means that the a temperature 
 * range of 50 C can be stored as 500 descrete values and only 7 bits are needed to store the 
 * temperature to the nearest 0.1 degree. 
 * 
 * The problem is that people can set the scaling factors to inappropriate values, using 25 bits 
 * of precision will was (disk) space. Consequently wgrib2 has a resetable limit of the precision 
 * of the scaled integers allowed. If you exceed the limit, the scaling factors are change so that 
 * the scaled integers are limited. 
 * 
 * The -set_grib_max_bits option sets the maximum number of bits that the scaled integers can have 
 * when encoding data into grib. The value should never be greater than 25 as that is the limit for 
 * wgrib2 encoding. However, some grib packages may not support 25 bit precision in decoding, so you 
 * may want to limit the precision to 24 to remain compatible with other software packages. 
 * 
 * The -set_grib_max_bits option does not set the binary precision of the grib output but sets the 
 * maximum possible precision of the numbers. 
 * 
 * ## Usage
 * -set_grib_max_bits N
 *
 * N = maximum number of bits used to encode data (1 <= N <= 25)
 * 
 * The default value is N = 16.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @note The -set_grib_max_bits option does not affect grib data written using the -grib option because 
 * the -grib option does not encode data but copies the input grib message. 
 * 
 * @author Wesley Ebisuzaki @date 12/2007
 */
int f_set_grib_max_bits(ARG1) {
    int i;

    i = atoi(arg1);
    if (mode >= -1) {
        if (i < 0) fatal_error_i("set_grib_max_bits: %d is less than zero", i);
        if (i > 25) fatal_error_i("set_grib_max_bits: %d > 25", i);
    }
    if (mode >= -1) max_bits = i;
    return 0;
}

/*
 * HEADER:100:grib_max_bits:inv:0:maximum bits used in grib encoding
 */

/**
 * Prints the maximum bits used in grib encoding.
 * 
 * When wgrib2 encodes a grib message (creates a new grib message), the grid values are usually 
 * stored as scaled integers. The number of bits required to store the scaled integers will 
 * depend on the precision/scaling factors. If these values are poorly set, the scaled integers 
 * could be 100 bits long. That would be very inconvenient for machines with 32-bit registers. 
 * To prevent this problem, you have to limit the size of the scaled integers. The wgrib2 default 
 * is 16 bits, and can be increased up to 25 bits by the -set_grib_max_bits option. Since the IEEE 
 * single precision floating point format only has 25 bits of precision, there is little need to 
 * support longer scaled integers at this point in time (9/2017). The -grib_max_bits option displays 
 * the current value of the maximum binary precision. 
 * 
 * Grib decoders usually have a limit to the size of the scaled integers used to store grid values. 
 * Wgrib2 has a limit of 25 bits which is determined by the minimum size of the integer (32 bits) 
 * and the algorithm used to convert between a bitstring and integer. I don't know the limits for 
 * other software packages. 
 * 
 * ## Usage
 * -grib_max_bits
 * 
 * Result is an integer between 1 and 25.
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 12/2007
 */
int f_grib_max_bits(ARG0) {
    if (mode >= 0) {
        sprintf(inv_out,"grib_max_bits=%d", max_bits);
    }
    return 0;
}

/**
 * Make bitmap section.
 * 
 * Based on mk_BMS (from gribw)
 *
 * @param data Pointer to data array. Will be modified (undefined values removed).
 * @param ndata Pointer to number of data points.
 * 
 * @return Pointer to bitmap section.
 *
 * @author Wesley Ebisuzaki @date 12/2007
 */
unsigned char *mk_bms(float *data, unsigned int *ndata) {

    int bms_size;
    unsigned char *bms, *cbits;
    unsigned int nn, i, start, c, imask, i0;

    nn = *ndata;

    /* find first grid point with undefined data */
    for (i = 0; i < nn; i++) {
        if (UNDEFINED_VAL(data[i])) break;
    }

    if (i == nn) {				/* all defined values, no need for bms */
        bms = (unsigned char *) malloc(6);
        if (bms == NULL) fatal_error("mk_bms: memory allocation problem","");
        uint_char(6, bms);		// length of section 6
        bms[4] = 6;			// section 6
        bms[5] = 255;			// no bitmap
        return bms;
    }

    /* bms_size = 6 + (nn+7) / 8; */
    /* nn+7 can overflow */
    bms_size = 6 + nn/8 + ((nn%8) != 0);

    bms = (unsigned char *) malloc(bms_size);
    if (bms == NULL) fatal_error("mk_bms: memory allocation problem","");

    uint_char(bms_size, bms);		// length of section 6
    bms[4] = 6;				// section 6
    bms[5] = 0;				// has bitmap

    /* bitmap is accessed by bytes, make i0=i/8 bytes of bitmap */
    cbits = bms + 6;
    i0 = i >> 3;
    for (i = 0; i < i0; i++) {
        *cbits++ = 255;
    }

    /* start processing data, skip i0*8 */

    c = 0;
    imask = 128;
    i0 = i0 << 3;
    start = i0;
    for (i = i0; i < nn; i++) {
        if (DEFINED_VAL(data[i])) {
            c += imask;
            data[start++] = data[i];
        }
        if ((imask >>= 1) == 0) {
            *cbits++ = c;
            c = 0;
            imask = 128;
        }
    }
    if (imask != 128) *cbits = c;
    *ndata = start;
    return bms;
}


/*
 * HEADER:100:set_bin_prec:misc:1:X use X bits and ECMWF-style grib encoding
 */

/**
 * Set wgrib2 to encode data using the ECMWF convention.
 *
 * The values at the grid points points are stored in a general format,
 * 
 * Y = (R + i*2**B)*(10**D)
 * 
 * R = reference value (32-bit IEEE floating point number)
 * i = integer, 0..2**N-1
 * N = binary bit precision
 * B = binary scaling, -127..127
 * D = decimal scaling, -127..127
 *
 * There are 3 systems for storing the number which I have called:
 * 
 * ECMWF convention: 
 * Y = R + i*2**B
 * 
 * R = reference value
 * i = integer, 0..2**N-1
 * N = binary bit precision, a parameter
 * B = binary scaling, determined by grib routines
 * D = 0
 * 
 * NCEP convention: 
 * 
 * R = reference value
 * i = integer, 0..2**N-1
 * N = binary bit precision, determined by grib routines
 * B = binary scaling, a parameter
 * D = decimal scaling, a parameter
 * 
 * Note, global model uses a variant: B = 0, D = parameter
 * 
 * Both the ECMWF and NCEP conventions have their advantages and disadvantages. The ECMWF method 
 * is easier to use, you just set the binary precision to N bits (12? 16?) for all variables and 
 * you are done. With the NCEP convention, you have to set the scaling for each variable separately. 
 * For some variables such as specific humidity, the scaling should be pressure dependent. On the 
 * other hand, if you are trying to get the smallest files, the NCEP convention is better. For 
 * example, you want to get the RH to the nearest integer. With the NCEP method, you simply set 
 * D = B = 0. For general use, I suggest that you use the ECMWF convention because people time is 
 * usually more valuable than disk space. Ok, I value my time more than a few GB. On the othe hand, 
 * I've been involved with more than my share of projects were disk space has been the critical issue. 
 * 
 * The wgrib2 default for encoding grib is 12-bits using the ECMWF convention. 
 * 
 * ## Usage
 * -set_bin_prec N
 * 
 * N = number of bits to encode grid point data (N <= 25)
 * 
 * If N is > 12, you need to increase the maximum bits of allowed precision by -set_grib_max_bits.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 12/2007
 */
int f_set_bin_prec(ARG1) {
    int i;

    if (mode >= -1) {
        i = atoi(arg1);
        if (i < 0) i = 0;
        if (i > max_bits) i = max_bits;
        wanted_bits = i;
        use_scale = 0;
    }
    return 0;
}

/*
 * HEADER:100:precision:inv:0:precision of packing
 */

/**
 * Print the precision of packing.
 * 
 * ## Precision
 * Except for grid values stored in IEEE format, grid values are stored with user-defined 
 * precision. By having a variable precision, the space required can be reduced. For having 
 * the temperature stored to a thousandth of a degree would be wasteful as thermometers typically 
 * give a result to a tenth of a degree. 
 * 
 * Grid point values are stored as scaled integers, the scaling determines the precision. 
 * 
 * val = (base+i*2^bin_scaling)*10^dec_scaling
 * 
 * base = real*4
 * bin_scaling = integer from -127 .. 127
 * dec_scaling = integer from -127 .. 127
 * 
 * The -precision option prints the bin_scaling and dec_scaling values. The -precision output 
 * can be read by -set_metadata and -set_metadata_str options to set the precision. 
 * 
 * ## Usage
 * -precision
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 12/2007
 */
int f_precision(ARG0) {
    if (mode >= 0) {
        if (use_scale == 0) sprintf(inv_out, "encode %d bits", wanted_bits);
        else sprintf(inv_out,"encode i*2^%d*10^%d", bin_scale, dec_scale);
    }
    return 0;
}

/*
 * HEADER:100:set_scaling:misc:2:set decimal scaling=X/same binary scaling=Y/same  new grib messages
 *
 * if arg1/arg2 == "same" then use the same value as input or -set_scaling (if after input)
 */

/**
 * Set the binary and decimal scaling parameters for the next write. (The binary and decimal 
 * scaling parameters will be reset by reads and calls to RPN.) This will set wgrib2 to to 
 * encode data using the NCEP convention. If you want to encode data using the ECMWF convention, 
 * you need to use the -set_bin_prec option perhaps with the -set_grib_max_bits option. 
 * 
 * The values at the grid points points are stored in a general format,
 * 
 * Y = (R + i*2**B)*(10**D)
 * 
 * R = reference value (32-bit IEEE floating point number)
 * i = integer, 0..2**N-1
 * N = binary bit precision
 * B = binary scaling, -127..127
 * D = decimal scaling, -127..127
 *
 * There are 3 systems for storing the number which I have called:
 * 
 * ECMWF convention: 
 * Y = R + i*2**B
 * 
 * R = reference value
 * i = integer, 0..2**N-1
 * N = binary bit precision, a parameter
 * B = binary scaling, determined by grib routines
 * D = 0
 * 
 * NCEP convention: 
 * 
 * R = reference value
 * i = integer, 0..2**N-1
 * N = binary bit precision, determined by grib routines
 * B = binary scaling, a parameter
 * D = decimal scaling, a parameter
 * 
 * Note, global model uses a variant: B = 0, D = parameter
 * 
 * Both the ECMWF and NCEP conventions have their advantages and disadvantages. The ECMWF method 
 * is easier to use, you just set the binary precision to N bits (12? 16?) for all variables and 
 * you are done. With the NCEP convention, you have to set the scaling for each variable separately. 
 * For some variables such as specific humidity, the scaling should be pressure dependent. On the 
 * other hand, if you are trying to get the smallest files, the NCEP convention is better. For 
 * example, you want to get the RH to the nearest integer. With the NCEP method, you simply set 
 * D = B = 0. For general use, I suggest that you use the ECMWF convention because people time is 
 * usually more valuable than disk space. Ok, I value my time more than a few GB. On the othe hand, 
 * I've been involved with more than my share of projects were disk space has been the critical issue. 
 * 
 * By default, wgrib2 will encode using the ECMWF convention using 12 bits. The number of bits can be 
 * changed by the -set_bin_prec option. The -set_grib_max_bits option will have to be used if the binary 
 * precision is set to more than 16. 
 * 
 * ## Default Scaling
 * When you read a field, the scaling of the field (B, D) are saved as the scaling parameter. However, 
 * some options such as the -rpn option can change the magnitude of the field and scaling from the input 
 * field may not longer be appropriate. So these options will revert to the default scaling (ECMWF-style 
 * using the N bits). However, in some cases such as time-interpolation or smoothing, the original scaling 
 * is appropriate. In this case you can set the B and D scaling to text string, same. 
 * 
 * ## Usage
 * -set_scaling D B
 * 
 * D = decimal scaling or the text 'same' with no quotes
 * B = binary scaling or the text 'same' with no quotes
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 12/2017
 */
int f_set_scaling(ARG2) {

    struct local_struct {
        int dec, bin;
        int use_previous_dec, use_previous_bin;
    };
    struct local_struct *save;

    if (mode == -1) {
        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("set_scaling: memory allocation ","");

        save->use_previous_dec = (strcmp(arg1,"same") == 0);
        save->use_previous_bin = (strcmp(arg2,"same") == 0);
        save->dec = atoi(arg1);
        save->bin = atoi(arg2);
        use_scale = 1;
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
        /* set scaling on,  use_scale = 1
           either keep old values or reset to new values
         */
        use_scale = 1;
       	if (save->use_previous_dec == 0) dec_scale = save->dec;
       	if (save->use_previous_bin == 0) bin_scale = save->bin;
        /* if previous values are bad then  input_scale = 0
           if previous values are used then don't use scaling
         */
        if (input_scale == 0 && (save->use_previous_dec || save->use_previous_bin)) use_scale = 0;

    }
    else if (mode == -2) {
        save = (struct local_struct *) *local;
        free(save);
    }
    return 0;
}


/*
 * grib: convert linear list of ints to a bitstream
 *
 * in public domain : 2007 Wesley Ebisuzaki
 */

/** Bitmask for extracting bits from integers */
static unsigned int mask[] = {0,1,3,7,15,31,63,127,255};


/**
 * Convert a list of floats to a bitstream.
 *
 * @param list Pointer to list of floats
 * @param bitstream Pointer to output bitstream
 * @param ndata Number of data points
 * @param nbits Number of bits per data point
 *
 * @author Wesley Ebisuzaki @date 2007
 */
void flist2bitstream(float *list, unsigned char *bitstream, unsigned int ndata, int nbits) 
{

    int cbits, jbits;
    unsigned int j, c;

    if (nbits == 0) {
        return;
    }
    if (nbits < 0) fatal_error_i( "flist2bitstream nbits < 0!  nbits = %d", nbits);

    cbits = 8;
    c = 0;
    while (ndata-- > 0) {
        /* note float -> unsigned int .. truncate */
        j = (unsigned int) (*list++ + 0.5);
        jbits = nbits;
        while (cbits <= jbits) {
            if (cbits == 8) {
                jbits -= 8;
                *bitstream++ = (j >> jbits) & 255;
            }
            else {
                jbits -= cbits;
                *bitstream++ = (c << cbits) + ((j >> jbits) & mask[cbits]);
                cbits = 8;
                c = 0;
            }
        }
        /* now jbits < cbits */
        if (jbits) {
            c = (c << jbits) + (j & mask[jbits]);
            cbits -= jbits;
        }
    }
    if (cbits != 8) *bitstream++ = c << cbits;
}

/**
 * Set the output order for the GRIB message.
 *
 * @param sec Pointer to the GRIB message sections
 * @param order Output order type
 *
 * @return 0 for success, 1 for failure
 *
 * @author Wesley Ebisuzaki @date 12/2017
 */
int set_order(unsigned char **sec, enum output_order_type order) {
    int flag_3_4;

    if (order == raw) return 0;
    flag_3_4 = flag_table_3_4(sec) & 15;
    if (order == wesn) return set_flag_table_3_4(sec, flag_3_4 | (4 << 4));
    if (order == wens) return set_flag_table_3_4(sec, flag_3_4);
    return 1;
}

/*
 * write sections as a grib message
 */

/**
 * Write sections as a GRIB message to a file.
 *
 * @param sec0 Pointer to section 0
 * @param sec1 Pointer to section 1
 * @param sec2 Pointer to section 2
 * @param sec3 Pointer to section 3
 * @param sec4 Pointer to section 4
 * @param sec5 Pointer to section 5
 * @param sec6 Pointer to section 6
 * @param sec7 Pointer to section 7
 * @param file Pointer to the seq_file struct
 *
 * @return 0 for success, 1 for failure
 *
 * @author Wesley Ebisuzaki @date 12/2017
 */
int wrt_sec(unsigned const char *sec0, unsigned const char *sec1, unsigned const char *sec2, 
        unsigned const char *sec3, unsigned const char *sec4, unsigned const char *sec5, 
        unsigned const char *sec6, unsigned const char *sec7, struct seq_file *file) {

    size_t size;
    unsigned char s[16];
    int i;
    unsigned int s1, s2, s3, s4, s5, s6, s7, s8;
   
    s1 = (sec1 ? uint4(sec1) : 0);
    s2 = (sec2 ? uint4(sec2) : 0);
    s3 = (sec3 ? uint4(sec3) : 0);
    s4 = (sec4 ? uint4(sec4) : 0);
    s5 = (sec5 ? uint4(sec5) : 0);
    s6 = (sec6 ? uint4(sec6) : 0);
    s7 = (sec7 ? uint4(sec7) : 0);
    s8 = GB2_Sec8_size;

    size = (size_t) GB2_Sec0_size + s1 + s2 + s3 + s4 + s5 + 
        (size_t) s6 + (size_t) s7 + s8;

    for (i = 0; i < 8; i++) s[i] = sec0[i];
    uint8_char(size, s+8);
    fwrite_file((void *) s, sizeof(char), 16, file);

    if (sec1) {
        if (fwrite_file((void *)sec1, sizeof(char), s1, file) != s1) return 1;
    }
    if (sec2) {
        if (fwrite_file((void *)sec2, sizeof(char), s2, file) != s2) return 1;
    }
    if (sec3) {
        if (fwrite_file((void *)sec3, sizeof(char), s3, file) != s3) return 1;
    }
    if (sec4) {
        if (fwrite_file((void *)sec4, sizeof(char), s4, file) != s4) return 1;
    }
    if (sec5) {
        if (fwrite_file((void *)sec5, sizeof(char), s5, file) != s5) return 1;
    }
    if (sec6) {
        if (fwrite_file((void *)sec6, sizeof(char), s6, file) != s6) return 1;
    }
    if (sec7) {
        if (fwrite_file((void *)sec7, sizeof(char), s7, file) != s7) return 1;
    }
    s[0] = s[1] = s[2] = s[3] = 55; /* s = "7777" */
    if (fwrite_file((void *) s, sizeof(char), 4, file) != 4) return 1;
    return 0;
}
