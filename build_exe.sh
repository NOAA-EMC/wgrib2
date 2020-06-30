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
  module load netcdf_parallel/4.7.4
  export CC=icc
  export FC=ifort
  export ip2_DIR=/scratch1/NCEPDEV/da/George.Gayno/ip2lib.git/NCEPLIBS-ip2/lib/cmake/ip2
  export sp_DIR=/scratch1/NCEPDEV/da/George.Gayno/splib.git/NCEPLIBS-sp/lib/cmake/sp
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
  module use /usrx/local/nceplibs/dev/NCEPLIBS/modulefiles
  module load netcdf_parallel/4.7.4
  module load jasper/1.900.29
  export Jasper_ROOT="/usrx/local/prod/packages/gnu/4.8.5/jasper/1.900.29"
  export CC=icc
  export FC=ifort
  export ip2_DIR=/gpfs/dell2/emc/modeling/noscrub/George.Gayno/ip2lib.git/NCEPLIBS-ip2/lib/cmake/ip2
  export sp_DIR=/gpfs/dell2/emc/modeling/noscrub/George.Gayno/splib.git/NCEPLIBS-sp/lib/cmake/sp
else
  set +x
  echo
  echo
  echo "ERROR: MACHINE " $machine " NOT SUPPORTED.  STOP."
  exit 1
fi

rm -fr ./build.exe
mkdir -p ./build.exe
cd build.exe

cmake -DMAKE_FTN_API=OFF -DUSE_SPECTRAL=ON -DUSE_IPOLATES=3  \
 -DUSE_NETCDF4=ON -DUSE_JASPER=OFF -DUSE_AEC=ON -DUSE_PROJ4=OFF \
 -DUSE_OPENMP=ON ../grib2

make -j 8 VERBOSE=1

make install

rm -fr ./install/lib


