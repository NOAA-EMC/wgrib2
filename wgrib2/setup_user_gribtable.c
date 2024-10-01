#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"

/* setup_user_gribtable.c        10/2024   Public Domain   Wesley Ebisuzaki
 *
 * read user defined grib table
 * add user defined grib table to wgrib2's own grib table
 *
 * source of user defined gribtable
 *
 * $GRIB2TABLE      (environment variable)
 * $grib2table      (environment variable)
 * gribtable        (file)
 */

struct gribtable_s *user_gribtable = NULL;

#define LINELEN 300
#define DELIM ':'

void setup_user_gribtable(void) {

    char *filename, line[LINELEN];
    char name[LINELEN], desc[LINELEN], units[LINELEN];
    int disc;
    int mtab_set;
    int mtab_low;
    int mtab_high;
    int cntr;
    int ltab;
    int pcat;
    int pnum; 

    FILE *input;
    int nline, k, cnt, i, j;
 
    user_gribtable = NULL;
    filename = getenv("GRIB2TABLE");
    if (filename == NULL) filename = getenv("grib2table");
    if (filename == NULL) filename = "grib2table";

    if ( (input = fopen(filename,"r")) == NULL) return;
//    printf("scanning %s\n", filename);
    nline = 0;
    while (fgets(line, LINELEN, input)) {
        if (line[0] == '#' || line[0] == '!' || line[0] == '*') continue;
	cnt = 0;
	for (i = 0; i < strlen(line); i++) {
	    if (line[i] == DELIM) cnt++;
	}
	if (cnt == 10) nline++;
    }
//    printf("scanning found %d lines\n", nline);
    if (nline == 0) {
	fclose(input);
	return;
    }	
    rewind(input);
    user_gribtable = malloc((nline + 1) * sizeof (struct gribtable_s));
    if (user_gribtable == NULL) fatal_error("user_gribtable: memory allocation","");

    k = 0;
    while (fgets(line, LINELEN, input)) {
        if (line[0] == '#' || line[0] == '!' || line[0] == '*') continue;
	cnt = 0;
	for (i = 0; i < strlen(line); i++) {
	    if (line[i] == DELIM) cnt++;
	}
	if (cnt > 2 && cnt != 10) {
	    fprintf(stderr,"user_gribtable: ignoring %s", line);
	}
	if (cnt == 10) {
	    j = sscanf(line,"%d:%d:%d:%d:%d:%d:%d:%d:%[^:]:%[^:]:%[^:\n\r]", &disc, &mtab_set, &mtab_low, &mtab_high, 
			&cntr, &ltab, &pcat,&pnum,name,desc,units);
	    if (j == 11) {
		user_gribtable[k].disc = disc;
		user_gribtable[k].mtab_set = mtab_set;
		user_gribtable[k].mtab_low = mtab_low;
		user_gribtable[k].mtab_high = mtab_high;
		user_gribtable[k].cntr = cntr;
		user_gribtable[k].ltab = ltab;
		user_gribtable[k].pcat = pcat;
		user_gribtable[k].pnum = pnum;

		i = strlen(name);
		user_gribtable[k].name = malloc(i+1);
		if (user_gribtable[k].name == NULL) fatal_error("user_gribtable: memory allocation","");
		memcpy((void *) user_gribtable[k].name, name, i+1);

		i = strlen(desc);
		user_gribtable[k].desc = malloc(i+1);
		if (user_gribtable[k].desc == NULL) fatal_error("user_gribtable: memory allocation","");
		if (user_gribtable[k].desc == NULL) fatal_error("user_gribtable: memory allocation","");
		memcpy((void *) user_gribtable[k].desc, desc, i+1);

		i = strlen(units);
		user_gribtable[k].unit = malloc(i+1);
		if (user_gribtable[k].unit == NULL) fatal_error("user_gribtable: memory allocation","");
		if (user_gribtable[k].unit == NULL) fatal_error("user_gribtable: memory allocation","");
		memcpy((void *) user_gribtable[k].unit, units, i+1);

	        k++;
	    }
	    else {
	        fprintf(stderr,"user_gribtable: ignoring #2 %s", line);
	    }

// 	 fprintf(stderr,"user_gribtab: j=%d %d %d %d %d %d %d (%s) (%s) (%s)\n", j, disc, mtab_set, 
//          cntr, ltab, pcat, pnum,name,desc,units);
        }
    }
    if (k != nline) fatal_error("user_gribtable: line match problem","");
    user_gribtable[k].disc = user_gribtable[k].mtab_set = user_gribtable[k].mtab_low = user_gribtable[k].mtab_high = -1;
    user_gribtable[k].cntr = user_gribtable[k].ltab = user_gribtable[k].pcat = -1;
    user_gribtable[k].name = user_gribtable[k].desc = user_gribtable[k].unit = NULL;
    fclose(input);
    return;
}
