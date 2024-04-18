#!/bin/sh
# This is an optional test script for the wgrib2 project.
#
# Alyson Stahl, 4/18/24

set -e
echo ""
echo "*** Running wgrib2 tests"

# Testing aec compression
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -set_grib_type -grib_out test.aec.grib2

# Check grid data are identical
../wgrib2/wgrib2 data/gdaswave.t00z.wcoast.0p16.f000.grib2 -rpn sto_1 -import_grib test.aec.grib2 -rpn "rcl_1:print_rms" | grep -v "rpn_rms=0:" | wc -l

echo "*** SUCCESS!"
exit 0
