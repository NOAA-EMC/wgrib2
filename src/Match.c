/** @file
 * @brief Process data that matches a regular expression.
 *
 * Requires POSIX-2. If regcomp and regexec are not available, set USE_REGEX = OFF.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2/2008 | W. Ebisuzaki | Initial
 * 4/2015 | W. Ebisuzaki | Added egrep and egrep_v
 * 2/2016 | W. Ebisuzaki | Updated f_match_inv to not depend on verbose mode
 *
 * @author Public Domain: Wesley Ebisuzaki @date 2/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_REGEX
#include <sys/types.h>
#include <regex.h>

/*
 * MATCH package
 *
 *   requires POSIX-2
 *    if regcomp and regexec are not available,  don't define USE_REGEX in makefile
 *
 * Only process messages that match the "-s inventory" using a regular expression
 *  This short circuits having to read the file twice.
 *  Can call f_match a multiple number of times.
 *  ex. -match (dates)  -match (levels)  -match (variables)
 *
 * 2/2008 in public domain Wesley Ebisuzaki
 * 4/2015 added egrep and egrep_v Wesley Ebisuzaki
 * 2/2016 f_match_inv: old match_inv depended on verbose mode, new: mode=0
 *
 */

/** Run matching routines flag. */
extern int match;	

/** Run flag. */
extern int run_flag;

/** Number of regular expressions to match. */
int match_count;

/** Flag to use egrep. */
int egrep;

/** Flag to use egrep (not used). */
int egrep_flag;

/** Number of egrep options. */
int egrep_count;

/** Buffer for regex patterns. */
static regex_t preg[MATCH_MAX];

/** Stores match types. */
static int type[MATCH_MAX];

/** Stores match results. */
static int match_val[MATCH_MAX];

/** Current regex type. */
int regex_type;

/** Stores egrep patterns. */
static regex_t egrep_preg[GREP_MAX];

/** Stores egrep types. */
static int egrep_type[GREP_MAX];

/*
 *  match, if use extended regex library call (like in egrep)
 * 
 *  sometimes want match patterns (like in grep) - i.e. no metacharacters
 *     to do this, we quote all the metacharacters
 *
 *  another mode: you need to quote metacharacters
 */

/**
 * Preprocess the match argument.
 * 
 * If using extended regex library call (like in egrep), sometimes we want match patterns
 * (like in grep) - i.e. no metacharacters. To do this, we quote all the metacharacters.
 *
 * Another mode: you need to quote metacharacters
 * 
 * @param arg The argument to preprocess.
 * 
 * @return The preprocessed argument.
 * 
 * @author Wesley Ebisuzaki @date 2/2008
 */
static char *preprocess_match(const char *arg) {
    int i, c;
    char *str, *p;

    i = strlen(arg)+1;

    if ((str = p = (char *) malloc(regex_type == 0 ? i : i+i)) == NULL) 
		fatal_error("Match: ran out of memory in preprocess_match","");

    // regex_type == 0   use extended POSIX regular expressions
    if (regex_type == 0) {
        strncpy(str,arg,i);
        return str;
    }

    // regex_type == 1   patterns
    if (regex_type == 1) {
        while ((c = *arg++)) {
            if (c == '?' || c == '+' || c == '|' || c == '.' || c == '[' || c == ']' || c == '^' || c == '*' || 
                    c == '$' || c == '(' || c == ')' || c == '}' || c == '{' || c == '\\') {
                *p++ = '\\';
            }
            *p++ = c;
        }
        *p = 0;
        return str;
    }

    // regex_type == 2   extended regular expressions but need to quote metacharacters
    if (regex_type == 2) {
        while ((c = *arg++)) {
            if (c == '?' || c == '+' || c == '|' || c == '.' || c == '[' || c == ']' || c == '^' || c == '$' || c == '*'
                || c == '(' || c == ')' || c == '}' || c == '{') {
                *p++ = '\\';
            }
            else if (c == '\\') {
                if (*arg) c = *arg++;
            }
            *p++ = c;
        }
        *p = 0;
        return str;
    }
    fatal_error("programing error in Match preprocessing","");
    return NULL;
}

