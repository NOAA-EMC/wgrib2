#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRSIZ 300
#define LOW 0
#define HIGH 255

int main(int argc, char **argv) {
    FILE *input;
    char line[STRSIZ], *rest, name[STRSIZ], desc[STRSIZ], unit[STRSIZ];    
    int i1, i2, i3, i4, i5, i6, i, n;

    if (argc == 0) exit(7);

    input = fopen(argv[1], "r");
    if (input == NULL) exit(8);

   while (fgets(line, STRSIZ, input)) {
	if (line[0] != '{' ) {
	    printf("%s", line);
	    continue;
	}
// { 0, 1, 8, 0, 192, 194, "DRYLIGHTNING", "Dry lightning", "??"}, 
	i = sscanf(line,"{%d,%d,%d,%d,%d,%d,%n",&i1,&i2,&i3,&i4,&i5,&i6,&n);
        if (i == 6) {
		printf("{%d,%d,%d,%d,%d,%d,%d,%d,%s", i1,i2,LOW,HIGH,i3,i4,i5,i6,line+n);
	}
	else {
	    fprintf(stderr,"***** error: line %s", line);
	}
    }
    return 0;
}

