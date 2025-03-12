#!/bin/sh
#  03/2025  Public Domain  Manfred Schwarb

cd tables.tmp || exit 1

#for fil in *.dat *.c; do
#  diff -Nu "../../wgrib2/$fil" .
#done

cp -vp ./*.dat ./*.c ../../wgrib2/

exit