/*
 * HEADER:100:match:setup:1:process data that matches X (POSIX regular expression)
 */

/**
 * Process data that matches a POSIX regular expression.
 * 
 * The -match option selects records which should be processed. When multiple -match 
 * options are used, all matches must be satisfied. The -match and -not options seem to 
 * be similar to the -if and -if_not options. The big difference is that the -match and 
 * -not options are processed before any other record processing. If the record satisfies 
 * the -match and -not options, then the record is processed. This include the optional 
 * decoding and latitude-longitude calculation and the other options. 
 * 
 * wgrib2 -match X (...)
 * 
 * is the same as
 * 
 * wgrib2 -match_inv file | egrep X | wgrib2 -i (...)
 * wgrib2 -match X -match Y -not Z (...)
 * 
 * is the same as
 * 
 * wgrib2 -match_inv file | egrep X | egrep Y | egrep -v Z | wgrib2 -i (...)
 * 
 * where X, Y and Z are regular expressions. 
 * 
 * If X, Y and Z are "fixed strings" rather than regular expressions, use -match_fs, 
 * and -not_fs.
 * 
 * ## Usage
 * -match X
 *
 * X is a posix extended regular expression.
 * 
 * The -match, and -not selection facility is more limited than the "wgrib2 | filter | wgrib2 -i" 
 * syntax. However, it can be more efficient especially when combined with the -end option. Note 
 * that the "match" inventory often expands. Usually the inventory expands by adding new items to 
 * the end of the inventory in order not the break scripts. 
 * 
 * ## -match vs -if
 * The -match, and -if can be confused. The -match option selects the fields that are to be processed 
 * by the command line. The -if option selects the fields that will be processed and the selection 
 * ends at the next -fi or output option. For example, 
 * 
 * 1. wgrib2 IN.grb -match ":UGRD:200 mb:anl:" -csv u200.csv
 * 2. wgrib2 IN.grb -if ":UGRD:200 mb:anl:" -csv u200.csv
 * 
 * Lines 1 and 2 will produce the same CSV file. However, line 1 will only process one field. For line 
 * 1, only only one field will be docoded and converted to a CSV file. For line 2, all the fields will 
 * be processed and only one field will be converted to a CSV file. The total processing will be the 
 * docoding of all the fields in the file and one conversion to a CSV file. 
 * 
 * ## Future Changes
 * The format of the "match inventory" has evolved and will continue to evolve. The rule for future changes 
 * is that new items in the "match inventory" will be added as the second last item. Consequently the last 
 * item in the inventory will always be ":vt=YYYYMMDDHH:". In order to future proof your -match, and -not 
 * selections, you must not include any item before the ":vt=YYYYMMDD:" field. 
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
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_match(ARG1)  {
    char *s;
    if (mode == -1) {
        if (match_count >= MATCH_MAX) fatal_error("too many -match, -not options","");
        match = 1;
        s = preprocess_match(arg1);
        if (regcomp(&(preg[match_count]), s, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) 
            fatal_error("bad regular expression \"%s\"", arg1);
        type[match_count] = 1;
        *local = &(preg[match_count]);	// pattern buffer to free at end
        match_count++;
        free(s);
    }
    else if (mode == -2) {
        regfree(*local);		// free patter buffer
    }
    return 0;
}

/*
 * HEADER:100:not:setup:1:process data that does not match X (POSIX regular expression)
 */

