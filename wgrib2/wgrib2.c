/* wgrib2 main module:  public domain 2005 w. ebisuzaki
 *
 * CHECK code is now duplicated
 *  if (decode) -- check before decoding
 *  if (!decode) -- check after processing one record so you can use wgrib2 to look at the field
 *
 * 1/2007 mods M. Schwarb: unsigned int ndata
 * 2/2008 WNE add -if support
 * 2/2008 WNE fixed bug in processing of submessages
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef CALLABLE_WGRIB2
#include <setjmp.h>
jmp_buf fatal_err;
#endif

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_G2CLIB
#include "grib2.h"
gribfield *grib_data;
int free_gribfield;			// flag for allocated gribfield
#endif

int initial_call = 1;

/* #define DEBUG */
// #define DEBUG
#define CHECK

/* global variables .. can be modified by funtions */

int mode; 		/* -2=finalize, -1=initialize,  0 .. N is verbosity mode */
int header;		/* file header flag */
int flush_mode;		/* flush of output 1 = yes */
int WxText;		/* decode NDFD keys */

int save_translation;
int use_g2clib;		/* use g2clib/emulation code for decoding */
int use_gctpc;		/* use gctpc for geolocation */
int use_proj4;		/* use Proj4 for geolocation */

int fix_ncep_2_flag;	
int fix_ncep_3_flag;	
int fix_ncep_4_flag;

int for_mode, for_start, for_end, for_step;
int for_n_mode, for_n_start, for_n_end, for_n_step;

int match, match_fs;
int match_flag;

int last_message;	/* last message to process if set */

struct seq_file inv_file;
struct seq_file rd_inventory_input;


enum input_type input;
enum output_order_type output_order, output_order_wanted;

extern char *inv_out;			/* all functions write to this buffer */

int only_submsg;    /* if only_submsg > 0 .. only process submsg number (only_submsg) */



/* output file variables */

int file_append;
int dump_msg, dump_submsg;
int ieee_little_endian;

int decode;		/* decode grib file flag */

const char *item_deliminator;
const char *nl;
const char *end_inv;	/* string to write after end of inventory */

/* current grib location */
long int pos;
unsigned long int len;
int submsg, msg_no, inv_no;

/* lat lon array .. used by Latlon.c to store location information */
double *lat = NULL, *lon = NULL;
int latlon;
int warn_nonzero_min_sec;		/* if reference time has nonzero minutes or seconds, warn if f_t(..) but not f_T(..) */
int GDS_change_no;
int old_GDS_size;
int GDS_max_size = 0;
unsigned char *old_gds;
int nx, ny, res, scan;
unsigned int nx_, ny_, npnts;
int use_scale, input_scale, dec_scale, bin_scale,  max_bits, wanted_bits;
enum output_grib_type grib_type;
int user_gribtable_enabled = 0;		/* potential user gribtable has been enabled */
int use_bitmap;		/* use bitmap when doing complex packing */

/*
 * wgrib2
 *
 * simple wgrib for GRIB2 files
 *
 */
#ifndef CALLABLE_WGRIB2

