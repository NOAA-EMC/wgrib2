WGRIB2 library repository.

# Build Instructions

Building wgrib2 requires, CMake, a C compiler, a Fortran compiler
(optional), and various 3rd party libraries depending on build
configuration.

```
mkdir build && cd build
cmake .. -DCMAKE\_INSTALL\_PREFIX=install -DCMAKE\_PREFIX\_PATH=path/to/dependencies
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

Built with NetCDF4 support

* USE_REGEX = ON

* USE_TIGGE = ON

* USE_MYSQL = OFF

* USE_OPENMP = OFF

* MAKE_FTN_API = ON

Build Fortran interface

* DISABLE_ALARM = OFF

* USE_G2CLIB = OFF

* USE_PNG = ON

* USE_JASPER = ON

* USE_AEC = OFF
