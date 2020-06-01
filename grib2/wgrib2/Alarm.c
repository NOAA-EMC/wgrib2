/* public domain 4/2017 Wesley Ebisuzaki   Alarm.c, stop wgrib2 after N seconds */

#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist.h"
/*
 * HEADER:100:alarm:setup:1:terminate after X seconds
 */

#ifndef DISABLE_ALARM

#include <unistd.h>
int f_alarm(ARG1) {
    int i;
    if (mode == -1) {
        // set the alarm for n seconds
        i = atoi(arg1);
        alarm(i <= 0 ? 0: (unsigned int) i);
    }
    else if (mode == -2) {
        // turn off the alarm
        alarm(0);
    }
    return 0;
}

#else

int f_alarm(ARG1) {
    fatal_error("alarm: not supported");
    return 0;
}

#endif
