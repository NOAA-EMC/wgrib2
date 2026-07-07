#!/bin/sh
# This tests support for jpeg2000 compressed grib files.
# Only available if USE_JASPER=ON or USE_OPENJPEG=ON CMake.
#
# Alyson Stahl 7/26/2024

n=$(../src/wgrib2 -config | grep -c "Supported encoding:.*jpeg2000")
if [ "$n" -eq 0 ] ; then
  echo "*** Not running jpeg tests"
  exit 0
fi
set -e
echo ""
echo "*** Running wgrib2 jpeg tests"

echo "*** Running stats on large grib2 file with JPEG packing"
../src/wgrib2 data/LARGECAT220250305_12_1443copy.grib2 -stats > junk_jpeg_stats.txt
diff -w junk_jpeg_stats.txt data/ref_LARGECAT220250305_12_1443copy_stats.txt
echo "*** SUCCESS!"

echo "*** Converting from jpeg to simple packing"
../src/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -set_grib_type simple -grib_out junk_jpeg2simple.grb
../src/wgrib2 junk_jpeg2simple.grb -v2 -s >  junk_jpeg2simple.txt
touch junk_jpeg2simple.txt
diff -w junk_jpeg2simple.txt data/ref_jpeg2simple.txt

exit 0
