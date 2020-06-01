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

/*
 * 3/2008 values inside index-space bounding box; based on Lola.c
 * 1/2011 replace new_GDS by GDS_change_no, WNE
 */

extern int decode, flush_mode;
extern int file_append;

extern int nx, ny, GDS_change_no, scan;

extern int header;
extern char *text_format;
extern int text_column;

struct Bbox {
  FILE *out;
  int i1, i2, di, ni;
  int j1, j2, dj, nj;
  int *iptr;
  int last_GDS_change_no;
};

/* return c-style index, with wrapping; on input i is a fortran-style
   index and nx is the */

static int get_cindex (int i, int nx) { 
   int j;
   j = (i - 1) % nx;
   if (j < 0) j += nx;
   return j;
}


/*
 * HEADER:100:ijbox:output:4:grid values in bounding box  X=i1:i2[:di] Y=j1:j2[:dj] Z=file A=[bin|text|spread]
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
