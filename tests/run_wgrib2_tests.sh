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
cksum_wind_0=`../wgrib2/wgrib2 data/ref_wind.grb -text - | cksum`
cksum_uv_0=`../wgrib2/wgrib2 data/ref_uv.grb -text - | cksum`
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -wind_dir wind.grb -wind_speed wind.grb -match "(UGRD|VGRD)"
cksum_wind_1=`../wgrib2/wgrib2 wind.grb -text - | cksum`
if [ "$cksum_wind_0" != "$cksum_wind_1" ] ; then
    echo "failed for computing wind speed and direction"
    exit 1
fi
../wgrib2/wgrib2 wind.grb -wind_uv uv.grb
cksum_uv_1=`../wgrib2/wgrib2 uv.grb -text - | cksum`
if [ "$cksum_uv_0" != "$cksum_uv_1" ] ; then
    echo "failed for computing UGRD & VGRD components"
    exit 1
fi

echo "*** SUCCESS!"
exit 0
