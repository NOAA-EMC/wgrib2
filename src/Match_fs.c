/** @file
 * @brief Process data that matches a fixed string.
 * 
 * Like Match package but uses "fixed strings" and not regular expressions.
 * @author Public Domain: Wesley Ebisuzaki @date 11/2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/** Run fixed-string matching routines flag. */
extern int match_fs;

/** Run flag. */
extern int run_flag;

/** Number of fixed strings to match. */
int match_count_fs;

/** Flag to use fgrep. */
int fgrep;

/** Flag to use fgrep (not used). */
int fgrep_flag;

/** Number of fgrep options. */
int fgrep_count;

/** Store fixed string arguments. */
static const char *match_fs_store[MATCH_MAX];

/** Stores match types. */
static int match_fs_type[MATCH_MAX];

/** Stores match results. */
static int match_fs_val[MATCH_MAX];

/** Stores fgrep arguments. */
static const char *fgrep_store[MATCH_MAX];

/** Stores fgrep types. */
static int fgrep_type[MATCH_MAX];

/**
 * Check if a string matches any fixed string inquiries.
 * 
 * @param s Pointer to string to check.
 *
 * @return 1 if a match is found, 0 otherwise.
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int is_match_fs(const char *s) {
    int i, j;

    /* process match and not tests */
    for (i = 0; i < match_count_fs; i++) {
        if (match_fs_type[i] == 2) continue;
        j = (strstr(s, match_fs_store[i]) == NULL);
        if (j == match_fs_type[i]) return 1;
    }

    /* process  if-tests */
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
    for (i = 0; i < match_count_fs; i++) {
        if (match_fs_type[i] == 2) match_fs_val[i] = (strstr(s, match_fs_store[i]) == NULL);
    }

    return 0;
}


/*
 * HEADER:100:match_fs:setup:1:process data that matches X (fixed string)
 */

/**
 * Process data that matches X (fixed string). 
 * 
 * Similar to -match option, but uses fixed string rather than regular expressions.

 * ## Usage
 * -match_fs "string"
 * 
 * This option works the same as:
 * -set_regex 1 -match "string"
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int f_match_fs(ARG1)  {
    if (mode == -1) {
        if (match_count_fs >= MATCH_MAX) fatal_error("too many -match_fs, -not_fs options","");
        match_fs = 1;
        match_fs_store[match_count_fs] = arg1;
        match_fs_type[match_count_fs] = 1;
        match_count_fs++;
    }
    return 0;
}

/*
 * HEADER:100:not_fs:setup:1:process data that does not match X (fixed string)
 */

/**
 * The -not option and the -not_fs option are very similar. The -not_fs is a not -match_fs. 
 * The difference between -not and -not_fs is the the former uses (extended) regular 
 * expressions and the latter uses "Fixed Strings" (fs). 
 * 
 * wgrib2 -not_fs X (...)
 * 
 * is the same as 
 * 
 * wgrib2 -match_inv file | fgrep -v X | wgrib2 -i (...)
 * wgrib2 -match_fs X  -not_fs Y (...)
 * 
 * is the same as
 * 
 * wgrib2 -match_inv file | fgrep X | fgrep -v Y | wgrib2 -i (...)
 * 
 * where X, and Y are strings. Note, X and Y will not match the second (byte location) field 
 * of the short inventory.
 * 
 * ## Usage
 * -not_fs X
 * 
 * X is a fixed string, not a regular expression.
 * 
 * The -match, and -not selection facility is more limited than the "wgrib2 | filter | wgrib2 -i" 
 * syntax. However, it can be more efficient especially when combined with the -end option. 
 * 
 * ## Future Changes
 * The format of the "match inventory" has evolved and will continue to evolve. The rule for 
 * future changes is that new items in the "match inventory" will be added as the second last 
 * item. Consequently the last item in the inventory will always be ":vt=YYYYMMDDHH:". In order 
 * to future proof your -match, and -not selections, you must not include any item before the 
 * ":vt=YYYYMMDD:" field. 
 * 
 *      -match ":vt=2011111500:"                  good
 *      -not ":vt=2011111500:$"                   good (dollar sign matches the end of the line)
 *      -not ":n=10:vt=2011111500:"               bad (item before :vt=)
 *      -match ":RH:975 mb:anl::vt=2010050806:"   bad (item before :vt=)
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int f_not_fs(ARG1)  {
    if (mode == -1) {
        if (match_count_fs >= MATCH_MAX) fatal_error("too many -match_fs, -not_fs options","");
        match_fs = 1;
        match_fs_store[match_count_fs] = arg1;
        match_fs_type[match_count_fs] = 0;
        match_count_fs++;
    }
    return 0;
}

/*
 * HEADER:100:if_fs:If:1:if X (fixed string), conditional execution on match
 */

