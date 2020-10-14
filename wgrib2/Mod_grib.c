#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * the set options
 *
 * routines to modify grib fields
 *
 * 3/2008 Public Domain by Wesley Ebisuzaki
 *
 */

extern int header, decode;
extern const char *level_table[192];
extern const char *ncep_level_table[64];
extern int use_scale, dec_scale, bin_scale;

/*
 * this takes a string and returns a fixed surface
 */

int parse_level1(unsigned char **sec, const char *string, int *table_4_5, int *scale_factor, int *scale_value) {

    int i, n_percent, slen, n, ival, len_string, center;
    const char *t;
    char tmp_string[100];
    float val;
    double dval;

    len_string = strlen(string);
    for (i = 1; i < 192; i++) {		// can skip i==0 because it is not allowed resevered
        if (strcmp("reserved", level_table[i]) == 0) continue;
        // count the number of % characters in the level table
        n_percent = 0;
        t = level_table[i];
	/* %% -> quoted %, otherwise a read */

        while (*t) { 
	    if (*t++ == '%') {
		if (*t == 0) break;
	        if (*t++ != '%') n_percent++; 
	    }
	}
        if (n_percent == 0) {
            if (strcmp(string, level_table[i]) == 0) {
                *table_4_5 = i;
		*scale_factor = *scale_value = 255;
                return 0;
            }
        }
	else if (n_percent == 1) {
	    slen=strlen(level_table[i]);
	    if (slen > sizeof(tmp_string) - 3) fatal_error("parse_level1: string overflow","");
	    strncpy(tmp_string,level_table[i],slen);
	    strncpy(tmp_string+slen,"%n",3);

            if (n = -1, sscanf(string,tmp_string,&val,&n), n == len_string) {
                 dval = (double) val;
                 *table_4_5 = i;
                 if (i == 100 || i == 108) dval = dval * 100; // convert mb to Pa

                 best_scaled_value(dval, &n, &ival);
                 *scale_factor = INT1(n);
		 *scale_value = ival;
                 return 0;
             }
        }

    }

    /* check local level tables (see Level.c) */
    center = GB2_Center(sec);
    if (center == NCEP) {
        for (i = 0; i < 64; i++) {
            if (strcmp("reserved", ncep_level_table[i]) == 0) continue;
            n_percent = 0;
            t = ncep_level_table[i];
	    /* %% -> quoted %, otherwise a read */

            while (*t) { 
	        if (*t++ == '%') {
		    if (*t == 0) break;
	            if (*t++ != '%') n_percent++; 
	        }
	    }
            if (n_percent == 0) {
                if (strcmp(string, ncep_level_table[i]) == 0) {
                    *table_4_5 = i + 192;
		    *scale_factor = *scale_value = 255;
                    return 0;
                }
            }
	    else if (n_percent == 1) {
	        slen=strlen(ncep_level_table[i]);
	        if (slen > sizeof(tmp_string) - 3) fatal_error("parse_level1: string overflow","");
	        strncpy(tmp_string,ncep_level_table[i],slen);
	        strncpy(tmp_string+slen,"%n",3);

                if (n = -1, sscanf(string,tmp_string,&val,&n), n == len_string) {
                    dval = (double) val;
                    *table_4_5 = i + 192;
                    if (i + 192 == 235) dval = dval * 10; // convert C to 1/10C

                    best_scaled_value(dval, &n, &ival);
                    *scale_factor = INT1(n);
		    *scale_value = ival;
                    return 0;
		}
            }
        }
    }

    return 1;
}

/*
 * HEADER:100:set_lev:misc:1:changes level code .. not complete
 */