/**
 * Process data that does not match a POSIX regular expression.
 * 
 *  When multiple -match/-not options are used, all must be satisfied. 
 * 
 * wgrib2 -not X (...)
 * 
 * is the same as
 * 
 * wgrib2 -match_inv file | egrep -v X | wgrib2 -i (...)
 * wgrib2 -match X  -not Y (...)
 * 
 * is the same as
 * 
 * wgrib2 -match_inv file | egrep X | egrep -v Y | wgrib2 -i (...)
 * 
 * where X, Y and Z are regular expressions. Note, X and Y will not match the second (byte 
 * location) field of the short inventory.
 * 
 * If X, Y and Z are "fixed strings" rather than regular expressions, use -match_fs, 
 * and -not_fs.
 * 
 * ## Usage
 * -not X
 *
 * X is a posix extended regular expression.
 * 
 * The -match, and -not selection facility is more limited than the "wgrib2 | filter | wgrib2 -i" 
 * syntax. However, it can be more efficient especially when combined with the -end option. 
 * 
 * ## Future Changes
 * The format of the "match inventory" has evolved and will continue to evolve. The rule for future changes 
 * is that new items in the "match inventory" will be added as the second last item. Consequently the last 
 * item in the inventory will always be ":vt=YYYYMMDDHH:". In order to future proof your -match, and -not 
 * selections, you must not include any item before the ":vt=YYYYMMDD:" field. 
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
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_not(ARG1)  {
    char *s;
    if (mode == -1) {
        if (match_count >= MATCH_MAX) fatal_error("too many -match, -not options","");
        match = 1;
        s = preprocess_match(arg1);
        if (regcomp(&(preg[match_count]), s, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) 
            fatal_error("bad regular expression \"%s\"", arg1);
        type[match_count] = 0;
        *local = &(preg[match_count]);	// pattern buffer to free at end
        match_count++;
        free(s);
    }
    else if (mode == -2) {
        regfree(*local);		// free patter buffer
    }
    return 0;
}

/*
 * HEADER:100:if:If:1:if X (POSIX regular expression), conditional execution on match
 */

/**
 * If X (POSIX regular expression), conditional execution on match.
 * 
 * The -if option returns either true or false. What happens next depends on whether you are 
 * using Version 1 or Version 2 of the "If blocks". Prior to wgrib2 v3.0.0, you only get 
 * Version 1. With wgrib2 3.0.0+, you can use either Version 1 or 2 but not both at the same 
 * time. 
 * 
 * ## Version 1
 * For Version 1 "IF blocks", the -if option returns true or false. If true, then all the 
 * options up to and including the next "output" option are executed. If false, then all the 
 * options up to and including the next "output" option are not executed. Options following 
 * the output option are executed as normal.With Version 1, nesting of IF blocks is not 
 * defined. The option -fi is a NULL output option. Note that -fi cannot be used in Version 2 
 * "IF blocks". 
 * 
 * ### Definition
 * 
 * wgrib2 (...) [if] [list of non-ouput options] [output option]
 * 
 * [if] = -if, -not_if, -if_fs, -not_if_fs, -if_n, -if_rec
 * 
 * if [if] returns the value of false, the list of non-output options and the next output 
 * options are not executed.
 * 
 * if [if] returns the value of true, the following options are executed.
 * 
 * [output option] is the terminator of the if block
 * 
 * If blocks should not be nested. The results are undefined.
 * 
 * ## Version 2
 * For Version 2 of the IF blocks, wgrib2 uses -if, -elseif, -else and -endif options to 
 * implement a proper IF blocking structure. IF blocks can be nested. Note, the "if" flag is 
 * reset prior to processing of a record. 
 * 
 * ### Definition
 * 1. The if blocks can now be nested, and are terminated by an -endif.
 * 2. Within the if block, you can have -else and -elseif options.
 * 3. The -elseif options must proceed the -else option.
 * 4. The -fi option is not allowed.
 * 5. Version 1 and Version 2 if blocks cannot be mixed.
 * 
 * ## Limitations
 * The Version 1 limits, the maximum number of -if options on a command line is limited by (1) 
 * system limit of open files (2) maximum length of a command line, (3) maximum number of regular 
 * expressions allowed MAX_MATCH (wgrib.h), and (4) maximum number of parsed arguments N_ARGLIST 
 * (wgrib.h). 
 * 
 * The Version 2 limits include the Version 1 limits and include a limit of the number of nested 
 * IF blocks (10). 
 * 
 * Version 1 and Version 2 IF blocks cannot be mixed. The -fi option is restricted to Version 1 
 * IF blocks. There are NO plans to eliminate Version 1 IF blocks. However, Version 2 IF blocks 
 * are recommended for future development. 
 * 
 * ## Usage
 * -if X
 * 
 * X is a regular expression.
 * 
 * Sets the "run_flag" if X matches the "match inventory".
 * 
 * ## Future Changes
 * The format of the "match inventory" has evolved and will continue to evolve. The rule for future changes 
 * is that new items in the "match inventory" will be added as the second last item. Consequently the last 
 * item in the inventory will always be ":vt=YYYYMMDDHH:". In order to future proof your -match, and -not 
 * selections, you must not include any item before the ":vt=YYYYMMDD:" field. 
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
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_if(ARG1)  {
    struct local_struct {
        int match_cnt;
        regex_t *preg;
    };
    struct local_struct *save;
    char *s;

    if (mode == -1) {
        if (match_count >= MATCH_MAX) fatal_error("too many -match, -not -if options","");
        match = 1;
        s = preprocess_match(arg1);
        if (regcomp(&(preg[match_count]), s, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) 
            fatal_error("bad regular expression \"%s\"", arg1);
        type[match_count] = 2;
        free(s);

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_if","");
        save->match_cnt = match_count;
        save->preg = &(preg[match_count]);
        match_count++;
    }
    else if (mode == -2) {
        save = (struct local_struct *) *local;
        regfree(save->preg);
        free(save);
    }
    else if (mode >= 0) {
        save = (struct local_struct *) *local;
        run_flag = match_val[save->match_cnt] == 0;
    }
    return 0;
}

/*
 * HEADER:100:not_if:If:1:not_if X (regular expression), conditional execution on not match
 */

