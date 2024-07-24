#!/bin/sh
# This checks the use of ip external library, only available if USE_IPOLATES=ON CMake.
#
# Alyson Stahl 5/9/2024

set -e

echo "create a new grib file with integer values"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match "(UGRD|VGRD)" -rpn floor -set_scaling 0 0 -grib_out test.grb

echo "*** Testing conversion from earth to grid"
../wgrib2/wgrib2 test.grb -new_grid_winds grid  \
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
../wgrib2/wgrib2 test.grb -match ":UGRD:" -grib_out test_badorder.grb
../wgrib2/wgrib2 test.grb -match ":VGRD:" -append -grib_out test_badorder.grb
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

echo "*** Testing conversion to NCEP grid definition 2"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 2 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_2.txt
touch ncep_grid_2.txt
diff -w ncep_grid_2.txt data/ref_new_grid_ncep_2.txt

echo "*** Testing conversion to NCEP grid definition 3"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 3 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_3.txt
touch ncep_grid_3.txt
diff -w ncep_grid_3.txt data/ref_new_grid_ncep_3.txt

echo "*** Testing conversion to NCEP grid definition 4"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 4 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_4.txt
touch ncep_grid_4.txt
diff -w ncep_grid_4.txt data/ref_new_grid_ncep_4.txt

echo "*** Testing conversion to NCEP grid definition 45"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 45 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_45.txt
touch ncep_grid_45.txt
diff -w ncep_grid_45.txt data/ref_new_grid_ncep_45.txt

echo "*** Testing conversion to NCEP grid definition 98"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 98 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_98.txt
touch ncep_grid_98.txt
diff -w ncep_grid_98.txt data/ref_new_grid_ncep_98.txt

echo "*** Testing conversion to NCEP grid definition 126"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 126 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_126.txt
touch ncep_grid_126.txt
diff -w ncep_grid_126.txt data/ref_new_grid_ncep_126.txt

echo "*** Testing conversion to NCEP grid definition 127"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 127 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_127.txt
touch ncep_grid_127.txt
diff -w ncep_grid_127.txt data/ref_new_grid_ncep_127.txt

echo "*** Testing conversion to NCEP grid definition 128"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 128 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_128.txt
touch ncep_grid_128.txt
diff -w ncep_grid_128.txt data/ref_new_grid_ncep_128.txt

echo "*** Testing conversion to NCEP grid definition 129"
../wgrib2/wgrib2 test.grb -new_grid_winds earth -new_grid ncep grid 129 junk.grb
../wgrib2/wgrib2 junk.grb -grid -v2 -s -lon 10 12 -lon 20 80 > ncep_grid_129.txt
touch ncep_grid_129.txt
diff -w ncep_grid_129.txt data/ref_new_grid_ncep_129.txt

echo "*** SUCCESS!"
exit 0
