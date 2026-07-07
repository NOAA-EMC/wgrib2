#!/bin/sh
# This script runs a series of tests for MRMS table update
#
# Wesley Ebisuzaki - 3/2025

set -e
echo ""
echo "*** Running wgrib2 update table tests"

echo "*** testing MRMS update 3/2025"
file=data/ref_simple_packing.grib2
# wgrib2=wgrib2
wgrib2=../src/wgrib2
inv=$($wgrib2 $file -set_var var209_255_1_161_3_57 | cut -f4 -d:)
if [ "$inv" != "ReflectivityAtLowestAltitude" ] ; then
   echo "failed: making ReflectivityAtLowestAltitude"
   exit 1
fi
inv=$($wgrib2 $file -set_var var209_255_1_161_3_40 | cut -f4 -d:)
if [ "$inv" != "VILMax1440min" ] ; then
   echo "failed: making VILMax1440min"
   exit 1
fi
echo "*** SUCCESS!"
exit 0
