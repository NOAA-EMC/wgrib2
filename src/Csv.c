/** @file
 * @brief Write data to a CSV file.
 * @author Niklas Sondell @date 2008
 */

/*
 Copyright (C) 2008 Niklas Sondell, Storm Weather Center
 This file is part of wgrib2 and could be distributed under terms of the GNU General Public License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Decode grib file flag. */
extern int decode;

/** Flush output flag. */
extern int flush_mode;

/** Append grib file flag. */
extern int file_append;

/** NDFD local section 2 keys. */
extern int WxText;

/** Number of keys. */
extern int WxNum;

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/*
 * HEADER:100:csv:output:1:make comma separated file, X=file (WxText enabled)
 */

/**
 * Writes the grid values to a specified file as a comma separated values (text) which 
 * can be imported into a spread sheet. This function is similar to -spread and -csv_long 
 * with a different output format. 
 * 
 * The format is: 
 *      "time0","time1","field","level",longitude,latitude,grid-value
 * 
 * For an analysis that is valid at a specific time, time0 and time1 will have the same value. 
 * For a temporal average or accumulation of analyses, time0 will be the start and time1 will 
 * be end of the average or accumulation.
 *
 * For a forecast, time0 will be the start of the forecast (i.e, date of the initial conditions). 
 * For a forecast for a specific time, then time1 will be time that the forecast is valid. For a 
 * forecast of a temporal average or accumulation, time1 wll be the end of the average or accumulation.
 *
 * In grib speak, time0 is the reference time and time1 is the verification time.
 * 
 * The -csv option only works on the grids for which wgrib2 can derive latitude and longitudes 
 * values. Otherwise no output is generated. The undefined value is 9.999e20. 
 * 
 * ## Extended Variable Names
 * The default field value (see above) is the grib name such as TMP or HGT. However, the grib name 
 * may not be unique. For example, the field could be the HGT from the 19th ensemble member. 
 * A better field name may be "HGT.ENS=+19". You enable the extended variable name by adding the 
 * option -set_ext_name 1.
 * 
 * ## Usage
 * -csv output_file_name
 * 
 * The CSV is written to output_file_name (cannot be a memory file)
 * 
 * -set_ext_name 1 -csv output_file_name
 * 
 * The field is the extended grib name.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success, error code otherwise.
 * 
 * @note The size of a CSV file can be overwhelming. One technique is to only generate the CSV for 
 * variables that you need. This can be done with the -match option. Even with the -match option, 
 * the high-resolution models can stil generate huge files. The next technique is to undefine the
 * grid points that you don't want. For example, you are only interested in Hawaii (its 0C outside 
 * as I write this). Then you can use the the -undefine option to set the grid points outside of 
 * the Hawaii domain to undefined. Since -csv_long doesn't print undefined grid points, the CSV 
 * files is much smaller. 
 * 
 * @note Warning 1: The options -csv, -csv_long, -spread and -text do not support memory files. You 
 * can blame sloth or lack of need. I like to think that text files with grid point values are insanely 
 * large and shouldn't be saved in memory. 
 * 
 * @note Warning 2: It may be tempting to take a grib file, convert it into a CSV file and then deal 
 * with the CSV file. After all, everybody can read a CSV file. Sure there is a litte overhead of reading 
 * a CSV file but who cares. Suppose you want to read some GFS forecasts files (20 forecast times, 5 days 
 * every 6 hours) at 0.25 x 0.25 degree global resolution. Your CSV file is going to be about 720 GBs. Suppose 
 * that our hard drive can write/read at 70 MB/s. Then we are talking about 3 hours to write the CSV file and 3 
 * hours to read the CSV file not including CPU time which will slow down the process. Converting grib into CSV 
 * is a viable strategy if the conversion is limited. You need to restrict the number of fields converted and 
 * should consider only converting a regional domain. Note, I wrote "viable" and not optimal. 
 * 
 * ## Example:
 * ???
 * 
 * @author Niklas Sondell @date 2008
 */
int f_csv(ARG1) {

    char new_inv_out[STRING_SIZE];
    char name[100], desc[100], unit[100];
    FILE *out;

    unsigned int j;
    char vt[20],rt[20];
    int year, month, day, hour, minute, second;

    /* initialization phase */

    if (mode == -1) {
        WxText = decode = latlon = 1;
        if ((*local = (void *) ffopen(arg1,file_append ? "a" : "w")) == NULL)
            fatal_error("csv could not open file %s", arg1);  
        return 0;
    }

    /* cleanup phase */

    if (mode == -2) {
        ffclose((FILE *) *local);
        return 0;
    }

    /* processing phase */

    if (lat == NULL || lon == NULL) {
        fprintf(stderr,"csv: latitude/longitude not defined, record skipped\n");
        return 0;
    }

    out = (FILE *) *local;

    /*Collect runtime and validtime into vt and rt*/

    reftime(sec, &year, &month, &day, &hour, &minute, &second);
    sprintf(rt, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year,month,day,hour,minute,second);

    vt[0] = 0;
    if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
        sprintf(vt,"%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year,month,day,hour,minute,second);
    }

    /*Get levels, parameter name, description and unit*/

    *new_inv_out = 0;
    f_lev(call_ARG0(new_inv_out,NULL));

    if (strcmp(new_inv_out, "reserved")==0) return 0;
//    getName(sec, mode, NULL, name, desc, unit);
    getExtName(sec, mode, NULL, name, desc, unit);
//	fprintf(stderr,"Start processing of %s at %s\n", name, new_inv_out);
//	fprintf(stderr,"Gridpoints in data: %d\n", ndata);
//	fprintf(stderr,"Description: %s, Unit %s\n", desc,unit);

     /* Lage if-setning rundt hele som sjekker om alt eller deler skal ut*/

    if (WxNum > 0) {
        for (j = 0; j < ndata; j++) {
            if (!UNDEFINED_VAL(data[j])) {
                fprintf(out,"\"%s\",\"%s\",\"%s\",\"%s\",%g,%g,\"%s\"\n",rt,vt,name,
                new_inv_out,lon[j] > 180.0 ?  lon[j]-360.0 : lon[j],lat[j],WxLabel(data[j]));
            }
        }
    }
    else {
        for (j = 0; j < ndata; j++) {
            if (!UNDEFINED_VAL(data[j])) {
                fprintf(out, "\"%s\",\"%s\",\"%s\",\"%s\",%g,%g,%lg\n",rt,vt,name,
                    new_inv_out,lon[j] > 180.0 ?  lon[j]-360.0 : lon[j],lat[j],data[j]);
            }
        }
    }
    if (flush_mode) fflush(out);
    return 0;
}