/**
 * Returns true or false and is the start of the IF block structure. The -not_if option is 
 * like the -if option except the results are reversed. For example, -if ':HGT:' is true if 
 * the match inventory contains the string ':HGT:', and the next block is executed. The option 
 * -not_if ':HGT:' does the opposite and does not execute the next block. See -if for details 
 * about the IF block structure. 
 * 
 * ## Usage
 * -not_if X
 *
 * X is a regular expression.
 * 
 * Returns true if X is not in the match inventory, false otherwise.
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_not_if(ARG1)  {
    struct local_struct {
        int match_cnt;
        regex_t *preg;
    };
    struct local_struct *save;
    char *s;

    if (mode == -1) {
        if (match_count >= MATCH_MAX) fatal_error("too many -match, -not -if options","");
        match = 1;
        s = preprocess_match(arg1);
        if (regcomp(&(preg[match_count]), s, REG_EXTENDED | REG_NOSUB | REG_NEWLINE))
            fatal_error("bad regular expression \"%s\"", arg1);
        type[match_count] = 2;
        free(s);

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("memory allocation f_if","");
        save->match_cnt = match_count;
        save->preg = &(preg[match_count]);
        match_count++;
    }
    else if (mode == -2) {
	save = (struct local_struct *) *local;
        regfree(save->preg);
        free(save);
    }
    else if (mode >= 0) {
        save = (struct local_struct*) *local;
        run_flag = match_val[save->match_cnt] != 0;
    }
    return 0;
}



/*
 * is_match
 *
 * return codes
 *     1                      ignore grib message
 *     0                      continue process
 */

/**
 * Checks if the input string matches any of the defined patterns.
 * 
 * @param s Pointer to the input string to check.
 * 
 * @return 1 if a match is found, 0 otherwise.
 *
 * @author Wesley Ebisuzaki @date 2/2008
 */
