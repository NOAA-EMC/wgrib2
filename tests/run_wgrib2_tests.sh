#!/bin/sh
# This is a test script for the wgrib2 project.
#
# Ed Hartnett, Alyson Stahl 3/27/24

set -e
echo ""
echo "*** Running wgrib2 tests"

# Just run executable; it returns 8.
echo "*** testing run with no args..."
ls -l ../src
../src/wgrib2 > out.txt  && exit 1
[ $? -ne 8 ] && exit 2

echo "*** testing inventory of gdaswave.t00z.wcoast.0p16.f000.grib2..."
../src/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -inv tmp.txt -Match_inv
cat tmp.txt
diff -w tmp.txt data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv

echo "*** Testing calculation of number of grid points"
../src/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -npts > npts.txt
cat npts.txt

echo "*** Testing calculation of wind speed, direction, and UGRD & VGRD components"
../src/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -wind_dir wind.grb -wind_speed wind.grb -match "(UGRD|VGRD)"
../src/wgrib2 wind.grb > wind.txt
cat wind.txt
#diff -w wind.txt data/ref_wind.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv
../src/wgrib2 wind.grb -wind_uv uv.grb
../src/wgrib2 uv.grb > uv.txt
cat uv.txt
#diff -w uv.txt data/ref_uv.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv

echo "*** Testing grid information"
../src/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -grid > grid_test.txt
cat grid_test.txt
diff -w grid_test.txt data/ref_grid.gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** Testing Sec0"
../src/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -Sec0 -grib_out sec0.grb
../src/wgrib2 sec0.grb > sec0.txt
cat sec0.txt
diff -w sec0.txt data/ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Testing checksum on section 0"
../src/wgrib2 sec0.grb -checksum 0 > chksum1.txt
../src/wgrib2 data/ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2 -checksum 0 > chksum2.txt
touch chksum1.txt
touch chksum2.txt
diff -w chksum1.txt chksum2.txt

echo "*** Testing sec_len"
../src/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -Sec_len > sec_len.txt
cat sec_len.txt
diff -w sec_len.txt data/ref_sec_len.gdaswave.t00z.wcoast.0p16.f000.grib2.txt

echo "*** Testing sec_len on a small file"
../src/wgrib2 data/ref_simple_packing.grib2 -Sec_len > sec_len_small.txt
cat sec_len_small.txt
diff -w sec_len_small.txt data/ref_sec_len.simple_packing.grib2.txt
diff chksum1.txt chksum2.txt

echo "*** Testing separating grib messages into separate files then recombining into single file"
# creating comparison file using match 
../src/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ':(UGRD|VGRD|HGT|TMP):0.4 mb' -grib_out htuv.grb
../src/wgrib2 htuv.grb -v2 > htuv.txt
touch htuv.txt
# separating messages into new grib files
../src/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -inv junk.inv
../src/wgrib2 -fgrep ":HGT:0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out hgt0p4.grb
../src/wgrib2 -fgrep ":TMP:0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out tmp0p4.grb
../src/wgrib2 -egrep ":(UGRD|VGRD):" -fgrep ":0.4 mb" -i_file junk.inv data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -grib_out winds0p4.grb
# recombining files
../src/wgrib2 hgt0p4.grb -grib result.grb
../src/wgrib2 tmp0p4.grb -append -grib result.grb
../src/wgrib2 winds0p4.grb -append -GRIB result.grb
../src/wgrib2 result.grb -v2 > result.txt
touch result.txt
diff -w result.txt htuv.txt

echo "*** Testing import_text"
../src/wgrib2 data/ref_simple_packing.grib2 -v2 > simple.txt
touch simple.txt
../src/wgrib2 data/ref_simple_packing.grib2 -rpn 0 -grib_out template.grb
../src/wgrib2 data/ref_simple_packing.grib2 -text grib_text.txt
../src/wgrib2 template.grb -import_text grib_text.txt -grib_out text2grib.grb
../src/wgrib2 text2grib.grb -v2 > text2grib.txt
touch text2grib.txt
diff -w text2grib.txt simple.txt 

echo "*** Testing import_bin"
../src/wgrib2 data/ref_simple_packing.grib2 -bin grib_bin.bin
../src/wgrib2 template.grb -import_bin grib_bin.bin -grib_out bin2grib.grb
../src/wgrib2 bin2grib.grb -v2 > bin2grib.txt
touch bin2grib.txt
diff -w bin2grib.txt simple.txt 