int f_set_lev(ARG1) {
    unsigned char *p1, *p2;
    float val1, val2;
    int i, n, len_arg1, ival, dash;
    char string[STRING_SIZE];
    char layer_type1[20], layer_type2[20];

    int table_4_5, scale_factor, scale_value;
 
    if (mode < 0) return 0;

    len_arg1 = strlen(arg1);
    if (len_arg1 > STRING_SIZE+1) fatal_error("set_lev: time string too long","");

if (mode == 99) fprintf(stderr,"set_lev: arg=%s\n", arg1);

    /* get fixed surface pointers */
    p1 = code_table_4_5a_location(sec);
    p2 = code_table_4_5b_location(sec);
    if (p1 == NULL) fatal_error("set_lev: PDT does not have fixed surfaces","");

    /* set fixed surface to undefined */

    for (i = 0; i < 6; i++) p1[i] = (unsigned char) 255;

    if (p2 != NULL) {
	for (i = 0; i < 6; i++) p2[i] = (unsigned char) 255;
    }

    // (level)

    if (parse_level1(sec, arg1, &table_4_5, &scale_factor, &scale_value) == 0) {
	p1[0] = table_4_5;
        p1[1] = INT1(scale_factor);
        int_char(scale_value, p1+2);
        return 0;
    }

    // (level) - (level)
    if (p2 != NULL) {
        for (dash = 1; dash < len_arg1-4; dash++) {
            if (arg1[dash] != ' ') continue;
            if (arg1[dash+1] != '-') continue;
            if (arg1[dash+2] != ' ') continue;
            for (i = 0; i < dash; i++) {
	        string[i] = arg1[i];
	    }
	    string[dash] = 0;
            if (parse_level1(sec, string, &table_4_5, &scale_factor, &scale_value) == 0) {
	        p1[0] = table_4_5;
                p1[1] = INT1(scale_factor);
                int_char(scale_value, p1+2);
                for (i = dash+3; i <= len_arg1; i++) {
	            string[i-dash-3] = arg1[i];
	        }
                if (parse_level1(sec, string, &table_4_5, &scale_factor, &scale_value) == 0) {
	            p2[0] = table_4_5;
                    p2[1] = INT1(scale_factor);
                    int_char(scale_value, p2+2);
		    return 0;
	        }
	        else {
                    for (i = 0; i < 6; i++) p1[i] = (unsigned char) 255;
	        }
	    }
        }
    }

if (mode == 99) fprintf(stderr,"in set_lev arg1=%s\n", arg1);

    // n-n (string) layer
    if (n=-1, sscanf(arg1,"%g-%g %20s layer%n",&val1,&val2, layer_type1, &n), n == len_arg1) {
       layer_type1[19] = 0;
       i = -1;
       if (strncmp(layer_type1,"sigma",20) == 0)  i = 104;
       else if (strncmp(layer_type1,"hybrid",20) == 0)  i = 105;
       else if (strncmp(layer_type1,"Eta",20) == 0)  i = 111;
       if (i != -1) {
	    p1[0] = p2[0] = i;
            best_scaled_value(val1, &n, &ival);
            p1[1] = INT1(n);
            int_char(ival, p1+2);

            best_scaled_value(val2, &n, &ival);
            p2[1] = INT1(n);
            int_char(ival, p2+2);

            return 0;
        }
    }

    // n-n (string) (string) layer
    if (n=-1, sscanf(arg1,"%g-%g %20s %20s layer%n",&val1,&val2, layer_type1, layer_type2, &n), n == len_arg1) {
       layer_type1[19] = layer_type2[19] = 0;
       i = -1;
       if (strncmp(layer_type1,"sigma",20) == 0 && strncmp(layer_type2,"height",20) == 0)  i = 115;
       else if (strncmp(layer_type1,"hybrid",20) == 0 && strncmp(layer_type2,"height",20) == 0)  i = 118;
       else if (strncmp(layer_type1,"hybrid",20) == 0 && strncmp(layer_type2,"pressure",20) == 0)  i = 119;
       else if (strncmp(layer_type1,"m",20) == 0 && strncmp(layer_type2,"ocean",20) == 0)  i = 161;
       if (i != -1) {
	    p1[0] = p2[0] = i;
            best_scaled_value(val1, &n, &ival);
            p1[1] = INT1(n);
            int_char(ival, p1+2);

            best_scaled_value(val2, &n, &ival);
            p2[1] = INT1(n);
            int_char(ival, p2+2);

            return 0;
        }
    }

    if (n=-1, sscanf(arg1,"%g-%g mb above ground%n",&val1,&val2,&n), n == len_arg1) {
	if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
	val1 *= 100.0;
	val2 *= 100.0;
	p1[0] = p2[0] = 108;
	p1[1] = 0;
	int_char((int) val1, p1+2);
	p2[1] = 0;
	int_char((int) val2, p2+2);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g mb%n",&val1,&val2,&n), n == len_arg1) {
	if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
	val1 *= 100.0;
	val2 *= 100.0;
	p1[0] = p2[0] = 100;
	p1[1] = 0;
	int_char((int) val1, p1+2);
	p2[1] = 0;
	int_char((int) val2, p2+2);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g m below ground%n",&val1,&val2,&n), n == len_arg1) {
	if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
	p1[0] = p2[0] = 106;

	best_scaled_value(val1, &n, &ival);
	p1[1] = INT1(n);
	int_char(ival, p1+2);

	best_scaled_value(val2, &n, &ival);
	p2[1] = INT1(n);
	int_char(ival, p2+2);
        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g m above ground%n",&val1,&val2,&n), n == len_arg1) {
	if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
	p1[0] = p2[0] = 103;

        best_scaled_value(val1, &n, &ival);
        p1[1] = INT1(n);
        int_char(ival, p1+2);

        best_scaled_value(val2, &n, &ival);
        p2[1] = INT1(n);
        int_char(ival, p2+2);

        return 0;
    }
    if (n=-1, sscanf(arg1,"%g-%g m below sea level%n",&val1,&val2,&n), n == len_arg1) {
	if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
	p1[0] = p2[0] = 160;

        best_scaled_value(val1, &n, &ival);
        p1[1] = INT1(n);
        int_char(ival, p1+2);

        best_scaled_value(val2, &n, &ival);
        p2[1] = INT1(n);
        int_char(ival, p2+2);

        return 0;
    }
    if (strcmp("atmos col", arg1) == 0 ||	// wgrib2 compatible
            strcmp("Entire atmosphere (considered as a single layer)", arg1) == 0) {
	if (p2 == NULL) fatal_error("set_lev: PDT has only 1 fixed surface, set_lev needs 2","");
        p1[0] = 1;
        p2[0] = 8;
        return 0;
    }

    fatal_error("need to modify set_lev for %s", arg1);
    return 0;
}