int main(int argc, char **argv) {

#else

int wgrib2(int argc, char **argv) {

#endif


//WNE    FILE *in;
    struct seq_file in_file;
    unsigned char *msg, *sec[10];	/* sec[9] = last valid bitmap */
    long int last_pos;

    int file_arg, i, j, num_submsgs;
    int n_arg;
    unsigned int k, ndata;
    static int err_4_3_count = 0;
    float *data;
    double ref;
//    double *ddata, ref;

#ifdef USE_G2CLIB
    float missing_c_val_1, missing_c_val_2;
    g2int *bitmap, has_bitmap;
    g2float *g2_data;
    int ii;
#endif

    struct ARGLIST arglist[N_ARGLIST];
    int narglist;
    const char *new_argv[N_ARGLIST];
    void *local[N_ARGLIST];
    int has_inv_option, last_submsg;
    int err, new_GDS, center;
    unsigned char dscale[2];

    init_globals();
    init_inv_out();
    ndata = 0;

    if (initial_call) {		/* only done 1st time */
	setup_user_gribtable();
//      jas_init();
//      gctpc initialiation
        init(-1,-1,"gctpc_error,txt", "gctpc_param.txt");
        initial_call = 0;
    }

    narglist = 0;
    dscale[0] = dscale[1] = 0;
    mode = 0;

    if (fopen_file(&(rd_inventory_input), "-", "r")) fatal_error("opening stdin for rd_inventory","");
    data = NULL;
//    ddata = NULL;

#ifdef CALLABLE_WGRIB2
    if (setjmp(fatal_err)) {
	fprintf(stderr,"*** arg list to wgrib2:");
	for (i=0; i < argc; i++) {
	    fprintf(stderr," %s", argv[i]);
	}
	fprintf(stderr,"\n\n");
	if (ndata && data != NULL) free(data);
	ndata=0;
        return 1;
    }
#endif

    /* no arguments .. help screen */
    if (argc == 1) {
	mode = -1;
	inv_out[0] = 0;
	f_h(call_ARG0(inv_out,NULL));
	// fprintf(inv_file, "%s\n", inv_out);
	i = strlen(inv_out);
	inv_out[i++] = '\n';
	inv_out[i] = '\0';
        fwrite_file(inv_out, 1, i, &inv_file);
	err_bin(1); err_string(1);
        return 8;
    }

    /* copy argv */

    for (i = 0; i < argc; i++) {
	new_argv[i] = argv[i];
    }

    has_inv_option = 0;
    file_arg = 0;
    in_file.file_type = NOT_OPEN;

    /* "compile" options */
#ifdef DEBUG
    fprintf(stderr,"going to compile phase\n");
#endif

    for (i = 1; i < argc; i++) {

	/* filename: either - or string that does not start with - */
	if (new_argv[i][0] != '-' || (strcmp(new_argv[i],"-") == 0))  {
	    if (file_arg) 
		fatal_error_ss("too many grib files .. 1st=%s 2nd=%s", new_argv[file_arg], new_argv[i]);
	    file_arg = i;
	    fopen_file(&in_file, new_argv[file_arg],"rb");
	    continue;
	}

	/* must be an option .. make the search faster - sort or openmp */

	for (j = 0; j < nfunctions; j++) {
	    if (strcmp(&(new_argv[i][1]),functions[j].name) == 0) break;
	}
	if (j == nfunctions) fatal_error("unknown option %s", new_argv[i]);

        /* add to function argument list */
	arglist[narglist].fn = j;
	arglist[narglist].i_argc = i+1;

	if (functions[j].type == inv) has_inv_option = 1;

	i += functions[j].nargs;
	if (i >= argc) fatal_error("missing arguments option=%s",functions[j].name);
	narglist++;
    }

    /* if no inv option, add -s */
    if (has_inv_option == 0) {
	for (j = 0; j < nfunctions; j++) {
	    if (strcmp("s",functions[j].name) == 0) break;
	}
	if (j == nfunctions) fatal_error("unknown option %s", "-s");
	if (functions[j].nargs != 0 || functions[j].type != inv) fatal_error("problem: -s was redefined","");

        /* add to function argument list */
	arglist[narglist].fn = j;
	arglist[narglist].i_argc = i+1;
	has_inv_option = 1;
	narglist++;
    }

    /* initialize options,  mode = -1 */

#ifdef DEBUG
    fprintf(stderr,"going to init options,  narglist %d\n",narglist);
#endif

    for (j = 0; j < narglist; j++) {
	new_inv_out();	/* inv_out[0] = 0; */
	n_arg = functions[arglist[j].fn].nargs;
        err = 0;
    }

    /* initialize options,  mode = -1 */

#ifdef DEBUG
    fprintf(stderr,"going to init options,  narglist %d\n",narglist);
#endif

    for (j = 0; j < narglist; j++) {
	new_inv_out();	/* inv_out[0] = 0; */
	n_arg = functions[arglist[j].fn].nargs;
        err = 0;
#ifdef DEBUG
    fprintf(stderr,"going to init option %s\n", functions[arglist[j].fn].name);
#endif
        if (n_arg == 0) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j);
	else if (n_arg == 1) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc]);
	else if (n_arg == 2) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc],new_argv[arglist[j].i_argc+1]);
	else if (n_arg == 3) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc],new_argv[arglist[j].i_argc+1],new_argv[arglist[j].i_argc+2]);
	else if (n_arg == 4) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc],new_argv[arglist[j].i_argc+1],
		new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3]);
	else if (n_arg == 5) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc],new_argv[arglist[j].i_argc+1],
		new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
		new_argv[arglist[j].i_argc+4]);
	else if (n_arg == 6) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc],new_argv[arglist[j].i_argc+1],
		new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
		new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5]);
	else if (n_arg == 7) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc],new_argv[arglist[j].i_argc+1],
		new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
		new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5],
		new_argv[arglist[j].i_argc+6]);
	else if (n_arg == 8) err = functions[arglist[j].fn].fn(-1,NULL,NULL,0, inv_out,local+j,
		new_argv[arglist[j].i_argc],new_argv[arglist[j].i_argc+1],
		new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
		new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5],
		new_argv[arglist[j].i_argc+6], new_argv[arglist[j].i_argc+7]);

        // if(inv_out[0] != 0)  fprintf(inv_file, "%s", inv_out);
        if(inv_out[0] != 0) fwrite_file(inv_out, 1, strnlen(inv_out,INV_BUFFER), &inv_file);

        if (err) {
	    err_bin(1); err_string(1);
	    // cleanup
            return 8;
	}
    }

    /* error and EOF handlers have been initialized */
