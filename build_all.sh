#!/bin/bash

#--------------------------------------------------------------
# See Release notes for how to build.
#
# Script takes two argument, the machine name and the
# path to the final install directory.
#--------------------------------------------------------------

set -x

if [ $# -lt 2 ]; then
  set +x
  echo '***ERROR*** must specify machine name and install directory as arguments.'
  exit 1
fi

machine=$1
echo $machine

final_install_dir=$2
echo will install in ${final_install_dir}

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
elif [ $machine = 'cray' ]; then
  module purge
  module load cmake/3.16.2
  module load PrgEnv-intel/5.2.56
  module rm intel
  module load intel/16.3.210
  module load craype-sandybridge
  module load sp-intel/2.0.2
  module load ip2-intel/1.0.0
  module load cray-netcdf/4.3.3.1
  export NETCDF=/opt/cray/netcdf/4.3.3.1/GNU/5.1
  module load cray-hdf5/1.8.14
  export Jasper_ROOT=/usrx/local/prod/jasper/1.900.1/intel/sandybridge
  export PNG_ROOT=/usrx/local/prod/png/1.2.49/intel/sandybridge
  export ZLIB_ROOT=/usrx/local/prod/zlib/1.2.7/intel/sandybridge
elif [ $machine = 'orion' ]; then
  module purge
  module load cmake/3.17.3
  module load intel/2018.4
  export CC=icc
  export FC=ifort
  module use -a /apps/contrib/NCEPLIBS/orion/modulefiles
  module load ip2/1.0.0
  module load sp/2.0.3
  module load netcdf/4.7.2
  export Jasper_ROOT="/apps/jasper-1.900.1"
elif [ $machine = 'jet' ]; then
  module purge
  module load cmake/3.16.1
  module load intel/18.0.5.274
  export CC=icc
  export FC=ifort
  module use -a /lfs4/HFIP/hfv3gfs/nwprod/NCEPLIBS/modulefiles
  module load netcdf/4.6.1
  module load sp/v2.0.2
  module load ip2/v1.0.0
elif [ $machine = 'dell' ]; then
  module purge
  module load cmake
  module load ips/18.0.1.163
  export CC=icc
  export FC=ifort
  module load sp/2.0.2
  module load ip2/1.0.0
  module load HDF5-serial/1.10.1
  module load NetCDF/4.5.0
  export Jasper_ROOT="/usrx/local/prod/packages/gnu/4.8.5/jasper/1.900.1"
else
  set +x
  echo
  echo
  echo "ERROR: MACHINE " $machine " NOT SUPPORTED.  STOP."
  exit 1
fi

# Build library without any external libs.

rm -fr ./build
mkdir -p ./build
cd build

cmake -DMAKE_FTN_API=ON -DUSE_IPOLATES=0 -DUSE_SPECTRAL=OFF -DUSE_NETCDF4=OFF -DUSE_PNG=OFF -DUSE_JASPER=OFF -DCMAKE_INSTALL_PREFIX=${final_install_dir}  ..

make -j 8 VERBOSE=1

make install

rm -fr ${final_install_dir}/bin

cd ..

# Now build executable.

rm -fr ./build.exe
mkdir -p ./build.exe
cd build.exe

cmake -DMAKE_FTN_API=OFF -DUSE_SPECTRAL=ON -DUSE_IPOLATES=3  \
 -DUSE_NETCDF4=ON -DUSE_JASPER=ON -DUSE_AEC=ON -DUSE_PROJ4=OFF \
 -DOPENMP=ON -DCMAKE_INSTALL_PREFIX=${final_install_dir}/binary ..

make -j 8 VERBOSE=1

make install

cp -r ${final_install_dir}/binary/bin ${final_install_dir}/bin

rm -fr ${final_install_dir}/binary

cd ..
rm -fr ./build.exe
