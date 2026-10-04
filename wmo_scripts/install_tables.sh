#!/bin/sh

## @file
## @brief This script installs WMO code info into wgrib2.
## @author Public Domain: Manfred Schwarb  @date 03/2025

cd tables.tmp || exit 1

#for fil in *.dat *.c; do
#  diff -Nu "../../src/$fil" .
#done

cp -vp ./*.dat ./*.c ./*.h ../../src/

exit
