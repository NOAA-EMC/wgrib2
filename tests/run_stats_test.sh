#!/bin/sh

set -e
echo ""

echo "*** JPEG File"
../wgrib2/wgrib2 temp_jpeg_test_file.grib2 -stats

echo "**********************************"
echo "*** PNG File"
../wgrib2/wgrib2 data/large_png.grb2 -stats


echo "**********************************"
echo "*** Simple File"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -stats

echo "*** SUCCESS!"
exit 0
