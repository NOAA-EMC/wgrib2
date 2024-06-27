#!/bin/sh
# This checks the use of AEC compression, only available if USE_AEC is turned on in CMake.
#
# Alyson Stahl, 4/18/24

set -e
echo ""
echo "*** Running wgrib2 tests"

echo "*** Converting simple packing test file to aec"
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -set_grib_type aec -grib_out junk_aec.grb

# Check result
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -v2 > ref_junk_simple.txt
../wgrib2/wgrib2 junk_aec.grb -v2 > junk_aec.txt
diff -w junk_aec.txt ref_junk_simple.txt

echo "*** Converting from aec back to simple packing"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type simple -grib_out junk_simple.grb

# Check result
../wgrib2/wgrib2 junk_simple.grb -v2 > junk_simple.txt
diff -w ref_junk_simple.txt junk_simple.txt

echo "*** Converting from aec to complex1"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex1 -grib_out junk_c1.grb

# Check result
../wgrib2/wgrib2 junk_c1.grb -v2 > junk_c1.txt
diff -w junk_c1.txt junk_aec.txt

echo "*** Converting from aec to complex1-bitmap"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex1-bitmap -grib_out junk_c1b.grb

# Check result
../wgrib2/wgrib2 junk_c1.grb -v2 > junk_c1b.txt
diff -w junk_c1b.txt junk_aec.txt

echo "*** Converting from aec to complex2"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex2 -grib_out junk_c2.grb

# Check result
../wgrib2/wgrib2 junk_c2.grb -v2 > junk_c2.txt
diff -w junk_c2.txt junk_aec.txt

echo "*** Converting from aec to complex2-bitmap"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex2-bitmap -grib_out junk_c2b.grb

# Check result
../wgrib2/wgrib2 junk_c2b.grb -v2 > junk_c2b.txt
diff -w junk_c2b.txt junk_aec.txt

echo "*** Converting from aec to complex3"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex3 -grib_out junk_c3.grb

# Check result
../wgrib2/wgrib2 junk_c3.grb -v2 > junk_c3.txt
diff -w junk_c3.txt junk_aec.txt

echo "*** Converting from aec to complex3-bitmap"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex3-bitmap -grib_out junk_c3b.grb

# Check result
../wgrib2/wgrib2 junk_c3b.grb -v2 > junk_c3b.txt
diff -w junk_c3b.txt junk_aec.txt

echo "*** Converting from aec to ieee"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type ieee -grib_out junk_ieee.grb

# Check result
../wgrib2/wgrib2 junk_ieee.grb -v2 > junk_ieee.txt
diff -w junk_ieee.txt junk_aec.txt

echo "*** Converting from aec to jpeg"
if ../wgrib2/wgrib2 junk_aec.grb -set_grib_type jpeg -grib_out junk_jpeg.grb; then
    # Check result
    ../wgrib2/wgrib2 junk_jpeg.grb -v2 > junk_jpeg.txt
    diff -w junk_jpeg.txt junk_aec.txt
else 
    echo "Use of jpeg not enabled. Skipping test."
fi

echo "*** Checking use of set_grib_type same"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type same -grib_out junk_same.grb
out1=$(../wgrib2/wgrib2 junk_aec.grb -packing)
echo $out1
out2=$(../wgrib2/wgrib2 junk_same.grb -packing)
echo $out2
if [ "$out1" != "$out2" ] ; then 
    echo "failed for returning same packing type"
    exit 1
fi

echo "*** SUCCESS!"
exit 0
