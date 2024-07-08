#!/bin/sh
# This checks the use of netcdf, only available if USE_NETCDF4 is turned on in CMake.
#
# Alyson Stahl, 4/22/24

set -e 

echo "*** Testing converting from grib to netcdf to grib"
# make template
../src/wgrib2 data/ref_simple_packing.grib2 -rpn 0 -grib_out junk_netcdf.template

# make netcdf file
../src/wgrib2 data/ref_simple_packing.grib2 -nc4 -netcdf junk_netcdf.nc

# convert netcdf to grb
../src/wgrib2 junk_netcdf.template -import_netcdf junk_netcdf.nc TMP_500mb "0:1:0:181:0:360" -grib_out junk_netcdf.grb

n=`../src/wgrib2 data/ref_simple_packing.grib2 -var -lev -rpn "sto_1" -import_grib junk_netcdf.grb -rpn "rcl_1:print_rms" | \
grep -v ":rpn=0:" | wc -l`

rm junk_netcdf.grb junk_netcdf.nc junk_netcdf.template
if [ "$n" -ne 1 ] ; then
  exit 1
fi

echo "*** Testing converting from grib to netcdf with variables stored in 3D structure"
../src/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -nc_nlev 7 -netcdf junk_netcdf.nc -match ":(UGRD|VGRD|HGT|TMP):" 
ncdump -v HGT,TMP,UGRD,VGRD junk_netcdf.nc > netcdf.txt
touch netcdf.txt
diff -w data/ref_ncdump4.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt netcdf.txt


echo "*** Testing converting from grib to netcdf using table" 
../src/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ":(UGRD|VGRD|TMP|HGT|RH):" -nc_table data/nctab.table -netcdf tablenc.nc
ncdump -h tablenc.nc > tablenc.txt
touch tablenc.txt
diff -w tablenc.txt data/ref_tablenc.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** SUCCESS!"
exit 0