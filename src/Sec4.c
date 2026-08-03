/** @file
 * @brief Some routines that involve Section 4 (Product Definition Section).
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2006 | W. Ebisuzaki | Initial
 * 1/2007 | M. Schwarb | Cleanup
 * 7/2009 | R. Bokhorst | Bug fix (buffer overflow)
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"

/*
 * HEADER:400:Sec4:inv:0:Sec 4 values (Product definition section)
 */

/** Prints a short summary of Section 4, the Product Definition section. 
 * 
 * ## Usage
 * -Sec4
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 -Sec4 png.grb2 
 * 1:4:Sec4 len=36 #vert coordinate=0 Product Defn Template=4.2 size=36 free=0
 * @endcode
 * 
 * <pre>
 * len=36                        Section 4 is 36 octets/bytes in length
 * #vert coordinate=0            no vertical coordinates have been defined
 * Product Defn Template=4.2     using Product Definition Template 4.2  (Code Table 4.0)
 * size=36                       Size of PDT excluding vertical coordinates
 * free=0                        Should be zero, len-size-8*#vert_coordinates
 * </pre>
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_Sec4(ARG0) {
    int calc_pdtsize, vert_coor, pdtsize;
    if (mode >= 0) {

        vert_coor = number_of_coordinate_values_after_template(sec);
        calc_pdtsize = pdt_len(sec, -1);
        pdtsize =  GB2_Sec4_size(sec);

        sprintf(inv_out,"Sec4 len=%u #vert coordinate=%u Product Defn Template=4.%d size=%d expected size=%d", 
                pdtsize, vert_coor, GB2_ProdDefTemplateNo(sec), pdtsize, calc_pdtsize);
        inv_out += strlen(inv_out);
    }
    return 0;
}

/*
 * HEADER:400:processid:inv:0:process id (locally defined)
 */

/**
 * Prints the background generating process identifier (defined by originating centre) and
 * analysis or forecast generating process identified (see Code ON388 Table A) which are
 * octects 13 and 14 in section 4 in many of the Product Definition Templates such 4.0. 
 * 
 * ## Usage
 * -processid
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 png.grb2 -processid
 * 1:4:background generating process=0 forecast generating process=80
 * @endcode
 * 
 * You can change the process id using the -set option.
 * 
 * @code{.sh}
 * $ wgrib2 png.grb2 -set analysis_or_forecast_process_id 11 -processid
 * 1:4:background generating process=0 forecast generating process=11
 * @endcode
 * 
 * You can add the process id to the match inventory by
 *
 * @code{.sh}
 * $ wgrib2 png.grb2 -match_inv_add processid x x -match_inv
 * 1:4:d=2009060500:RH:2 m above ground:330 hour fcst:ens std dev:
 *  RH.ens_std_dev:n=1:npts=65160:var0_2_1_7_1_1:pdt=2:D=20090605000000:
 *  start_FT=20090618180000:end_FT=20090618180000:scaling ref=0 dec_scale=-1 bin_scale=0 
 *  nbits=16:background generating process=0 forecast generating process=80:
 *  330 hour fcst:vt=2009061818:
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_processid(ARG0) {
    int i;
    const char *space;


    if (mode >= 0) {
        space = "";
        i = observation_generating_process_identifier(sec);
        if (i >= 0) {
            sprintf(inv_out,"%sobservation generating process=%d", space, i);
            inv_out += strlen(inv_out);
            space = " ";
        }

        i = background_generating_process_identifier(sec);
        if (i >= 0) {
            sprintf(inv_out,"%sbackground generating process=%d", space, i);
            inv_out += strlen(inv_out);
            space = " ";
        }

        i = analysis_or_forecast_generating_process_identifier(sec);
        if (i >= 0) {
            sprintf(inv_out,"%sforecast generating process=%d", space, i);
            inv_out += strlen(inv_out);
            space = " ";
        }
    }
    return 0;
}


/*
 * HEADER:400:0xSec:inv:1:Hex dump of section X (0..8)
 */

