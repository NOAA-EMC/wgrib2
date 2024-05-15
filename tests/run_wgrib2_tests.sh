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

echo "*** testing inventory of gdaswave.t00z.wcoast.0p16.f000.grib2..."
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 > tmp.txt
cat tmp.txt
cmp tmp.txt data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv

echo "*** Testing calculation of number of grid points"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -npts > npts.txt
cat npts.txt
cmp npts.txt data/ref_npts_gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** Testing calculation of wind speed, direction, and UGRD & VGRD components"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -wind_dir wind.grb -wind_speed wind.grb -match "(UGRD|VGRD)"
../wgrib2/wgrib2 wind.grb > wind.txt
cat wind.txt
cmp wind.txt data/ref_wind.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv
../wgrib2/wgrib2 wind.grb -wind_uv uv.grb
../wgrib2/wgrib2 uv.grb > uv.txt
cat uv.txt
cmp uv.txt data/ref_uv.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv

echo "*** Testing grid information"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -grid > grid_test.txt
cat grid_test.txt
cmp grid_test.txt data/ref_grid.gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** Testing Sec0"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -Sec0 -grib_out sec0.grb
../wgrib2/wgrib2 sec0.grb > sec0.txt
cat sec0.txt
cmp sec0.txt data/ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Testing checksum on section 0"
../wgrib2/wgrib2 sec0.grb -checksum 0 > chksum1.txt
../wgrib2/wgrib2 data/ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2 -checksum 0 > chksum2.txt
touch chksum1.txt
touch chksum2.txt
cmp chksum1.txt chksum2.txt

echo "*** Testing sec_len"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -Sec_len > sec_len.txt
cat sec_len.txt
cmp sec_len.txt data/ref_sec_len.gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** Testing sec_len on a small file"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -Sec_len > sec_len_small.txt
cat sec_len_small.txt
cmp sec_len_small.txt data/ref_sec_len.simple_packing.grib2.txt

echo "*** Testing separating grib messages into separate files then recombining into single file"
# creating comparison file using match 
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ':(UGRD|VGRD|HGT|TMP):0.4 mb' -grib_out htuv.grb
# separating messages into new grib files
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -inv junk.inv
../wgrib2/wgrib2 -fgrep ":HGT:0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out hgt0p4.grb
../wgrib2/wgrib2 -fgrep ":TMP:0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out tmp0p4.grb
../wgrib2/wgrib2 -egrep ":(UGRD|VGRD):" -fgrep ":0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out winds0p4.grb
# recombining files
../wgrib2/wgrib2 hgt0p4.grb -grib result.grb
../wgrib2/wgrib2 tmp0p4.grb -append -grib result.grb
../wgrib2/wgrib2 winds0p4.grb -append -GRIB result.grb

cksum0=`../wgrib2/wgrib2 htuv.grb -text -  | cksum`
cksum1=`../wgrib2/wgrib2 result.grb -text -  | cksum`

if [ "$cksum0" != "$cksum1" ] ; then
    echo "*** Failed combining files"
    exit 1
fi

echo "*** SUCCESS!"
exit 0