extern struct gribtable_s gribtable[], *user_gribtable;

/*
 * HEADER:100:set_var:misc:1:changes variable name
 *
 * 1.1 2/2012 WNE: old return 1st match
 *                 new match that is not in local tables,
 *                 if nothing, return match in local tables
 *                   not perfect, doesn't know center.
 * 1.2 4/2017 WNE: understands 
 *                 var discipline=0 center=34 local_table=1 parmcat=1 parm=203
 *                 var discipline=10 master_table=2 parmcat=0 parm=11
 *                 var10_2_1_7_0_11
 */



int f_set_var(ARG1) {
    struct gribtable_s *p;
    int i;
    unsigned int discipline, mastertab, localtab, center, parmcat, parmnum;

    /* this function needs to be called with changing parameters */
    if (mode < 0) return 0;

    /* var discipline=0 center=34 local_table=1 parmcat=1 parm=203 */
    /* var discipline=10 master_table=2 parmcat=0 parm=11 */
    /* var10_2_1_7_0_11 */
    if (arg1[0] == 'v' && arg1[1] == 'a' && arg1[2] == 'r') {
        i = sscanf(arg1,"var%u_%u_%u_%u_%u_%u", &discipline, &mastertab, &localtab, &center, &parmcat, &parmnum);
	if (i == 6) {
            sec[0][6] = discipline;
            sec[1][9] = mastertab;
	    uint2_char(center,sec[1]+5);
            sec[1][10] = localtab;
            sec[4][9] = parmcat;
            sec[4][10] = parmnum;
	    return 0;
	}
	i = sscanf(arg1,"var discipline=%u master_table=%u parmcat=%u parm=%u",&discipline,&mastertab,&parmcat,&parmnum);
	if (i == 4) {
            sec[0][6] = discipline;
            sec[1][9] = mastertab;
            sec[4][9] = parmcat;
            sec[4][10] = parmnum;
	    return 0;
	}
        i = sscanf(arg1,"var discipline=%u center=%u local_table=%u parmcat=%u parm=%u",
                  &discipline, &center, &localtab, &parmcat, &parmnum);
	if (i == 5) {
            sec[0][6] = discipline;
	    uint2_char(center,sec[1]+5);
            sec[1][10] = localtab;
            sec[4][9] = parmcat;
            sec[4][10] = parmnum;
	    return 0;
	}
fprintf(stderr,">>> i = %d\n",i);
    }


        p = NULL;
        /* try user table */

        if (user_gribtable != NULL) {
            p = user_gribtable;
            center = GB2_Center(sec);
            while (p->disc != -1) {
                if (strcmp(arg1,p->name) == 0) {
                    if (center == p->cntr) break;
                    if (p->disc < 192 && p->pcat < 192 && p->pnum < 192) break;
                }
                p++;
             }
        }

        /* search for non-local table match first */
	if (p == NULL || p->disc == -1) {
	    p = gribtable;
            while (p->disc != -1) {
	        if (p->disc < 192 && p->pcat < 192 && p->pnum < 192 && strcmp(arg1,p->name) == 0) {
                    break;
                }
                p++;
            }
	}

        /* try local tables */
        if (p->disc == -1) {
            p = gribtable;
	    center = GB2_Center(sec);
            while (p->disc != -1) {
                if (center == p->cntr && strcmp(arg1,p->name) == 0) {
		    break;
	        }
	        p++;
	    }
        }

        if (p->disc == -1) fatal_error("set_var: could not find %s", arg1);
        sec[0][6] = p->disc;
        sec[1][9] = p->mtab_set;
        sec[1][10] = p->ltab;
        sec[4][9] = p->pcat;
        sec[4][10] = p->pnum;

    return 0;
}


