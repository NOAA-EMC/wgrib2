/** @file
 * @brief Routines for setting metadata in GRIB files.
 * @author Public Domain: Wesley Ebisuzaki @date 3/2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Use scaling flag. */
extern int use_scale;

/** Decimal scaling. */
extern int dec_scale;

/** Binary scaling. */
extern int bin_scale;

/** Number of bits wanted. */
extern int wanted_bits;

/** Maximum number of bits. */
extern int max_bits;

/** Version of ftime to use. */
extern int version_ftime;

/*
 * HEADER:100:set_metadata:misc:1:read meta-data for grib writing from file X
 */

/**
 * Read meta-data for GRIB writing from specified file.
 * 
 * Whenever -set_metadata option is "executed", one line of the data file is read, the metadata 
 * is then applied to the current (sub-)message. For example, your grib file has 3 messages 
 * and you want to alter the metadata. Here is a metadata file that could be used to alter 
 * the grib file. 
 * 
 *      0:0:d=2009010100:HGT:500 mb:anl:scale=0,0:
 *      0:0:d=2009010100:TMP:2 m above ground:anl
 *      0:0:d=2009010106:HGT:500 mb:12 hour forecast:scale=0,0
 * 
 * The format of the metadata file resembles the wgrib2 inventory by design. The first and second 
 * fields are ignored by by -set_metadata. In practice, the first first is the record number and 
 * the second field is the byte location of the template to be used. (A template file could be a 
 * collection of templates.) The third through sixth fields are the date code, variable name, 
 * level and ftime. Finally the remaining fields are optional and order independent. 
 * 
 * The date field has been extended from the default d=YYYYMMDDHH. The date field can be d=format-N 
 * or D=format-N.
 * 
 * ### Format 1
 * YYYY, YYYYMM, YYYYMMDD, YYYYMMDDHH, YYYYMMDDHHmm or YYYYMMDDHHmmss
 *
 * where YYYY = 4 digit year
 *         MM = 2 digit month
 *         DD = 2 digit day
 *         HH = 2 digit hour
 *         mm = 2 digit minute
 *         ss = 2 digit second
 * If MM, DD, mm, ss are missing, the values are unchanged.
 * 
 * ## Format 2
 * +N(units) 
 * 
 * N is an integer, units = hr, dy, mo or yr. This adds N units to the reference time (ex. +6hr, +1dy).
 * 
 * ### Format 3
 * -N(units)
 *
 * N is an integer, units = hr, dy, mo or yr. This subtracts N units from the reference time (ex. -6hr, -1dy).
 * 
 * ### Optional Fields
 * scale=I,J			    set decimal scaling (I) and binary scaling (J) for encoding
 * encode i*2^J*10^I        set decimal scaling (I) and binary scaling (J) for encoding (same as scale=I,J)
 * encode I bits            store grid values using I bits, no decimal scaling (ECMWF style)
 * grib_max_bits=I          set maximum number of bits used to store grid point data (max 25)
 * packing=X                set compression/packing to X 
 *                              X = simple,complex1,complex2,complex3,jpeg,aec (long form)
 *                              X = s,c1,c2,c3,j (short form)
 * N%d level                set percentile
 * prob ...                 set probability
 * XYZ                      XYZ=ens mean, wt ens mean, ens std dev, cluster std dev, normalized ens std dev, 
 *                          normalized cluster std dev, ens spread, ens large anom index, wt ens mean, unwt 
 *                          cluster mean, 25%-75% range, min all members, max all members
 * ENS=...                  set ensemble member info (ENS=hi-res, low-res, +N, or -N)
 * N ens members            set number of ensemble members (ex. 22 ens members)
 * code table X.Y=Z         set code table, equivalent -set table_X.Y Z
 * flag table X.Y=Z         set flag table, equivalent -set table_X.Y Z (not all X.Y have been implemented)
 * 
 * ### Following options follows -set VAR VAL and do not follow the wgrib2 inventory format
 * 
 *      disciple=
 *      local_table=
 *      master_table=
 *      center=                             set center
 *      subcenter=                          set sub-center
 *      background_process_id=
 *      analysis_or_forecast_process_id=
 *      table_M.N=                          only for selected M.N
 * 
 * ### Usage
 * To change the metadata, you can do:
 * @code{.sh}
 * # Change meta
 * wgrib2 in.grb -s >meta
 * # Set metadata
 * wgrib2 in.grb -set_metadata  meta -grib out.grb
 * @endcode
 * 
 * Note that -grib does not change the grid point data/packing.
 *
 * The -set_metadata option is used for creating grib2 files. Note that both -set_metadata and 
 * the various -import options will change the output precision. Consequently the -import option 
 * should preceed the -set_metadata option.
 *
 * @note Only a subset of levels and ftime parameters is currently implemented. The -set_metadata 
 * option does not support all metadata. The format of the levels and ftime are the same was used 
 * by wgrib2 inventories. 
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 3/2008
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
        i = set_metadata_string(call_ARG1(inv_out,local,line));
        if (i) fatal_error("set_metadata: processing %s", line);
    }
    return 0;
}

/*
 * HEADER:100:set_metadata_str:misc:1:X = metadata string
 */