/**
 * Prints a hex dump of any of the grib sections. The format of the hex dump depends on the 
 * verbosity level. 
 * 
 * ## Usage
 * -0xSec N
 * 
 * N = section number
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 png.grb2 -0xSec 0
 * 1:4:Sec0(1..16)=0x4752494200000002000000000000ad9c
 * $ wgrib2 png.grb2 -0xSec 0 -v1
 * 1:4:Sec0(1..16)= 47 52 49 42 00 00 00 02 00 00 00 00 00 00 ad 9c
 * $ wgrib2 png.grb2 -0xSec 0 -v2
 * 1:4:Sec0(1..16)=1:47 2:52 3:49 4:42 5:00 6:00 7:00 8:02 9:00 10:00 11:00 12:00 13:00 14:00 15:ad 16:9c 
 * @endcode
 * 
 * The above 3 examples show a hex dump of Section 0 using the different verbosity levels.
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_0xSec(ARG1) {
    int i, sec_no;
    unsigned int j, len;
    double tot;
    unsigned char *s;

    if (mode >= 0) {
        sec_no = i = atoi(arg1);
        if (sec_no < 0 || sec_no > 8) fatal_error_i("0xSec: bad section number %d", sec_no);
        if ( (s = sec[sec_no]) == NULL) {
            sprintf(inv_out,"sec%d is missing", sec_no);
            return 0;
        }
        if (sec_no == 0) { 
            len = GB2_Sec0_size;
        }
        else if (sec_no == 8) {
            len = GB2_Sec8_size;
        }
        else { 
            len = uint4(&(sec[sec_no][0]));
        }

        /* Calculate number of bytes to print */

        if (mode == 0) tot = 2.0 * len;
        else if (mode == 1) tot = 3.0 * len;
        else {		// mode >= 2
            j = len;
            i = 0;
            while (j) {
                j /= 10;
                i++;
            }
            // i = maximum number of digits in address
            tot = len*i			// maximum of characters for address
                + 4*len		// blank : 2 digits
                + 4*(len % 15);	// new line (2 for windows) + two blanks
        }

        if (tot >= INV_BUFFER - 1000) {	// 1000 is precaution
            sprintf(inv_out,"Sec%d=too long to print",sec_no);
            return 0;
        }

        if (mode == 0) sprintf(inv_out,"Sec%d(1..%u)=0x",sec_no,len);
        else sprintf(inv_out,"Sec%d(1..%u)=",sec_no,len);
        inv_out += strlen(inv_out);

        if (mode == 0) {
            while (len--) {
                sprintf(inv_out,"%.2x", *s++);
                inv_out += strlen(inv_out);
            }
        }
        else if (mode == 1) {
            for (j = 1; j <= len; j++) {
                sprintf(inv_out," %.2x", *s++);
                inv_out += strlen(inv_out);
            }
        }
        else if (mode >= 2) {
            for (j = 1; j <= len; j++) {
                sprintf(inv_out,"%u:%.2x", j, *s++);
                inv_out += strlen(inv_out);
                sprintf(inv_out,j % 15 == 0 ? "\n  " : " ");
                inv_out += strlen(inv_out);
            }
        }
    }
    return 0;
}

/*
 * HEADER:400:var:inv:0:short variable name
 */