#ifdef DEBUG
    fprintf(stderr,"initial error and EOF handlers\n");
#endif

    if (has_inv_option == 0) fatal_error("missing arguments on last option","");
    if (in_file.file_type == NOT_OPEN) {
	if (file_arg == 0) fatal_error("no input file defined","");
	else fatal_error("missing input file %s", new_argv[file_arg]);
    }

    if (latlon == 1 && output_order_wanted != wesn) 
           fatal_error("latitude-longitude information is only available with -order we:sn","");

    if (input == inv_mode && (in_file.file_type != DISK && in_file.file_type != MEM)) 
	fatal_error("wgrib2 cannot random access grib input file","");

#ifdef DEBUG
    fprintf(stderr,"going to process data\n");
#endif

    msg_no = 1;
    inv_no = 0;
    len = pos = 0;
    submsg = 0;
    msg = NULL;

    last_pos = -1;
    last_submsg = -1;

    /* if dump mode .. position io stream */
    if (input == dump_mode) {
        while (msg_no < dump_msg) {
// //	    if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq(sec, in, &pos, &len, &num_submsgs);
//	    if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
//	    else if (in_file.file_type == DISK) msg = rd_grib2_msg(sec, in, &pos, &len, &num_submsgs);

	    msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
            if (msg == NULL) fatal_error_i("record %d not found", dump_msg);
            last_pos = pos;
            pos += len;
            msg_no++;
        }
#ifdef DEBUG
        printf("dump mode msg=%d\n", msg_no);
#endif
    }

    /* 
     * submsg = 0 .. beginning of unread record
     * submsg = i .. start at ith submsg
     * num_submsgs = number of submessages in grib message
     */

    /* inventory loop */ 

    for (;last_message == 0;) {

        /* need position and submessage number of message */
        if (input == inv_mode || input == dump_mode) {
            if (input == inv_mode) {
                if (rd_inventory(&msg_no,&submsg, &pos, &rd_inventory_input)) break;
		if (fseek_file(&in_file, pos,SEEK_SET) != 0) fatal_error("fseek_file failed","");
            }
            else if (input == dump_mode) {
                if (dump_msg == -1) break;
                submsg = dump_submsg;
                dump_msg = -1;
	    }

            if (pos != last_pos) {
// //	        if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq(sec, in, &pos, &len, &num_submsgs);
//	        if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
//		else if (in_file.file_type == DISK) msg = rd_grib2_msg(sec, in, &pos, &len, &num_submsgs);
	        msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
	        if (msg == NULL) {
                    fatal_error_i("grib message #%d not found", msg_no);
                    break;
                }
                last_pos = pos;
		last_submsg = -1;
            }

            if (pos == last_pos && submsg == last_submsg + 1) {
                /* read previous submessage */
		if (parse_next_msg(sec) != 0) {
                    fprintf(stderr,"\n*** grib message #%d.%d not found ***\n\n", msg_no, submsg);
                    break;
		}
            }
            else {
                /* need to get desired submessage into sec */
		if (parse_1st_msg(sec) != 0) {
                    fprintf(stderr,"\n*** grib message #%d.1 not found ***\n\n", msg_no);
                    break;
		}
                for (i = 2; i <= submsg; i++) {
		    if (parse_next_msg(sec) != 0) {
                        fprintf(stderr,"\n*** grib message #%d.%d not found ***\n\n", msg_no, i);
                        break;
                    }
		}
	    }
            last_submsg = submsg;
	}
        else if (input == all_mode) {
	    if (submsg == 0) {
// //	        if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq(sec, in, &pos, &len, &num_submsgs);
//	        if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
//		else if (in_file.file_type == DISK) msg = rd_grib2_msg(sec, in, &pos, &len, &num_submsgs);
	        msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
		if (msg == NULL) break;
                submsg = 1;
	    }
	    else if (submsg > num_submsgs) {
		pos += len;
                msg_no++;
// //	        if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq(sec, in, &pos, &len, &num_submsgs);
//	        if (in_file.file_type == PIPE) msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
//		else if (in_file.file_type == DISK) msg = rd_grib2_msg(sec, in, &pos, &len, &num_submsgs);
	        msg = rd_grib2_msg_seq_file(sec, &in_file, &pos, &len, &num_submsgs);
		if (msg == NULL) break;
                submsg = 1;
	    }
            if (submsg == 1) {
		if (parse_1st_msg(sec) != 0) {
		    fprintf(stderr,"illegal format: parsing 1st submessage\n");
		}
            }
            else {
		if (parse_next_msg(sec) != 0) {
                    fprintf(stderr,"illegal format: parsing submessages\n");
                }
	    }
	}
        if (only_submsg > 0 && only_submsg != submsg) {
	    submsg++;
	    continue;
	}

	if (for_mode) {
	    if (msg_no < for_start || msg_no > for_end || ((msg_no - for_start) % for_step) != 0) {
	        if (msg_no > for_end && input != inv_mode) last_message = 1;
		submsg++;
		continue;
	    }
	}

	/* move inv_no++ before match_inv is made */
	inv_no++;

        if (match || match_fs) {
	   inv_out[0] = 0;
	   if (num_submsgs > 1) {
	       sprintf(inv_out,"%d.%d:", msg_no, submsg);
	   }
           else {
	       sprintf(inv_out,"%d:", msg_no);
	   }

           f_match_inv(call_ARG0(inv_out+strlen(inv_out), NULL));

           if (is_match_fs(inv_out) != 0) {
              submsg++;
	      inv_no--;
              continue;
           }

#ifdef USE_REGEX
           if (is_match(inv_out) != 0) {
              submsg++;
	      inv_no--;
              continue;
           }
#endif

        }
	match_flag = 0;

        if (for_n_mode) {
            if (inv_no < for_n_start || inv_no > for_n_end || ((inv_no - for_n_start) % for_n_step) != 0) {
                if (inv_no > for_n_end) last_message = 1;
                submsg++;
                continue;
            }
        }

        /* see if new GDS */

	if ((i = (int) GB2_Sec3_size(sec)) != old_GDS_size) {
	    new_GDS = 1;
	}
	else {
	    new_GDS = 0;
	    for (j = 0; j < i; j++) {
		if (old_gds[j] != sec[3][j]) new_GDS = 1;
	    }
	}
	if (new_GDS) {
	    GDS_change_no++;
	    if (i > GDS_max_size) {
		if (GDS_max_size) free(old_gds);
		GDS_max_size = i + 100;		/* add 100 just to avoid excessive memory allocations */
    		if ((old_gds = (unsigned char *) malloc(GDS_max_size) ) == NULL) {
			fatal_error("memory allocation problem old_gds in wgrib2.main","");
		}
	    }
	    for (j = 0; j < i; j++) {
		old_gds[j] = sec[3][j];
            }
	    old_GDS_size = i;
	    /* update grid information */
            get_nxny(sec, &nx, &ny, &npnts, &res, &scan);	 /* get nx, ny, and scan mode of grid */
            get_nxny_(sec, &nx_, &ny_, &npnts, &res, &scan);	 /* get nx, ny, and scan mode of grid */

	    output_order = (nx_ < 1 || ny_ < 1) ? raw : output_order_wanted;

            if (latlon) {
		i = 1;

#ifdef USE_PROJ4
		if (use_proj4 && i != 0) {				/* use Proj4 to get lat lon values */
		    i = proj4_get_latlon(sec, &lon, &lat);
//		    if (i == 0) fprintf(stderr,"proj4_get_lat used\n");
		}
#endif

		if (use_gctpc && i != 0) {				/* use gctpc to get lat lon values */
		    i = gctpc_get_latlon(sec, &lon, &lat);
//		    if (i == 0) fprintf(stderr,"gctpc_get_lat used\n");
		}

		if (i != 0) get_latlon(sec, &lon, &lat);		 /* get lat lon of grid points using built-in code */
	    }
	}

	/* Decode NDFD WxText */
	if (WxText) mk_WxKeys(sec);

	// any fixes to raw grib message before decode need to be placed here
	if (fix_ncep_2_flag) fix_ncep_2(sec);
	if (fix_ncep_3_flag) fix_ncep_3(sec);
	if (fix_ncep_4_flag) fix_ncep_4(sec);

#ifdef CHECK
	j = code_table_5_0(sec);		// type of compression

	/* yes this can be simplified but want to split it up in case other decoders have problems */
	if (j == 0 && sec[5][19] == 0 && int2(sec[5] + 17) != 0 && ieee2flt(sec[5]+11) != 0.0) 
		fprintf(stderr,"Warning: g2lib/g2clib/grib-api simple encode/decode may differ from WMO standard, use -g2clib 0 for WMO standard\n");
	if ((j == 2 || j == 3) && int2(sec[5]+17) != 0 && int4(sec[5] + 31) == 0 && ieee2flt(sec[5]+11) != 0.0) 
		fprintf(stderr,"Warning: g2lib/g2clib complex encode/decode may differ from WMO standard, use -g2clib 0 for WMO standard\n");
	if (j == 40 && sec[5][19] == 0 && int2(sec[5] + 17) != 0 && ieee2flt(sec[5]+11) != 0.0) 
		fprintf(stderr,"Warning: g2lib/g2clib jpeg encode/deocde may differ from WMO standard, use -g2clib 0 for WMO standard\n");
	if (j == 41 && sec[5][19] == 0 && int2(sec[5] + 17) != 0 && ieee2flt(sec[5]+11) != 0.0) 
		fprintf(stderr,"Warning: g2lib/g2clib/grib-api png encode/decode may differ from WMO standard, use -g2clib 0 for WMO standard\n");

	/* check the size of Section 7 */
	/* code to check the other sizes needs to be placed in decode routines */

	j = code_table_5_0(sec);		// type of compression
	if (j == 0) {		/* simple */
	    /* to avoid overflow on 32 bit machines */
	    /* old:  k = (GB2_Sec5_nval(sec) * sec[5][19] + 7) / 8 + 5; */
	    k = 5 + (GB2_Sec5_nval(sec)/8) * sec[5][19] +  (GB2_Sec5_nval(sec)%8) * (sec[5][19]/8)
	    	+ ( (GB2_Sec5_nval(sec)%8) * (sec[5][19]%8) + 7) / 8;

	    if (k != GB2_Sec7_size(sec)) {
		fprintf(stderr,"Detected a size mismatch, Section 7, wanted %d found %d\n", k, GB2_Sec7_size(sec));
		if (decode) fatal_error("Section 7 size, mismatch, simple packing","");
	    }
	}
	else if (j == 4) {		/* IEEE */
	    k = GB2_Sec5_nval(sec) * 4 + 5;
	    if (k != GB2_Sec7_size(sec)) {
		fprintf(stderr,"Detected a size mismatch, Section 7, wanted %d found %d\n", k, GB2_Sec7_size(sec));
		if (decode) fatal_error("Section 7 size, mismatch, IEEE packing","");
	    }
	}

	/* code table 4.3 can change units, warn if undefined */

	if (err_4_3_count < 2) {
	    if (code_table_4_3(sec) == 255) {
		fprintf(stderr,"** WARNING input Code Table 4.3 = 255 (undefined) **\n");
		err_4_3_count++;
	    }
        }
#endif

	if (decode) {

#ifdef CHECK
            if (code_table_6_0(sec) == 0) {                         // has bitmap
                k = GB2_Sec3_npts(sec) -  GB2_Sec5_nval(sec);
                if (k != missing_points(sec[6]+6, GB2_Sec3_npts(sec)))
                    fatal_error_uu("inconsistent number of bitmap points sec3-sec5: %u sec6: %u",
			k, missing_points(sec[6]+6, GB2_Sec3_npts(sec)));
            }
            else if (code_table_6_0(sec) == 255) {                  // no bitmap
                if (GB2_Sec3_npts(sec) != GB2_Sec5_nval(sec))
                    fatal_error_uu("inconsistent number of data points sec3: %u sec5: %u",
                        GB2_Sec3_npts(sec), GB2_Sec5_nval(sec));
            }
#endif

            /* allocate data */
            if (GB2_Sec3_npts(sec) != ndata) {
		if (ndata) free(data);
                ndata = GB2_Sec3_npts(sec);
		if (ndata) {
                    data = (float *) malloc(sizeof(float) * (size_t) ndata);
                    if (data == NULL) {
			ndata = 0;
			fatal_error("main: memory allocation failed data","");
		    }
		}
                else { data = NULL; }
            }

	    j = code_table_5_0(sec);		// type of compression

            /* USE G2CLIB */

#ifdef USE_G2CLIB
            if (use_g2clib == 2) {
                err = g2_getfld(msg,submsg,1,1,&grib_data);
                if (err != 0) fatal_error_ii("Fatal g2clib decode err=%d msg=%d", err, msg_no);
                free_gribfield = 1;

                has_bitmap = grib_data->ibmap;
                g2_data = &(grib_data->fld[0]);
                if (has_bitmap == 0 || has_bitmap == 254) {
                    bitmap = grib_data->bmap;
                    for (k = 0; k < ndata; k++) {
                         data[k] = (bitmap[k] == 0) ? UNDEFINED : g2_data[k];
                    }
                }
                else {
                    for (k = 0; k < ndata; k++) {
                        data[k] = g2_data[k];
                    }
                }

                /* complex packing uses special values for undefined */
                ii = sub_missing_values(sec, &missing_c_val_1, &missing_c_val_2);
                if (ii == 1) {
                    for (k = 0; k < ndata; k++) {
                        if (data[k] == missing_c_val_1) data[k] = UNDEFINED;
                    }
                }
                else if (ii == 2) {
                    for (k = 0; k < ndata; k++) {
                        if (data[k] == missing_c_val_1) data[k] = UNDEFINED;
                        if (data[k] == missing_c_val_2) data[k] = UNDEFINED;
                    }
                }
            }
#endif

            /* USE INTERNAL DECODER */

            if (use_g2clib != 2) {
                center = GB2_Center(sec);
                if (use_g2clib == 1) {	// introduce g2clib constant field error
		    /* g2clib ignores decimal scaling for constant fields make internal decoders look like g2clib*/
                    if ( (j == 0 && sec[5][19] == 0) || ((j == 2 || j == 3) && int4(sec[5] + 31) == 0) ||
                         (j == 40 && sec[5][19] == 0) || (j == 41 && sec[5][19] == 0) ||
                         (center == NCEP && j == 40000 && sec[5][19] == 0) || 
                         (center == NCEP && j == 40010 && sec[5][19] == 0)  ) {
			dscale[0] = sec[5][17];
			dscale[1] = sec[5][18];
			sec[5][17] = sec[5][18] = 0;
                    }
		}

		err = unpk_grib(sec, data);
                if (err != 0) fatal_error_i("Fatal decode packing type %d",err);

		if (use_g2clib == 1) {  // fix up data 
		    /* restore decimal scaling */
                    if ( (j == 0 && sec[5][19] == 0) || ((j == 2 || j == 3) && int4(sec[5] + 31) == 0) ||
                         (j == 40 && sec[5][19] == 0) || (j == 41 && sec[5][19] == 0) ||
                         (center == NCEP && j == 40000 && sec[5][19] == 0) || 
                         (center == NCEP && j == 40010 && sec[5][19] == 0)  ) {
			sec[5][17] = dscale[0];
			sec[5][18] = dscale[1];
                    }
		}
            }

	    /* convert to standard output order we:sn */

	    if (output_order_wanted == wesn) to_we_sn_scan(data, scan, npnts, nx, ny, save_translation);
	    else if (output_order_wanted == wens) to_we_ns_scan(data, scan, npnts, nx, ny, save_translation);
	}
        else {
	    if (ndata) free(data);
            ndata = 0;
            data = NULL;
        }

	/* get scaling parameters */

	use_scale = input_scale = scaling(sec, &ref, &dec_scale, &bin_scale, &i) == 0;

	/* make sure msg_no:pos is put in inv_out so that -last will work */
	new_inv_out();	// inv_out[0] = 0;
	if (num_submsgs > 1) {
	    sprintf(inv_out, "%d.%d%s%ld", msg_no, submsg, ":", pos);
	}
        else {
	    sprintf(inv_out, "%d%s%ld", msg_no, ":", pos);
	}
        // fprintf(inv_file, "%s", inv_out);
        fwrite_file(inv_out, 1, strnlen(inv_out,INV_BUFFER), &inv_file);

	for (j = 0; j < narglist; j++) {

	    /* skip execution if match_flag == 1 */
	    /* an output option acts as endif for match_flag */
	    if (match_flag == 1) {
                if (functions[arglist[j].fn].type == output)  match_flag = 0;
		continue;
	    }


            // if (functions[arglist[j].fn].type == inv) fprintf(inv_file, "%s", item_deliminator);
            if (functions[arglist[j].fn].type == inv) fwrite_file(item_deliminator, 1, strlen(item_deliminator), &inv_file);
            if (functions[arglist[j].fn].type != setup) {
		new_inv_out();	// inv_out[0] = 0;
	        n_arg = functions[arglist[j].fn].nargs;
		if (n_arg == 0) 
                    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j);
		else if (n_arg == 1)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			 new_argv[arglist[j].i_argc]);
		else if (n_arg == 2)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1]);
		else if (n_arg == 3)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2]);
		else if (n_arg == 4)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3]);
		else if (n_arg == 5)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4]);
		else if (n_arg == 6)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5]);
		else if (n_arg == 7)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5],
			new_argv[arglist[j].i_argc+6]);
		else if (n_arg == 8)
		    functions[arglist[j].fn].fn(mode, sec, data, ndata, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5],
			new_argv[arglist[j].i_argc+6], new_argv[arglist[j].i_argc+7]);

        	// if(inv_out[0] != 0)  fprintf(inv_file, "%s", inv_out);
        	if(inv_out[0] != 0) fwrite_file(inv_out, 1, strnlen(inv_out,INV_BUFFER), &inv_file);
           }
	}

