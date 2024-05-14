#!/bin/sh
# This checks the use of AEC compression, only available if USE_NETCDF4 is turned on in CMake.
#
# Alyson Stahl, 4/22/24

set -e 
set -x

echo "*** Testing converting from grib to netcdf to grib"
# make template
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -rpn 0 -grib_out junk_netcdf.template

# make netcdf file
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -nc4 -netcdf junk_netcdf.nc

# convert netcdf to grb
../wgrib2/wgrib2 junk_netcdf.template -import_netcdf junk_netcdf.nc TMP_500mb "0:1:0:181:0:360" -grib_out junk_netcdf.grb

n=`../wgrib2/wgrib2 data/ref_simple_packing.grib2 -var -lev -rpn "sto_1" -import_grib junk_netcdf.grb -rpn "rcl_1:print_rms" | \
grep -v ":rpn=0:" | wc -l`

rm junk_netcdf.grb junk_netcdf.nc junk_netcdf.template
if [ "$n" -ne 1 ] ; then
  exit 1
fi

echo "*** Testing converting from grib to netcdf on larger grib file using nc_nlev"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ":(UGRD|VGRD|HGT|TMP):" -grib_out junk_netcdf.template
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -nc_nlev 7 -netcdf junk_netcdf.nc -match ":(UGRD|VGRD|HGT|TMP):" 
ncdump -v HGT,TMP,UGRD,VGRD junk_netcdf.nc > netcdf.txt
touch netcdf.txt
cmp data/ref_ncdump.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt netcdf.txt


echo "*** SUCCESS!"
exit 0