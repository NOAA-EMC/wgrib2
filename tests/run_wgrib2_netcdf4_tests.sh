#!/bin/sh
# This checks the use of AEC compression, only available if USE_NETCDF4 is turned on in CMake.
#
# Alyson Stahl, 4/22/24

set -e 
set -x

# make template
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -rpn 0 -grib_out junk_netcdf.template

# make netcdf file
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -nc4 -netcdf junk_netcdf.nc

# convert netcdf to grb
../wgrib2/wgrib2 junk_netcdf.template -import_netcdf junk_netcdf.nc TMP_500mb "0:1:0:181:0:360" -grib_out junk_netcdf.grb

n=`../wgrib2/wgrib2 data/ref_simple_packing.grib2 -var -lev -rpn "sto_1" -import_grib junk_netcdf.grb -rpn "rcl_1:print_rms" | \
grep -v ":rpn=0:" | wc -l`

rm junk_netcdf.grb junk_netcdf.nc junk_netcdf.template
if [ "$n" -eq 1 ] ; then
  echo "success"
  exit 0
else
  exit 1
fi

