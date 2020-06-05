#!/bin/bash

#------------------------------------------------------
#
# Compile the wgrib2 stand-alone executable.
# To compile the wgrib2 library, use compile_lib.sh.
#
# Invoke this script with one argument - the
# machine name.  Valid choices are: 'dell',
# 'cray', 'jet', 'orion', 'hera'.
#
# The default is to build with NETCDF and with
# IPOLATES.  To modify the build, set the
# appropriate "USE" flags in ./grib2/makefile_exec
# See the comments in the makefile for more
# details.
# 
#------------------------------------------------------

set -x

if [ $# -lt 1 ]; then
  set +x
  echo
  echo '***ERROR*** must specify machine name as argument.'
  exit 1
fi

machine=$1
echo $machine

if [ $machine = 'dell' ]; then
  module purge
  module load ips/18.0.1.163 
  module list
  export CC=icc
  export FC=ifort
  export COMP_SYS=intel_linux
elif [ $machine = 'cray' ]; then
  echo cray
  module purge
  module load gcc/4.9.2
  export CC=gcc
  export FC=gfortran
elif [ $machine = 'jet' ]; then
  module purge
  module load intel/18.0.5.274
  export CC=icc
  export FC=ifort
  export COMP_SYS=intel_linux
elif [ $machine = 'orion' ]; then
  module purge
  module load intel/2018.4
  export CC=icc
  export FC=ifort
  export COMP_SYS=intel_linux
elif [ $machine = 'hera' ]; then
  module purge
  module load intel/18.0.5.274
  export CC=icc
  export FC=ifort
  export COMP_SYS=intel_linux
else
  set +x
  echo
  echo "ERROR: MACHINE " $machine " NOT SUPPORTED.  STOP."
  exit 1
fi

rm -fr ./bin

cd grib2
make -f makefile_exec deep-clean
make -f makefile_exec clean
make -f makefile_exec
cd ..

mkdir ./bin

cd bin
cp ../grib2/wgrib2/wgrib2  .

exit 0