/**
 * Prints the variable name of the grib message.
 * 
 * In the beginning, grib fields were identified by a name, a level and some timing information. 
 * Life was simple and the people were happy. This was soon to pass, for the ensemble people 
 * had to specify the ensemble number and the probability of events. The dust people needed to 
 * specify dust density by composition and by size. Thinking big, ensembles of dust models were 
 * in the future albeit obscured by the haze. Consequently the old name (ex. HGT, TMP) was often 
 * no longer a good way to specify a specific field. For example, a TMP field could be measured 
 * in degrees K or fraction if the TMP had a probability modifier. The -set_ext_name and 
 * -ext_name options are a way to help fix this problem. Using these two options, you can get a 
 * extended name with many of the modfiers added to the name. This option will have to be updated 
 * when more modifiers are used to distiguish the fields.
 * 
 * The -var option prints the VARIABLE name of the grib message. Common names would be HGT and TMP 
 * for the geopotential height and the temperature. For most knowing the variable name, the level 
 * and the timimg information is all you need. Then things became more complicated. Eventually a 
 * file came along which had only one variable type (MASSDEN, mass density) but had a couple of 
 * important qualifier chemical type (H2O/O3/N02) and ensemble member ID. The -AAIG output was 
 * useless because its output used the variable name. 
 * 
 * To fix the -AAIG output, an extended name was introduced. You can see the extended name by the 
 * -ext_name option. 
 * 
 * ## Usage
 * -var
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 chem.grb2 -var
 * 1:0:MASSDEN
 * $ wgrib2 chem.grb2 -ext_name
 * 1:0:MASSDEN.hi-res_ctl.Water_Vapour
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_var(ARG0) {
    if (mode >= 0) {
        getName(sec, mode, inv_out, NULL, NULL, NULL);
        inv_out += strlen(inv_out);
    }
    return 0;
}

/*
 * HEADER:400:varX:inv:0:raw variable name - discipline mastertab localtab center parmcat parmnum
 */

/**
 * Writes a raw variable name. It shows the discipline, master table, local table, center, 
 * parameter category and parameter number. The default format is:
 * 
 * var{discipline}_{master table}_{local table}_{center}_{parameter_category}_{parameter}
 * 
 * When you use verbose mode > 0 (-v, -v2, -v98, -v99), then the format of -varX changes to 
 * 
 * (WMO defined variables)
 * 
 * ex. var discipline=0 master_table=2 parmcat=2 parm=3
 * 
 * For locally defined variables, the local table is important and the format is
 * 
 * ex.  var discipline=0 local_table=1 center=7 parmcat=1 parm=195
 * 
 * One would use the -varX option when the built-in tables have a problem. For example, suppose 
 * SGP is a new parameter this is not in your version of wgrib2. However, you want to write a 
 * script that will extract SGP and work with both the old and future versions of wgrib2. By 
 * using -varX, your inventories will have a variable name that is the same in both versions of 
 * wgrib2. The varX name has been added to the match_inventory (2.0.2 3/2015) and is understood 
 * by -set_var and -set_metadata (2.0.7 12/2017). 
 * 
 * ## Usage
 * -varX
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 test.grb2 -varX
 * 1:0:var0_2_1_7_3_5
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_varX(ARG0) {

    int discipline, center,mastertab,localtab,parmcat,parmnum;

    if (mode >= 0) {
        discipline = GB2_Discipline(sec);
        center = GB2_Center(sec);
        mastertab = GB2_MasterTable(sec);
        localtab = GB2_LocalTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);
        if (mode == 0) {
            if (parmnum == 255) {
                sprintf(inv_out,"missing");
                return 0;
            }
            sprintf(inv_out,"var%d_%d_%d_%d_%d_%d", discipline, mastertab, localtab, center, parmcat, parmnum);
        }
        else if (mode > 0) {
            if (parmnum == 255) {
                sprintf(inv_out,"missing definition [?]");
                return 0;
            }
            if (parmnum < 192) {
                sprintf(inv_out,"var discipline=%d master_table=%d parmcat=%d parm=%d", 
                      discipline, mastertab, parmcat, parmnum);
            }
            else {
                sprintf(inv_out,"var discipline=%d center=%d local_table=%d parmcat=%d parm=%d",
                  discipline, center, localtab, parmcat, parmnum);
            }
        }
    }
    return 0;
}

/*
 * HEADER:400:pdt:inv:0:Product Definition Table (Code Table 4.0)
 */

/**
 * Prints the Product Definition Template (Code Table 4.0).
 * 
 * The -pdt option is an alias to -code_table_4.0t. 
 * 
 * ## Usage
 * -pdt
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 -pdt png.grb2
 * 1:4:code table 4.0=2 Derived forecasts based on all ensemble members at a horizontal level or in a horizontal layer at a point in time.
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_pdt(ARG0) {
    if (mode < 0) return 0;
    sprintf(inv_out,"pdt=%d", code_table_4_0(sec));
    return 0;
}
