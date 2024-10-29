#!/bin/sh
# This checks the use of ip external library, only available if USE_IPOLATES=ON CMake.
#
# Alyson Stahl 5/9/2024

set -e

echo "create a new grib file with integer values"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match "(UGRD|VGRD)" -rpn floor -set_scaling 0 0 -grib_out new_grid_test.grb

echo "*** Testing conversion from earth to grid"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds grid  \
    -new_grid latlon 0:360:1 00:91:1 new_grid.grb
../wgrib2/wgrib2 new_grid.grb -grid -v2 -s -lon 10 12 -lon 20 80 > new_grid.txt
touch new_grid.txt
diff -w new_grid.txt data/ref_new_grid_gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Testing conversion from grid to earth"
../wgrib2/wgrib2 new_grid.grb  -new_grid_winds earth  \
    -new_grid latlon 0:360:1 00:91:1 new_grid_earth.grb
../wgrib2/wgrib2 new_grid.grb -grid -v2 -s -lon 10 12 -lon 20 80 > new_grid_earth.txt
touch new_grid_earth.txt
diff -w new_grid_earth.txt data/ref_new_grid_earth_gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Testing new_grid on file in incorrect order. This will return an incomplete output file and error message."
../wgrib2/wgrib2 new_grid_test.grb -match ":UGRD:" -grib_out test_badorder.grb
../wgrib2/wgrib2 new_grid_test.grb -match ":VGRD:" -append -grib_out test_badorder.grb
{ out_err=$(../wgrib2/wgrib2 test_badorder.grb -new_grid_winds grid  \
    -new_grid latlon 0:360:1 00:91:1 junk_badorder.grb 2>&1 1>&$out); } {out}>&1
if [[ -z "$out_err" ]]; then 
    exit 10
fi

echo "*** Testing new_grid_order on file in incorrect order."
../wgrib2/wgrib2 test_badorder.grb -new_grid_order - junk | \
    ../wgrib2/wgrib2 - -new_grid_winds grid -new_grid latlon 0:360:1 00:91:1 new_grid_reorder.grb
../wgrib2/wgrib2 new_grid_reorder.grb -grid -v2 -s -lon 10 12 -lon 20 80 > new_grid_reorder.txt
touch new_grid_reorder.txt
diff -w new_grid_reorder.txt new_grid.txt

echo "*** Testing conversion to wmo rot lat-lon grid"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid rot-ll:10:-40:0 342:665:0.0625 -20:657:0.0625 \
   new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -d 1 -rpn "10:*:floor:10:/" -ijlat 10 20 >new_grid_junk.txt

echo "1:0:(10,20),lon=-8.920754,lat=28.980832,val=-35.4" >new_grid_junk2.txt
diff -w new_grid_junk2.txt new_grid_junk.txt

echo "*** Testing conversion wmo rot lat-lon to lat-lon grid"
../wgrib2/wgrib2 new_grid_junk.grb -new_grid_winds earth -new_grid ncep grid 3 new_grid_junk2.grb

../wgrib2/wgrib2 new_grid_junk2.grb -d 1 -rpn "10:*:floor:10:/" -ijlat 1 150 >new_grid_junk.txt
echo "1:0:(1,150),lon=0.000000,lat=59.000000,val=-33.9" >new_grid_junk2.txt
diff -w new_grid_junk2.txt new_grid_junk.txt
echo $?

echo "*** Testing conversion from NCEP rot lat lon"
../wgrib2/wgrib2 data/ref_new_grid_gdt_32769.grib2 -new_grid_winds earth -new_grid ncep grid 3 new_grid_ncep_rot.grb

echo "*** Testing conversion to NCEP grid definition 2"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 2 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_2.txt
touch ncep_grid_2.txt
#diff -w ncep_grid_2.txt data/ref_new_grid_ncep_2.txt

echo "*** Testing conversion to NCEP grid definition 3"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 3 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_3.txt
touch ncep_grid_3.txt
#diff -w ncep_grid_3.txt data/ref_new_grid_ncep_3.txt

echo "*** Testing conversion to NCEP grid definition 4"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 4 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_4.txt
touch ncep_grid_4.txt
#diff -w ncep_grid_4.txt data/ref_new_grid_ncep_4.txt

echo "*** Testing conversion to NCEP grid definition 45"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 45 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_45.txt
touch ncep_grid_45.txt
#diff -w ncep_grid_45.txt data/ref_new_grid_ncep_45.txt

echo "*** Testing conversion to NCEP grid definition 98"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 98 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_98.txt
touch ncep_grid_98.txt
#diff -w ncep_grid_98.txt data/ref_new_grid_ncep_98.txt

echo "*** Testing conversion to NCEP grid definition 126"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 126 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_126.txt
touch ncep_grid_126.txt
#diff -w ncep_grid_126.txt data/ref_new_grid_ncep_126.txt

echo "*** Testing conversion to NCEP grid definition 127"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 127 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_127.txt
touch ncep_grid_127.txt
#diff -w ncep_grid_127.txt data/ref_new_grid_ncep_127.txt

echo "*** Testing conversion to NCEP grid definition 128"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 128 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_128.txt
touch ncep_grid_128.txt
#diff -w ncep_grid_128.txt data/ref_new_grid_ncep_128.txt

echo "*** Testing conversion to NCEP grid definition 129"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 129 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_129.txt
touch ncep_grid_129.txt
#diff -w ncep_grid_129.txt data/ref_new_grid_ncep_129.txt