/**
 * The -if_fs option is the same as the the -if option except it takes "fixed strings" rather than 
 * extended regular expressions. The -not_if_fs is the same as the -not_if option except it takes 
 * "fixed strings". 
 * 
 * The "fixed string" options have the advantage that matches do not use the regex wildcards. For 
 * example, "-if 3.0" will match "300" because the period is a wildcard character that matches any 
 * character including a zero. 
 * 
 * ## Usage
 * -if_fs X
 * -not_if_fs X
 * 
 * X is a fixed string, not a regular expression.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int f_if_fs(ARG1) {
    struct local_struct {
        int match_cnt;
    };
    struct local_struct *save;

    if (mode == -1) {
        if (match_count_fs >= MATCH_MAX) fatal_error("too many -match_fs, -not_fs -if_fs options","");
        match_fs = 1;
        match_fs_store[match_count_fs] = arg1;
        match_fs_type[match_count_fs] = 2;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation if_fs","");
        save->match_cnt = match_count_fs;
        match_count_fs++;
    }
    else if (mode == -2) {
        free(*local);
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
        run_flag = match_fs_val[save->match_cnt] == 0;
    }
    return 0;
}

/*
 * HEADER:100:not_if_fs:If:1:if X (fixed string) does not match, conditional execution up to next output/fi
 */

/**
 * The -not_if_fs is the same as the -not_if option except it takes "fixed strings". 
 * 
 * The "fixed string" options have the advantage that matches do not use the regex wildcards. For 
 * example, "-if 3.0" will match "300" because the period is a wildcard character that matches any 
 * character including a zero. 
 * 
 * ## Usage
 * -if_fs X
 * -not_if_fs X
 * 
 * X is a fixed string, not a regular expression.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int f_not_if_fs(ARG1) {
    struct local_struct {
        int match_cnt;
    };
    struct local_struct *save;

    if (mode == -1) {
        if (match_count_fs >= MATCH_MAX) fatal_error("too many -match_fs, -not_fs -not_if_fs options","");
        match_fs = 1;
        match_fs_store[match_count_fs] = arg1;
        match_fs_type[match_count_fs] = 2;
        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation not_if_fs","");
        save->match_cnt = match_count_fs;
        match_count_fs++;
    }
    else if (mode == -2) {
        free(*local);
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
        run_flag = (match_fs_val[save->match_cnt] != 0);
    }
    return 0;
}

/*
 * HEADER:100:fgrep:setup:1:fgrep X | wgrib2
 */

