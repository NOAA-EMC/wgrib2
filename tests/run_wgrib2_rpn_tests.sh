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
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ":TMP:0.01" -rpn "273.15:-:9:*:5:/:32:+" -text fahrenheit.txt
touch fahrenheit.txt
cmp fahrenheit.txt data/ref_fahrenheit.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Takes relative humidity values, limits them to 100 and outputs to grib file"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -match ":RH:" -if ":RH:" -rpn "100:min" -fi -grib_out rh.grb -not_if ":RH:" -grib rh.grb
../wgrib2/wgrib2 rh.grb > rh.txt
touch rh.txt
cmp rh.txt data/ref_rh.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** Calculates wind speed for records 1-25, then returns the average, min, and max values"
../wgrib2/wgrib2 data/gdas.t12z.pgrb2.1p00.anl.75r.grib2 -for 1:25 -match ":(UGRD|VGRD):" \
    -if ":UGRD:" -rpn "sto_1" -fi \
    -if ":VGRD:" -rpn "sto_2" -fi \
    -if_reg 1:2 \
        -rpn "rcl_1:sq:rcl_2:sq:+:sqrt:clr_1:clr_2" \
        -set_var WIND \
        -grib_out tmp_windspeed.grb

../wgrib2/wgrib2.exe tmp_windspeed.grb -rpn print_min -rpn print_max -rpn print_ave > min_max_ave_windspeed.txt
touch min_max_ave_windspeed.txt
cmp min_max_ave_windspeed.txt data/ref_rpn.windspeed.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt

echo "*** SUCCESS!"
exit 0
