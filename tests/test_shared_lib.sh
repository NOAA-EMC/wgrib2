#!/bin/sh
#
# script to see if shared lib was made
#

echo "see if shared library made"
set -xe

if [ ! -f ../src/libwgrib2.so ] && [ ! -f ../src/libwgrib2.dylib ]; then
   echo "failed: did not find shared library"
   exit 1
fi

echo "*** SUCCESS!"
exit 0
