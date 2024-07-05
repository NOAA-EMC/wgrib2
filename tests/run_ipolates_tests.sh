#!/bin/sh
# This checks the use of ip external library, only available if USE_IPOLATES=ON CMake.
#
# Alyson Stahl 5/9/2024

set -e

../wgrib2/wgrib2 data/ref_simple_packing.grib2 -new_grid_winds earth  -new_grid latlon 0:359:1 0:90:1 OUT.grb

echo "*** SUCCESS!"
exit 0