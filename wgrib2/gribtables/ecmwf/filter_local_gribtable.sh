#!/bin/sh
# (c) 2020 Manfred Schwarb <schwarb@meteodat.ch>
# Released under the General Public License Version 3 (GPLv3).

# local_gribtable includes definitions for other than ECMWF
# remove non-ECMWF definitions
# local table was undefined, changed to 1
# mtab_set set to 1

#  int disc;   Section 0 Discipline
#  int mtab_set;   Section 1 Master Tables Version Number
#  int mtab_low;   Section 1 Master Tables Version Number
#  int mtab_high;   Section 1 Master Tables Version Number
#  int cntr;   Section 1 originating centre, used for local tables
#  int ltab;   Section 1 Local Tables Version Number
#  int pcat;   Section 4 Template 4.0 Parameter category
#  int pnum;   Section 4 Template 4.0 Parameter number
#  const char *name;
#  const char *desc;
#  const char *unit;


# 10:0:0:255:7:j
# grep -G  '^[0-9]+:[0-9]+:[0-9]+:[0-9]+:7:' <local_gribtable
# grep  -E -e '^[0-9]+:[0-9]+:[0-9]+:[0-9]+:80:' <local_gribtable >local_gribtable.ecmwf

# problem local table is undefined, set to 1

# test version 
awk '{ if ($5 == 98 && ($1 > 191 || $7 > 191 || $8 > 191) ) { print $0 ; print $1":1:"$3":"$4":98:1:"$7":"$8":"$9":"$10":"$11 } }' FS=: <local_gribtable

# good version
awk '{ if ($5 == 98 && ($1 > 191 || $7 > 191 || $8 > 191) ) { print $1":1:"$3":"$4":98:1:"$7":"$8":"$9":"$10":"$11 } }' FS=: <local_gribtable >local_gribtable_ecmwf

