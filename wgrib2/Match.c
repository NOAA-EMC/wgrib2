#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
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

extern int match;			/* run matching routines */
extern int run_flag;

int match_count;
int egrep, egrep_flag, egrep_count;


static regex_t preg[MATCH_MAX];
static int type[MATCH_MAX];
static int match_val[MATCH_MAX];
int regex_type;

static regex_t egrep_preg[GREP_MAX];
static int egrep_type[GREP_MAX];

/*
 *  match, if use extended regex library call (like in egrep)
 * 
 *  sometimes want match patterns (like in grep) - i.e. no metacharacters
 *     to do this, we quote all the metacharacters
 *
 *  another mode: you need to quote metacharacters
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
#pragma omp parallel for private(i)
    for (i = 0; i < match_count; i++) {
        if (type[i] == 2) match_val[i] = regexec(&(preg[i]), s, (size_t) 0, NULL, 0);
    } 

    return 0;
}

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

