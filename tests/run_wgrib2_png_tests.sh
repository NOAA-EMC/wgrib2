#!/bin/sh
# This script runs a series of tests using the RPN calculator
#
# Alyson Stahl 5/7/2024

set -e
echo ""
echo "*** Running wgrib2 png tests"



r1="1:0:(2000,1000),lon=304.937500,lat=-27.584172,val=1"
r2=`../wgrib2/wgrib2 data/png_4bits.png -ijlat 2000 1000`
if [ "$r1" != "$r2" ] ; then
  echo "failed png test1 $r2"
  echo "expected         $r1"
  exit 1
fi

r1="1:0:(2000,2000),lon=249.995002,lat=39.995000,val=0"
r2=`../wgrib2/wgrib2 data/large_png.grb2 -ijlat 2000 2000`
if [ "$r1" != "$r2" ] ; then
  echo "failed png test2 $r2"
  echo "expected         $r1"
  exit 1
fi

echo "*** SUCCESS!"
exit 0
