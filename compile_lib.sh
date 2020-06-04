#!/bin/bash

#------------------------------------------------------
# Compile the wgrib2 library api.  To compile
# the wgrib2 stand-alone executable, use
# compile_exe.sh.
#
# Invoke this script with one argument - the
# machine name.  Valid choices are: 'dell',
# 'cray', 'jet', 'orion', 'hera'.
#
# The default is to build without NETCDF and
# without IPOLATES.  To modify the build, set
# the appropriate "USE" flags in ./grib2/makefile_lib
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
  module load PrgEnv-intel/5.2.56
  module rm intel
  module load intel/16.3.210
  module load craype-haswell
  export CC=icc
  export FC=ifort
  export COMP_SYS=intel_linux
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

rm -fr ./include
rm -fr ./lib

cd grib2
make -f makefile_lib deep-clean
make -f makefile_lib clean
make -f makefile_lib lib
cd ..

mkdir ./include
mkdir ./lib

cd include
cp ../grib2/lib/wgrib2api.mod .
cp ../grib2/lib/wgrib2lowapi.mod .
cd ..

cd lib
cp ../grib2/lib/libwgrib2.a .
cp ../grib2/lib/libwgrib2_api.a .

exit 0
