#!/bin/sh
# This is an optional test script for the wgrib2 project. 
# Requires FTP_EXTRA_LARGE_TEST_FILES=ON
#
# Alyson Stahl, 4/17/24

set -e
echo ""
echo "*** Running wgrib2 tests on extra large FTP test files"

echo "*** Running an inventory test on extra large FTP test file"
../wgrib2/wgrib2 data/rrfs.t18z.prslev.f000.grib2 > ftp_inv3.txt
cat ftp_inv3.txt
diff -w ftp_inv3.txt data/ref_rrfs.t18z.prslev.f000.grib2.inv

echo "*** SUCCESS!"
exit 0

