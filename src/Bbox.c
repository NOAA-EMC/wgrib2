/**
 * @file
 * @brief Take a rectangular box of data from a rectangular grid and 
 * write it to a file.
 * @author Arlindo da Silva @date 3/2008
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/2008 | A. da Silva | Initial
 * 1/2011 | W. Ebisuzaki | replace new_GDS by GDS_change_no, WNE
 */

/*

    Copyright (C) 2008 by Arlindo da Silva <dasilva@opengrads.org>
    All Rights Reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; using version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, please consult  
              
              http://www.gnu.org/licenses/licenses.html

    or write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA 02111-1307 USA

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Flush of output flag. */
extern int flush_mode;

/** Append grib file flag. */
extern int file_append;

/** Number of grid points in the x direction. */
extern int nx;

/** Number of grid points in the y direction. */
extern int ny;

/** GDS change number. */
extern int GDS_change_no;

/** Scan mode flag. */
extern int scan;

/** File header flag. */
extern int header;

/** Pointer to text format. */
extern char *text_format;

/** Number of columns in text output. */
extern int text_column;

/** Struct to hold bounding box information. */
struct Bbox {
    FILE *out; /**< Output file pointer. */
    int i1; /**< Starting index in the x direction. */
    int i2; /**< Ending index in the x direction. */
    int di; /**< Increment in the x direction. */
    int ni; /**< Number of grid points in the x direction. */
    int j1; /**< Starting index in the y direction. */
    int j2; /**< Ending index in the y direction. */
    int dj; /**< Increment in the y direction. */
    int nj; /**< Number of grid points in the y direction. */
    int *iptr; /**< Pointer to the grid data. */
    int last_GDS_change_no; /**< Last GDS change number. */
};

/**
 * Get the C-style index with wrapping.
 * 
 * @param i Index in Fortran-style (1-offset).
 * @param nx Number of grid points in the x direction.
 * 
 * @return C index
 */
static int get_cindex (int i, int nx) { 
    int j;
    j = (i - 1) % nx;
    if (j < 0) j += nx;
    return j;
}


/*
 * HEADER:100:ijbox:output:4:grid values in bounding box  X=i1:i2[:di] Y=j1:j2[:dj] Z=file A=[bin|text|spread]
 */

/**
 * Takes a rectangular box of data from a rectangular grid and writes it out in either text, bin, 
 * or spread (sheet) format. The extracted box can have every point or every n-th point. 
 * 
 * ## Usage
 * -ijbox i1:i2:di j1:j2:dj output_file format
 * 
 * - i1:i2:di specifies the x-dimension, where i1 is the starting index, i2 is the ending index, and di is the increment.
 * - j1:j2:dj specifies the y-dimension, where j1 is the starting index, j2 is the ending index, and dj is the increment.
 * - output_file is written in the specified format (e.g., "bin", "text", "spread").
 * 
 * @note -big_endian and -little_endian have no effect the bin type output
 * 
 * @param ARG4 List of function arguments set by wgrib2's main() function (see @ref ARG4). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * ## Example: 
 * 
 * Wgrib2 when writing binary, can make a fortran header at the beginning of the grid and a fortran trailer at the end of 
 * the grid. I wanted to create a file with fortran header and trailer surrounding each row of the grid. I was able to do 
 * it with -ijox. The grid of rtgssthr_grb2 is 1041 x 441.
 * 
 * @code{.sh}
 * rm output
 * i=1
 * while [ $i -le 441 ] ; do
 *    wgrib2 rtgssthr_grb2 -append -header ijbox 1:1041 $i:$i output bin
 *    i=`expr $i + 1`
 * done
 * @endcode
 *
 * @author Arlindo da Silva @date 3/2008
 */
