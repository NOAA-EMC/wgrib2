#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* 2/2019 in public domain Wesley Ebisuzaki */
/* routines for version 2 of if blocks */

/* runtime flags */

int run_flag, if_run, else_allowed;		/* current flags */

static struct { int run_flag, if_run, else_allowed; }  if_state[MAX_IF];	/* old flags */
static int if_state_top;

static int check_state[MAX_IF];		/* 1..N IF, 0 = NULL, ELSE_NOT_ALLOWED */
static int check_state_top;

static int is_v2_if;
static int has_fi;

int init_check_v1_v2(void) {

   if_state_top = -1;
   has_fi = 0;
   check_state_top = 0;
   check_state[check_state_top] = 0;
   is_v2_if = 0;
   return 0;
}

/* check correctness of if/else/elseif/endif structure
 *
 * complication: can be old if structure where ifs are not
 * terminated by output rather than if.
 *
 * 
 * State:
 *  0  = start . no ifs
 * -1 = else
 *  1..N  number of if blocks
 *
 *  if:  if state is if, state++
 *  	 else state = 1;
 *
 *  else: if state is not if .. error
 *        push state, state = -1 (else)
 *
 *  elseif: if state is not if then error
 *
 *  endif: if state == else, pop state
 *          state--;
 *          if (state == 0) pop state
 *
 *
 * to check if version 0 or 1 of if structure
 *
 * has else/elseif/endif and no fi -> version 1
 * otherwise version 0
 *
 * 
 * execution:   this assumes a correct syntax
 * init: run_flag = 0;
 *       empty stack;
 *       if_run = 0;
 *
 * if (version_if == 0) {
 *      if (run_flag == 0) {
 *              run option;
 *              // some options (if) can change the run_flag to 0/1
 *      }
 *      else {
 *              if (option_type == output) run_flag = 0;
 *      }
 * }
 * else {
 *      if (option_type == if) {
 *         push(run_flag,if_run,else_allowed);
           if_run = 0;
 *      }
 *      elseif (option_type == endif) {
 *         pop(run_flag,if_run);
 *      }
 *      elseif (option_type == else) {
 *         if (top(run_flag) == 1) {
 *             if_run = max(if_run, run_flag);
 *             if (if_run == 0) run_flag = 1
 *         }
 *      }
 *      elseif (option_type == elseif) {
 *         if (top(run_flag) == 1) {
 *             if_run = max(if_run, run_flag);
 *             if (if_run == 0) run_flag = 1
 *         }
 *      }
 *      if (run_flag) {
 *              run option;
 *              // some options (if) can change the run_flag to 0/1
 *      }
 * }
 *
 */

int check_v1_v2(enum fntype type, const char *name) {

    switch(type) {
	case If: 
		/* push IF */
		if (check_state[check_state_top] > 0) {
		     check_state[check_state_top]++;
		}
		else {
		     if (++check_state_top == MAX_IF) fatal_error("too many nested -if/else","");
		     check_state[check_state_top] = 1;
		}
// fprintf(stderr, "if state[%d]=%d  ", check_state_top, check_state[check_state_top]);
		break;

	case Else:
		if (check_state[check_state_top] < 1) fatal_error("unexpected -else","");
		if (++check_state_top == MAX_IF) fatal_error("too many nested -if/else","");
		check_state[check_state_top] = -1;
		is_v2_if = 1;
// fprintf(stderr, "else state[%d]=%d  ", check_state_top, check_state[check_state_top]);
		break;

	case Elseif:
		if (check_state[check_state_top] < 1) fatal_error("unexected -elseif","");
		is_v2_if = 1;
// fprintf(stderr,"elseif state[%d]=%d  ", check_state_top, check_state[check_state_top]);
		break;

	case Endif:
		if (check_state[check_state_top] == -1) {
		    check_state_top--;
		}
		if (check_state[check_state_top] < 1) fatal_error_i("unexpected -endif (%d)",check_state[check_state_top]);
		if (check_state[check_state_top] == 1)  check_state_top--;
		else check_state[check_state_top]--;
// fprintf(stderr,"endif state[%d]=%d  ", check_state_top, check_state[check_state_top]);
		is_v2_if = 1;
		break;
	default:
		if (strcmp(name,"fi") == 0) has_fi = 1;
	        return 0;
    }
    return 0;
}

/*
 * return   0 if version 1 of if structure
 *          1 if version 2 of if structure
 *
 */
int is_v1_v2(void) {

	if (is_v2_if && has_fi) fatal_error("unexepected -fi","");
	if (is_v2_if && check_state_top != 0) fatal_error("too many-if","");
	return is_v2_if;
}

/* if_block states
 *
 * state:  run (was match_flag)
 *         if_run (new)
 &         else_allowed  (new, not needed if checking not done)
 *
 * if:  push_state, if_run=NOT_SET, else_allowed = true
 *
 * endif: pop state
 *
 * else: if (no previous state) fatal_error("illegal else")
 *       if (else_allowed == false) fatal_error("illegal else")
 *       else_allowed = false
 *       if (previous_state->run == RUN) then
 *          if_run=max(if_run, run)       if_run: NOT_SET=-1, NO=0, YES=1
 *          if (if_run = YES) then
 *               run = false
 *          else
 *               run = true
 *          endif
 *       endif
 *
 * elif: if (no_previous_state) fatal_error("illegal elseif")
 *       if (else_allowed == false) fatal_error("illegal else")
 *       if (previous_state->run == RUN) then
 *          if_run=max(if_run, run)       if_run NOT_SET=0, NO=1, YES=2
 *          if (if_run = YES) then
 *               run = false
 *          else
 *               run = true
 *          endif
 *       endif
 *
 * The code to check for syntax should be done in the compile stage
 * advantage: feedback is immediate
 *            save time
 *            the checking code should be a variant of above
 *
 */

void v1_if(void) {
    if (++if_state_top == MAX_IF) fatal_error("too many ifs","");
    // push state
    if_state[if_state_top].run_flag = run_flag;
    if_state[if_state_top].if_run = if_run;
    if_state[if_state_top].else_allowed = else_allowed;
    if_run = 0;
    else_allowed = 1;
}

void v1_endif(void) {
    if (if_state_top < 0) fatal_error("programming error: if_state_top","");
    // pop state
    run_flag = if_state[if_state_top].run_flag;
    if_run = if_state[if_state_top].if_run;
    else_allowed = if_state[if_state_top].else_allowed;
    if_state_top--;
}

void v1_else(void) {
    else_allowed = 0;
    if (if_state[if_state_top].run_flag == 1) {
	if_run = if_run | run_flag;
        run_flag = if_run == 0 ? 1 : 0;
    }
}
void v1_elseif(void) {
    if (if_state[if_state_top].run_flag == 1) {
        if_run = if_run | run_flag;
        run_flag = if_run == 0 ? 1 : 0;
    }
}
