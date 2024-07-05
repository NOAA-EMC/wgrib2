#!/bin/sh
# This is an optional test script for the wgrib2 project.
# Requires FTP_TEST_FILES=ON
#
# Alyson Stahl, 4/17/24

set -e
echo ""
echo "*** Running wgrib2 tests on FTP test files"

echo "*** Running an inventory test with FTP test file"
../wgrib2/wgrib2 data/WW3_Regional_US_West_Coast_20220718_0000.grib2 > ftp_inv1.txt
cat ftp_inv1.txt
diff -w ftp_inv1.txt data/ref_WW3_Regional_US_West_Coast_20220718_0000.grib2.inv

echo "*** Testing merge_fcst"
../wgrib2/wgrib2 data/aqm.t12z.max_8hr_o3.227.grib2 -merge_fcst 3 merge.grb
../wgrib2/wgrib2 merge.grb > merge.txt
cat merge.txt
diff -w merge.txt data/ref_merge_fcst.aqm.t12z.max_8hr_o3.227.grib2.txt

echo "*** SUCCESS!"
exit 0

