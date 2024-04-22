#!/bin/sh
# This is an optional test script for the wgrib2 project.
#
# Alyson Stahl, 4/17/24

set -e
echo ""
echo "*** Running wgrib2 tests on FTP test files"

# Run an inventory on test files.
../wgrib2/wgrib2 data/WW3_Regional_US_West_Coast_20220718_0000.grib2 > ftp_inv1.txt
cat ftp_inv1.txt
cmp ftp_inv1.txt data/ref_WW3_Regional_US_West_Coast_20220718_0000.grib2.inv

#../wgrib2/wgrib2 data/rrfs.t18z.prslev.f000.grib2 > ftp_inv2.txt
#cat ftp_inv2.txt
#cmp ftp_inv2.txt data/ref_rrfs.t18z.prslev.f000.grib2.inv

echo "*** SUCCESS!"
exit 0

