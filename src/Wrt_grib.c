#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * ieee-Grib: hooks for making own grib files
 *
 * 12/2007 Public Domain by Wesley Ebisuzaki
 * 5/2008: more symbols
 * 5/2008: fix "tail"
 * 5/2008: keep original scan order
 * 7/2017: write sections into *.h file
 *
 */


int grib_ieee(unsigned char **sec, float *data, unsigned int ndata, FILE *out, FILE *head, FILE *tail, FILE *c);
static int output_c(FILE *c, unsigned char *s, unsigned int n);

extern int decode, nx, ny, scan;
extern unsigned int last_message;
extern int flush_mode;
extern enum output_order_type output_order;

/*
 * HEADER:100:grib_ieee:output:1:writes data[] to X.grb, X.head, X.tail, and X.h
 */

int f_grib_ieee(ARG1) {

    struct local_struct {
        FILE *grib, *head, *tail, *c;
    }; 
    struct local_struct *save;
    char filename[STRING_SIZE];

    if (mode == -1) {
        decode = 1;

        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("grib_ieee memory allocation ","");
	if (strlen(arg1) > STRING_SIZE-6) fatal_error("filename is too long",arg1);

	strncpy(filename, arg1,STRING_SIZE-6);
	filename[STRING_SIZE-7] = 0;
	strncat(filename, ".grb",5);

	if (( save->grib = ffopen(filename, "wb") ) == NULL)
		fatal_error("Could not open %s", filename);

	strncpy(filename, arg1,STRING_SIZE-6);
	filename[STRING_SIZE-7] = 0;
	strncat(filename, ".head",6);
	if ((save->head = ffopen(filename, "wb") ) == NULL)
		fatal_error("Could not open %s", filename);

	strncpy(filename, arg1, STRING_SIZE-6);
	filename[STRING_SIZE-7] = 0;
	strncat(filename, ".tail",6);
	if ((save->tail = ffopen(filename, "wb") ) == NULL)
		fatal_error("Could not open %s", filename);

	strncpy(filename, arg1,STRING_SIZE-6);
	filename[STRING_SIZE-7] = 0;
	strncat(filename, ".h",3);
	if ((save->c = ffopen(filename, "wb") ) == NULL)
		fatal_error("Could not open %s", filename);
    }
    else if (mode >= 0) {
        save = *local;
        grib_ieee(sec, data, ndata, save->grib, save->head, save->tail, save->c);
        last_message = 1;			// stop
    }
    else if (mode == -2) {
        save = *local;
	ffclose(save->grib);
	ffclose(save->head);
	ffclose(save->tail);
	ffclose(save->c);
	free(*local);
    }
    return 0;
}


/*
 * write grib-2 ieee file
 */

