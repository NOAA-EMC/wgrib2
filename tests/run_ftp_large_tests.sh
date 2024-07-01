#!/bin/sh
# This is an optional test script for the wgrib2 project. 
# Requires FTP_LARGE_TEST_FILES=ON
#
# Alyson Stahl, 4/17/24

set -e
echo ""
echo "*** Running wgrib2 tests on large FTP test files"

echo "*** Running an inventory test on large FTP test file"
../src/wgrib2 data/fv3lam.t00z.prslev.f000.grib2 > ftp_inv2.txt
cat ftp_inv2.txt
diff -w ftp_inv2.txt data/ref_fv3lam.t00z.prslev.f000.grib2.inv

echo "*** SUCCESS!"
exit 0

