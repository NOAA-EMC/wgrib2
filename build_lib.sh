#!/bin/bash

set -x

if [ $# -lt 1 ]; then
  set +x
  echo '***ERROR*** must specify machine name as argument.'
  exit 1
fi

machine=$1
echo $machine

if [ $machine = 'hera' ]; then
  module purge
  module load cmake/3.16.1
  module load intel/18.0.5.274
  export CC=icc
  export FC=ifort
elif [ $machine = 'cray' ]; then
  module purge
  module load cmake/3.16.2
  module load PrgEnv-intel/5.2.56
  module rm intel
  module load intel/16.3.210
  module load craype-haswell
elif [ $machine = 'orion' ]; then
  module purge
  module load cmake/3.17.3
  module load intel/2018.4
  export CC=icc
  export FC=ifort
elif [ $machine = 'jet' ]; then
  module purge
  module load cmake/3.16.1
  module load intel/18.0.5.274
  export CC=icc
  export FC=ifort
elif [ $machine = 'dell' ]; then
  module purge
  module load cmake
  module load ips/18.0.1.163
  export CC=icc
  export FC=ifort
else
  set +x
  echo
  echo
  echo "ERROR: MACHINE " $machine " NOT SUPPORTED.  STOP."
  exit 1
fi

rm -fr ./build.lib
mkdir -p ./build.lib
cd build.lib

cmake -DMAKE_FTN_API=ON -DUSE_IPOLATES=0 -DUSE_SPECTRAL=OFF -DUSE_NETCDF4=OFF -DUSE_PNG=OFF -DUSE_JASPER=OFF ../grib2

make -j 8 VERBOSE=1

make install

rm -fr ./install/bin