echo "*** Testing spread output"
../src/wgrib2 data/ref_simple_packing.grib2 -v2 -spread spread.txt
touch spread.txt
#diff -w data/ref_simple_packing.grib2.spread.txt spread.txt

echo "*** Testing write/read section"
../src/wgrib2 data/ref_simple_packing.grib2 -write_sec 0 sec0.dat -write_sec 1 sec1.dat \
    -write_sec 2 sec2.dat -write_sec 3 sec3.dat -write_sec 4 sec4.dat -write_sec 5 sec5.dat \
    -write_sec 6 sec6.dat -write_sec 7 sec7.dat -write_sec 8 sec8.dat
../src/wgrib2 template.grb -read_sec 0 sec0.dat -read_sec 1 sec1.dat -read_sec 2 sec2.dat \
    -read_sec 3 sec3.dat -read_sec 4 sec4.dat -read_sec 5 sec5.dat -read_sec 6 sec6.dat \
    -read_sec 7 sec7.dat -read_sec 8 sec8.dat -grib secs.grb
../src/wgrib2 secs.grb -v2 > secs.txt
touch secs.txt
diff -w secs.txt simple.txt 


echo "*** test pdt 48 ***"
n=$(../src/wgrib2 ./data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -d 1 -set_pdt +48  -set_byte 4 12 00:12:0 | grep -c "aerosol_size")
if [ "$n" -ne 1 ] ; then
 exit 1
fi
set -x
echo "*** test pdt 49 ***"
n=$(../src/wgrib2 ./data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -d 1 -set_pdt +49  -set_byte 4 12 00:12:0 | grep -c "aerosol_size")
if [ "$n" -ne 1 ] ; then
 exit 1
fi

echo "*** test import_ieee big-endian ***"

../src/wgrib2 data/ref_simple_packing.grib2 -ieee ieee.bin -inv ieee.inv -d 1
../src/wgrib2 data/ref_simple_packing.grib2 -d 1 -rpn 0 -import_ieee ieee.bin -grib_out ieee.grb
line=$(../src/wgrib2 -d 1 data/ref_simple_packing.grib2 -rpn sto_1 -import_ieee ieee.bin -rpn "rcl_1:print_rms")

if [ "$(echo "$line" | grep -c ":rpn_rms=0:")" -ne 1 ] ; then
  exit 1
fi

echo "*** test import_ieee little-endian ***"

../src/wgrib2 data/ref_simple_packing.grib2 -little_endian -ieee ieee.bin.le -inv ieee.inv.le -d 1
../src/wgrib2 data/ref_simple_packing.grib2 -little_endian -d 1 -rpn 0 -import_ieee ieee.bin.le -grib_out ieee.grb.le
line=$(../src/wgrib2 -little_endian -d 1 data/ref_simple_packing.grib2 -rpn sto_1 -import_ieee ieee.bin.le -rpn "rcl_1:print_rms")

if [ "$(echo "$line" | grep -c ":rpn_rms=0:")" -ne 1 ] ; then
  exit 1
fi

../src/wgrib2 data/ref_c3_overflow.grib2 -set_grib_type c3 -set_grib_max_bits 25 -grib_out c3_overflow.grib2

echo "*** test import_ieee little-endian ***"

echo "*** test set_prob ***"
line=$(../src/wgrib2 data/ref_simple_packing.grib2 -d 1 -set_prob 1 2 3  222.2 0)
echo "$line"
if [ "$line" != '1:0:d=2009060500:TMP:500 mb:180 hour fcst:prob >222.2:prob fcst 1/2' ] ; then
  exit 1
fi


echo '*** test new code table 4.6 entries ***'
line=$(../src/wgrib2 data/ref_simple_packing.grib2 -set table_4.6 6)
if [ "$line" != "1:0:d=2009060500:TMP:500 mb:180 hour fcst:P-ENS=19" ] ; then
  exit 1
fi

echo "*** test set_ts_dates ***"
dates=$(../src/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -set_ts_dates 200001 1dy 1 | cut -f3 -d:)

if [ "$(echo "$dates" | head -n 1)" != "d=2000010100" ] ; then
  exit 1
fi
if [ "$(echo "$dates" | tail -n 1)" != "d=2000011900" ] ; then
  exit 1
fi

echo "*** SUCCESS!"
exit 0
