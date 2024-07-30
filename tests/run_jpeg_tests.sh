#!/bin/sh
# This tests support for jpeg2000 compressed grib files.
# Only available if USE_JASPER=ON or USE_OPENJPEG=ON CMake.
#
# Alyson Stahl 7/26/2024

set -e
echo "*** Running jpeg tests"

echo "create a new grib file with integer values"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -rpn floor -set_scaling 0 0 -set_grib_type same -grib_out jpeg.grb
../wgrib2/wgrib2 jpeg.grb -v2 -s > jpeg.txt

echo "*** Converting from jpeg to simple packing and back"
../wgrib2/wgrib2 jpeg.grb -set_grib_type simple -grib_out jpeg2simple.grb
../wgrib2/wgrib2 jpeg2simple.grb -set_grib_type jpeg -grib_out simple2jpeg.grb
../wgrib2/wgrib2 simple2jpeg.grb -v2 -s > simple2jpeg.txt
touch simple2jpeg.txt 
diff -w simple2jpeg.txt jpeg.txt

echo "*** SUCCESS!"
exit 0