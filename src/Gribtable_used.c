/** @file
 * @brief Write out the GRIB table that was used. 
 *
 * Make a copy of the gribtable used, edit and use it with wgrib2
 * @author Manfred Schwarb @date 10/2020
 */

/*
 This file is part of wgrib2 and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

 Copyright (C) 2020 Manfred Schwarb

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/** Line size for temporary storage. */
#define LINE_SIZE 1000

/** Append grib file flag. */
extern int file_append;

/** Flush of output flag. */
extern int flush_mode;

/*
 * HEADER:100:gribtable_used:output:1:write out sample gribtable as derived from grib file, X=file
 */

/**
 * Write out sample gribtable as derived from grib file.
 * 
 * The grib variable table that is built in to wgrib2 is based on the NCEP table and entries 
 * from other centers. For the WMO-defined entries, NCEP names are used. You may want to change 
 * the grib table because 
 * 
 * 1. grib table is incomplete
 * 2. you want to use names that make sense
 * 3. the center's names for locally defined fields are not included in wgrib2
 * 4. you want to use an unofficial table
 * 
 * [To solve this problem, you can set up user grib tables.]
 * (https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/user_grib2tables.html)
 * 
 * Now the -gribtable_used option helps you set up the user grib table. The option writes grib 
 * table entries used by a file. 
 * 
 * ## Usage
 * -gribtable_used OUTPUT
 * 
 * OUTPUT is a file with the grib table entries for each field
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * 
 * @code{.sh}
 * $ wgrib2 gep19.t00z.pgrb2af180 -gribtable_used junk
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * 2:46042:d=2009060500:TMP:200 mb:180 hour fcst:ENS=+19
 * 3:63079:d=2009060500:RH:200 mb:180 hour fcst:ENS=+19
 * ...
 * @endcode
 * 
 * Get rid of duplicate entries. Edit grbtbl, change UGRD to U, VGRD to V and TMP to T.
 * 
 * @code{.sh}
 * sort -u junk > grbtbl
 * @endcode
 * 
 * Define a user grib table.
 * @code{.sh}
 * $ export grib2table=`pwd`/grbtbl  
 * @endcode
 * 
 * @code{.sh}
 * $ wgrib2 gep19.t00z.pgrb2af180 -match '(U|V|T)'
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * 2:46042:d=2009060500:T:200 mb:180 hour fcst:ENS=+19
 * 3:63079:d=2009060500:RH:200 mb:180 hour fcst:ENS=+19
 * 4.1:86046:d=2009060500:U:200 mb:180 hour fcst:ENS=+19
 * 4.2:86046:d=2009060500:V:200 mb:180 hour fcst:ENS=+19
 * 5:137483:d=2009060500:HGT:250 mb:180 hour fcst:ENS=+19
 * 6:184669:d=2009060500:T:250 mb:180 hour fcst:ENS=+19
 * ..
 * @endcode
 * 
 * Note: U and V will not be treated as vectors in -new_grid. You have to use -new_grid_vectors to 
 * get U and V to be vectors.
 * 
 * @author Manfred Schwarb @date 10/2020
 */
int f_gribtable_used(ARG1) {
    int discipline, center, localtab, parmcat, parmnum;
    int mtab_set, mtab_low, mtab_high;
    char name[STRING_SIZE], desc[STRING_SIZE], unit[STRING_SIZE], tmp_line[LINE_SIZE];
    struct seq_file *save;

    /* initialization phase */
    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("bin: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
        return 0;
    }
    /* cleanup phase */
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
        free(save);
        return 0;
    }
    save = *local;

    /* processing phase */
    discipline = GB2_Discipline(sec);
    center = GB2_Center(sec);
    localtab = GB2_LocalTable(sec);
    parmcat = GB2_ParmCat(sec);
    parmnum = GB2_ParmNum(sec);
    getName_all(sec, mode, NULL, name, desc, unit, &mtab_set, &mtab_low, &mtab_high);

    /* Example output string: "0:1:0:255:0:0:0:1:VTMP:Virtual Temperature:K" */
    snprintf(tmp_line,LINE_SIZE,"%d:%d:%d:%d:%d:%d:%d:%d:%s:%s:%s\n",discipline,mtab_set,mtab_low,mtab_high,
            center,localtab,parmcat,parmnum,name,desc,unit);

    fwrite_file((void *) tmp_line, sizeof(char), strnlen(tmp_line,LINE_SIZE), save);
    if (flush_mode) fflush_file(save);
    return 0;
}
