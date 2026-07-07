/** @file
 * @brief Initialize the global variables.
 * 
 * Changed from original:
 * int variable = value;
 * 
 * To:
 * init() {
 *   variable = value;
 * }
 * 
 * Needed to make wgrib2 a callable routine.
 * @author Public Domain: John Howard, Wesley Ebisuzaki @date 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grb2.h"  /* for center codes */
#include "wgrib2.h"

#ifdef USE_G2CLIB
#include <grib2.h>
extern gribfield *grib_data;    /**< Pointer to GRIB data. */
extern int free_gribfield;      /**< Flag for allocated gribfield. */
#endif

/* global variables .. can be modified by funtions */

extern int mode;            /**< Mode of operation. -2=finalize, -1=initialize,  0 .. N is verbosity mode  */
extern int header;    		/**< File header flag. */
extern int flush_mode;		/**< Flush of output flag. */
extern int WxText;		    /**< Decode NDFD keys. */
extern int ftime_mode;      /**< Ftime control. */

extern int use_g2clib;		                /**< Use g2clib/emulation code for decoding. */
extern int use_gctpc;		                /**< Use gctpc for geolocation. */
extern int use_proj4;		                /**< Use Proj4 for geolocation. */
extern enum geolocation_type geolocation;   /**< Geolocation type. */


extern int fix_ncep_2_flag;	    /**< Fix NCEP 2 flag. */
extern int fix_ncep_3_flag;	    /**< Fix NCEP 3 flag. */
extern int fix_ncep_4_flag;	    /**< Fix NCEP 4 flag. */
extern int fix_undef_flag;	    /**< Fix undefined flag. */

extern int for_mode;            /**< For loop mode flag. */
extern int for_start;           /**< For loop start value. */
extern int for_end;             /**< For loop end value. */
extern int for_step;            /**< For loop step value. */
extern int for_n_mode;          /**< For loop n mode flag. */
extern int for_n_start;         /**< For loop n start value. */
extern int for_n_end;           /**< For loop n end value. */
extern int for_n_step;          /**< For loop n step value. */

extern int match;               /**< Run matching routines flag. */
extern int match_fs;            /**< Run fixed-string matching routines flag. */
extern int run_flag;            /**< Run flag. */

extern int fgrep;               /**< Flag to use fgrep. */
extern int fgrep_flag;          /**< Flag to use fgrep (not used). */
extern int fgrep_count;         /**< Number of fgrep options. */
#ifdef USE_REGEX
extern int egrep;               /**< Flag to use egrep. */
extern int egrep_flag;          /**< Flag to use egrep (not used). */
extern int egrep_count;         /**< Number of egrep options. */
#endif

extern unsigned int last_message;	/**< Last message to process if set. */

extern struct seq_file inv_file;    /**< Sequence file for inventory. */

extern enum input_type input;                           /**< Input type. */
extern enum output_order_type output_order;             /**< Output order type. */
extern enum output_order_type  output_order_wanted;     /**< Desired output order type. */

extern char inv_out[INV_BUFFER]; 		/**< Buffer for inv functions. */

extern int only_submsg;    /**< If only_submsg > 0 .. only process submsg number (only_submsg) */

// extern int io_buffer_cnt;                  /* rd_grib_msg.c */
// extern long int pos_input;


/* output file variables */

extern int file_append;             /**< Append grib file flag. */
extern int dump_msg;                /**< Dump message flag. */
extern int dump_submsg;             /**< Dump submessage flag. */
extern size_t dump_offset;          /**< Dump offset. */
extern int ieee_little_endian;      /**< IEEE little endian flag. */

extern int decode;		/**< Decode grib file flag */

extern const char *item_deliminator;    /**< Item deliminator. */
extern const char *nl;                    /**< New line character string. */
extern const char *end_inv;               /**< String to write after end of inventory. */