/*
 * HEADER:-1:set_center:misc:1:changes center X = C or C:S     C and S are center/subcenter numbers
 */

int f_set_center(ARG1) {
   int i, center, subcenter;

   if (mode >= 0) {
       i = sscanf(arg1,"%d:%d", &center, &subcenter);
       if (i == 0) fatal_error("set_center: bad arg %s", arg1);
       int2_char(center, sec[1]+5);
       if (i == 2) int2_char(subcenter, sec[1]+7);
    }
    return 0;
}

/*
 * HEADER:100:set:misc:2:set X = Y, X=local_table,etc (help: -set help help)
 */

static const char *set_options="discipline, center, subcenter, master_table, local_table, background_process_id, "
        "analysis_or_forecast_process_id, model_version_date, chemical, table_1.2, table_1.3, table_1.4, "
        "table_3.0, table_3.1/GDT, table_3.2, " 
	"table_3.3, table_3.4, table_4.0/PDT, table_4.1, table_4.2, table_4.3, table_4.5a, table_4.5b, table_4.6, "
        "table_4.7, table_4.8, table_4.10, "
        "table_4.11, table_4.230, table_5.0/DRT, table_6.0, %";
extern struct codetable_4_230  codetable_4_230_table[];


int f_set(ARG2) {
    int i, j;
    double val;
    int year,mon,day,hr,minute,second;
    unsigned char *p;
    char *endptr;
    unsigned int k;

    if (mode == -1) {
	if (strcmp(arg1,"help") == 0) {
	    sprintf(inv_out,"-set: change values of %s\n", set_options);
	    return -1;
	}
	return 0;
    }

    if (mode >= 0) {


	if (strcmp(arg1,"data_*") == 0) {
	    val = strtod(arg2, &endptr);
	    if (*endptr == 0) {
		if (decode == 1 && data != NULL) {
#pragma omp parallel for private(k)
		    for (k = 0; k < ndata; k++) {
			if (DEFINED_VAL(data[k])) {
			    data[k] *= val;
			}
		    }
		}
		else {
		    fatal_error("set data_*: need option that uses grid data","");
		}
	    }
	    else {
		fatal_error("set data_*: bad value %s", arg2);
	    }
	    return 0;
	}

	if (strcmp(arg1,"data_+") == 0) {
	    val = strtod(arg2, &endptr);
	    if (*endptr == 0) {
		if (decode == 1 && data != NULL) {
#pragma omp parallel for private(k)
		    for (k = 0; k < ndata; k++) {
			if (DEFINED_VAL(data[k])) {
			    data[k] += val;
			}
		    }
		}
		else {
		    fatal_error("set data_+: need option that uses grid data","");
		}
	    }
	    else {
		fatal_error("set data_+: bad value %s", arg2);
	    }
	    return 0;
	}

	i = atoi(arg2);
	if (strcmp(arg1,"discipline") == 0 || strcmp(arg1,"table_0.0") == 0) {
	    sec[0][6] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"local_table") == 0 || strcmp(arg1,"table_1.1") == 0) {
	    sec[1][10] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"master_table") == 0 || strcmp(arg1,"table_1.0") == 0) {
	    sec[1][9] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"center") == 0) {
            int2_char(i, sec[1]+5);
	    return 0;
	}
	if (strcmp(arg1,"subcenter") == 0) {
            int2_char(i, sec[1]+7);
	    return 0;
	}
	if (strcmp(arg1,"background_process_id") == 0) {
	    p = background_generating_process_identifier_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"analysis_or_forecast_process_id") == 0) {
	    p = analysis_or_forecast_generating_process_identifier_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"model_version_date") == 0) {
            p = year_of_model_version_date_location(sec);
            if (p) {
                i = sscanf(arg2,"%4d%2d%2d%2d%2d%2d", &year,&mon,&day,&hr,&minute,&second);
		if (i != 6) fatal_error("set model_version_date YYYYMMDDHHmmSS","");
                uint2_char(year, p);
		p += 2;
		*p++ = mon;
		*p++ = day;
		*p++ = hr;
		*p++ = minute;
		*p++ = second;
            }
	    return 0;
        }
	if (strcmp(arg1,"table_1.2") == 0) {
	    sec[1][11] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_1.3") == 0) {
	    sec[1][19] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_1.4") == 0) {
	    sec[1][20] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_3.0") == 0) {
	    sec[3][5] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_3.1") == 0 || strcmp(arg1,"GDT") == 0) {
            uint2_char(i, sec[3]+12);
	    return 0;
	}
	if (strcmp(arg1,"table_3.2") == 0) {
	    p = code_table_3_2_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
        }
	if (strcmp(arg1,"table_3.3") == 0 || strcmp(arg1,"flag_table_3.3") == 0) {
	    return set_flag_table_3_3(sec, i);
	}
	if (strcmp(arg1,"table_3.4") == 0 || strcmp(arg1,"flag_table_3.4") == 0) {
	    return set_flag_table_3_4(sec, i);
	}
	if (strcmp(arg1,"table_4.0") == 0 || strcmp(arg1,"PDT") == 0) {
            uint2_char(i, sec[4]+7);
	    return 0;
	}
	if (strcmp(arg1,"table_4.1") == 0) {
	    sec[4][9] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.2") == 0) {
	    sec[4][10] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.3") == 0) {
	    p = code_table_4_3_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.5a") == 0) {
	    p = code_table_4_5a_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.5b") == 0) {
	    p = code_table_4_5b_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.6") == 0) {
	    p = code_table_4_6_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.7") == 0) {
	    p = code_table_4_7_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.8") == 0) {
	    p = code_table_4_8_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.10") == 0) {
	    p = code_table_4_10_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.11") == 0) {
	    p = code_table_4_11_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"table_4.230") == 0 || strcmp(arg1, "chemical") == 0) {		// chemical
	    p = code_table_4_230_location(sec);
	    if (p) {
		j = 0;
		while (codetable_4_230_table[j].no != 65535) {
		    if (strcmp(arg2, codetable_4_230_table[j].name) == 0) {
		        uint2_char(codetable_4_230_table[j].no, p);
			return 0;
		    }
		    j++;
		}
		if (i == 0 && arg2[0] != '0') fatal_error("set table_4.230/chemical wrong chemical=%s\n", arg2);
		uint2_char(i, p);
		return 0;
	    }
	    /* could not change chemical type */
	    fatal_error("set table_4.230/chemical wrong template chemical=%s\n", arg2);
	    return 0;
	}
	if (strcmp(arg1,"table_5.0") == 0 || strcmp(arg1,"DRT") == 0) {
            uint2_char(i, sec[5]+9);
	    return 0;
	}
	if (strcmp(arg1,"table_6.0") == 0) {
	    sec[6][5] = (unsigned char) i;
	    return 0;
	}
	if (strcmp(arg1,"%") == 0) {
	    p = percentile_value_location(sec);
	    if (p) *p = (unsigned char) i;
	    return 0;
	}

	fatal_error("set: allowed values: %s", set_options);

	return 1;
    }
    return 0;
}


