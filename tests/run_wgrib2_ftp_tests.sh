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
cmp ftp_inv1.txt data/ref_WW3_Regional_US_West_Coast_20220718_0000.grib2.inv

echo "*** SUCCESS!"
exit 0

