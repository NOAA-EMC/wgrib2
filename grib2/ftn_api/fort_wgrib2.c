#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * wgrib2c: a fortran callable wrapper for callable wgrib2
 *
 * 5/2015 Wesley Ebisuzaki   Public Domain
 *
 */

int wgrib2(int argc, char **argv);
int wgrib2c(int n, const char *lines, int len);

int wgrib2c(int n, const char *lines, int len) {
    int i, argc;
    char *buf, **argv, *s, *t;

//    fprintf(stderr,">> wgrib2c n=%d len=%d\n", n, len);

    argc = n + 1;
    buf = (char *) malloc( (argc - 1) * (len+1));
    argv = (char **) malloc (argc * sizeof(char *));

    if (buf == NULL || argv == NULL) {
	fprintf(stderr,"wgrib2c memory error-not enough free memory\n");
	return 1;
    }

    argv[0] = "wgrib2c";

    for (i = 1; i < argc; i++) {
	argv[i] = buf + (i-1)*(len+1);
	strncpy(argv[i], lines+(i-1)*len, len);
	argv[i][len] = 0;
	s = argv[i];
	t = argv[i]+len-1;
	while (t >= s && *t == ' ') *t-- = '\0';
    }
//    for (i = 0; i < argc; i++) {
//	fprintf(stderr,"wgrib2c: %d (%s)\n", i, argv[i]);
//    }
    i = wgrib2(argc, argv);
//    fprintf(stderr,"wgrib2 errcode=%d\n", i);

    free(buf);
    free(argv);
    return i;
}
