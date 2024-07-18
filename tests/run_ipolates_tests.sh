#!/bin/sh
# This checks the use of ip external library, only available if USE_IPOLATES=ON CMake.
#
# Alyson Stahl 5/9/2024

set -e

echo "create a new grib file with intger values"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -rpn floor -set_scaling 0 0 -grib_out test.grb

echo "*** Testing conversion from earth to grid"
../wgrib2/wgrib2 test.grb -match "(UGRD|VGRD)" -new_grid_winds grid  \
    -new_grid latlon 0:360:1 00:91:1 new_grid.grb
../wgrib2/wgrib2 new_grid.grb -grid -v2 -s -lon 10 12 -lon 20 80 > new_grid.txt
touch new_grid.txt
diff -w new_grid.txt data/ref_new_grid_gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Testing conversion from grid to earth"
../wgrib2/wgrib2 new_grid.grb  -new_grid_winds earth  \
    -new_grid latlon 0:360:1 00:91:1 new_grid_earth.grb
../wgrib2/wgrib2 new_grid.grb -grid -v2 -s -lon 10 12 -lon 20 80 > new_grid_earth.txt
touch new_grid_earth.txt
diff -w new_grid_earth.txt data/ref_new_grid_earth_gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** SUCCESS!"
exit 0