int is_match(const char *s) {
    int i, j;

    /* process match and not tests */

    /* typically # -match and -not is small .. tests show openmp doesn't help */
    for (i = 0; i < match_count; i++) {
        if (type[i] == 2) continue;
        j = regexec(&(preg[i]), s, (size_t) 0, NULL, 0) != 0;
        if (j == type[i]) return 1;
    }

    /* no need to process if-tests if no match */

    /* process  if-tests, regexec is thread safe by POSIX standard */
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
    for (i = 0; i < match_count; i++) {
        if (type[i] == 2) match_val[i] = regexec(&(preg[i]), s, (size_t) 0, NULL, 0);
    } 

    return 0;
}

/**
 * Checks if the input string matches any of the defined egrep patterns.
 * 
 * @param s Pointer to the input string to check.
 * 
 * @return 1 if a match is found, 0 otherwise.
 *
 * @author Wesley Ebisuzaki @date 2/2008
 */
int is_egrep(const char *s) {
    int i, j;

    /* process egrep and egrep_v  tests */
    for (i = 0; i < egrep_count; i++) {
        j = regexec(&(egrep_preg[i]), s, (size_t) 0, NULL, 0) != 0;
        if (j == egrep_type[i]) return 1;
    }
    return 0;
}



/*
 * HEADER:100:set_regex:setup:1:set regex mode X = 0:extended regex (default) 1:pattern 2:extended regex & quote metacharacters 
 */

/**
 * Set regex mode.
 * 
 * Modes are
 * 
 * 0 - extended regex (default)
 * 1 - fixed string mode, no metacharacters
 * 2 - metacharacters need to be quoted
 * 
 * One common feature of many Unix/Linux commands is the support of regular expressions (regex). 
 * Several of wgrib2's options support regex (-if, -not_if, -match, -not, -egrep, -egrep_v) which 
 * make the various string comparisons much more versatile (compare egrep vs windows find command). 
 * The -set_regex option changes the flavor of regex evaluation. 
 * 
 * NOTE: the -set_regex option MUST preceed the -match/-not/-if/-not_if/-egrep/-egrep_v options. 
 * All these options are "setup" options and the processing the the -set_regex must occur before the 
 * processing of the -match and other regex options. 
 * 
 * Grib files can have hundreds of records and most people only want a few of the records. Rather 
 * than processing everything, you can select the specific records to process by the -match, -not, 
 * -if and -not if options. These options take an extended POSIX regular expression (regex) as their 
 * sole argument. These options check the regex with the "match inventory" (see -match_inv). Some 
 * examples are 
 * 
 * @code{.sh}
 * wgrib2 input.grb -match ':UGRD:200 mb:' -grib u.grb
 * wgrib2 input.grb -match ':(UGRD|VGRD|TMP):200 mb:' -grib uvt.grb
 * @endcode
 * 
 * Now regex are powerful but can produce some surprises. For example, you want the 19th (positive 
 * perturbation) ensemble member which is denoted by 'ENS=+19' in the match inventory. You try, 
 * 
 * @code{.sh}
 * wgrib2 input.grb -match ':ENS=+19:' -grib e19.grb
 * @endcode
 * 
 * Surprise, the above line does not work. The plus sign is a regex metacharacter indicating that the 
 * previous character would be matched 1 or more times. Consequently the plus sign wouldn't be matched. 
 * To get the above match to work, you can quote the plus sign with a backslash. 
 * 
 * @code{.sh}
 * wgrib2 input.grb -match ':ENS=\+19:' -grib e19.grb
 * @endcode
 * 
 * Alternatively you could change the regex match into fixed-string mode. In fixed-string mode, the regex 
 * metacharacters are considered to be ordinary characters. 
 * 
 * @code{.sh}
 * wgrib2 -set_regex 1 input.grb -match ':ENS=+19:' -grib e19.grb
 * @endcode
 * 
 * Most of the regex options have a fs (fixed string) version, such as -match and -match_fs. It is better 
 * to used the fixed-string versions are equivalent to the regex mode set to "fixed string". The fixed 
 * string versions were added because regex library may be unavaible on non-POSIX systems. 
 * 
 * @code{.sh}
 * wgrib2 input.grb -match_fs ':ENS=+19:' -grib e19.grb
 * @endcode
 * 
 * The third mode is the metacharacters have to be quoted. Here is an example that gets the 19th, 20th and 
 * 21th ensemble members. This mode was added because it is easier to remember to quote the '(|)' 
 * metacharacters than to quote the ordinary characters correspond to metacharacters. 
 * 
 * @code{.sh}
 * wgrib2 -set_regex 2 input.grb -match ':ENS=+\(19\|20\|21\):' -grib e19_20_21.grb
 * @endcode
 * 
 * ## Usage
 * -set_regex X
 * 
 * X = 0, 1, 2
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_set_regex(ARG1)  {
    if (mode == -1) {
        if (strcmp(arg1,"0") == 0) regex_type = 0;
        else if (strcmp(arg1,"1") == 0) regex_type = 1;
        else if (strcmp(arg1,"2") == 0) regex_type = 2;
        else fatal_error("-set_regex %s", arg1);
    }
    return 0;
} 

/*
 * HEADER:100:egrep:setup:1:egrep X | wgrib2 (X is POSIX regular expression)
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
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_egrep(ARG1)  {
    if (mode == -1) {
        if (match_count >= GREP_MAX) fatal_error("too many -egrep/-egrep_v options","");
        egrep = 1;

        if (regcomp(&(egrep_preg[egrep_count]), arg1, REG_EXTENDED | REG_NOSUB | REG_NEWLINE))
            fatal_error("bad regular expression \"%s\"", arg1);
        egrep_type[egrep_count] = 1;

        *local = &(egrep_preg[egrep_count]);  // pattern buffer to free at end
        egrep_count++;
    }
    else if (mode == -2) {
        regfree(*local);                // free patter buffer
    }
    return 0;
}

/*
 * HEADER:100:egrep_v:setup:1:egrep -v X | wgrib2 (X is POSIX regular expression)
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
 * @author Wesley Ebisuzaki @date 2/2008
 */
