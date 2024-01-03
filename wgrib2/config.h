/* config.h */
#define USE_REGEX
#define USE_TIGGE
//#define DISABLE_ALARM
//#define DISABLE_STAT
#define USE_NAMES NCEP
//#define USE_UDF
#define IPOLATES_LIB "ip2lib_d"
#define USE_IPOLATES 3
#define USE_OPENMP
#define MAKE_SHARED_LIB
#define CPPFLAGS "-march=native -mtune=native -I/home/ebis/grib2/wgrib2/include -Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3 -DGFORTRAN -fopenmp -fPIC -g"
#define FFLAGS "-march=native -mtune=native -c -O3 -fopenmp -fPIC -g"
//#define USE_G2CLIB
#define USE_PROJ4
#define USE_JASPER
//#define USE_AEC
#define USE_SPECTRAL 1
#define USE_NETCDF3
//#define USE_MYSQL
#define CC "gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0"
#define FORTRAN "GNU Fortran (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0"
#define BUILD_COMMENTS "stock build"
#define USE_PNG
//#define WMO_VALIDATION