/**
 * When you use wgrib2 extensively, common sequences keep occuring, such as, 
 * 
 * @code{.sh}
 * wgrib2 A.grb >A.inv
 * cat A.inv | fgrep ":HGT:" | fgrep ":500 mb:" | wgrib2 -i A.grb -grib hgt500.grb
 * cat A.inv | fgrep ":TMP:" | fgrep ":500 mb:" | wgrib2 -i A.grb -grib tmp500.grb
 * cat A.inv | egrep ":(UGRD|VGRD):" | fgrep ":500 mb:" | wgrib2 -i A.grb -grib wind500.grb
 * @endcode
 * 
 * Using the various -grep, -inv and the -i_file option, the above example can be written as 
 * 
 * @code{.sh}
 * wgrib2 A.grb -inv A.inv
 * wgrib2 -fgrep ":HGT:" -fgrep ":500 mb:" -i_file A.inv A.grb -grib hgt500.grb
 * wgrib2 -fgrep ":TMP:" -fgrep ":500 mb:" -i_file A.inv A.grb -grib tmp500.grb
 * wgrib2 -egrep ":(UGRD|VGRD):" -fgrep ":500 mb:" -i_file A.inv A.grb -grib wind500.grb
 * @endcode
 * 
 * The first version is easier to read. So why were the extra options added? 
 * 
 * 1. Some shells have problems with pipes. 
 *      - Some versions of Windows dos-prompt have problems with pipes. 
 *      - RNomads: solved Windows 7 problem by using these options 
 * 2. More efficient when you avoid multiple processes and pipes. Every millisecond and K byte
 *  of RAM usage counts! 
 * 3. Used by callable wgrib2. A subroutine (wgrib2) can read a field using the index file! 
 * 
 * The options were added for the third reason, but one and two are some nice side effects. 
 * The 4 examples can be coded in fortran as, 
 * 
 * @code{.f}
 * include wgrib2api
 * ...
 * i = wgrib2a('A.grb','-inv','A.inv')
 * i = wgrib2a('-fgrep',':HGT:','-fgrep',':500 mb:','-i_file,'A.inv','A.grb','-grib','hgt500.grb')
 * i = wgrib2a('-fgrep',':TMP:','-fgrep',':500 mb:','-i_file','A.inv','A.grb','-grib','tmp500.grb')
 * i = wgrib2a('-egrep',':(UGRD|VGRD):','-fgrep',':500','mb:','-i_file','A.inv','A.grb','-grib','wind500.grb')
 * @endcode
 * 
 * The -grep options are used in wgrib2api's grb2_inq(..) function. 
 * 
 * Definition of grep options:
 * 
 * (...) | wgrib2 -OP1 X (...)
 * behaves like
 * (...) | OP2 X | wgrib2 (...) 
 * 
 *      if OP1 == egrep       then OP2 = egrep
 *      if OP1 == fgrep       then OP2 = fgrep
 *      if OP1 == egrep_v     then OP2 = egrep -v
 *      if OP1 == fgrep_v     then OP2 = fgrep -v
 * 
 * X is a posix extended regular expression (egrep, egrep_v) or a fixed string (fgrep, fgrep_v)
 * 
 * The number of -fgrep and -fgrep_v options is limited to 200.
 * 
 * The number of -egrep and -egrep_v options is limited to 200.
 * 
 * The wgrib2 option -set_regex does not affect the -grep options.
 * 
 * ## Usage
 * -fgrep X
 * 
 * X is a fixed string (not a regular expression)
 * 
 * Note: -set_regex does not modify the type of regex for these options
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int f_fgrep(ARG1)  {
    if (mode == -1) {
        if (fgrep_count >= GREP_MAX) fatal_error("too many -grep options","");
        fgrep = 1;
        fgrep_store[fgrep_count] = arg1;
        fgrep_type[fgrep_count] = 1;
        fgrep_count++;
    }
    return 0;
}

/*
 * HEADER:100:fgrep_v:setup:1:fgrep -v X | wgrib2
 */

