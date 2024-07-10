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
diff -w tmp.txt data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv

echo "*** Testing calculation of number of grid points"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -npts > npts.txt
cat npts.txt

echo "*** Testing calculation of wind speed, direction, and UGRD & VGRD components"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -wind_dir wind.grb -wind_speed wind.grb -match "(UGRD|VGRD)"
../wgrib2/wgrib2 wind.grb > wind.txt
cat wind.txt
#diff -w wind.txt data/ref_wind.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv
../wgrib2/wgrib2 wind.grb -wind_uv uv.grb
../wgrib2/wgrib2 uv.grb > uv.txt
cat uv.txt
#diff -w uv.txt data/ref_uv.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv

echo "*** Testing grid information"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -grid > grid_test.txt
cat grid_test.txt
diff -w grid_test.txt data/ref_grid.gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** Testing Sec0"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -Sec0 -grib_out sec0.grb
../wgrib2/wgrib2 sec0.grb > sec0.txt
cat sec0.txt
diff -w sec0.txt data/ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Testing checksum on section 0"
../wgrib2/wgrib2 sec0.grb -checksum 0 > chksum1.txt
../wgrib2/wgrib2 data/ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2 -checksum 0 > chksum2.txt
touch chksum1.txt
touch chksum2.txt
diff -w chksum1.txt chksum2.txt

echo "*** Testing sec_len"
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -Sec_len > sec_len.txt
cat sec_len.txt
diff -w sec_len.txt data/ref_sec_len.gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** Testing sec_len on a small file"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -Sec_len > sec_len_small.txt
cat sec_len_small.txt
diff -w sec_len_small.txt data/ref_sec_len.simple_packing.grib2.txt
diff chksum1.txt chksum2.txt

echo "*** Testing separating grib messages into separate files then recombining into single file"
# creating comparison file using match 
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ':(UGRD|VGRD|HGT|TMP):0.4 mb' -grib_out htuv.grb
../wgrib2/wgrib2 htuv.grb -v2 > htuv.txt
touch htuv.txt
# separating messages into new grib files
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -inv junk.inv
../wgrib2/wgrib2 -fgrep ":HGT:0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out hgt0p4.grb
../wgrib2/wgrib2 -fgrep ":TMP:0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out tmp0p4.grb
../wgrib2/wgrib2 -egrep ":(UGRD|VGRD):" -fgrep ":0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out winds0p4.grb
# recombining files
../wgrib2/wgrib2 hgt0p4.grb -grib result.grb
../wgrib2/wgrib2 tmp0p4.grb -append -grib result.grb
../wgrib2/wgrib2 winds0p4.grb -append -GRIB result.grb
../wgrib2/wgrib2 result.grb -v2 > result.txt
touch result.txt
diff -w result.txt htuv.txt

echo "*** Testing import_text"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -v2 > simple.txt
touch simple.txt
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -rpn 0 -grib_out template.grb
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -text grib_text.txt
../wgrib2/wgrib2 template.grb -import_text grib_text.txt -grib_out text2grib.grb
../wgrib2/wgrib2 text2grib.grb -v2 > text2grib.txt
touch text2grib.txt
diff -w text2grib.txt simple.txt 

echo "*** Testing import_bin"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -bin grib_bin.bin
../wgrib2/wgrib2 template.grb -import_bin grib_bin.bin -grib_out bin2grib.grb
../wgrib2/wgrib2 bin2grib.grb -v2 > bin2grib.txt
touch bin2grib.txt
diff -w bin2grib.txt simple.txt 

echo "*** Testing spread output"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -v2 -spread spread.txt
touch spread.txt
#diff -w data/ref_simple_packing.grib2.spread.txt spread.txt

echo "*** Testing write/read section"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -write_sec 0 sec0.dat -write_sec 1 sec1.dat \
    -write_sec 2 sec2.dat -write_sec 3 sec3.dat -write_sec 4 sec4.dat -write_sec 5 sec5.dat \
    -write_sec 6 sec6.dat -write_sec 7 sec7.dat -write_sec 8 sec8.dat
../wgrib2/wgrib2 template.grb -read_sec 0 sec0.dat -read_sec 1 sec1.dat -read_sec 2 sec2.dat \
    -read_sec 3 sec3.dat -read_sec 4 sec4.dat -read_sec 5 sec5.dat -read_sec 6 sec6.dat \
    -read_sec 7 sec7.dat -read_sec 8 sec8.dat -grib secs.grb
../wgrib2/wgrib2 secs.grb -v2 > secs.txt
touch secs.txt
diff -w secs.txt simple.txt 

echo "*** SUCCESS!"
exit 0
