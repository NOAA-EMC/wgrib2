#!/bin/sh
# This is a test script for the wgrib2 project.
#
# Ed Hartnett, 3/27/24

set -e
echo ""
echo "*** Running wgrib2 tests"

# Just run executable; it returns 8.
echo "*** testing run with no args..."
ls -l ../wgrib2
../wgrib2/wgrib2 > out.txt  && exit 1
[ $? -ne 8 ] && exit 2

# Run an inventory on a test file. This does not work.
#echo "*** testing inventory of gdaswave.t00z.wcoast.0p16.f000.grib2..."
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 > tmp.txt
cat tmp.txt
cmp tmp.txt data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv

# Returns number of grid points in grid
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -npts > npts.txt
cat npts.txt
cmp npts.txt data/ref_npts_gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** SUCCESS!"
exit 0
