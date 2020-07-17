#include <stdio.h>
#include <stdlib.h>
//
// 4 files .. same fields but with different packing
// returns smaller of indiv fields
//
// fixed 3/4/2010 - used realloc rather than malloc, use big buffers
// 1/2010 added fflush
// 2/2011 added 4 files

#define SIZE 4096*4
unsigned long int uint8(unsigned char *p);

int main(int argc, char **argv) {

    FILE *out, *in1, *in2, *in3, *in4;
    unsigned char *buffer1, *buffer2, *buffer3, *buffer4;

    int size1, size2, size3, size4;
    int i, k, count, smallest, size[5];

    if (argc != 6) {
	fprintf(stderr,"bad arg: output in-1 in-2 in-3 in-4\n");
	fprintf(stderr," selects smallest grib message: 1 of 4\n");
	fprintf(stderr," for optimal grib compression\n");
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
    if ((in4 = fopen(argv[5],"rb")) == NULL) {
	fprintf(stderr,"bad arg: in-4=%s\n",argv[5]);
	exit(8);
    }

    buffer1 = malloc(SIZE);
    buffer2 = malloc(SIZE);
    buffer3 = malloc(SIZE);
    buffer4 = malloc(SIZE);

    if (buffer1 == NULL || buffer2 == NULL || buffer3 == NULL || buffer4 == NULL ) {
	fprintf(stderr,"not enough memory\n");
	exit(8);
    }
    size1 = size2 = size3 = size4 = SIZE;

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
    		if (buffer1 == NULL) { fprintf(stderr,"not enough memory\n"); exit(8); }
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
                if (buffer2 == NULL) { fprintf(stderr,"not enough memory\n"); exit(8); }
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
                if (buffer3 == NULL) { fprintf(stderr,"not enough memory\n"); exit(8); }
        }
        k = fread(buffer3+16,1,size[3]-16,in3);
        if (k != size[3]-16) exit(4);

        // read message4

        i = fread(buffer4, 1, 16, in4);
        if (i != 16) break;
        if (buffer4[0] != 'G' || buffer4[1] != 'R' || buffer4[2] != 'I' ||
                buffer4[3] != 'B') exit(1);
                size[4] = uint8(&(buffer4[8]));
                if (size4 < size[4]) {
                size4 = size[4];
                buffer4 = realloc(buffer4, size4);
                if (buffer4 == NULL) { fprintf(stderr,"not enough memory\n"); exit(8); }
        }
        k = fread(buffer4+16,1,size[4]-16,in4);
        if (k != size[4]-16) exit(4);

	smallest = 1;
	if (size[2] < size[smallest]) smallest = 2;
	if (size[3] < size[smallest]) smallest = 3;
	if (size[4] < size[smallest]) smallest = 4;

printf("code=%d n1 %d n2 %d n3 %d n4 %d\n", smallest, size[1], size[2], size[3], size[4]);

        if (smallest == 1) fwrite(buffer1,1,size[1],out);
        if (smallest == 2) fwrite(buffer2,1,size[2],out);
        if (smallest == 3) fwrite(buffer3,1,size[3],out);
        if (smallest == 4) fwrite(buffer4,1,size[4],out);
	fflush(out);
	count++;
    }
    printf("records = %d\n", count);
    return 0;
}