#ifdef CHECK
	if (!decode) {
            if (code_table_6_0(sec) == 0) {                         // has bitmap
                k = GB2_Sec3_npts(sec) -  GB2_Sec5_nval(sec);
                if (k != missing_points(sec[6]+6, GB2_Sec3_npts(sec)))
                    fatal_error_uu("inconsistent number of bitmap points sec3-sec5: %u sec6: %u",
			k, missing_points(sec[6]+6, GB2_Sec3_npts(sec)));
            }
            else if (code_table_6_0(sec) == 255) {                  // no bitmap
                if (GB2_Sec3_npts(sec) != GB2_Sec5_nval(sec))
                    fatal_error_ii("inconsistent number of data points sec3: %d sec5: %d",
                        (int) GB2_Sec3_npts(sec), (int) GB2_Sec5_nval(sec));
            }
	}
#endif

	submsg++;

#ifdef USE_G2CLIB
	if (free_gribfield) { g2_free(grib_data); free_gribfield = 0;}
#endif

	// fprintf(inv_file, "%s",end_inv);
        fwrite_file(end_inv, 1, strlen(end_inv), &inv_file);

	if (flush_mode) fflush_file(&inv_file);
	if (dump_msg > 0) break;
    }

    /* for CW2, make inv file readable after subroutine call */
    fflush_file(&inv_file);

    /* finalize all functions, call with mode = -2 */

    err = 0;
    for (j = 0; j < narglist; j++) {
//        if (functions[arglist[j].fn].type != setup) {
	    n_arg = functions[arglist[j].fn].nargs;
	    new_inv_out();	// inv_out[0] = 0;
	    if (n_arg == 0) 
                err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j);
	    else if (n_arg == 1)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc]);
	    else if (n_arg == 2)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1]);
	    else if (n_arg == 3)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2]);
	    else if (n_arg == 4)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3]);
	    else if (n_arg == 5)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4]);
	    else if (n_arg == 6)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5]);
	    else if (n_arg == 7)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5],
			new_argv[arglist[j].i_argc+6]);
	    else if (n_arg == 8)
		err |= functions[arglist[j].fn].fn(-2, NULL, NULL, 0, inv_out, local+j,
			new_argv[arglist[j].i_argc], new_argv[arglist[j].i_argc+1],
			new_argv[arglist[j].i_argc+2], new_argv[arglist[j].i_argc+3],
			new_argv[arglist[j].i_argc+4], new_argv[arglist[j].i_argc+5],
			new_argv[arglist[j].i_argc+6], new_argv[arglist[j].i_argc+7]);
            // if (inv_out[0]) fprintf(stderr, "%s\n", inv_out);
            if (inv_out[0]) fprintf(stderr, "%s%s", inv_out, end_inv);
//        }
    }
    err_bin(0); err_string(0);
    fclose_file(&in_file);
    if (ndata) {
	ndata = 0;
	free(data);
    }
    // return 0;
    return err;
}

void set_mode(int new_mode) {
	mode = new_mode;
}
