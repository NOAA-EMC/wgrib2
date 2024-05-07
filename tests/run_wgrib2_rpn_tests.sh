#!/bin/sh
# This script runs a series of tests using the RPN calculator
#
# Alyson Stahl 5/7/2024

set -e
echo ""
echo "*** Running wgrib2 rpn tests"

echo "*** Converting temperatures from K to C"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ":TMP:0.01" -rpn "273.15:-" -text celsius.txt
touch celsius.txt
cmp celsius.txt data/ref_celsius.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Converting temperatures from K to F"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ":TMP:0.01" -rpn "273.15:-:9:*:5:/:32:+" -text farenheit.txt
touch farenheit.txt
cmp farenheit.txt data/ref_farenheit.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Takes relative humidity values, limits them to 100 and outputs to grib file"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ":RH:" -if ":RH:" -rpn "100:min" -fi -grib_out rh.grb -not_if ":RH:" -grib rh.grb
../wgrib2/wgrib2 rh.grb > rh.txt
touch rh.txt
cmp rh.txt data/ref_rh.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** SUCCESS!"
exit 0