/**
 * When you use wgrib2 extensively, common sequences keep occuring, such as, 
 * 
 * @code{.sh}
 * wgrib2 A.grb >A.inv
 * cat A.inv | fgrep ":HGT:" | fgrep ":500 mb:" | wgrib2 -i A.grb -grib hgt500.grb
 * cat A.inv | fgrep ":TMP:" | fgrep ":500 mb:" | wgrib2 -i A.grb -grib tmp500.grb
 * cat A.inv | egrep ":(UGRD|VGRD):" | fgrep ":500 mb:" | wgrib2 -i A.grb -grib wind500.grb
 * @endcode
 * 
 * Using the various -grep, -inv and the -i_file option, the above example can be written as 
 * 
 * @code{.sh}
 * wgrib2 A.grb -inv A.inv
 * wgrib2 -fgrep ":HGT:" -fgrep ":500 mb:" -i_file A.inv A.grb -grib hgt500.grb
 * wgrib2 -fgrep ":TMP:" -fgrep ":500 mb:" -i_file A.inv A.grb -grib tmp500.grb
 * wgrib2 -egrep ":(UGRD|VGRD):" -fgrep ":500 mb:" -i_file A.inv A.grb -grib wind500.grb
 * @endcode
 * 
 * The first version is easier to read. So why were the extra options added? 
 * 
 * 1. Some shells have problems with pipes. 
 *      - Some versions of Windows dos-prompt have problems with pipes. 
 *      - RNomads: solved Windows 7 problem by using these options 
 * 2. More efficient when you avoid multiple processes and pipes. Every millisecond and K byte
 *  of RAM usage counts! 
 * 3. Used by callable wgrib2. A subroutine (wgrib2) can read a field using the index file! 
 * 
 * The options were added for the third reason, but one and two are some nice side effects. 
 * The 4 examples can be coded in fortran as, 
 * 
 * @code{.f}
 * include wgrib2api
 * ...
 * i = wgrib2a('A.grb','-inv','A.inv')
 * i = wgrib2a('-fgrep',':HGT:','-fgrep',':500 mb:','-i_file,'A.inv','A.grb','-grib','hgt500.grb')
 * i = wgrib2a('-fgrep',':TMP:','-fgrep',':500 mb:','-i_file','A.inv','A.grb','-grib','tmp500.grb')
 * i = wgrib2a('-egrep',':(UGRD|VGRD):','-fgrep',':500','mb:','-i_file','A.inv','A.grb','-grib','wind500.grb')
 * @endcode
 * 
 * The -grep options are used in wgrib2api's grb2_inq(..) function. 
 * 
 * Definition of grep options:
 * 
 * (...) | wgrib2 -OP1 X (...)
 * behaves like
 * (...) | OP2 X | wgrib2 (...) 
 * 
 *      if OP1 == egrep       then OP2 = egrep
 *      if OP1 == fgrep       then OP2 = fgrep
 *      if OP1 == egrep_v     then OP2 = egrep -v
 *      if OP1 == fgrep_v     then OP2 = fgrep -v
 * 
 * X is a posix extended regular expression (egrep, egrep_v) or a fixed string (fgrep, fgrep_v)
 * 
 * The number of -fgrep and -fgrep_v options is limited to 200.
 * 
 * The number of -egrep and -egrep_v options is limited to 200.
 * 
 * The wgrib2 option -set_regex does not affect the -grep options.
 * 
 * ## Usage
 * -fgrep_v X
 * 
 * X is a fixed string (not a regular expression)
 * 
 * Note: -set_regex does not modify the type of regex for these options
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int f_fgrep_v(ARG1)  {
    if (mode == -1) {
        if (fgrep_count >= GREP_MAX) fatal_error("too many -grep options","");
        fgrep = 1;
        fgrep_store[fgrep_count] = arg1;
        fgrep_type[fgrep_count] = 0;
        fgrep_count++;
    }
    return 0;
}

/**
 * Check if a string matches any of the fgrep patterns.
 * 
 * @param s Pointer to the string to check.
 * 
 * @return 1 if the string matches any fgrep pattern, 0 otherwise.
 * 
 * @author Wesley Ebisuzaki @date 11/2014
 */
int is_fgrep(const char *s) {
    int i, j;

    for (i = 0; i < fgrep_count; i++) {
        if (fgrep_type[i] == 2) continue;
        j = (strstr(s, fgrep_store[i]) == NULL);
        if (j == fgrep_type[i]) return 1;
    }

    return 0;
}
