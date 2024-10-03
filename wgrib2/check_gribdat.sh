#!/bin/sh
# 10/2024   Public Domain  Wesley Ebisuzaki
#
# This script checks parameter info for wgrib2. This is a test script
# that is manually run when updating the grib2 table from the NCO web
# pages.

if [ $# -ne 2 ] ; then
  echo "$0 gribtab.dat.1 gribtab.dat.2"
  exit 8
fi
s=/tmp/junk.$$.1
t=/tmp/junk.$$.2

cat $1 |  sed 's/",.*/"/' >$s
cat $2 |  sed 's/",.*/"/' >$t

diff $s $t 

rm $s $t
