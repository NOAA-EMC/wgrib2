#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
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

extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern int version_ftime;

/*
 * HEADER:100:set_metadata:misc:1:read meta-data for grib writing from file X
 */

int f_set_metadata(ARG1) {

    char line[STRING_SIZE+1];
    struct seq_file *save;
    int i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("set_metadata: memory allocation","");
        if (fopen_file(save, arg1, "r+") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
    }
    else if (mode >= 0) {
        save = *local;
	if (fgets_file(line, STRING_SIZE+1, save) == NULL) return 1;
        // i = set_metadata_string(CALL_ARG0, line);
        i = set_metadata_string(call_ARG1(inv_out,local,line));
	if (i) fatal_error("set_metadata: processing %s", line);
    }
    return 0;
}

/*
 * HEADER:100:set_metadata_str:misc:1:X = metadata string
 */

int f_set_metadata_str(ARG1) {
   int i;
   if (mode  < 0) return 0;
   // i = set_metadata_string(CALL_ARG0, arg1);
   i = set_metadata_string(call_ARG1(inv_out,local,arg1));
   if (i != 0) fatal_error("set_metadata_str: %s", arg1);
   return 0;
}

int set_metadata_string(ARG1) {
    
    int i, n, j, i0, i1;
    char *p;

    char date[STRING_SIZE];
    char var[STRING_SIZE];
    char lev[STRING_SIZE];
    char string[STRING_SIZE];
    char ftime[STRING_SIZE];

    char field[8][100], str1[100], str2[100], str3[100];
    double value1, value2;
  
    /* clear fields */
 
    i = sizeof(field[0]);
    n = sizeof(field) / i;
    for (j = 0; j < n; j++) field[j][i-1] = 0;

//    i = sscanf(line, "%*[^:]:%*[^:]:d=%99[^:]:%[^:]:%[^:]:%[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]", 
//    i = sscanf(line, 
//      "%*[^:]:%*[^:]:d=%99[^:]:%[^:]:%[^:]:%[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]", 
//	date, var, lev, ftime,field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7]);
    i = sscanf(arg1, 
      "%*[^:]:%*[^:]:%*[dD]=%99[^:]:%[^:]:%[^:]:%[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]", 
	date, var, lev, ftime,field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7]);

    if (i < 4) {
	return 8;
    }
    n = i - 4;

    if (mode == 99) fprintf(stderr,"set_metadata: ftime %s, f0=%s f1=%s\n",ftime,field[0],field[1]);

    if (strlen(date)) {
        if (f_set_date(call_ARG1(inv_out, NULL, date)) != 0) return 1;
    }

    if (strlen(var)) {
  	if (f_set_var(call_ARG1(inv_out,NULL,var)) != 0) return 1;
    }

    if (strlen(lev)) {
	if (f_set_lev(call_ARG1(inv_out,NULL,lev)) != 0) return 1;
    }

    if (strlen(ftime)) {
	if (f_set_ftime2(call_ARG1(inv_out,NULL,ftime)) != 0) return 1;
    }
