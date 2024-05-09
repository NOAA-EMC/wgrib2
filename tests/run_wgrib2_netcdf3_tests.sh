#!/bin/sh
# This checks the use of AEC compression, only available if USE_NETCDF4 is turned on in CMake.
#
# Alyson Stahl, 4/22/24

set -e 
set -x

# make template
../wgrib2/wgrib2 data/tmp_int.grb -rpn 0 -grib_out junk.template

# make netcdf file
../wgrib2/wgrib2 data/tmp_int.grb -nc3 -netcdf junk.nc

# convert netcdf to grb
../wgrib2/wgrib2 junk.template -import_netcdf junk.nc TMP_500mb "0:1:0:181:0:360" -grib_out junk.grb

n=`../wgrib2/wgrib2 data/tmp_int.grb -var -lev -rpn "sto_1" -import_grib junk.grb -rpn "rcl_1:print_rms" | \
grep -v ":rpn=0:" | wc -l`

rm junk.grb junk.nc junk.template
if [ "$n" -eq 1 ] ; then
  echo "success"
  exit 0
else
  exit 1
fi

