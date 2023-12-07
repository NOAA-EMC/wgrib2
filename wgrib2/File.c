#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
/*
 * File.c
 *   routines that write output to files
 *
 * 2006: Public Domain, Wesley Ebisuzaki
 * 1/2007: cleanup M. Schwarb
 * 1/2008: lat and lon changed from float to double
 * 1/2008: W. Ebisuzaki, use filename "-" to write to stdout (-text, -spread)
 * 2/2008: W. Ebisuzaki, use filename "-" to write to stdout (-text, -spread) revised
 * 2/2008: W. Ebisuzaki, use filename "-" to write to stdout (-text, -spread) revised
 * 2/2008: W. Ebisuzaki remove decode flags for grib, GRIB
 * 6/2011: W. Ebisuzaki WxText enabled -spread, added flush_mode to -spread
 */

extern int header;
extern int file_append;
extern int decode, latlon;
extern int flush_mode;
extern int nx, ny;
extern double *lat, *lon;
extern int WxText, WxNum;

/* parameters for text mode */
const char *text_format;
int text_column;
extern const char *nl;

/*
 * HEADER:100:bin:output:1:write binary data to X
 */

int f_bin(ARG1) {
    unsigned int i, j;
    struct seq_file *save;

    if (mode == -1) {
	*local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
	if (save == NULL) fatal_error("bin: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        decode = 1;
    }
    else if (mode == -2) {
	save = *local;
	fclose_file(save);
	free(save);
    }
    else if (mode >= 0) {
	save = *local;
	if (header) {
	    if (ndata > 4294967295U / sizeof(float))
	        fatal_error("bin: 4-byte header overflow","");
	    i = ndata * sizeof(float);
            j = fwrite_file((void *) &i, sizeof(int), 1, save);
	    if (j != 1) fatal_error("bin: write header","");
	}
        j = fwrite_file((void *) data, sizeof(float), ndata, save);
	if (j != ndata) fatal_error_u("bin: error writing grid point written=%u", j);
        if (header) {
	    i = ndata * sizeof(float);
	    j = fwrite_file((void *) &i, sizeof(int),1, save);
	    if (j != 1) fatal_error("bin: write header","");
	}
        if (flush_mode) fflush_file(save);
    }
    return 0;
}

/*
 * HEADER:100:ieee:output:1:write (default:big-endian) IEEE data to X
 */

int f_ieee(ARG1) {
    struct seq_file *save;

    if (mode == -1) {
	*local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
	if (save == NULL) fatal_error("ieee: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        decode = 1;
    }
    else if (mode == -2) {
	save = *local;
	fclose_file(save);
	free(save);
    }
    else if (mode >= 0) {
	save = *local;
	wrtieee(data, ndata, header, save);
        if (flush_mode) fflush_file(save);
    }
    return 0;
}

/*
 * HEADER:100:text_fmt:misc:1:format for text output (C)
 */
int f_text_fmt(ARG1) {
    if (mode >= -1) text_format = arg1;
    return 0;
}

/*
 * HEADER:100:text_col:misc:1:number of columns on text output
 */
int f_text_col(ARG1) {
    if (mode >= -1) {
        text_column = atoi(arg1);
        if (text_column < 1) text_column = 1;
    }
    return 0;
}

/*
 * HEADER:100:text:output:1:write text data into X
 */
int f_text(ARG1) {
    unsigned int i;

    if (mode == -1) {
	if ((*local = (void *) ffopen(arg1,file_append ? "a" : "w")) == NULL) 
	        fatal_error("Could not open %s", arg1);
        decode = 1;
    }
    else if (mode == -2) {
	ffclose((FILE *) *local);
	// free(*local);
    }
    else if (mode >= 0) {
        if (header == 1) {
	    fprintf((FILE *) *local,"%d %d\n", nx, ny);
	}
	for (i = 0; i < ndata; i++) {
	    fprintf((FILE *) *local, text_format, data[i]);
            fprintf((FILE *) *local, ((i+1) % text_column) ? " " : "\n");
        }
        if (flush_mode) fflush((FILE *) *local);
    }
    return 0;
}

/*
 * HEADER:100:spread:output:1:write text - spread sheet format into X (WxText enabled)
 */
int f_spread(ARG1) {
    unsigned int i;

    if (mode == -1) {
        if ((*local = (void *) ffopen(arg1,file_append ? "a" : "w")) == NULL)
	        fatal_error("Could not open %s", arg1);
        WxText = latlon = decode = 1;
    }
    else if (mode == -2) {
	ffclose((FILE *) *local);
    }
    else if (mode >= 0) {
	if (lat == NULL || lon == NULL || data == NULL) {
	    fprintf(stderr,"no code to determine lat-lon information, no spread sheet output\n");
	    return 0;
	}
	set_mode(0);
	f_var(call_ARG0(inv_out,NULL));
	fprintf((FILE *) *local,"lon,lat,%s",inv_out);
	f_lev(call_ARG0(inv_out,NULL));
	fprintf((FILE *) *local," %s", inv_out);
	f_t(call_ARG0(inv_out,NULL));
	fprintf((FILE *) *local," %s", inv_out);
	f_ftime(call_ARG0(inv_out,NULL));
	fprintf((FILE *) *local," %s\n", inv_out);

	if (WxNum > 0) {
	    for (i = 0; i < ndata; i++) {
	        if(!UNDEFINED_VAL(data[i])) 
	            fprintf((FILE *) *local,"%lf,%lf,\"%s\"\n",lon[i],lat[i],WxLabel(data[i]));
	    }
	}
	else {
	    for (i = 0; i < ndata; i++) {
	        if(!UNDEFINED_VAL(data[i])) 
	            fprintf((FILE *) *local,"%lf,%lf,%g\n",lon[i],lat[i],data[i]);
	    }
	}
        if (flush_mode) fflush((FILE *) *local);
	inv_out[0] = 0;
	set_mode(mode);
    }

    return 0;
}

/*
 * HEADER:100:GRIB:output:1:writes entire GRIB record (all submessages)
 */

int f_GRIB(ARG1) {
    size_t size;
    struct seq_file *save;

    if (mode == -1) {
	*local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
	if (save == NULL) fatal_error("GRIB: memory allocation","");
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
        /* figure out size of grib file */
        size = uint8(sec[0]+8);
        /* write entire record to out */
        fwrite_file((void *) sec[0], sizeof(char), size, save);
        if (flush_mode) fflush_file(save);
    }
    return 0;
}

/*
 * HEADER:100:grib:output:1:writes GRIB record (one submessage) to X
 */

int f_grib(ARG1) {
    int i;
    struct seq_file *save;

    i = 0;
    if (mode == -1) {
	*local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
	if (save == NULL) fatal_error("grib: memory allocation","");

	if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
	    free(save);
	    fatal_error("Could not open %s", arg1);
	}
    }
    else if (mode == -2) {
	save = (struct seq_file *) *local;
	fclose_file(save);
	free(save);
    }
    else if (mode >= 0) {
	save = (struct seq_file *) *local;
	i = wrt_sec(sec[0], sec[1], sec[2], sec[3], sec[4], sec[5], sec[6],
             sec[7], save);
        if (flush_mode) fflush_file(save);
    }
    return i;
}

/*
 * HEADER:100:persistent:setup:1:makes file X persistent if already opened (default on open), CW2
 *   only useful when wgrib is called as a subroutine, no error if failure
 */

int f_persistent(ARG1) {
    if (mode == -1) {
	mk_file_persistent(arg1);
    }
    return 0;
}

/*
 * HEADER:100:transient:setup:1:make file X transient, CW2
 *   only useful when wgrib is called as a subroutine, no error if failure
 */

int f_transient(ARG1) {
    if (mode == -1) {
	mk_file_transient(arg1);
    }
    return 0;
}

/*
 * HEADER:100:rewind_init:setup:1:rewinds file X on initialization if already opened, CW2
 *   only useful when wgrib is called as a subroutine, no error if failure
 */
int f_rewind_init(ARG1) {
    int i;
    if (mode == -1) {
	i = rewind_file(arg1);
	if (i) fprintf(stderr,"WARNING: -rewind_init failed on %s\n", arg1);
    }
    return 0;
}

/*
 * HEADER:100:rewind_proc:misc:1:rewinds file X on processing step if already opened, CW2
 *   only useful when wgrib is called as a subroutine, no error if failure
 */
int f_rewind_proc(ARG1) {
    int i;
    if (mode >= 0) {
        i = rewind_file(arg1);
        if (i) fprintf(stderr,"WARNING: -rewind failed on %s\n", arg1);
        return i;
    }
    return 0;
}

/*
 * HEADER:100:rewind_final:misc:1:rewinds file X on cleanup step if already opened, CW2
 *   only useful when wgrib is called as a subroutine, no error if failure
 */
int f_rewind_final(ARG1) {
    int i;
    if (mode == -2) {
        i = rewind_file(arg1);
        if (i) fprintf(stderr,"WARNING: -rewind failed on %s\n", arg1);
        return i;
    }
    return 0;
}
