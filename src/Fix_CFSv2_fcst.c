/** @file
 * @brief Fixes CFSv2 monthly forecast fields.
 * @author Public Domain: Wesley Ebisuzaki @date 08/2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"

/*
 * HEADER:100:fix_CFSv2_fcst:misc:3:fixes CFSv2 monthly fcst  X=daily or 00/06/12/18 Y=pert no. Z=number ens fcsts  v1.0
 */

/**
 * Alters the metadata of the CFSRR/CFSv2 monthly forecasts files to make them grib2 
 * compliant and to fix the metadata.
 * 
 * The Climate Forecast System version 2 (CFSv2) is a coupled atmosphere/ocean forecast 
 * system that was developed at NCEP. CFSv2 produces analyses (CFSR) and daily to seasonal
 * forecasts. The seasonal forecasts consists of hindcasts (CFSRR) and the real-time forecasts
 * (CFSv2 forecasts).
 *
 * The monthly-mean forecasts from the CFSRR and CFSv2 are originally written in grib1 format 
 * and later converted to grib2 format for public distribution. In this process, the resulting 
 * grib2 files are mangled and the metadata no longer correctly describes the contents of the 
 * file. The -fix_CFSv2_fcst option alters the metadata of the CFSRR/CFSv2 monthly forecasts 
 * files to make them grib2 compliant and to fix the metadata. After the data has been fixed, 
 * the files can then be read with grib2 decoders such as GrADS.
 * 
 * ## CFSRR/CFSv2 Monthly Forecast File Names
 * The CFSv2 monthly forecasts have a naming convention.
 * 
 * (NAME)(DATE0).(EN).(DATE1).avrg[.(HH)Z].grb2
 * 
 * NAME=type of file
 *      flxf       forecast (f) data on special levels (flx)
 *      ipvf       forecast (f) data on isentropic surfaces (ipv)
 *      ocnf       forecast (f) data for the oceans (ocn)
 *      pgbf       forecast (f) data for pressure (p) levels grib (gb)
 * 
 * DATE0=starting time of the forecasts
 *      YYYYMMDDHH
 * 
 * EN=ensemble member number
 * 
 * DATE1=target month of the forecast
 *      YYYYMM
 * 
 * [..] = contents in the square brackets are optional
 * 
 * ### Case 1: contents of [] are missing or null
 * The forecast system produces daily forecasts ending at 00Z, 06Z, 12Z and 18Z.
 * All these forecasts are included in the forecast of the monthly mean.
 * 
 * ### Case 2: contents of [] are present, i.e., .(HH)Z
 * HH = 00, 06, 12, 18
 * 
 * The monthly forecast is for a specific cycle of the day.
 * For example the 00Z cycle.  For instantaneous quantities, the monthly forecast will 
 * be consist of the mean of all daily forecasts that verify on the 00Z hour. For quantities 
 * that are 6 hour averages or accumulations, they will be the average of all the forecasts 
 * with averages/accumulations that end at 00Z hour (18Z-24Z average/accumulations).
 *
 * Unfortunately the original file doesn't contain any metadata concerning the hour or the 
 * type of quantity, so the hour has to be entered as an argument and the forecasts are 
 * assumed to be the average of instantaneous fields.
 * 
 * ## Fixing the CFSRR/CFSv2 monthly forecasts format
 * A filter was added to wgrib2 that fixes the metadata in the CFSv2 monthly forecasts. To fix the metadata, 
 * 
 * wgrib2 IN -fix_CFSv2_fcst TIME EN N_ENS -grib OUT
 * 
 * IN=name of input grib file, ex. flxf2009010100.01.200901.avrg.06Z.grb2
 * 
 * TIME=daily, 00, 06, 12, 18
 *      use daily if .(HH)Z is missing from the input filename
 *      use 00 if .00Z is part of the input filename
 *      use 06 if .06Z is part of the input filename
 *      use 12 if .12Z is part of the input filename
 *      use 18 if .18Z is part of the input filename
 * 
 * EN=ensemble member number, suggested that EN from the filename is used but value is arbitrary.
 * N_ENS=number of ensemble members, use max of (EN).  Value is arbitrary.
 * OUT=name of the output grib file
 * 
 * ## Example of Fixing the CFSRR monthly forecast
 * 
 * @code{.sh}
 * $ wgrib2 flxf2009010100.01.200903.avrg.06Z.grb2 -for 1:3 -fix_CFSv2_fcst 06 1 1  -grib out.grb
 * 1:0:d=2009010100:UFLX:surface:1422 hour-(1422 hour+30 day) ave@(fcst,dt=1 day),missing=0:ENS=+1
 * 2:66720:d=2009010100:VFLX:surface:1422 hour-(1422 hour+30 day) ave@(fcst,dt=1 day),missing=0:ENS=+1
 * 3:133122:d=2009010100:SHTFL:surface:1422 hour-(1422 hour+30 day) ave@(fcst,dt=1 day),missing=0:ENS=+1
 * @endcode
 * 
 * <pre>
 * flxf2009010100.01.200903.avrg.06Z.grb2   input file
 * -for 1:3                                 process records 1..3
 * -fix_CFSv2_fcst 06 1 1                   fix metadata for 06Z type file, 
 * -grib out.grb                            save corrected file in out.grb
 * </pre>
 * 
 * ### Examining our corrected file
 * 
 * The default wgrib2 inventory looks like
 * 
 * @code{.sh}
 * $ wgrib2 out.grb 
 * 1:0:d=2009010100:UFLX:surface:1422 hour-(1422 hour+30 day) ave@(23 hour fcst)++,missing=0:ENS=+1
 * 2:66719:d=2009010100:VFLX:surface:1422 hour-(1422 hour+30 day) ave@(23 hour fcst)++,missing=0:ENS=+1
 * 3:133120:d=2009010100:SHTFL:surface:1422 hour-(1422 hour+30 day) ave@(23 hour fcst)++,missing=0:ENS=+1
 * @endcode
 * 
 * <pre>
 * d=2009010100                             initial time of the forecast
 * UFLX                                     zonal wind stress
 * 1422 hour-(1422 hour+30 day) ave         average for 1422 hour to (1422 hours + 30 day)
 * (fcst,dt=1 day)                          average the forecast at 1 day intervales
 * missing=0                                no missing fields in the average
 * </pre>
 * 
 * Now I can't figure out 1422 hours in my head, so I can print out the start and and of the forecast time interval by,
 * 
 * @code{.sh}
 * $ wgrib2 out.grb -start_ft -end_ft
 * 1:0:start_ft=2009030106:end_ft=2009033106
 * 2:66719:start_ft=2009030106:end_ft=2009033106
 * 3:133120:start_ft=2009030106:end_ft=2009033106
 * @endcode
 * 
 * Notice the start_ft has a 06 hour as you would have expected from the original file, flxf2009010100.01.200903.avrg.06Z.grb2.
 * 
 * ### Adding Ocean Metadata
 * 
 * In the NCDC archives, the DBSS variable has bad level information. For example, the time series of N degree isotherm 
 * is not encoded correctly
 * 
 * @code{.sh}
 * $ wgrib2 dt5c.ensm.apr.cfsv2.data.grb2
 * 1:0:d=1982040618:DBSS:0C ocean isotherm:0-1 month ave fcst:
 * 2:42547:d=1982040618:DBSS:0C ocean isotherm:1-2 month ave fcst:
 * 3:85611:d=1982040618:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * 4:128757:d=1982040618:DBSS:0C ocean isotherm:3-4 month ave fcst:
 * ...
 * @endcode
 * 
 * The level should be 5C ocean isotherm as indicated by the file name.
 * 
 * To fix this time series, you need to rewrite the level information.
 * 
 * @code{.sh}
 * $ wgrib2 dt5c.ensm.apr.cfsv2.data.grb2 -set_lev "5C ocean isotherm" -grib dt5c.ensm.apr.cfsv2.data.grb2.new
 * 1:0:d=1982040618:DBSS:5C ocean isotherm:0-1 month ave fcst:
 * 2:42547:d=1982040618:DBSS:5C ocean isotherm:1-2 month ave fcst:
 * 3:85611:d=1982040618:DBSS:5C ocean isotherm:2-3 month ave fcst:
 * 4:128757:d=1982040618:DBSS:5C ocean isotherm:3-4 month ave fcst:
 * ...
 * @endcode
 * 
 * In monthly ocean forecasts @ NCDC, we can find bad level data.  For example, one file had bad level metadata for messages 216-222.
 * 
 * <pre>
 * 216:11232042:d=1999052200:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * 217:11278152:d=1999052200:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * 218:11322088:d=1999052200:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * 219:11360222:d=1999052200:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * 220:11392818:d=1999052200:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * 221:11419935:d=1999052200:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * 222:11440970:d=1999052200:DBSS:0C ocean isotherm:2-3 month ave fcst:
 * </pre>
 * 
 * The following code snippet will fix the level data for the above file.
 * 
 * @code{.sh}
 * wgrib2 $f \
 * -if "^216:" -set_lev "2.5C ocean isotherm"  -fi \
 * -if "^217:" -set_lev "5C ocean isotherm"  -fi \
 * -if "^218:" -set_lev "10C ocean isotherm"  -fi \
 * -if "^219:" -set_lev "15C ocean isotherm"  -fi \
 * -if "^220:" -set_lev "20C ocean isotherm"  -fi \
 * -if "^221:" -set_lev "25C ocean isotherm"  -fi \
 * -if "^222:" -set_lev "28C ocean isotherm"  -fi  \
 * -grib $f.fixed
 * @endcode
 *
 * ### Making GrADS control files
 * 
 * Once you have converted the files using the -fix_CFSv2_fcst option, you have to use the -b option in g2ctl/gribmap to get the 
 * target times to line up correctly. The -b option sets the start of the target month as the GrADS time.  If you use the default 
 * option, you will get the end of the forecast period.  For the ordinary forecast files, that will be at 18Z of the last month.  
 * Definitely not as friendly.
 * 
 * @code{.sh}
 * $ g2ctl -b out.grb >out.ctl
 * $ gribmap -b -i out.ctl
 * grib2map: scanning GRIB2 file: out.grb
 * grib2map: Writing out the index file 
 * @endcode
 * 
 * ## A Real Example
 * The previous examples were the conversion, inventory and display of a single file. Most of the time, 
 * people would want to examine the entire forecast run.  In this following example, we look at a 
 * forecast run.  Here we cannot include the file with the target month which is the same as the starting 
 * month because this file does not start on the 1st of the month like the other files.
 *
 * CFSRR files:
 * 
 * <pre>
 *     (do not include this file) -->   pgbf2009081500.01.200908.avrg.grb2
 *     pgbf2009081500.01.200909.avrg.grb2
 *     pgbf2009081500.01.200910.avrg.grb2
 *     pgbf2009081500.01.200911.avrg.grb2
 * </pre>
 * 
 * Fixing the metadata in the bash shell on a linux box.
 * 
 * @code{.sh}
 * $ for f in pgbf2009081500.01.2009??.avrg.grb2
 * >do
 * > wgrib2 $f -fix_CFSv2_fcst_daily 1 1 -grib $f.new >/dev/null
 * >done
 * fix_CFSRv2_fcst 524 fields fixed
 * fix_CFSRv2_fcst 524 fields fixed
 * fix_CFSRv2_fcst 524 fields fixed
 * @endcode
 * 
 * Making the GrADS ctl and idx files.  Note the -b options to both g2ctl and gribmap.
 * Note that gribmap v2 and grads v2 have to be used.  Note the use of the template (%m2) in
 * the g2ctl line.
 *
 * @code{.sh}
 * $ g2ctl -b pgbf2009081500.01.2009%m2.avrg.grb2.new >pgb.ctl
 * $ gribmap -b -i pgb.ctl
 * grib2map: scanning GRIB2 file: pgbf2009081500.01.200909.avrg.grb2.new 
 * grib2map: scanning GRIB2 file: pgbf2009081500.01.200910.avrg.grb2.new 
 * grib2map: scanning GRIB2 file: pgbf2009081500.01.200911.avrg.grb2.new 
 * grib2map: reached end of files
 * grib2map: Writing out the index file 
 * @endcode
 * 
 * Running grads v2 with the new ctl/idx file.
 * 
 * @code{.sh}
 * $ grads
 * Grid Analysis and Display System (GrADS) Version 2.0.a9
 * Copyright (c) 1988-2010 by Brian Doty and the
 * Institute for Global Environment and Society (IGES)
 * GrADS comes with ABSOLUTELY NO WARRANTY
 * See file COPYRIGHT for more information
 *
 * Config: v2.0.a9 little-endian readline printim grib2 netcdf hdf4-sds hdf5 opendap-grids,stn geotiff shapefile
 * Issue 'q config' command for more detailed configuration information
 * Landscape mode? ('n' for portrait):
 * GX Package Initialization: Size = 11 8.5 
 * ga-> open pgb.ctl
 * Scanning description file:  pgb.ctl
 * Data file pgbf2009081500.01.2009%m2.avrg.grb2.new is open as file 1
 * LON set to 0 360 
 * LAT set to -90 90 
 * LEV set to 1000 1000 
 * Time values set: 2009:8:15:0 2009:8:15:0 
 * E set to 1 1 
 * ga-> set lev 500
 * LEV set to 500 500 
 * ga-> d hgtprs
 * Contouring: 4900 to 5900 interval 100 
 * ga-> 
 * @endcode
 * 
 * ## Encoding the Ensemble Information
 * The -fix_CFSv2_fcst option adds enesemble information.  The directions suggest that you number 
 * the runs using the same numbers as suggested by the CFSv2 filename convention. That will minimize 
 * confusion.  However, you are allowed to give any ensemble number up to 254. Using unique ensemble 
 * member numbers would be useful for GrADS.
 * 
 * ## Caveats
 * The filtering action of -fix_CFSv2_fcst assumes that the fields that were averaged were instantaneous 
 * fields (ex. Z500 at a specific time).  This is not true as some fields are 6-hour accumulations or 
 * averages.  Grib2 can describe averages of accumulations/averages.  However, this nuance was ignored.
 *
 * The instructions suggest that you encode the CFSRR ensemble member number as 1 to be consistent with the 
 * CFSv2 file name convention.  For CFSRR, there is only one ensemble member (01) which by convention would 
 * be considered the high-resolution control run.
 * 
 * ## Usage
 * -fix_CFSv2_fcst X Y Z
 * X=daily, 00, 06, 12, 18
 * Y=Ensemble member number (perturbation number)
 * Z=number of ensemble members
 * 
 * @param ARG3 List of function arguments set by wgrib2's main() function (see @ref ARG3). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 08/2011
 */