/*
 * HEADER:100:set_ave:misc:1:set ave/acc .. only use on pdt=4.0/4.8 (old code)
 */

/* the old set_ftime/set_ave is being replaced by a new version of set_ftime */

int f_set_ave(ARG1) {
    int i, tr, tr2, len, len_arg1;
    char string[STRING_SIZE];
    char string2[STRING_SIZE];
    char string3[STRING_SIZE];
    const char *string4;
    static unsigned char new_sec4[58+12+12];		// now use n == 3 forms

    int year, month, day, hour, minute, second;
    int j,k,j2,k2, m, m2, missing;

    if (mode < 0) return 0;

    len_arg1 = strlen(arg1);

//  6 hour ave anl

    i = sscanf(arg1,"%d %s %s %s%n",&j,string,string2,string3,&len);
    if (len != len_arg1) i = 0;
    if (i == 4 && ((tr = a2time_range(string)) >= 0)) {

	for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];
	get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, j, tr);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);
        uint2_char(8, new_sec4+7);              // pdt = 8
        new_sec4[17] = tr;		    	// hour, etc
        int_char(0, new_sec4+18);              // start time
        new_sec4[41] = 1;                       // number of time ranges
        uint_char(0, new_sec4+42);              // missing

        if (strcmp(string2,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string2,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string2,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string2,"min") == 0) new_sec4[46] = 3;
        else if (strcmp(string2,"last-first") == 0) new_sec4[46] = 4;
        else if (strcmp(string2,"RMS") == 0) new_sec4[46] = 5;
        else if (strcmp(string2,"StdDev") == 0) new_sec4[46] = 6;
        else if (strcmp(string2,"covar") == 0) new_sec4[46] = 7;
        else if (strcmp(string2,"first-last") == 0) new_sec4[46] = 8;
	else fatal_error("set_ave: unknown statistical operator %s",string2);

        new_sec4[47] = 1;

        new_sec4[48] = tr;                       // hour
        uint_char(j, new_sec4+49);
        new_sec4[53] = tr;
        uint_char(0, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }

    // 1-5 hour ave fcst

    i = sscanf(arg1,"%d-%d %s %s %s%n",&j,&k,string3,string,string2,&len);
    if (len != len_arg1) i = 0;
    if (i == 5 && (tr = a2time_range(string3)) >= 0) {

	for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

	get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, k, tr);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);
        uint2_char(8, new_sec4+7);              // pdt = 8
        new_sec4[17] = tr;			// hour
        int_char(j, new_sec4+18);		// start time
        new_sec4[41] = 1;			// number of time ranges
        uint_char(0, new_sec4+42);		// missing

        if (strcmp(string,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string,"min") == 0) new_sec4[46] = 3;
        else if (strcmp(string,"last-first") == 0) new_sec4[46] = 4;
        else if (strcmp(string,"RMS") == 0) new_sec4[46] = 5;
        else if (strcmp(string,"StdDev") == 0) new_sec4[46] = 6;
        else if (strcmp(string,"covar") == 0) new_sec4[46] = 7;
        else if (strcmp(string,"first-last") == 0) new_sec4[46] = 8;
	else fatal_error("set_ave: unknown statistical operator %s", string);

        if (strcmp(string2,"anl") == 0) new_sec4[47] = 1;
        else if (strcmp(string2,"fcst") == 0) new_sec4[47] = 2;
	else fatal_error("set_ave: expecting anl/fcst got %s", string);

        new_sec4[48] = tr;			// hour
	uint_char(k-j, new_sec4+49);
        new_sec4[53] = tr;
        uint_char(0, new_sec4+54);
	sec[4] = &(new_sec4[0]);
        return 0;
    }

    // obsolete form: 1@6 hour ave anl,missing=0

    i = sscanf(arg1,"%d@%d %s ave anl,missing=%d%n",&j,&k,string,&missing,&len);
    if (len != len_arg1) i = 0;

    // new form 1@6 hour ave(anl),missing=0

    if (i != 4) {
        i = sscanf(arg1,"%d@%d %s ave(anl),missing=%d%n",&j,&k,string,&missing,&len);
        if (len != len_arg1) i = 0;
    }

    if (i == 4) {
	tr = a2time_range(string);
	if (tr == -1) fatal_error("set_ave: bad time range %s", string);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);		// length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr;                      // hour
        int_char(0, new_sec4+18);              // start time 0 = analysis

        new_sec4[41] = 1;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;                       // 0 = ave 1 = acc
        new_sec4[47] = 1;                       // 1 = start of forecast increased
	new_sec4[53] = new_sec4[48] = tr;
	uint_char((j-1)*k, new_sec4+49);
	uint_char(k, new_sec4+54);
	sec[4] = &(new_sec4[0]);
	return 0;
    }

