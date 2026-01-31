@mainpage

# Introduction

This document briefly describes wgrib2, a utility and library for 
interacting with, reading, writing, and manipulating GRIB2 files.

Wgrib2 is maintained as a collaboration between [Climate Prediction
Center (CPC)](https://www.cpc.ncep.noaa.gov/) and the [Environmental
Modeling Center (EMC)](https://www.emc.ncep.noaa.gov/emc.php).

## Installation

Download the tarball from the release page and unpack it, and cd into the main directory of the library. Then run the following commands, substituting your directory locations for the CMAKE_INSTALL_PREFIX (where wgrib2 will be installed), and the CMAKE_PREFIX_PATH (where the build will look for dependencies):

```
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH="/usr/local/NCEPLIBS-g2c;/usr/local/NCEPLIBS-ip" <Additional CMake options>
make
make install
```

Additional CMake build options can be used to configure the build by setting them with `-D<OPTION>=<VALUE>`. 

### Common CMake Options

| Option | Description | Default | Additional Requirements |
|--------|-------------|---------|-------------------------|
| BUILD_LIB | Build wgrib2 library | ON | N/A |
| BUILD_SHARED_LIB | Build wgrib2 library as a shared library | ON | N/A |
| MAKE_FTN_API | Build Fortran API | OFF | BUILD_LIB = ON |
| USE_NETCDF | Read and write NetCDF files | OFF | NetCDF-c | 
| USE_IPOLATES | Use NCEPLIBS-ip library (required for grid interpolation) | OFF | [NCEPLIBS-ip v5.2.0+](https://github.com/NOAA-EMC/NCEPLIBS-ip) |
| USE_G2CLIB_HIGH | Use NCEPLIBS-g2c high-level decoder (needed for '-g2clib 2' option)  | OFF | [NCEPLIBS-g2c 2.2.0+](https://github.com/NOAA-EMC/NCEPLIBS-g2c) |
| USE_G2CLIB_LOW | Use NCEPLIBS-g2c low-level decoders for PNG (packing type 41) and Jpeg2000 (packing type 40) | OFF | [NCEPLIBS-g2c 2.2.0+](https://github.com/NOAA-EMC/NCEPLIBS-g2c) with desired decoders enabled |
| USE_AEC | Use AEC to enable encoding/decoding of GRIB2 data with CCSDS packing (packing type 42) | OFF | [libaec v1.0.6+](https://gitlab.dkrz.de/k202009/libaec) |
| USE_OPENMP | Use with OpenMP | OFF | OpenMP-c |
| USE_PROJ4 | Use with Proj.4 library | OFF | N/A |
| USE_REGEX | Enable regular expression matching options | ON | Regex Library (may be unavailable on non-POSIX systems) |

### Less Common CMake and CTest Options

| Option | Description | Default | 
|--------|-------------|---------|
| CMAKE_INSTALL_PREFIX | Installation path | /usr/local |
| ENABLE_DOCS | Enable generation of doxygen-based documentation | OFF |
| BUILD_WGRIB | Build original wgrib code for GRIB1. This code is no longer supported - use at your own risk. | OFF |
| FTP_TEST_FILES | Fetch and test with files on FTP site. | OFF |
| FTP_LARGE_TEST_FILES | Fetch and test with very large files on FTP site. | OFF |
| FTP_EXTRA_TEST_FILES | Test with more large files fetched via FTP. | OFF |
| USE_TIGGE | Enable use of TIGGE gribtables | ON |
| USE_UDF | Enable User-Defined functions | OFF |
| USE_WMO_VALIDATION | Use WMO Validation Tables (unofficial templates) | OFF |
| DISABLE_ALARM | Disable alarm for non-compatible systems | OFF |
| DISABLE_STAT | Disable POSIX feature | OFF |
| BUILD_EXTRA | Build user-contributed code. This code is no longer supported - use at your own risk. | OFF |
| USE_MYSQL | Use with BUILD_EXTRA = ON to enable user-contributed MySQL options. It is user's responsibility to ensure MySQL is available. | OFF |
 
## Documentation for Previous Versions of wgrib2

* [Legacy Web Docs](https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/)