#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"

/*
 * time range: ascii to int, int to ascii
 */

static struct tr_table_struct {
	const char *name; const int val;
} tr_table [] = {
	{ "min", 0 },
	{ "hour", 1 }, 
	{ "day", 2 },
	{ "month", 3 },
	{ "year", 4 },
	{ "decade", 5 },
	{ "normal", 6 },
	{ "century", 7 },
	{ "3-hours", 10 },
	{ "6-hours", 11 },
	{ "12-hours", 12 },
	{ "sec", 13 },
	{ "missing", 255 },
};

int a2time_range(const char * string) {
    int i;
    for (i = 0; i < sizeof (tr_table) / sizeof(struct tr_table_struct); i++) {
	if (strcmp(string,tr_table[i].name) == 0) return tr_table[i].val;
    }
    return -1;
}

const char *time_range2a(int tr) {
    int i;
    for (i = 0; i < sizeof (tr_table) / sizeof(struct tr_table_struct); i++) {
	if (tr_table[i].val == tr) return tr_table[i].name;
    }
    return "?";
}

/*
 * converts units to single digit units
 * i.e. 3 hour -> hours
 */

int normalize_time_range(int *tr, int *val) {
    switch(*tr) {
	case 10:		// 3 hours
	    *tr = 1;
	    *val *= 3;
	    break;
	case 11:		// 6 hours
	    *tr = 1;
	    *val *= 6;
	    break;
	case 12:		// 12 hours
	    *tr = 1;
	    *val *= 12;
	    break;
    }
    return 0;
}

void simple_time_range(int *tr, int *val) {

    if (*tr == 10) {	// 3 hours
	*tr = 1;
	*val *= 3;
    }
    else if (*tr == 11) {	// 6 hours
	*tr = 1;
	*val *= 6;
    }
    else if (*tr == 12) {	// 12 hours
	*tr = 1;
	*val *= 12;
    }

    if (*tr == 13 && (*val % 60 == 0) && (*val != 0)) {		// seconds
	*val /= 60;
	*tr = 0;						// minutes
    }
    if (*tr == 0 && (*val % 60 == 0) && (*val != 0)) {		// minutes
	*val /= 60;
	*tr = 1;						// hours
    }
    if (*tr == 1 && (*val % 24 == 0) && (*val != 0)) {		// hours
	*val /= 24;
	*tr = 2;						// days
    }
}

/*
 * a2code_4_10(const char *string)
 *  converts a string to code 4.10
 */
int a2code_4_10(const char *string) {
        int i;

        i = -1;
        if (strcmp(string,"ave") == 0) i = 0;
        else if (strcmp(string,"acc") == 0) i = 1;
        else if (strcmp(string,"max") == 0) i = 2;
        else if (strcmp(string,"min") == 0) i = 3;
        else if (strcmp(string,"last-first") == 0) i = 4;
        else if (strcmp(string,"RMS") == 0) i = 5;
        else if (strcmp(string,"StdDev") == 0) i = 6;
        else if (strcmp(string,"covar") == 0) i = 7;
        else if (strcmp(string,"first-last") == 0) i = 8;
        return i;
}

const char *code_4_10_name(int code_4_10) {
    const char *string;
    
    string = "???";
    switch(code_4_10) {
#include           "CodeTable_4.10.dat"
    }
    return string;
}


/*
 * a2anl_fcst(const char *string)
 *
 * returns 0 == anl
 *         1 == fcst
 *        -1    not above
 */
int a2anl_fcst(const char *string) {
	int i;
	i = -1;
        if (strcmp(string,"anl") == 0) i = 0;
        else if (strcmp(string,"fcst") == 0) i = 1;
	return i;
}
