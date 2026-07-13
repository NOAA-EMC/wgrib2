/** @file
 * @brief Subtract two times.
 * @author Public Domain: Wesley Ebisuzaki @date 8/2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "wgrib2.h"
#include "CodeTable4_4.h"

/** Days in each month */
static int monthjday[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};

/**
 * Check if a year is a leap year.
 * 
 * @param year The year to check.
 * 
 * @return 1 if the year is a leap year, 0 otherwise.
 * 
 * @author Wesley Ebisuzaki @date 8/2011
 */
static int leap(int year) {
    if (year % 4 != 0) return 0;
    if (year % 100 != 0) return 1;
    return (year % 400 == 0);
}

// return julian day (1..366)

/**
 * Get the Julian day (1..366) for a given date.
 * 
 * @param year The year of the date.
 * @param month The month of the date (1..12).
 * @param day The day of the date (1..31).
 * 
 * @return The Julian day (1..366) for the given date.
 * 
 * @author Wesley Ebisuzaki @date 8/2011
 */
int jday(int year, int month, int day) {
    int i;
    i = monthjday[month-1] + day;
    if (leap(year) && month > 2) i++;
    return i;
}

/**
 * Get the number of days in a month for a given year.
 * 
 * @param year The year of the month.
 * @param month The month (1..12).
 * 
 * @return The number of days in the specified month of the given year.
 * 
 * @author Wesley Ebisuzaki @date 8/2011
 */
int num_days_in_month(int year, int month) {
   if (month != 2) return monthjday[month]-monthjday[month-1];
   return 28 + leap(year);
}
 
/**
 * Subtract two dates (year, month, day) and return the number of days between them.
 * 
 * Requires time1 >= time0.
 * 
 * @param year1 The year of the first date.
 * @param month1 The month of the first date (1..12).
 * @param day1 The day of the first date (1..31).
 * @param year0 The year of the second date.
 * @param month0 The month of the second date (1..12).
 * @param day0 The day of the second date (1..31).
 * 
 * @return The number of days between the two dates.
 * 
 * @author Wesley Ebisuzaki @date 8/2011
 */
static int sub_day(int year1, int month1, int day1, int year0, int month0, int day0) {
    int i, jday1 , jday0, year;
    jday1 = jday(year1,month1,day1);
    jday0 = jday(year0,month0,day0);

    i = -jday0;
    year = year0;
    while (year < year1) {
        i += 365 + leap(year);
        year++;
    }
    i = i + jday1;
    return i;
}

/**
 * Subtract two times and return the difference in the largest possible unit.
 * 
 * @param year1 The year of the first time.
 * @param month1 The month of the first time (1..12).
 * @param day1 The day of the first time (1..31).
 * @param hour1 The hour of the first time (0..23).
 * @param minute1 The minute of the first time (0..59).
 * @param second1 The second of the first time (0..59).
 * @param year0 The year of the second time.
 * @param month0 The month of the second time (1..12).
 * @param day0 The day of the second time (1..31).
 * @param hour0 The hour of the second time (0..23).
 * @param minute0 The minute of the second time (0..59).
 * @param second0 The second of the second time (0..59).
 * @param dtime Pointer to store the difference in time.
 * @param unit Pointer to store the unit of time.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 8/2011
 */
int sub_time(int year1, int month1, int day1, int hour1, int minute1, int second1, 
        int year0, int month0, int day0, int hour0, int minute0, int second0, 
        int *dtime, int *unit) {

    int sign, i, dt;

    // different years
    if (month0 == month1 && day0 == day1 && hour0 == hour1 && minute0 == minute1 && second0 == second1) {
        *dtime = year1 - year0;
        *unit = YEAR;
        return 0;
    }


    // only different year+month
    if (day0 == day1 && hour0 == hour1 && minute0 == minute1 && second0 == second1) {
        *dtime = (year1-year0)*12 + month1 - month0;
        *unit = MONTH;
        return 0;
    }


    sign = cmp_time(year1, month1, day1, hour1, minute1, second1, year0, month0, day0, hour0, minute0, second0);

    // time1 < time0, swap(time1, time0)
    if (sign < 0) {
        i = year0; year0 = year1; year1 = i;
        i = month0; month0 = month1; month1 = i;
        i = day0; day0 = day1; day1 = i;
        i = hour0; hour0 = hour1; hour1 = i;
        i = minute0; minute0 = minute1; minute1 = i;
        i = second0; second0 = second1; second1 = i;
    }

    // only different year+month+day
    if (hour0 == hour1 && minute0 == minute1 && second0 == second1) {
        *dtime = sign * sub_day(year1, month1, day1, year0, month0, day0);
        *unit = DAY;
        return 0;
    }

    dt = sub_day(year1, month1, day1, year0, month0, day0) - 1;
    i = 24 + hour1 - hour0;

    // only different year+month+day+hour
    if (minute0 == minute1 && second0 == second1) {
        if (i % 12 == 0) {
            dt = dt * 2 + i/12;
            *dtime = sign * dt;
            *unit = HOUR12;
            return 0;
        }
        if (i % 6 == 0) {
            dt = dt * 4 + i/6;
            *dtime = sign * dt;
            *unit = HOUR6;
            return 0;
        }
        if (i % 3 == 0) {
            dt = dt * 8 + i/3;
            *dtime = sign * dt;
            *unit = HOUR3;
            return 0;
        }
        dt = dt * 24 + i;
        *dtime = sign * dt;
        *unit = HOUR;
        return 0;
    }

    // only different year+month+day+hour+minute
    if (second0 == second1) {
        dt = (dt * 24 + i) * 60 + (minute1-minute0);
        *dtime = sign * dt;
        *unit = MINUTE;
        return 0;
    }

    // seconds
    dt = (dt * 24 + i) * 60 * 60 + (minute1-minute0)*60 + (second1-second0);
    *dtime = sign * dt;
    *unit = SECOND;
    return 0;
}

