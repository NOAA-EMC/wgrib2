#!/bin/bash
set -ex

# libmariadb is in lib/mariadb
export LDFLAGS="${LDFLAGS} -Wl,-rpath,${PREFIX}/lib/mariadb -L${PREFIX}/lib/mariadb"

cmake -S . -B build ${CMAKE_ARGS} \
  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DMYSQL_CONFIG="${PREFIX}/bin/mariadb_config" \
  -DBUILD_EXTRA=ON \
  -DUSE_MYSQL=ON \
  -DUSE_NETCDF=ON \
  -DUSE_IPOLATES=ON \
  -DUSE_OPENMP=ON \
  -DUSE_G2CLIB_HIGH=ON \
  -DUSE_G2CLIB_LOW=ON \
  -DBUILD_SHARED_LIB=OFF

cmake --build build -j"${CPU_COUNT:-4}"
cmake --install build