/*
    if ((len_arg1 = strlen(ftime))) {
	if (version_ftime == 1) {
	    // if "anl" or "(number) %s %s" call set_ftime
	    // else call set_ave
	    if (strcmp(ftime,"anl") == 0) f_set_ftime1(call_ARG1(inv_out,NULL,ftime));
	    else if ( ((i = sscanf(ftime,"%*d %*s %s%n", string, &len)) == 1) && len == len_arg1) 
                f_set_ftime1(call_ARG1(inv_out,NULL,ftime));
            else f_set_ave(call_ARG1(inv_out,NULL,ftime));
	}
	else f_set_ftime(call_ARG1(inv_out,NULL,ftime));
    }
 */

    // process arguments that can be in any order
    //
    // if make -set var val smarter .. can eliminate this code

    for (i = 0; i < n; i++) {
	p = field[i];
	if (mode == 99) fprintf(stderr,"set_metadata field[%i]=%s\n",i,p);
	// get rid of training newline
	j = strlen(p);
	if (j == 0) continue;
	if (p[j-1] == '\n') p[j-1] = 0;
	if (p[0] == 0) continue;

	j = sscanf(p,"scale=%d,%d", &i0, &i1);
	if (j == 2) {
	    dec_scale = i0;
	    bin_scale = i1;
	    use_scale = 1;
	    continue;
	}

	j = sscanf(p,"packing=%s", string);
	if (j == 1) {
            f_set_grib_type(call_ARG1(inv_out,NULL,string) );
	    continue;
	}

	j = sscanf(p,"encode %d bits", &i0);
	if (j == 1) {
	    wanted_bits = i0;
	    use_scale = 0;
	    continue;
	}
	j = sscanf(p,"encode i*2^%d*10^%d", &i0, &i1);
	if (j == 2) {
	    dec_scale = i1;
	    bin_scale = i0;
	    use_scale = 1;
	    continue;
	}
	j = sscanf(p,"grib_max_bits=%d", &i0);
	if (j == 1) {
	    if (i0 < 0) i0 = 0;
	    if (i0 > 25) i0 = 25;
	    max_bits = i0;
	    continue;
	}

	// percentile   N% level note: ignores characters after level
	i1 = 0;
	j = sscanf(p, "%d%% level%n", &i0, &i1);
	if (i1 > 0) {
	    sprintf(str1,"%d", i0);
	    f_set_percentile(call_ARG1(inv_out,NULL,str1));
	    continue;
	}

	// probability
	j = sscanf(p,"prob <%lf",&value1);
	if (j == 1) {
	    sprintf(str1,"%lg", value1);
	    f_set_prob(call_ARG5(inv_out,NULL,"255","255","0", str1, str1));
	    continue;
	}

	j = sscanf(p,"prob >%lf",&value1);
	if (j == 1) {
	    sprintf(str1,"%lg", value1);
	    f_set_prob(call_ARG5(inv_out,NULL,"255","255","1", str1, str1));
	    continue;
	}

	j = sscanf(p,"prob =%lf",&value1);
	if (j == 1) {
	    sprintf(str1,"%lg", value1);
	    f_set_prob(call_ARG5(inv_out,NULL,"255","255","2", str1, str1));
	    continue;
	}

	j = sscanf(p,"prob >=%lf <%lf",&value1, &value2);
	if (j == 2) {
	    sprintf(str1,"%lg", value1);
	    sprintf(str2,"%lg", value2);
	    f_set_prob(call_ARG5(inv_out,NULL,"255","255","2", str1, str2));
	    continue;
	}

	// ensemble mean,  .. should put this into a table

	if (strcmp(p,"ens mean") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"0", "255"));
	    continue;
	}
	if (strcmp(p,"wt ens mean") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"1", "255"));
	    continue;
	}
	if (strcmp(p,"ens std dev") == 0 || strcmp(p,"cluster std dev") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"2", "255"));
	    continue;
	}
	if (strcmp(p,"normalized ens std dev") == 0 || strcmp(p," normalized cluster std dev") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"3", "255"));
	    continue;
	}
	if (strcmp(p,"ens spread") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"4", "255"));
	    continue;
	}
	if (strcmp(p,"ens large anom index") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"5", "255"));
	    continue;
	}
	if (strcmp(p,"unwt ens mean") == 0 || strcmp(p,"unwt cluster mean") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"6", "255"));
	    continue;
	}
	if (strcmp(p,"25%-75% range") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"7", "255"));
	    continue;
	}
	if (strcmp(p,"min all members") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"8", "255"));
	    continue;
	}
	if (strcmp(p,"max all members") == 0) {
	    f_set_ensm_derived_fcst(call_ARG2(inv_out,NULL,"9", "255"));
	    continue;
	}

	// ensemble members
	j = sscanf(p,"ENS=%s", str1);
	if (j == 1) {
	    // if (strcmp(str1,"hi-res ctl") == 0) {
	    if (strcmp(str1,"hi-res") == 0) {
	        f_set_ens_num(call_ARG3(inv_out,NULL,"0", str1, "-1"));
		continue;
	    }
	    if (strcmp(str1,"low-res") == 0) {
		f_set_ens_num(call_ARG3(inv_out,NULL,"1", str1, "-1"));
		continue;
	    }
	    j = sscanf(p,"ENS=+%[0-9]", str1);
	    if (j == 1) {
		f_set_ens_num(call_ARG3(inv_out,NULL,"3", str1, "-1"));
		continue;
	    }
	    j = sscanf(p,"ENS=-%[0-9]", str1);
	    if (j == 1) {
		f_set_ens_num(call_ARG3(inv_out,NULL,"2", str1, "-1"));
		continue;
	    }
	    fatal_error("set_metadata: unknown ENS=%s", str1);
	}
	j = sscanf(p,"%[0-9] ens members", str1);
	if (j == 1) {
	    f_set_ens_num(call_ARG3(inv_out,NULL,"-1", "-1", str1));
	    continue;
	}

//	
//	j = sscanf(p,"chemical=%s", str1);
//	if (j == 1) {
//	    f_set(call_ARG2(inv_out,NULL,"code_table_4.230",str1));
//	    continue;
//	}

	/* do better
	j = sscanf(p,"background generating process=%d forecast generating process=%d", &i0, &i1);
	if (j == 2) {
	    p = background_generating_process_identifier_location(sec);
	    if (p) *p = i0;
	    p = analysis_or_forecast_generating_process_identifier_location(sec);
	    if (p) *p = i1;
	    continue;
	}
	*/

	// code table 4.3=255  -> -set table_4.3 255
	j = sscanf(p,"code table %[^=]=%[0-9]",str1, str2);
	if (j == 2) {
	    sprintf(str3,"table_%s", str1);
	    if (f_set(call_ARG2(inv_out,NULL,str3,str2)) == 0) continue;
	}

	// flag table 3.3=48 -> set table_3.3 48
	j = sscanf(p,"flag table %[^=]=%[0-9]",str1, str2);
	if (j == 2) {
	    sprintf(str3,"table_%s", str1);
	    if (f_set(call_ARG2(inv_out,NULL,str3,str2)) == 0) continue;
	}

	// see if -set X T will work
	j = sscanf(p,"%[^=]=%[^:]",str1, str2);
	if (j == 2) {
	    if (mode == 99 && j==2) fprintf(stderr,"metadata_str -set (%s) (%s)\n", str1, str2);
	    if (f_set(call_ARG2(inv_out,NULL,str1,str2)) == 0) continue;
	}

	if (mode == 99) fprintf(stderr,"metadata_str j=%d\n", j);
	if (mode == 99 && j==2) fprintf(stderr,"metadata_str -set (%s) (%s) failed\n", str1, str2);

	fatal_error("set_metadata: field not understood = %s", p);
   }
   return 0;
}