/* current grib location */
extern long int pos;                   /**< Current position in grib file. */
extern unsigned long int len;          /**< Length of current grib message. */
extern int submsg;                     /**< Current submessage number. */
extern int msg_no;                     /**< Current message number. */
extern int inv_no;                     /**< Current inventory number. */

/* lat lon array .. used by Latlon.c to store location information */
extern int latlon;                              /**< Flag to indicate lat-lon grid processing. */
extern int warn_nonzero_min_sec;                /**< Warning for non-zero min/sec. */
extern int GDS_change_no;                       /**< GDS change number. */
extern int old_GDS_size;                        /**< Old GDS size. */
extern int GDS_max_size;                        /**< Maximum GDS size. */
extern unsigned char *old_gds;                  /**< Pointer to old GDS. */
extern int nx;                                  /**< Number of grid points in x direction. */
extern int ny;                                  /**< Number of grid points in y direction. */
extern int res;                                 /**< Grid resolution. */
extern int scan;                                /**< Scan order mode. */
extern unsigned int nx_;                        /**< Number of grid points in x direction (unsigned). */
extern unsigned int ny_;                        /**< Number of grid points in y direction (unsigned). */
extern unsigned int npnts;                      /**< Number of grid points. */
extern int use_scale;                           /**< Use scale flag. */
extern int input_scale;                         /**< Input scale factor. */
extern int dec_scale;                           /**< Decimal scale factor. */
extern int bin_scale;                           /**< Binary scale factor. */
extern int max_bits;                            /**< Maximum number of bits. */
extern int wanted_bits;                         /**< Desired number of bits. */
extern enum output_grib_type grib_type;         /**< Output GRIB type. */
extern int use_bitmap;                          /**< Use bitmap flag. */
extern enum wind_rotation_type wind_rotation;   /**< Wind rotation type. */

extern const char **vectors;                        /**< Vector fields. */
extern const char *default_vectors[];               /**< Default vector fields. */
extern enum new_grid_format_type new_grid_format;   /**< New grid format type. */

/* Match.c */
#ifdef USE_REGEX
extern int match_count;     /**< Number of regular expressions to match. */
extern int regex_type;      /**< Regex type. 0 - extended POSIX, 1 - patterns, 2 - extended regex & quote metacharacters */
#endif
extern int match_extra_fn_n;    /**< Number of inv functions added to match_inv. */
/* Match_fs.c */
extern int match_count_fs;       /**< Number of fixed strings to match. */


extern unsigned int type_ext_name;	    /**< Extended name type. */
extern int WxNum;			            /**< Number of keys. */
extern char *WxTable;                   /**< NDFD weather information table. */
extern char **WxKeys;                   /**< NDFD weather information keys. */

extern const char *text_format;		/**< Pointer to text format. */
extern int text_column;			    /**< Number of columns in text output. */

extern int tigge;			/**< Use TIGGE flag. */
extern int names;			/**< Name convention to use. */

// extern long int pos_input;		/* seq read of grib */


extern FILE *err_file;			    /**< Pointer to error file. */
extern FILE *err_str_file;		    /**< Pointer to error string file. */
extern char err_str[STRING_SIZE];	/**< Error string buffer to write to err_str_file. */
extern int err_int;			        /**< Error integer to write to err_file. */

extern int save_translation;		/**< Flag to indicate whether to save translation information. */

extern char *match_extra_fn_arg1[MATCH_EXTRA_FN];   /**< Arg1 for match_inv. */
extern char *match_extra_fn_arg2[MATCH_EXTRA_FN];   /**< Arg2 for match_inv. */

extern char ndates_fmt[NAMELEN];    /**< Format for ndates option. */

extern int check_pdt_size_flag;     /**< Flag to indicate whether to check the size of the Product Definition Template. */
extern int warn_check_pdt;          /**< Flag to indicate whether to issue a warning for PDT size checks. */

extern char ext_name_field;         /**< Extended name field. */
extern char ext_name_space;        /**< Extended name space. */

