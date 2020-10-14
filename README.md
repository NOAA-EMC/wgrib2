![Status](https://github.com/NOAA-EMC/NCEPLIBS-wgrib2/workflows/Build/badge.svg)

# WGRIB 2

Provides functionality for interacting with, reading, writing, and manipulating grib2 files, with a CMake build. 

See wgrib2 documentation and release page [here](https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/)

The CMake build provided here supports most build options (NetCDF, PNG, Jasper, spectral, and ipolates), but not certain features such as MySQL. If those features are necessary please use the makefile build provided in the link above.

### Authors

Wesley Ebisuzaki - CPC

Kyle Gerheiser (CMake build) - NOAA/EMC

# Installing

Building wgrib2 requires, CMake, a C compiler, a Fortran compiler
(optional), and various 3rd party libraries depending on build
configuration.

```
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH=path/to/dependencies
make
make install
```

# Using the wgrib2 library

To use the wgrib2 library the CMake build offers a package config.

After running `find_package(wgrib2)` CMake will generate the following targets for use in your project:

```
wgrib2::wgrib2_lib (c library)
wgrib2::wgrib2_api (fortran target)
```

## Default Build Options

* USE_NETCDF3 = OFF

* USE_NETCDF4 = ON

* USE_REGEX = ON

* USE_TIGGE = ON

* USE_MYSQL = OFF (not yet supported in CMake)

* USE_IPOLATES = 0 (= 3 requires ip2 lib)

* USE_SPECTRAL = OFF (requires SP lib)

* USE_UDF = OFF (not yet supported in CMake)

* USE_OPENMP = OFF

* USE_PROJ4 = OFF (not yet supported in CMake)

* MAKE_FTN_API = ON

* DISABLE_ALARM = OFF

* USE_G2CLIB = OFF (not yet supported in CMake)

* USE_PNG = ON (requires PNG library)

* USE_JASPER = ON (requires Jasper library)

* USE_AEC = OFF (not yet supported in CMake)

## Disclaimer

The United States Department of Commerce (DOC) GitHub project code is
provided on an "as is" basis and the user assumes responsibility for
its use. DOC has relinquished control of the information and no longer
has responsibility to protect the integrity, confidentiality, or
availability of the information. Any claims against the Department of
Commerce stemming from the use of its GitHub project will be governed
by all applicable Federal law. Any reference to specific commercial
products, processes, or services by service mark, trademark,
manufacturer, or otherwise, does not constitute or imply their
endorsement, recommendation or favoring by the Department of
Commerce. The Department of Commerce seal and logo, or the seal and
logo of a DOC bureau, shall not be used in any manner to imply
endorsement of any commercial product or activity by DOC or the United
States Government.
