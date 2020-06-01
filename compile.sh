#!/bin/bash

#--------------------------------------------------
# Compile wgrib2 api on WCOSS-DELL.
#
# Invoke this script with no arguments.
#--------------------------------------------------

set -x

machine=$(cat /etc/prod)
echo $machine

if [ $machine = 'mars' ] || [ $machine = 'venus' ]; then
  module purge
  module load ips/18.0.1.163 
  module list
  export CC=icc
  export FC=ifort
  export COMP_SYS=intel_linux
elif [ $machine = 'luna' ] || [ $machine = 'surge' ]; then
  echo cray
  exit
else
  $machine NOT SUPPORTED.  STOP
  exit 1
fi

rm -fr ./include
rm -fr ./lib

cd grib2
make deep-clean
make clean
make lib
cd ..

mkdir ./include
mkdir ./lib

cd include
ln -fs ../grib2/lib/wgrib2api.mod .
ln -fs ../grib2/lib/wgrib2lowapi.mod .
cd ..

cd lib
ln -fs ../grib2/lib/libwgrib2.a
ln -fs ../grib2/lib/libwgrib2_api.a

exit 0
