#!/bin/sh
# This checks the use of AEC compression, only available if USE_AEC is turned on in CMake.
#
# Alyson Stahl, 4/18/24

set -e
echo ""
echo "*** Running wgrib2 tests"

cksum0=`../wgrib2/wgrib2 data/ref_simple_packing.grib2 -text -  | cksum`
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -set_grib_type aec -grib_out junk_aec.grb
cksum1=`../wgrib2/wgrib2 junk_aec.grb -text -  | cksum`

if [ "$cksum0" != "$cksum1" ] ; then
    echo "failed for compressing to aec and reading from aec"
    exit 1
fi

echo "*** SUCCESS!"
exit 0
