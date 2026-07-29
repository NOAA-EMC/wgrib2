/** @file
 * @brief Selects the shortest (byte length) GRIB message from three input files and writes
 * to the output file/pipe.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2010 | W. Ebisuzaki | Initial
 * 03/4/2010 | W. Ebisuzaki | used realloc rather than malloc, use big buffers
 * 01/2010 | W. Ebisuzaki | added fflush
 * 03/2014 | W. Ebisuzaki | added line number to printf
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 05/2009
 */

#include <stdio.h>
#include <stdlib.h>

#include "aux_progs.h"

/** Maximum Buffer Size */
#define SIZE 4096*4

/**
 * Takes 3 input files/pipes, reads 1 grib message from each, and writes the shortest message 
 * (by byte length) to the output file/pipe. This process continues until one file runs out
 * of data.
 * 
 * This program is used for compression.
 * 
 * ## Example
 * @code{.sh}
 * mkfifo pipe1 pipe2 pipe3
 * wgrib2 IN.grb -set_grib_type c1 -grib_out pipe1 \
 *      -set_grib_type c2 -grib_out pipe2 \
 *      -set_grib_type c3 -grib_out pipe3 &
 * smallest_grib2 OUT.grb pipe1 pipe2 pipe3
 * rm pipe1 pipe2 pipe3
 * @endcode
 * 
 * OUT.grb contains the same data as IN.grb but using the best complex packing (c1, c2, or c3).
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 *
 * @return 
 * - 0 :: Success
 * - 1 :: First 4 bytes not "GRIB"
 * - 4 :: Read error occurred
 * - 8 :: Exact error is written to stderr
 *
 * @author Wesley Ebisuzaki @date 2010
 */
int main(int argc, char **argv) {

    FILE *out, *in1, *in2, *in3;
    unsigned char *buffer1, *buffer2, *buffer3;
    int size1, size2, size3;
    int i, k, count, smallest, size[4];

    if (argc != 5) {
        fprintf(stderr,"bad arg: %s output in-1 in-2 in-3\n", argv[0]);
        fprintf(stderr,"selects the smallest grib message and writes it to output\n");
        exit(8);
    }
    if ((out = fopen(argv[1],"wb")) == NULL) {
        fprintf(stderr,"bad arg: output=%s\n",argv[1]);
        exit(8);
    }
    if ((in1 = fopen(argv[2],"rb")) == NULL) {
        fprintf(stderr,"bad arg: in-1=%s\n",argv[2]);
        exit(8);
    }
    if ((in2 = fopen(argv[3],"rb")) == NULL) {
        fprintf(stderr,"bad arg: in-2=%s\n",argv[3]);
        exit(8);
    }
    if ((in3 = fopen(argv[4],"rb")) == NULL) {
        fprintf(stderr,"bad arg: in-3=%s\n",argv[4]);
        exit(8);
    }

    buffer1 = malloc(SIZE);
    buffer2 = malloc(SIZE);
    buffer3 = malloc(SIZE);
    if (buffer1 == NULL || buffer2 == NULL || buffer3 == NULL) {
        fprintf(stderr,"not enough memory\n");
        exit(8);
    }
    size1 = size2 = size3 = SIZE;


    count = 0;
    while (1 == 1) {

        // read message1

        i = fread(buffer1, 1, 16, in1);
        if (i != 16) break;
        if (buffer1[0] != 'G' || buffer1[1] != 'R' || buffer1[2] != 'I' || 
            buffer1[3] != 'B') exit(1);
        size[1] = uint8(&(buffer1[8]));
        if (size1 < size[1]) {
            size1 = size[1];
            buffer1 = realloc(buffer1, size1);
            if (buffer1 == NULL) { 
                fprintf(stderr,"not enough memory\n");
                exit(8);
            }
        }
        k = fread(buffer1+16,1,size[1]-16,in1);
        if (k != size[1]-16) exit(4);


        // read message2

        i = fread(buffer2, 1, 16, in2);
        if (i != 16) break;
        if (buffer2[0] != 'G' || buffer2[1] != 'R' || buffer2[2] != 'I' ||
            buffer2[3] != 'B') exit(1);
        size[2] = uint8(&(buffer2[8]));
        if (size2 < size[2]) {
            size2 = size[2];
            buffer2 = realloc(buffer2, size2);
            if (buffer2 == NULL) { 
                fprintf(stderr,"not enough memory\n");
                exit(8);
            }
        }
        k = fread(buffer2+16,1,size[2]-16,in2);
        if (k != size[2]-16) exit(4);

        // read message3

        i = fread(buffer3, 1, 16, in3);
        if (i != 16) break;
        if (buffer3[0] != 'G' || buffer3[1] != 'R' || buffer3[2] != 'I' ||
            buffer3[3] != 'B') exit(1);
        size[3] = uint8(&(buffer3[8]));
        if (size3 < size[3]) {
            size3 = size[3];
            buffer3 = realloc(buffer3, size3);
            if (buffer3 == NULL) {
                fprintf(stderr,"not enough memory\n");
                exit(8);
            }
        }
        k = fread(buffer3+16,1,size[3]-16,in3);
        if (k != size[3]-16) exit(4);

        smallest = 1;
        if (size[2] < size[smallest]) smallest = 2;
        if (size[3] < size[smallest]) smallest = 3;

        count++;
        printf("%d: code=%d n1 %d n2 %d n3 %d\n", count, smallest, size[1], size[2], size[3]);

        if (smallest == 1) fwrite(buffer1,1,size[1],out);
        if (smallest == 2) fwrite(buffer2,1,size[2],out);
        if (smallest == 3) fwrite(buffer3,1,size[3],out);
        fflush(out);
    }
    printf("records = %d\n", count);
    return 0;
}
