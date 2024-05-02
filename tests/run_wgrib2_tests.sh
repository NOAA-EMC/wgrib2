#!/bin/sh
# This is a test script for the wgrib2 project.
#
# Ed Hartnett, Alyson Stahl 3/27/24

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

# Tests the calculation of wind speed, direction, and UGRD & VGRD components
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -wind_dir wind.grb -wind_speed wind.grb -match "(UGRD|VGRD)"
../wgrib2/wgrib2 wind.grb > wind.txt
cat wind.txt
cmp wind.txt data/ref_wind.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv
../wgrib2/wgrib2 wind.grb -wind_uv uv.grb
../wgrib2/wgrib2 uv.grb > uv.txt
cat uv.txt
cmp uv.txt data/ref_uv.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv

# Tests printing of grid information
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -grid > grid_test.txt
cat grid_test.txt
cmp grid_test.txt data/ref_grid.gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** SUCCESS!"
exit 0
