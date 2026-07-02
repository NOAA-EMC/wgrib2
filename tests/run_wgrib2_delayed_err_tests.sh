#!/bin/sh
# This script tests delayed errors 
#
# 1. badly encoded GDS for thinned grids
#
# w. ebisuzaki 5/2026

set -ex
echo ""
echo "*** Running wgrib2 delayed error tests"
wgrib2='../src/wgrib2'
# wgrib2=wgrib2

file='data/delayed_error.grib2'

# check if fatal error 
set +e
$wgrib2 $file
err=$?
if [ "$err" -ne 1 ] ; then
  echo "failed test 1a"
  exit 1
fi

# echo check if reset_delayed_error works
set -e
$wgrib2 $file -reset_delayed_error 
err=$?
if [ "$err" -ne 0 ] ; then
  echo "failed test 1b"
  exit 1
fi

echo "*** SUCCESS!"
exit 0