int f_ijbox(ARG4) {

    int i1, i2, di, ni;
    int j1, j2, dj, nj;
    int i, j, k, nk, n;
    int x, y, nxny;
    
    struct Bbox *self;
    char open_mode[4];

    /*                      -------------------- 
                            Initialization Phase 
                            --------------------
    */
    if (mode == -1) {

        decode = 1;

        /* parse arguments */
        n = sscanf(arg1,"%d:%d:%d", &i1, &i2, &di);
        if (n < 2) fatal_error("ijbox parsing i-dimension, expecting i1:i2:di but got %s", arg1);
        if (n == 2) di = 1;
    
        n = sscanf(arg2,"%d:%d:%d", &j1,&j2,&dj);
        if (n < 2) fatal_error("ijbox parsing j-dimension, expecting j1:j2:dj but got %s", arg2);
        if (n == 2) dj = 1;

        if (strcmp(arg4,"spread") != 0 && 
                strcmp(arg4,"text")   != 0 && 
                strcmp(arg4,"bin")    != 0  ) 
            fatal_error("ijbox bad write mode %s", arg4);

        strcpy(open_mode, file_append ? "a" : "w");
        if (strcmp(arg4,"bin")  == 0 || 
                strcmp(arg4,"ieee") == 0 || /* ieee not implemented yet */ 
                strcmp(arg4,"grib") == 0 ) /* grib not implemented yet */ 
            strcat(open_mode,"b");

        ni = 1 + (i2-i1)/di;  
        nj = 1 + (j2-j1)/dj;  
        i2 = i1 + (ni-1) * di; /* get real end point */
        j2 = j1 + (nj-1) * dj; /* get real end point */

        /* state holder */
        self = (struct Bbox *)malloc( sizeof(struct Bbox));
        if (self == NULL) fatal_error("ijbox memory allocation ","");
        *local = self;

        /* initialize state*/
        self->i1=i1; self->i2=i2; self->di=di; self->ni=ni;
        self->j1=j1; self->j2=j2; self->dj=dj; self->nj=nj;

        self->iptr = (int *) malloc(ni*((size_t) nj) * sizeof(int));
        if (self->iptr == NULL) fatal_error("memory allocation in ijbox","");

        if ((self->out = ffopen(arg3,open_mode)) == NULL)
        fatal_error("ijbox could not open output file %s", arg3);
        self->last_GDS_change_no = 0;
        return 0;
    }

    self = (struct Bbox *) *local;

    if (mode == -2) {
        ffclose(self->out);
        free(self->iptr);
        free(self);
        return 0;
    }


  /*                      ---------------- 
                          Processing Phase 
                          ----------------
  */


    i1 = self->i1; i2=self->i2; di=self->di; ni=self->ni;
    j1 = self->j1; j2=self->j2; dj=self->dj; nj=self->nj;

    nk = ni * nj;
    nxny = nx * ny;

//    printf("ni=%d:nj=%d:di=%d:dj=%d:",ni,nj,di,dj);

    /* check if grid changed */
    if (self->last_GDS_change_no != GDS_change_no) {
        self->last_GDS_change_no = GDS_change_no;

        if (data == NULL) fatal_error("ijbox: no data","");
        if (GDS_Scan_staggered(scan)) fatal_error("ijbox does not support staggered grids","");
        
        /* record pointers, with wrapping */
        k = 0;
        /* Note: (i,j) are fortran-style 1-offset arrays */
        for (j = j1; j <= j2; j+=dj) {
            y = j - 1;
            for (i = i1; i <= i2; i+=di) {
                x = get_cindex (i, nx);
                n = x + y * nx;
                if (n < 0 || n >= nxny) fatal_error("ijbox invalid index", "");
                self->iptr[k++] = n; 
            }    
        }    
    }
    
    /*                     -----------
                           File Output 
                           -----------
    */

    /* Spreadsheet output */
    if (strcmp(arg4,"spread") == 0) {
        fprintf(self->out, " i-index, j-index, value,\n");
        for (k=0; k<nk; k++ ) {
            i = get_cindex(di*(k % ni) + i1, nx) + 1; 
            j = dj*(k/ni) + j1;
            fprintf(self->out, "%d, %d, %g,\n", i,j,data[self->iptr[k]]);
        }
      
    /* binary output */
    } else if (strcmp(arg4,"bin") == 0) {
        i = nk * sizeof(float);
        if (header) fwrite((void *) &i, sizeof(int), 1, self->out);
        for (k = 0; k < nk; k++) 
        fwrite(data + self->iptr[k], sizeof(float), 1, self->out);
        if (header) fwrite((void *) &i, sizeof(int), 1, self->out);

    /* text output */
    } else if (strcmp(arg4,"text") == 0) {
        if (header == 1) fprintf(self->out ,"%d %d\n", ni, nj);
        for (k = 0; k < nk; k++) {
            fprintf(self->out, text_format, data[self->iptr[k]]);
            fprintf(self->out, ((k+1) % text_column) ? " " : "\n");
        }
    }   /* out file type */

    if (flush_mode) fflush(self->out);
    return 0;
}
