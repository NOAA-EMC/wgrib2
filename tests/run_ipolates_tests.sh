#!/bin/sh
# This checks the use of ip external library, only available if USE_IPOLATES=ON CMake.
#
# Alyson Stahl 5/9/2024

set -e

../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match "(UGRD|VGRD)" -new_grid_winds grid  \
    -new_grid latlon 0:359:1 0:90:1 new_grid.grb
../wgrib2/wgrib2 new_grid.grb -grid -v2 > new_grid.txt
touch new_grid.txt
diff -w new_grid.txt data/ref_new_grid_gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** SUCCESS!"
exit 0