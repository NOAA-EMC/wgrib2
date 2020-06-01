#include <stdio.h>
#include <stdlib.h>

/*
 * grib_split    w. ebisuzaki
 * public domain March 2013
 *
 * takes 1 grib2 file and splits it into N files  where N <= 32
 *
 * grib_split IN OUT-1 OUT-2 OUT-3 ... OUT-N
 *
 * IN and out can be pipes
 *
 * v1.0     3/2013 W. Ebisuzaki (public release version)
 * v1.0a    5/2013 W. Ebisuzaki fix:  long int len -> unsigned long int len
 *                              decl of fatal_error, and minor edit to remove clang warning
 */


unsigned char *rd_grib2_msg_seq(FILE *input, long int *pos, unsigned long int *len, int *num_submsgs);
void fatal_error(const char *fmt, const char *string);

#define N 32

int main(int argc, char **argv) {

    FILE *in, *p[N];
    int i, n, done;
    unsigned char *grib2;
    long int pos;
    unsigned long int len;
    int num_submsgs, err;
    int last_wrt;

    if (argc < 4) {
	fprintf(stderr,"grib_split: input-grib2 (list of grib2 output files/pipes)\n");
	fprintf(stderr," takes a grib2 file and splits in into N pieces\n");
	exit(8);
    }
    if ((in = fopen(argv[1],"rb")) == NULL) {
	fprintf(stderr,"bad arg: input grib file=%s\n",argv[1]);
	exit(8);
    }
    n = argc - 2;
    if (n > N) {
	fprintf(stderr,"Too many output files (max=%d), change N and recompile\n", N);
	exit(8);
    }

    for (i = 0; i < n; i++) {
	p[i] = fopen(argv[i+2], "wb");
	if (p[i] == NULL) {
	    fprintf(stderr,"bad file: output file=%s\n",argv[i+2]);
	    exit(8);
	}
	/* change size of buffer */
    }

    pos = i = 0;
    last_wrt = -1;
    while ( (grib2 = rd_grib2_msg_seq(in, &pos, &len, &num_submsgs)) != NULL) {
	err = (int) fwrite((void *) grib2,  1, len, p[i]);
	if (err != len) {
	    fatal_error("problem writing to file/pipe %s", argv[i+2]);
	}
	// fflush(p[i]);
 	if (last_wrt != -1) fflush(p[last_wrt]);
	last_wrt = i;
	i = (i == n-1) ? 0 : i+1;
        pos += len;
    }
    exit(0);
}
