#!/bin/sh
# This checks the use of AEC compression, only available if USE_AEC is turned on in CMake.
#
# Alyson Stahl, 4/18/24

set -e
echo ""
echo "*** Running wgrib2 tests"

echo "*** Converting simple packing test file to aec"
cksum0=`../wgrib2/wgrib2 data/ref_simple_packing.grib2 -checksum data`
../wgrib2/wgrib2 data/ref_simple_packing.grib2 -set_grib_type aec -grib_out junk_aec.grb
cksum1=`../wgrib2/wgrib2 junk_aec.grb -checksum data`

if [ "$cksum0" != "$cksum1" ] ; then
    echo "failed for compressing to aec and reading from aec"
    exit 1
fi

echo "*** Converting from aec back to simple packing"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type simple -grib_out junk_simple.grb
cksum2=`../wgrib2/wgrib2 junk_simple.grb -checksum data`

if [ "$cksum1" != "$cksum2" ] ; then
    echo "failed for converting to simple packing"
    exit 1
fi

echo "*** Converting from aec to complex1"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex1 -grib_out junk_c1.grb
cksum3=`../wgrib2/wgrib2 junk_c1.grb -checksum data`

if [ "$cksum1" != "$cksum3" ] ; then
    echo "failed for converting to complex1 packing"
    exit 1
fi

echo "*** Converting from aec to complex1-bitmap"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex1-bitmap -grib_out junk_c1b.grb
cksum4=`../wgrib2/wgrib2 junk_c1b.grb -checksum data`

if [ "$cksum1" != "$cksum4" ] ; then
    echo "failed for converting to complex1 packing (with bitmap)"
    exit 1
fi

echo "*** Converting from aec to complex2"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex2 -grib_out junk_c2.grb
cksum5=`../wgrib2/wgrib2 junk_c2.grb -checksum data`

if [ "$cksum1" != "$cksum5" ] ; then
    echo "failed for converting to complex2 packing"
    exit 1
fi

echo "*** Converting from aec to complex2-bitmap"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex2-bitmap -grib_out junk_c2b.grb
cksum6=`../wgrib2/wgrib2 junk_c2b.grb -checksum data`

if [ "$cksum1" != "$cksum6" ] ; then
    echo "failed for converting to complex2 packing (with bitmap)"
    exit 1
fi

echo "*** Converting from aec to complex3"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex3 -grib_out junk_c3.grb
cksum7=`../wgrib2/wgrib2 junk_c3.grb -checksum data`

if [ "$cksum1" != "$cksum7" ] ; then
    echo "failed for converting to complex3 packing"
    exit 1
fi

echo "*** Converting from aec to complex3-bitmap"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type complex3-bitmap -grib_out junk_c3b.grb
cksum8=`../wgrib2/wgrib2 junk_c3b.grb -text -checksum data`

if [ "$cksum1" != "$cksum8" ] ; then
    echo "failed for converting to complex3 packing (with bitmap)"
    exit 1
fi

echo "*** Converting from aec to ieee"
../wgrib2/wgrib2 junk_aec.grb -set_grib_type ieee -grib_out junk_ieee.grb
cksum9=`../wgrib2/wgrib2 junk_ieee.grb -checksum data`

if [ "$cksum1" != "$cksum9" ] ; then
    echo "failed for converting to ieee packing"
    exit 1
fi

echo "*** Converting from aec to jpeg"
if ../wgrib2/wgrib2 junk_aec.grb -set_grib_type jpeg -grib_out junk_jpeg.grb; then
    cksum10=`../wgrib2/wgrib2 junk_jpeg.grb -checksum data`
    if [ "$cksum1" != "$cksum10" ] ; then
        echo "failed for converting to jpeg packing"
        exit 1
    fi
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