int f_fix_CFSv2_fcst(ARG3) {

    int subcenter, n_time_ranges, fcst_time_0, fcst_time_1, i;
    int dtime, unit, id;

    int ref_year, ref_month, ref_day, ref_hour, ref_minute, ref_second;
    int fcst0_year, fcst0_month, fcst0_day, fcst0_hour, fcst0_minute, fcst0_second;
    int fcst1_year, fcst1_month, fcst1_day, fcst1_hour, fcst1_minute, fcst1_second;

    static unsigned char new_sec4[61];

    struct local_struct {
        int fixed;			// number of fields processed
        int dt;				// dt either 6 or 24
        int hour0;			// initial hour
        int pert;			// perturbation no, 0 = ctl
        int num_ensemble_members;
    };
    struct local_struct *save;

    if (mode == -1) {
        *local = save = (struct local_struct *)malloc( sizeof(struct local_struct));
        save->fixed = 0;
        save->dt = 24;
        if (strcmp(arg1,"daily") == 0) {
            save->dt = 6;
            save->hour0 = 0;
        }
        else if (strcmp(arg1,"00") == 0) {
            save->hour0 = 0;
        }
        else if (strcmp(arg1,"06") == 0) {
            save->hour0 = 6;
        }
        else if (strcmp(arg1,"12") == 0) {
            save->hour0 = 12;
        }
        else if (strcmp(arg1,"18") == 0) {
            save->hour0 = 1;
        }
        else fatal_error("fix_CFSv2_fcst: bad arg %s, wanted daily,00,06,12 or 18",arg1);

        save->pert = atoi(arg2);
        save->num_ensemble_members = atoi(arg3);

    }
    save = *local;

    if (mode == -2) {
        sprintf(inv_out,"fix_CFSRv2_fcst %d fields fixed" , save->fixed);
        free(*local);
    }

    if (mode < 0) return 0;

    // only process NCEP CFSv2 monthly forecasts

    // must be NCEP generated
    if (GB2_Center(sec) != NCEP) return 0;

    // subcenter should be 0, 1 (reanalysis), 3 (EMC) or 4 (NCO)
    subcenter = GB2_Subcenter(sec);
    if (subcenter != 0 && subcenter != 1 && subcenter != 3 && subcenter != 4) return 0;

    // must have process id = 82 or 98
    id = analysis_or_forecast_generating_process_identifier(sec);
    if (id != 82 && id != 98) return 0;

    // product defn table must be 8 (fcst average)
    if ( code_table_4_0(sec) != 8) return 0;

    // n_time_ranges should be 1
    if ( (n_time_ranges = sec[4][41]) != 1) {
        fprintf(stderr,"unexected CFSv2 type forecast field .. pdt=8, n=%d\n", n_time_ranges);
        return 0;
    }

    // forecast time units should be months
    if (sec[4][17] != 3 || sec[4][48] != 3) return 0;

    fcst_time_0  = int4(sec[4]+18);
    fcst_time_1  = int4(sec[4]+49) + fcst_time_0;

    // get reference time
    reftime(sec, &ref_year, &ref_month, &ref_day, &ref_hour, &ref_minute, &ref_second);

    if (ref_minute != 0 || ref_second != 0) {
        fprintf(stderr,"unexected CFSv2 minute/second value != 0\n");
        return 0;
    }

    if (fcst_time_0 != 0) {	// start at day=1 hour=save->hour0 of proper month
        fcst0_year = ref_year;
        fcst0_month = ref_month + fcst_time_0;
        if (fcst0_month > 12) {
            i = (fcst0_month - 1) / 12;
            fcst0_year += i;
            fcst0_month -= (i*12);
        }
        fcst0_day = 1;
        fcst0_hour = save->hour0;
        fcst0_minute = 0;
        fcst0_second= 0;
    }
    else {			// start at current month 
        fcst0_year = ref_year;
        fcst0_month = ref_month;
        fcst0_day = ref_day;
        fcst0_hour = ref_hour;
        fcst0_minute = ref_minute;
        fcst0_second= ref_second;
        if (save->dt == 24) {
            i = save->hour0 - fcst0_hour;
            if (i < 0) i += 24;
            add_time(&fcst0_year, &fcst0_month, &fcst0_day, &fcst0_hour, &fcst0_minute, &fcst0_second, i, HOUR);
        }
    }

    sub_time(fcst0_year, fcst0_month, fcst0_day, fcst0_hour, fcst0_minute, fcst0_second, 
            ref_year, ref_month, ref_day, ref_hour, ref_minute, ref_second,
            &dtime, &unit);
    // set forecast time.
    if (dtime == 0) unit = MONTH;
    sec[4][17] = unit;
    int_char(dtime, sec[4]+18);

    if (fcst_time_1 == 0) fatal_error("fix-CFSv2_fcst: unexpected end_ft","");

    if (save->dt == 6) {		// ends at 18Z of last day of month
        fcst1_year = ref_year;
        fcst1_month = ref_month + fcst_time_1-1;
        if (fcst1_month > 12) {
            i = (fcst1_month - 1) / 12;
            fcst1_year += i;
            fcst1_month -= (i*12);
        }
        fcst1_day = num_days_in_month(fcst1_year, fcst1_month);
        fcst1_hour = 18;
        fcst1_minute = 0;
        fcst1_second= 0;

        // time increment = 6 hours
        sec[4][53] = (unsigned char) HOUR;
        int_char(6, sec[4]+54);
    }
    else {
        fcst1_year = ref_year;
        fcst1_month = ref_month + fcst_time_1-1;
        if (fcst1_month > 12) {
            i = (fcst1_month - 1) / 12;
            fcst1_year += i;
            fcst1_month -= (i*12);
        }
        fcst1_day = num_days_in_month(fcst1_year, fcst1_month);
        fcst1_hour = save->hour0;
        fcst1_minute = 0;
        fcst1_second= 0;

        // time increment = 24 hours
        sec[4][53] = (unsigned char)  HOUR;
        int_char(24, sec[4]+54);
    }

    // find stat processing time
    sub_time(fcst1_year, fcst1_month, fcst1_day, fcst1_hour, fcst1_minute, fcst1_second, 
            fcst0_year, fcst0_month, fcst0_day, fcst0_hour, fcst0_minute, fcst0_second, 
            &dtime, &unit);

    // change length of processing
    sec[4][48] = (unsigned char) unit;
    int_char(dtime, sec[4]+49);

    // fprintf(stderr,"length of processing %d unit (%d)\n", dtime, unit);

    // save end-of-processing time
    save_time(fcst1_year, fcst1_month, fcst1_day, fcst1_hour, fcst1_minute, fcst1_second, sec[4]+34);

    // make sure that some basic info is correct
    sec[4][47] = 2;	// fcst_time++


    // now to add ensemble information
    for (i = 0; i < 34; i++) new_sec4[i] = sec[4][i];
    for (i = 34; i <= 57; i++) new_sec4[i+3] = sec[4][i];

    uint_char(61, new_sec4);			// length of new sec[4[
    new_sec4[7] = 0;				// pdt = 11
    new_sec4[8] = 11;

    // add perturbation info
    if (save->pert == 0) {			// pert == 0 means control forecast
        new_sec4[34] = 0;
        new_sec4[35] = 0;
    }
    else { 
        new_sec4[34] = 3;
        new_sec4[35] = save->pert;
    }
    new_sec4[36] = save->num_ensemble_members;
   
    sec[4] = new_sec4;

    save->fixed = save->fixed + 1;
    return 0;
}
