#!/bin/sh
# This tests support for jpeg2000 compressed grib files.
# Only available if USE_JASPER=ON or USE_OPENJPEG=ON CMake.
#
# Alyson Stahl 7/26/2024

set -e
echo ""
echo "*** Running wgrib2 jpeg tests"

echo "*** Converting from jpeg to simple packing"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -set_grib_type simple -grib_out jpeg2simple.grb
../wgrib2/wgrib2 jpeg2simple.grb -v2 -s >  jpeg2simple.txt
touch jpeg2simple.txt
diff -w jpeg2simple.txt data/ref_jpeg2simple.txt

echo "*** SUCCESS!"
exit 0