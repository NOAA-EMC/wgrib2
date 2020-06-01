#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"


/*
 * MATCH_fs package
 *
 * like Match package but uses "fixed strings" and not regular expressions
 *
 * 11/2014 in public domain Wesley Ebisuzaki
 *
 */

extern int match_fs;
extern int match_flag;
int match_count_fs;

int fgrep, fgrep_flag, fgrep_count;

static const char *match_fs_store[MATCH_MAX];
static int match_fs_type[MATCH_MAX];
static int match_fs_val[MATCH_MAX];

static const char *fgrep_store[MATCH_MAX];
static int fgrep_type[MATCH_MAX];


int is_match_fs(const char *s) {
    int i, j;

    /* process match and not tests */
    for (i = 0; i < match_count_fs; i++) {
        if (match_fs_type[i] == 2) continue;
        j = (strstr(s, match_fs_store[i]) == NULL);
        if (j == match_fs_type[i]) return 1;
    }

    /* process  if-tests */
#pragma omp parallel for private(i)
    for (i = 0; i < match_count_fs; i++) {
        if (match_fs_type[i] == 2) match_fs_val[i] = (strstr(s, match_fs_store[i]) == NULL);
    }

    return 0;
}


/*
 * HEADER:100:match_fs:setup:1:process data that matches X (fixed string)
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
 * HEADER:100:if_fs:misc:1:if X (fixed string) matches, conditional execution up to next output/fi
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
        match_flag = match_fs_val[save->match_cnt];
    }
    return 0;
}

/*
 * HEADER:100:not_if_fs:misc:1:if X (fixed string) does not match, conditional execution up to next output/fi
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
        match_flag = (match_fs_val[save->match_cnt] == 0);
    }
    return 0;
}

/*
 * HEADER:100:fgrep:setup:1:fgrep X | wgrib2
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

int is_fgrep(const char *s) {
    int i, j;

    for (i = 0; i < fgrep_count; i++) {
        if (fgrep_type[i] == 2) continue;
        j = (strstr(s, fgrep_store[i]) == NULL);
        if (j == fgrep_type[i]) return 1;
    }

    return 0;
}
