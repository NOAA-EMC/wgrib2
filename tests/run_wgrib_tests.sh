#!/bin/sh
# This script runs tests for the wgrib utility.
# Requires BUILD_WGRIB=ON
#
# Alyson Stahl, 1-/18/24

set -e
echo ""
echo "*** Running wgrib tests"

../wgrib/wgrib data/ref_ecmwf.jul.grb1 > wgrib_junk.txt
diff -w wgrib_junk.txt data/ref_ecmwf.jul.grb1.inv

echo "*** SUCCESS!"
exit 0