// 1@6 hour ave(6 hour fcst),missing=0


    i = sscanf(arg1,"%d@%d %s ave(%d %s fcst),missing=%d%n",&j,&k,string, &m, string2, &missing,&len);
    if (len != len_arg1) i = 0;
    if (i == 6) {
        tr = a2time_range(string);
	if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
	if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18) ;              // start time 0 = analysis

        new_sec4[41] = 1;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;                       // 0 = ave 1 = acc
        new_sec4[47] = 1;                       // 1 = start of forecast increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }


// 1@6 hour ave(6 hour fcst)++,missing=0

    i = sscanf(arg1,"%d@%d %s ave(%d %s fcst)++,missing=%d%n",&j,&k,string, &m, string2, &missing,&len);
    if (len != len_arg1) i = 0;
    if (i == 6) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(58, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 1;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;                       // 0 = ave 1 = acc
        new_sec4[47] = 2;                       // 2 = forecast time is increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);
        sec[4] = &(new_sec4[0]);
        return 0;
    }


// 1@6 hour ave(0-6 hour ave fcst),missing=0

    i = sscanf(arg1,"%d@%d %s ave(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
    string4 = "ave";
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s acc(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "acc";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s min(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "min";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s max(%d-%d %s %s fcst),missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "max";
    }

    if (len != len_arg1) i = 0;
    if (i == 8) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m2, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(70, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 2;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

	// string4
        if (strcmp(string4,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string4,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string4,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string4,"min") == 0) new_sec4[46] = 3;
	else fatal_error("set_ave: unknown statistical operator %s",string4);

        new_sec4[47] = 1;                       // 1 = start of forecast increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);

        if (strcmp(string3,"ave") == 0) new_sec4[46+12] = 0;
        else if (strcmp(string3,"acc") == 0) new_sec4[46+12] = 1;
        else if (strcmp(string3,"max") == 0) new_sec4[46+12] = 2;
        else if (strcmp(string3,"min") == 0) new_sec4[46+12] = 3;
	else fatal_error("set_ave: unknown statistical operator %s",string3);
        // new_sec4[46+12] = 0;                       // 0 = ave 1 = acc
        new_sec4[47+12] = 2;                       // same forecast
        new_sec4[53+12] = new_sec4[48+12] = tr2;
        uint_char(m2-m , new_sec4+49+12);
        uint_char(0, new_sec4+54+12);

        sec[4] = &(new_sec4[0]);
        return 0;
    }

// 1@6 hour ave(0-6 hour ave fcst)++,missing=0

    i = sscanf(arg1,"%d@%d %s ave(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
    string4 = "ave";
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s acc(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "acc";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s min(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "min";
    }
    if (i != 8) {
        i = sscanf(arg1,"%d@%d %s max(%d-%d %s %s fcst)++,missing=%d%n",&j,&k,string, &m, &m2,string2, string3, &missing,&len);
        string4 = "max";
    }
    if (len != len_arg1) i = 0;
    if (i == 8) {
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, m2, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);

        uint_char(70, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr2;                     // forecast time range
        int_char(m, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 2;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        // string4
        if (strcmp(string4,"ave") == 0) new_sec4[46] = 0;
        else if (strcmp(string4,"acc") == 0) new_sec4[46] = 1;
        else if (strcmp(string4,"max") == 0) new_sec4[46] = 2;
        else if (strcmp(string4,"min") == 0) new_sec4[46] = 3;
        else fatal_error("set_ave: unknown statistical operator %s",string4);

        new_sec4[47] = 2;                       // 2 = fcst time increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);

        if (strcmp(string3,"ave") == 0) new_sec4[46+12] = 0;
        else if (strcmp(string3,"acc") == 0) new_sec4[46+12] = 1;
        else if (strcmp(string3,"max") == 0) new_sec4[46+12] = 2;
        else if (strcmp(string3,"min") == 0) new_sec4[46+12] = 3;
        else fatal_error("set_ave: unknown statistical operator %s",string3);
        // new_sec4[46+12] = 0;                       // 0 = ave 1 = acc
        new_sec4[47+12] = 2;                       // same forecast
        new_sec4[53+12] = new_sec4[48+12] = tr2;
        uint_char(m2-m , new_sec4+49+12);
        uint_char(0, new_sec4+54+12);

        sec[4] = &(new_sec4[0]);
        return 0;
    }




// old 10@30 year ave(124@6 hour ave anl)
// 10@30 year ave(124@6 hour ave (anl))

    i = sscanf(arg1,"%d@%d %s ave(%d@%d %s ave (anl)),missing=%d%n",&j,&k,string, &j2,&k2,string2, &missing,&len);

    if (len != len_arg1) i = 0;
    if (i == 7) {
fprintf(stderr,">>> need to check out climo1\n");
        tr = a2time_range(string);
        if (tr == -1) fatal_error("set_ave: bad time range %s", string);
        tr2 = a2time_range(string2);
        if (tr2 == -1) fatal_error("set_ave: bad time range %s", string2);

        for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];

        get_time(sec[1]+12, &year, &month, &day, &hour, &minute, &second);
        add_time(&year, &month, &day, &hour, &minute, &second, (j-1)*k, tr);
        add_time(&year, &month, &day, &hour, &minute, &second, (j2-1)*k2, tr2);
        save_time(year, month, day, hour, minute, second, new_sec4+34);
        

        uint_char(58, new_sec4);                // length of section
        uint2_char(8, new_sec4+7);              // pdt = 8

        new_sec4[17] = tr;                      // hour
        int_char(0, new_sec4+18);               // start time 0 = analysis

        new_sec4[41] = 2;                       // number of time ranges
        uint_char(missing, new_sec4+42);        // missing

        new_sec4[46] = 0;			// ave
        new_sec4[47] = 1;                       // 1 = start of forecast increased
        new_sec4[53] = new_sec4[48] = tr;
        uint_char((j-1)*k, new_sec4+49);
        uint_char(k, new_sec4+54);


        new_sec4[46+12] = 0;			// ave
        new_sec4[47+12] = 1;                    // 1 = start of forecast increased
        new_sec4[53+12] = new_sec4[48+12] = tr2;
        uint_char((j2-1)*k2, new_sec4+49+12);
        uint_char(k2, new_sec4+54+12);

        sec[4] = &(new_sec4[0]);
	return 0 ;
    }


// 10@30 year ave(124@6 hour ave 6 hour fcst)
// 10@30 year ave(124@6 hour ave(0-6 hour ave fcst)



    fatal_error("set_ave: not implemented %s", arg1);
    return 0;
}

/*
 * HEADER:-1:set_flag_table_3.3:misc:1:flag table 3.3 = X
 */

int f_set_flag_table_3_3(ARG1) {
    if (mode >= 0) return set_flag_table_3_3(sec, atoi(arg1));
    return 0;
}

/*
 * HEADER:-1:set_flag_table_3.4:misc:1:flag table 3.4 = X
 */

int f_set_flag_table_3_4(ARG1) {
    if (mode >= 0) return set_flag_table_3_4(sec, atoi(arg1));
    return 0;
}
