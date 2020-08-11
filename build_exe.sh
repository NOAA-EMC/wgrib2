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
  module load netcdf/4.6.1
  export CC=icc
  export FC=ifort
  module use -a /scratch2/NCEPDEV/nwprod/NCEPLIBS/modulefiles
  module load ip2/1.0.0
  module load sp/2.0.2
elif [ $machine = 'orion' ]; then
  module purge
  module load cmake/3.17.3
  module load intel/2018.4
  export CC=icc
  export FC=ifort
  export ip2_DIR=/work/noaa/da/ggayno/save/ip2lib.git/NCEPLIBS-ip2/lib/cmake/ip2
  export sp_DIR=/work/noaa/da/ggayno/save/splib.git/NCEPLIBS-sp/lib/cmake/sp
  module load netcdf/4.7.2
  export Jasper_ROOT=/apps/jasper-1.900.1
elif [ $machine = 'jet' ]; then
  module purge
  module load cmake/3.16.1
  module load intel/18.0.5.274
  export CC=icc
  export FC=ifort
  module load netcdf/4.6.1
  export ip2_DIR=/lfs4/HFIP/emcda/George.Gayno/ip2lib.git/NCEPLIBS-ip2/lib/cmake/ip2
  export sp_DIR=/lfs4/HFIP/emcda/George.Gayno/splib.git/NCEPLIBS-sp/lib/cmake/sp
elif [ $machine = 'dell' ]; then
  module purge
  module load cmake
  module load ips/18.0.1.163
  module use /usrx/local/nceplibs/dev/NCEPLIBS/modulefiles
  module load NetCDF/4.5.0
  export Jasper_ROOT=/usrx/local/prod/packages/gnu/4.8.5/jasper/1.900.1
  export CC=icc
  export FC=ifort
  export ip2_DIR=/gpfs/dell2/emc/modeling/noscrub/George.Gayno/ip2lib.git/NCEPLIBS-ip2/lib/cmake/ip2
  export sp_DIR=/gpfs/dell2/emc/modeling/noscrub/George.Gayno/splib.git/NCEPLIBS-sp/lib/cmake/sp
elif [ $machine = 'cray' ]; then
  module purge
  module load cmake/3.16.2
  module load PrgEnv-intel/5.2.56
  module rm intel
  module load intel/16.3.210
  module load craype-sandybridge
  module load NetCDF-intel-haswell/4.2
  export ZLIB_ROOT=/usrx/local/prod/zlib/1.2.7/intell/haswell
  export PNG_ROOT=/usrx/local/prod/png/1.2.49/intel/haswell
  export Jasper_ROOT=/usrx/local/prod/jasper/1.900.1/intel/haswell
  export ip2_DIR=/gpfs/hps3/emc/global/noscrub/George.Gayno/ip2lib.git/NCEPLIBS-ip2/lib/cmake/ip2
  export sp_DIR=/gpfs/hps3/emc/global/noscrub/George.Gayno/splib.git/NCEPLIBS-sp/lib/cmake/sp
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
 -DUSE_NETCDF4=ON -DUSE_JASPER=ON -DUSE_AEC=ON -DUSE_PROJ4=OFF \
 -DOPENMP=ON ..

make -j 8 VERBOSE=1

make install

rm -fr ./install/lib
