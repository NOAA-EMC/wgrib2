#!/bin/sh
#
# script to see if shared lib was made
#

echo "see if shared library made"
set -xe

if [ ! -f ../wgrib2/libwgrib2.so -a ! -f ../wgrib2/libwgrib2.a ] ; then
   echo "failed: did not find libwgrib2"
   exit 1
fi

echo "*** SUCCESS!"
exit 0