#if defined USE_NETCDF
extern int nc4;         /**< NetCDF version 4 flag. */
#endif

/**
 * Initialize global variables.
 * 
 * @author John Howard, Wesley Ebisuzaki @date 2014
 */
void init_globals(void) {
    int i;

#ifdef USE_G2CLIB
    free_gribfield = 0;			// flag for allocated gribfield
#endif

    mode=0;             /* -2=finalize, -1=initialize,  0 .. N is verbosity mode */
    header=1;           /* file header flag */
    flush_mode = 0;	/* flush of output 1 = yes */
    WxText = 0;		/* decode NDFD keys */
    ftime_mode = 0;	

    use_g2clib = DEFAULT_G2CLIB;        /* use g2clib/emulation code for decoding */
    use_gctpc = DEFAULT_GCTPC;          /* use gctpc for geolocation */
    use_proj4 = DEFAULT_PROJ4;          /* use Proj4 for geolocation */
    geolocation = not_used;

    fix_ncep_2_flag = 0;
    fix_ncep_3_flag = 0;
    fix_ncep_4_flag = 0;
    fix_undef_flag = 0;

    for_mode = 0;
    for_n_mode = 0;

    match = match_fs = 0;

    fgrep = fgrep_flag = fgrep_count = 0;
#ifdef USE_REGEX
    egrep = egrep_flag = egrep_count = 0;
#endif
    match_extra_fn_n = 0;

    last_message = 0;   /* last message to process if set */
    /* set inv_file to stdout */
    if (fopen_file(&inv_file,"-","w") != 0) fatal_error("Could not set inv_file to stdout","");

    input = all_mode;
    output_order = wesn;
    output_order_wanted = wesn;

    only_submsg = 0;    /* if only_submsg > 0 .. only process submsg number (only_submsg) */

    /* output file variables */

    file_append = 0;
    dump_msg = 0, dump_submsg = 0;
    dump_offset = 0;
    ieee_little_endian = 0;
    check_pdt_size_flag = 1;
    warn_check_pdt = 1;

    decode = 0;         /* decode grib file flag */

    item_deliminator = ":";
    nl = "\n\t";
    end_inv = "\n";     /* string to write after end of inventory */

    /* lat lon array .. used by Latlon.c to store location information */
    latlon = 0;
    warn_nonzero_min_sec = 1;
    GDS_change_no = 0;
    old_GDS_size = 0;
    use_scale = input_scale = 0;
    max_bits = 16;
    wanted_bits = 12;
    grib_type = simple;
    use_bitmap = 0;		/* when complex packing, do not use bitmap */

    vectors = default_vectors;	/* New_grid.c */

#ifdef USE_REGEX
    match_count = 0;		/* for Match.c */
    regex_type = 0;
#endif
    match_count_fs = 0;		/* Match_fs.c */

    type_ext_name = 0;		/* ExtName.c */
    WxTable = NULL;		/* wxtest.c */
    WxKeys = NULL;
    WxNum = 0;			/*  wxtext.c */

    text_format = "%g";		/* File.c */
    text_column = 1;

    tigge = 0;			/* Tigge.c */

    names = USE_NAMES;

    save_translation = 0;	/* Scan.c */

    init_mem_buffers();         /* mem_buffer.c */

//    io_buffer_cnt = 0;                  /* rd_grib_msg.c */
//    pos_input = 0;

    err_file = err_str_file = NULL;
    err_str[0] = err_int = 0;

    for (i = 0; i < MATCH_EXTRA_FN; i++) {
        match_extra_fn_arg1[i] = NULL;
        match_extra_fn_arg2[i] = NULL;
    }
    match_extra_fn_n = 0;
    strncpy(ndates_fmt," %s",4);

#ifdef USE_IPOLATES
    wind_rotation = undefined;
    new_grid_format = grib;
#endif

    ext_name_field = '.';
    ext_name_space = '_';

#if defined USE_NETCDF
    nc4 = 0;
#endif

    return;
}
