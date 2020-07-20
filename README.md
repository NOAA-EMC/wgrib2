WGRIB2 library repository.

# Build Instructions

Building wgrib2 requires, CMake, a C compiler, a Fortran compiler
(optional), and various 3rd party libraries depending on build
configuration.

```
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH=path/to/dependencies
make
make install
```

wgrib2 will generate the following CMake targets:

```
wgrib2::wgrib2_lib
wgrib2::wgrib2_api (fortran target)
```

## Build Options

* USE_NETCDF3 = OFF

* USE_NETCDF4 = ON

* USE_REGEX = ON

* USE_TIGGE = ON

* USE_MYSQL = OFF

* USE_IPOLATES = 3 (requires ip2 lib)

* USE_OPENMP = OFF

* MAKE_FTN_API = ON

* DISABLE_ALARM = OFF

* USE_G2CLIB = OFF

* USE_PNG = ON

* USE_JASPER = ON

* USE_AEC = OFF
