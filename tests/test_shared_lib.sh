#!/bin/sh
#
# script to see if shared lib was made
#

echo "see if shared library made"
set -xe

if [ ! -f ../wgrib2/libwgrib2.so ] ; then
   echo "failed: did not find libwgrib2.so"
   exit 1
fi

echo "*** SUCCESS!"
exit 0
