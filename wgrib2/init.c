/*
 * Public Domain 2014 John Howard, Wesley Ebisuzaki
 *
 *  Initialize the global variables
 *  change from original  int variable = value;
 *  to                    init() {
 *                          variable = value;
 *                        }
 * 
 * needed to make wgrib2 a callable routine
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_G2CLIB
#include "grib2.h"
gribfield *grib_data;
int free_gribfield;			// flag for allocated gribfield
#endif

/* global variables .. can be modified by funtions */

extern int mode;		/*-2=finalize, -1=initialize,  0 .. N is verbosity mode */
extern int header;    		/* file header flag */
extern int flush_mode;		/* flush of output 1 = yes */
extern int WxText;		/* decode NDFD keys */

extern int use_g2clib;		/* use g2clib/emulation code for decoding */
extern int use_gctpc;		/* use gctpc for geolocation */
extern int use_proj4;		/* use Proj4 for geolocation */

extern int fix_ncep_2_flag;	
extern int fix_ncep_3_flag;	
extern int fix_ncep_4_flag;

extern int for_mode, for_start, for_end, for_step;
extern int for_n_mode, for_n_start, for_n_end, for_n_step;

extern int match, match_fs;
extern int match_flag;

extern int fgrep, fgrep_flag, fgrep_count;
#ifdef USE_REGEX
extern int egrep, egrep_flag, egrep_count;
#endif

extern int last_message;	/* last message to process if set */

extern struct seq_file inv_file;

extern enum input_type input;
extern enum output_order_type output_order, output_order_wanted;

extern char inv_out[INV_BUFFER]; 		/* inv functions write to this buffer */

extern int only_submsg;    /* if only_submsg > 0 .. only process submsg number (only_submsg) */

// extern int io_buffer_cnt;                  /* rd_grib_msg.c */
// extern long int pos_input;


/* output file variables */

extern int file_append;
extern int dump_msg, dump_submsg;
extern int ieee_little_endian;

extern int decode;		/* decode grib file flag */

extern const char *item_deliminator;
extern const char *nl;
extern const char *end_inv;	/* string to write after end of inventory */

/* current grib location */
extern long int pos;
extern unsigned long int len;
extern int submsg, msg_no, inv_no;

/* lat lon array .. used by Latlon.c to store location information */
extern int latlon;
extern int warn_nonzero_min_sec;
extern int GDS_change_no;
extern int old_GDS_size;
extern int GDS_max_size;
extern unsigned char *old_gds;
extern int nx, ny, res, scan;
extern unsigned int nx_, ny_;
extern unsigned int npnts;
extern int use_scale, input_scale, dec_scale, bin_scale,  max_bits, wanted_bits;
extern enum output_grib_type grib_type;
extern int use_bitmap;

extern const char **vectors, *default_vectors[];   /* New_grid.c */

/* Match.c */
#ifdef USE_REGEX
extern int match_count;
extern int regex_type;
#endif
extern int match_extra_fn_n;
/* Match_fs.c */
extern int match_count_fs;


extern int use_ext_name;		/* ExtName.c */
extern int WxNum;			/* wxtext.c */
extern char *WxTable, **WxKeys;

extern const char *text_format;		/* File.c */
extern int text_column;			/* File.c */

#ifdef USE_TIGGE
extern int tigge;			/* Tigge.c */
#endif

// extern long int pos_input;		/* seq read of grib */


extern FILE *err_file;			/* EOF.c */
extern FILE *err_str_file;


extern int save_translation;		/* Misc */


extern char *match_extra_fn_arg1[MATCH_EXTRA_FN];		/* match_inv_add */
extern char *match_extra_fn_arg2[MATCH_EXTRA_FN];
extern int match_extra_fn_n;

extern char ndates_fmt[NAMELEN];

void init_globals(void) {
    int i;

#ifdef USE_G2CLIB
    free_gribfield = 0;			// flag for allocated gribfield
#endif

    mode=0;             /* -2=finalize, -1=initialize,  0 .. N is verbosity mode */
    header=1;           /* file header flag */
    flush_mode = 0;	/* flush of output 1 = yes */
    WxText = 0;		/* decode NDFD keys */

    use_g2clib = DEFAULT_G2CLIB;        /* use g2clib/emulation code for decoding */
    use_gctpc = DEFAULT_GCTPC;          /* use gctpc for geolocation */
    use_proj4 = DEFAULT_PROJ4;          /* use Proj4 for geolocation */

    fix_ncep_2_flag = 0;
    fix_ncep_3_flag = 0;
    fix_ncep_4_flag = 0;

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
    output_order = output_order_wanted = wesn;

    only_submsg = 0;    /* if only_submsg > 0 .. only process submsg number (only_submsg) */

    /* output file variables */

    file_append = 0;
    dump_msg = 0, dump_submsg = 0;
    ieee_little_endian = 0;

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

    use_ext_name = 0;		/* ExtName.c */
    WxTable = NULL;		/* wxtest.c */
    WxKeys = NULL;
    WxNum = 0;			/*  wxtext.c */

    text_format = "%g";		/* File.c */
    text_column = 1;

#ifdef USE_TIGGE
    tigge = 0;			/* Tigge.c */
#endif

    save_translation = 0;	/* Scan.c */

    init_mem_buffers();         /* mem_buffer.c */

//    io_buffer_cnt = 0;                  /* rd_grib_msg.c */
//    pos_input = 0;

    err_file = err_str_file = NULL;

    for (i = 0; i < MATCH_EXTRA_FN; i++) {
	match_extra_fn_arg1[i] = NULL;
	match_extra_fn_arg2[i] = NULL;
    }
    match_extra_fn_n = 0;
    strncpy(ndates_fmt," %s",4);
    return;
}