/**
 * The -set_metadata_str "string" option is similar to the older -set_metadata FILE option. 
 * Instead of reading the metadata from a file, the metadata is on the command line. The 
 * latter option is generally more useful as each grib message can have its own set of 
 * metadata. The former option is only useful for specifying the metadata for a single grib 
 * message. The -set_metadata_str option was added to facilitate the creation of an "callable 
 * wgrib2" API for writing grib2. See -set_metadata for the format of the metadata string. 
 * 
 * ## Usage
 * -set_metadata_str "metadata"
 * 
 * metadata is a string
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 4/2019
 */
int f_set_metadata_str(ARG1) {
    int i;
    if (mode  < 0) return 0;
    i = set_metadata_string(call_ARG1(inv_out,local,arg1));
    if (i != 0) fatal_error("set_metadata_str: %s", arg1);
    return 0;
}

/**
 * Reads metadata from a string and sets the appropriate fields in the GRIB message.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 
 * - 0 :: success
 * - 1 :: failure setting metadata
 * - 8 :: insufficient fields in metadata string
 *
 * Will also throw fatal_error() for other errors.
 * @author Wesley Ebisuzaki @date 4/2019
 */
int set_metadata_string(ARG1) {
    
    int i, n, j, i0, i1;
    char *p;

    char date[STRING_SIZE];
    char var[STRING_SIZE];
    char lev[STRING_SIZE];
    char string[STRING_SIZE];
    char ftime[STRING_SIZE];

    char field[8][STRING_SIZE], str1[STRING_SIZE], str2[STRING_SIZE], str3[STRING_SIZE+6];
    double value1, value2;
  
    /* clear fields */
 
    i = sizeof(field[0]);
    n = sizeof(field) / i;
    for (j = 0; j < n; j++) field[j][i-1] = 0;

    if (*arg1 == 'd' || *arg1 == 'D') {
        i = sscanf(arg1+1, 
            "=%99[^:]:%[^:]:%[^:]:%[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]", 
            date, var, lev, ftime,field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7]);
    }
    else {
        i = sscanf(arg1, 
            "%*[^:]:%*[^:]:%*[dD]=%99[^:]:%[^:]:%[^:]:%[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]:%99[^:]", 
            date, var, lev, ftime,field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7]);
    }

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
            f_set_prob(call_ARG5(inv_out,NULL,"","","0", str1, str1));
            continue;
        }
        j = sscanf(p,"prob >%lf",&value1);
        if (j == 1) {
            sprintf(str1,"%lg", value1);
            f_set_prob(call_ARG5(inv_out,NULL,"","","1", str1, str1));
            continue;
        }
        j = sscanf(p,"prob >=%lf <%lf",&value1, &value2);
        if (j == 2) {
            sprintf(str1,"%lg", value1);
            sprintf(str2,"%lg", value2);
            f_set_prob(call_ARG5(inv_out,NULL,"","","2", str1, str2));
            continue;
        }
        j = sscanf(p,"prob =%lf",&value1);
        if (j == 1) {
            sprintf(str1,"%lg", value1);
            f_set_prob(call_ARG5(inv_out,NULL,"","","5", str1, str1));
            continue;
        }

        if (strcmp(p,"prob above normal") == 0) {
            f_set_prob(call_ARG5(inv_out,NULL,"","","6", "0", "0"));
            continue;
        }
        if (strcmp(p,"prob near normal") == 0) {
            f_set_prob(call_ARG5(inv_out,NULL,"","","7", "0", "0"));
            continue;
        }
        if (strcmp(p,"prob below normal") == 0) {
            f_set_prob(call_ARG5(inv_out,NULL,"","","8", "0", "0"));
            continue;
        }

        j = sscanf(p,"prob fcst %d/%d", &i0, &i1);
        if (j == 2) {
            sprintf(str1,"%d", i0);
            sprintf(str2,"%d", i1);
            f_set_prob(call_ARG5(inv_out,NULL,str1,str2,"", "", ""));
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

        // lvl1=
        j = sscanf(p,"lvl1=%s",str1);
        if (j == 1) {
            f_set_lvl1(call_ARG1(inv_out,NULL,str1));
            continue;
        }

        // lvl2=
        j = sscanf(p,"lvl2=%s",str1);
        if (j == 1) {
            f_set_lvl2(call_ARG1(inv_out,NULL,str1));
            continue;
        }


        // X=T
        // see if -set X T will work
        j = sscanf(p,"%[^ =]=%[^:]",str1, str2);
        if (j == 2) {
            if (mode == 99 && j==2) fprintf(stderr,"metadata_str (x=y) (%s) -set (%s) (%s)\n", p, str1, str2);
            if (f_set(call_ARG2(inv_out,NULL,str1,str2)) == 0) continue;
        }

        // X T
        // see if -set X T will work
        j = sscanf(p,"%s %[^:]",str1, str2);
        if (j == 2) {
            if (mode == 99 && j==2) fprintf(stderr,"metadata_str (x y) (%s) -set (%s) (%s)\n", p, str1, str2);
            if (f_set(call_ARG2(inv_out,NULL,str1,str2)) == 0) continue;
        }

        if (mode == 99) fprintf(stderr,"metadata_str j=%d\n", j);
        if (mode == 99 && j==2) fprintf(stderr,"metadata_str -set (%s) (%s) failed\n", str1, str2);

        fatal_error("set_metadata: field not understood = %s", p);
   }
   return 0;
}
