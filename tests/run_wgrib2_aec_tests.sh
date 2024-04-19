#!/bin/sh
# This checks the use of AEC compression, only available if USE_AEC is turned on in CMake.
#
# Alyson Stahl, 4/18/24

set -e
echo ""
echo "*** Running wgrib2 tests"

cksum0=`../wgrib2/wgrib2 data/tmp_int.grb -text -  | cksum`
../wgrib2/wgrib2 data/tmp_int.grb -set_grib_type aec -grib_out junk.grb
cksum1=`../wgrib2/wgrib2 junk.grb -text -  | cksum`

if [ "$cksum0" != "$cksum1" ] ; then
    echo "failed for compressing to aec and reading from aec"
    exit 1
fi

echo "*** SUCCESS!"
exit 0
