#!/bin/sh
# This checks the use of ip external library, only available if USE_IPOLATES=1 CMake.
#
# Alyson Stahl 5/9/2024

set -e

../wgrib2/wgrib2.exe data/tmp_int.grb -new_grid_winds earth  -new_grid latlon 0:359:1 0:90:1 OUT.grb
echo "*** SUCCESS!"
exit 0