int f_egrep_v(ARG1)  {
    if (mode == -1) {
        if (match_count >= GREP_MAX) fatal_error("too many -egrep/-egrep_v options","");
        egrep = 1;

        if (regcomp(&(egrep_preg[egrep_count]), arg1, REG_EXTENDED | REG_NOSUB | REG_NEWLINE))
            fatal_error("bad regular expression \"%s\"", arg1);
        egrep_type[egrep_count] = 0;

        *local = &(egrep_preg[egrep_count]);  // pattern buffer to free at end
        egrep_count++;
    }
    else if (mode == -2) {
        regfree(*local);                // free patter buffer
    }
    return 0;
}


#else
int f_match(ARG1)  {
    if (mode == -1) {fprintf(stderr,"MATCH package not installed\n"); return 1;}
    return 1;
}
int f_not(ARG1)  {
    if (mode == -1) {fprintf(stderr,"MATCH package not installed\n"); return 1;}
    return 1;
}
int f_if(ARG1)  {
    if (mode == -1) {fprintf(stderr,"MATCH package not installed\n"); return 1;}
    return 1;
}
int f_not_if(ARG1)  {
    if (mode == -1) {fprintf(stderr,"MATCH package not installed\n"); return 1;}
    return 1;
}
int f_set_regex(ARG1)  {
    if (mode == -1) {fprintf(stderr,"MATCH package not installed\n"); return 1;}
    return 1;
}
int f_egrep(ARG1)  {
    if (mode == -1) {fprintf(stderr,"MATCH package not installed\n"); return 1;}
    return 1;
}
int f_egrep_v(ARG1)  {
    if (mode == -1) {fprintf(stderr,"MATCH package not installed\n"); return 1;}
    return 1;
}
#endif

