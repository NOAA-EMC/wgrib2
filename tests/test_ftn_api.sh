#!/bin/sh
#
# script to see if ftn_api compiled
#

echo "see if ftn_api compiled"
set -xe

if [ ! -f ../wgrib2/ftn_api/include/wgrib2api.mod ] ; then
   echo "failed: did not find wgrib2api.mod"
   exit 1
fi

echo "*** SUCCESS!"
exit 0