echo "*** Testing conversion to NCEP grid definition 170"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 170 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_170.txt
touch ncep_grid_170.txt
#diff -w ncep_grid_170.txt data/ref_new_grid_ncep_170.txt

echo "*** Testing conversion to NCEP grid definition 173"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 173 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_173.txt
touch ncep_grid_173.txt
#diff -w ncep_grid_173.txt data/ref_new_grid_ncep_173.txt

echo "*** Testing conversion to NCEP grid definition 184"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 184 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_184.txt
touch ncep_grid_184.txt
#diff -w ncep_grid_184.txt data/ref_new_grid_ncep_184.txt

echo "*** Testing conversion to NCEP grid definition 194"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 194 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_194.txt
touch ncep_grid_194.txt
#diff -w ncep_grid_194.txt data/ref_new_grid_ncep_194.txt

echo "*** Testing conversion to NCEP grid definition 221"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 221 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_221.txt
touch ncep_grid_221.txt
#diff -w ncep_grid_221.txt data/ref_new_grid_ncep_221.txt

echo "*** Testing conversion to NCEP grid definition 230"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 230 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_230.txt
touch ncep_grid_230.txt
#diff -w ncep_grid_230.txt data/ref_new_grid_ncep_230.txt

echo "*** Testing conversion to NCEP grid definition 242"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 242 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_242.txt
touch ncep_grid_242.txt
#diff -w ncep_grid_242.txt data/ref_new_grid_ncep_242.txt

echo "*** Testing conversion to NCEP grid definition 249"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 249 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_249.txt
touch ncep_grid_249.txt
#diff -w ncep_grid_249.txt data/ref_new_grid_ncep_249.txt

echo "*** Testing conversion to NCEP grid definition t62"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t62 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t62.txt
touch ncep_grid_t62.txt
#diff -w ncep_grid_t62.txt data/ref_new_grid_ncep_t62.txt

echo "*** Testing conversion to NCEP grid definition t126"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t126 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t126.txt
touch ncep_grid_t126.txt
#diff -w ncep_grid_t126.txt data/ref_new_grid_ncep_t126.txt

echo "*** Testing conversion to NCEP grid definition t170"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t170 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t170.txt
touch ncep_grid_t170.txt
#diff -w ncep_grid_t170.txt data/ref_new_grid_ncep_t170.txt

echo "*** Testing conversion to NCEP grid definition t190"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t190 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t190.txt
touch ncep_grid_t190.txt
#diff -w ncep_grid_t190.txt data/ref_new_grid_ncep_t190.txt

echo "*** Testing conversion to NCEP grid definition t254"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t254 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t254.txt
touch ncep_grid_t254.txt
#diff -w ncep_grid_t254.txt data/ref_new_grid_ncep_t254.txt

echo "*** Testing conversion to NCEP grid definition t382"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t382 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t382.txt
touch ncep_grid_t382.txt
#diff -w ncep_grid_t382.txt data/ref_new_grid_ncep_t382.txt

echo "*** Testing conversion to NCEP grid definition t574"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t574 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t574.txt
touch ncep_grid_t574.txt
#diff -w ncep_grid_t574.txt data/ref_new_grid_ncep_t574.txt

echo "*** Testing conversion to NCEP grid definition t1148"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t1148 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t1148.txt
touch ncep_grid_t1148.txt
#diff -w ncep_grid_t1148.txt data/ref_new_grid_ncep_t1148.txt

echo "*** Testing conversion to NCEP grid definition t1534"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid t1534 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_t1534.txt
touch ncep_grid_t1534.txt
#diff -w ncep_grid_t1534.txt data/ref_new_grid_ncep_t1534.txt

echo "*** Testing conversion to Mercator grid"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid mercator:0 0:361:1:360 0:91:1:90 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > mercator_grid.txt
touch mercator_grid.txt
diff -w mercator_grid.txt data/ref_mercator_grid.txt

echo "*** Testing conversion to HRRR grid"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid hrrr new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > hrrr_grid.txt
touch hrrr_grid.txt
diff -w hrrr_grid.txt data/ref_hrrr_grid.txt

echo "*** Testing conversion to NAM 12 grid"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid nam12 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > nam12_grid.txt
touch nam12_grid.txt
diff -w nam12_grid.txt data/ref_nam12_grid.txt

echo "*** Testing conversion to NCEP grid definition 130"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 130 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_130.txt
touch ncep_grid_130.txt
diff -w ncep_grid_130.txt data/ref_new_grid_ncep_130.txt

echo "*** Testing conversion to NCEP grid definition 132"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 132 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_132.txt
touch ncep_grid_132.txt
diff -w ncep_grid_132.txt data/ref_new_grid_ncep_132.txt

echo "*** Testing conversion to NCEP grid definition 187"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 187 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_187.txt
touch ncep_grid_187.txt
diff -w ncep_grid_187.txt data/ref_new_grid_ncep_187.txt

echo "*** Testing conversion to NCEP grid definition 197"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 197 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_197.txt
touch ncep_grid_197.txt
diff -w ncep_grid_197.txt data/ref_new_grid_ncep_197.txt

echo "*** Testing conversion to NCEP grid definition 236"
../wgrib2/wgrib2 new_grid_test.grb -new_grid_winds earth -new_grid ncep grid 236 new_grid_junk.grb
../wgrib2/wgrib2 new_grid_junk.grb -grid -v2 -s > ncep_grid_236.txt
touch ncep_grid_236.txt
diff -w ncep_grid_236.txt data/ref_new_grid_ncep_236.txt

echo "*** SUCCESS!"
exit 0
