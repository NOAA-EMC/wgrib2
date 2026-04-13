#!/bin/sh
# This script runs a series of tests of the gmerge program.
#
# Wesley Ebisuzaki 10/2025

set -e
set --

echo ""
echo "*** Running gmerge tests"

file=data/gdas.t12z.pgrb2.1p00.anl.75r.grib2

i=0
while [ $i -lt 201 ]
do
  set -- "$@" "$file"
  i=$((i + 1))
done

../aux_progs/gmerge tmp.gmerge.grb "$@"

echo "*** running gmerge and ens_qc test "

../aux_progs/gmerge - "$@" | ../src/wgrib2 - -ens_qc ens_qc.x ens_qc.y ens_qc.z 1 >/dev/null

ck="1559805086 4891"
newck=$(../src/wgrib2 ens_qc.x -match spread -stats | cksum)
echo "ck=$ck"
echo "newck=$newck"
if [ "$ck" != "$newck" ] ; then
    echo "ck=$ck"
    echo "newck=$newck"
    echo "error in ens_qc"
    exit 1
fi

echo "Testing error cases."
echo "Testing with too few arguments."

../aux_progs/gmerge && exit 1
if [ $? -ne 8 ]; then
    exit 1
fi

echo "Testing with bad argument."
../aux_progs/gmerge /bad_directory/tmp.gmerge.grb "$@" && exit 1
if [ $? -ne 8 ]; then
    exit 1
fi

echo "Testing with bad file."
../aux_progs/gmerge tmp.gmerge.grb $file bad_file.grb && exit 1
if [ $? -ne 8 ]; then
    exit 1
fi

echo "*** SUCCESS!"
exit 0