int grib_ieee(unsigned char **sec, float *data, unsigned int ndata, FILE *out, FILE *head, FILE *tail, FILE *c) {

    int i;
    unsigned int n_defined, j;
//    int flag;
    size_t size;
    unsigned char *p, *sec0, *sec1, *sec2, *sec3, *sec4, *sec5, *sec6, *sec7;
    unsigned char s[8];
    float *new_data;

    /* required passed sections */
    sec0 = sec[0];
    sec1 = sec[1];
    sec2 = sec[2];
    sec3 = sec[3];
    sec4 = sec[4];

    /* change scan mode */
//    flag = flag_table_3_4(sec);
//    set_order(sec, output_order);

    /* make a new section 6 */

    n_defined = ndata;
    sec6 = (unsigned char *) malloc(6);
    if (sec6 == NULL) fatal_error("grib_out ieee memory allocation sec6","");
    uint_char(6 * sizeof (unsigned char), sec6);
    sec6[4] = 6;			// section 6
    sec6[5] = 255;			// no bitmap

    
    /* data representation section */

    sec5 = (unsigned char *) malloc(12 * sizeof(unsigned char));
    if (sec5 == NULL) fatal_error("grib_out ieee memory allocation sec5","");
    uint_char(12 * sizeof (unsigned char), sec5);
    sec5[4] = 5;			// section 5
    uint_char(ndata, sec5+5);		// number of points
    uint2_char(4,sec5+9);		// data template 4
    sec5[11] = 1;			// precision: ieee 32-bit


    /* data section */

    new_data = (float *) malloc(sizeof(float) * (size_t) n_defined);
    if (new_data == NULL) fatal_error("grib_out ieee memory allocation data","");
    undo_output_order(data, new_data, n_defined);

    sec7 = (unsigned char *) malloc(5 + 4 * (size_t) n_defined);
    if (sec7 == NULL) fatal_error("grib_out ieee memory allocation sec7","");
    uint_char(5+n_defined*4, sec7);
    sec7[4] = 7;
    p = sec7 + 5;
    for (j = 0; j < n_defined; j++) {
	flt2ieee_nan(new_data[j], p);
	p += 4;
    }
    free(new_data);

    size = (size_t) GB2_Sec0_size + GB2_Sec8_size +
         (sec1 ? uint4(sec1) : 0) +
         (sec2 ? uint4(sec2) : 0) +
         (sec3 ? uint4(sec3) : 0) +
         (sec4 ? uint4(sec4) : 0) +
         (sec5 ? uint4(sec5) : 0) +
         (sec6 ? uint4(sec6) : 0) +
         (sec7 ? uint4(sec7) : 0);

    fprintf(c,"unsigned char head[] = {");

    /* section 0 */
    fwrite((void *) sec0, sizeof(char), 8, out);
    fwrite((void *) sec0, sizeof(char), 8, head);
    output_c(c, sec0, 8);

    uint8_char(size, s);
    fwrite((void *) s, sizeof(char), 8, out);
    fwrite((void *) s, sizeof(char), 8, head);
    output_c(c, s, 8);

    fwrite((void *)sec1, sizeof(char), uint4(sec1), out);
    fwrite((void *)sec1, sizeof(char), uint4(sec1), head);
    output_c(c, sec1, uint4(sec1));

    if (sec2) fwrite((void *)sec2, sizeof(char), uint4(sec2), out);
    if (sec2) fwrite((void *)sec2, sizeof(char), uint4(sec2), head);
    if (sec2) output_c(c, sec2, uint4(sec2));

    if (sec3) fwrite((void *)sec3, sizeof(char), uint4(sec3), out);
    if (sec3) fwrite((void *)sec3, sizeof(char), uint4(sec3), head);
    if (sec3) output_c(c, sec3, uint4(sec3));

    if (sec4) fwrite((void *)sec4, sizeof(char), uint4(sec4), out);
    if (sec4) fwrite((void *)sec4, sizeof(char), uint4(sec4), head);
    if (sec4) output_c(c, sec4, uint4(sec4));

    if (sec5) fwrite((void *)sec5, sizeof(char), uint4(sec5), out);
    if (sec5) fwrite((void *)sec5, sizeof(char), uint4(sec5), head);
    if (sec5) output_c(c, sec5, uint4(sec5));

    if (sec6) fwrite((void *)sec6, sizeof(char), uint4(sec6), out);
    if (sec6) fwrite((void *)sec6, sizeof(char), uint4(sec6), head);
    if (sec6) output_c(c, sec6, uint4(sec6));

    if (sec7) fwrite((void *)sec7, sizeof(char), uint4(sec7), out);
    if (sec7) fwrite((void *)sec7, sizeof(char), 5, head);
    if (sec7) output_c(c, sec7, 5);
    fprintf(c,"};\n\n");

    s[0] = s[1] = s[2] = s[3] = 55; /* s = "7777" */
    fprintf(c,"unsigned char tail[4] = {55, 55, 55, 55};\n\n");
    fwrite((void *) s, sizeof(char), 4, out);
    fwrite((void *) s, sizeof(char), 4, tail);


    fprintf(c,"#define NDATA %u\n", ndata);

    i = 0;
    fprintf(c,"#define SEC0 0\n");
    fprintf(c,"#define DISCIPLINE %d\n",6);
    fprintf(c,"#define EDITION %d\n",7);

    i = 16;
    fprintf(c,"#define SEC1 %d\n",i);
    fprintf(c,"#define CENTER %d\n",i+5);
    fprintf(c,"#define SUBCENTER %d\n",i+7);
    fprintf(c,"#define MASTERTABLE %d\n",i+9);
    fprintf(c,"#define LOCALTABLE %d\n",i+9);
    fprintf(c,"#define YEAR %d\n",i+12);
    fprintf(c,"#define MONTH %d\n",i+14);
    fprintf(c,"#define DAY %d\n",i+15);
    fprintf(c,"#define HOUR %d\n",i+16);
    fprintf(c,"#define MINUTE %d\n",i+17);
    fprintf(c,"#define SECOND %d\n",i+18);

    i = i +  uint4(sec1);
    if (sec2) {
        fprintf(c,"#define SEC2 %d\n",i);
	i = i +  uint4(sec2);
    }

    if (sec3) {
        fprintf(c,"#define SEC3 %d\n",i);
	i = i +  uint4(sec3);
    }

    if (sec4) {
        fprintf(c,"#define SEC4 %d\n",i);
        fprintf(c,"#define PRODUCTDEFTEMPLATENUM %d\n",i+7);
        fprintf(c,"#define PRODUCTDEFTEMPLATE %d\n",i+9);
        fprintf(c,"#define PRODUCTCATEGORY %d\n",i+9);
        fprintf(c,"#define PRODUCTNUMBER %d\n",i+10);

	i = i +  uint4(sec4);
    }
    if (sec5) {
        fprintf(c,"#define SEC5 %d\n",i);
	i = i +  uint4(sec5);
    }
    if (sec6) {
        fprintf(c,"#define SEC6 %d\n",i);
	i = i +  uint4(sec6);
    }
    if (sec7) {
        fprintf(c,"#define SEC7 %d\n",i);
	i = i +  uint4(sec4);
    }

    /* write out all the sections */

    fprintf(c,"unsigned char sec0[] = { ");
    for (i = 0; i < 8; i++) {
	fprintf(c,"%u, ", sec0[i]);
    }
    fprintf(c,"};\n");

    j = uint4(sec1);
    fprintf(c,"unsigned char sec1[] = { ");
    for (i = 0; i < j; i++) {
	fprintf(c,"%u, ", sec1[i]);
    }
    fprintf(c,"};\n");

    if (sec2) {
	j = uint4(sec2);
        fprintf(c,"unsigned char sec2[] = { ");
        for (i = 0; i < j; i++) {
	    fprintf(c,"%u, ", sec2[i]);
        }
        fprintf(c,"};\n");
    }

    if (sec3) {
	j = uint4(sec3);
        fprintf(c,"unsigned char sec3[] = { ");
        for (i = 0; i < j; i++) {
            if (i % 20 == 0) fprintf(c, "\n");
	    fprintf(c,"%u, ", sec3[i]);
        }
        fprintf(c,"};\n");
    }

    if (sec4) {
	j = uint4(sec4);
        fprintf(c,"unsigned char sec4[] = { ");
        for (i = 0; i < j; i++) {
            if (i % 20 == 0) fprintf(c, "\n");
	    fprintf(c,"%u, ", sec4[i]);
        }
        fprintf(c,"};\n");
    }
    fprintf(c,"unsigned char sec7[4] = {55, 55, 55, 55};\n");


    free(sec5);
    free(sec6);
    free(sec7);

//    /* set scan mode to original order */
//    set_flag_table_3_4(sec, flag);
    
    return 0;
}

static int output_c(FILE *c, unsigned char *s, unsigned int n) {
    static unsigned int j = 0;
    while (n-- > 0) {
        if (j++ % 20 == 0) fprintf(c, "\n");
        fprintf(c," %u,", *s++);
    }
    return 0;
}



