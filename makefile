# REQUIRES GMAKE!!!!
#
# wgrib2 uses components of varying copyrights and licences.  See wgrib2/LICENSE-wgrib2
#
# makefile for wgrib2
# 
# compiles every #@?! library needed by wgrib2
# then tries to compile wgrib2
#
# (1) must use gnu-make
# (2) the environment veriable CC must be set to the C compiler
# (3) the environment variable FC must be set to fortran-90 compiler or 
#        higher in order to compile the optional netcdf, the optional IPOLATES
#        and fortran API. Not needed if netcdf and IPOLATES and fortran API are not used
#
#
# mod 1/07 M. Schwarb (libgrib2c name change)
# mod 4/09 W. Ebisuzaki (use config.h)
# mod 6/10 W. Ebisuzaki ipolates
# mod 8/11 W. Ebisuzaki support environment variable FC={fortran 90+ compiler}
#              needed by optional netcdf4 and ipolates
# mod 3/12 W. Ebisuzaki support openmp, gctpc
# mod 8/12 M. Schwarb  gunzip -n -f,  cd "$var"
# mod 10/12 W. Ebisuzaki
# mod 7/13 W. Ebisuzaki added got netcdf4 working again, added subdirectroy lib, bin, include, man
# mod 11/14 W. Ebisuzaki added target lib, make callable wgrib2
# mod 05/16 G. Schnee add support for template 5.42 compression using libaec
# mod 10/16 W. Ebisuzaki add support for COMP_SYS
# mod 10/17 W. Ebisuzaki modifications for vlab 
# mod 07/20 W. Ebisuzaki modifications for make -j N (alpha)
# mod 10/20 W. Ebisuzaki make portable between gmake 3 and 4 (having "echo ..#..")
# mod 11/22 M. Fontana add ability to use externam netcdf4/hdf5 libaries (USE_NETCDF4/USE_HDF5)
# mod 12/22 W. Ebisuzaki prelim oneapi_linux support
# mod 4/23  W. Ebisuzaki, remove fast-math, Proj4 failed verification
#
#   Configuration
#
# OPENMP:  OpenMP v3.1 compiler support is required for using OpenMP.
#
# NETCDF3:
#    USE_NETCDF3=0   do not use netcdf3 library
#    USE_NETCDF3=1   compile netcdf3 library
#    cannot use NETCDF3 and NETCDF4 libraries at the same time
#
# NETCDF4: 
#    USE_NETCDF4=0         do not use netcdf4 library
#    USE_NETCDF4=compile   compile netcdf4 library from source code
#    USE_NETCDF4=1         legacy, same as compile
#    USE_NETCDF4=system    use system netcdf library, system netcdf4 includes
#                           and libraries must found at normally searched locations
#    USE_NETCDF4=INC:LIB   INC=directory of the netcdf4 includes
#                          LIB=directory of the netcdf4 libraries
#
#    USE_HDF5              HDF5 is needed for netcdf4, only valid if USE_NETCDF4 != 0
#                          USE_HDF5 is ignored if USE_NETCDF4=0
#    USE_HDF5=0            not use hdf5 library, note netcdf4 requires hdf5
#    USE_HDF5=compile	   compile hdf5 library
#    USE_HDF5=system       use system hdf5 library, system hdf5 includes and
#			   libraries must be found at normally searched locations 
#    USE_HDF5=INC:LIB      INC=directory of the hdf5 includes
#			   LIB=directory of the hdf5 libraries
#
# to compile netcdf4 and hdf5
#    need fortran90+ to compiler (FC)
#
# IPOLATES: link in IPOLATES library to interpolate to new grids
#    USE_IPOLATES=0    no ipolates library, no -new_grid option
#    USE_IPOLATES=1    use the grib1 version of ipolates (supports NCEP rotated lat-lon)
#    USE_IPOLATES=3    use the double precision grib2 ipolates (default, supports WMO rotated lat-lon)
#    if USE_IPOLATES=3, the fortran compiler needs to have options to convert reals 
#    to double precision and must not convert double precision to quad precision 
#     gfortran, g95, clang and intel have it. 
#
# SPECTRAL: USE_SPECTRAL=1 adds spectral interpolation to -new_grid
#    using NCEP spectral library. Needs USE_IPOLATES=3
#
# MAKE_FTN_API: makes the fortran wgrib2api which allows fortran code to read/write grib2
#    needs fortran2003 or fortran95 with extensions
#
#  MYSQL: link in interface to MySQL to write to mysql database
#    change: USE_MYSQL=1 in configuration below
#    need to have mysql installed
#    may need to modify makefile
#  
#  UDF: add commands for user-defined functions and shell commands
#    change: USE_UDF=1 in configuration below
#
#  REGEX: use regular expression library (POSIX-2), on by default
#    change: USE_REGEX=0 if REGEX library is not available
#     (preferred: get gnu source code to REGEX library)
#
#  TIGGE: ability for TIGGE-like variable names, on by default
#    change: USE_TIGGE=0 to turn off (configuration below)
#
#  USE_PROJ4: the proj4 library for geolocation, needed for
#    non-spherical Lambert Azimuthal Equal Area grid (10/2022),
#    used for testing the gctpc library.
#
#  USE_JASPER: use Jasper library for jpeg2000 compression support
#        compiler support is lacking
#  USE_OPENJPEG: use OpenJPEG library for jpeg2000 compression support
#    OpenJPEG is supported by more C compilers, It requires cmake.
#  cannot use JASPER and OPENJPEG at the same time
# 
#  USE_AEC: enable use of the libaec library for packing with GRIB2 template
#    5.42 (https://gitlab.dkrz.de/k202009/libaec/), requires cmake
#
#  USE_G2CLIB: include NCEP's g2clib (mainly for testing purposes)
#              USE_G2CLIB = 1, g2clib can be used for decoding by -g2clib 1
#                  requires USE_PNG=1 and USE_JASPER=1
#              I use this option to defuse complaints about wgrib2 from Operations.
#                Operations:  Wgrib2 is not working with this file!
#                Me:  I get the same results with your g2clib library.
#
#  DISABLE_ALARM: some machines do not support alarm(..)
#     alarm() is in POSIX-1 and IEEE Std 1003.1
#     -alarm N, terminates wgrib2 after N seconds which is good for web servers
# 
#  DISABLE_STAT: stat() and S_ISREG(), S_ISFIFO(), S_ISCHR(), S_ISDIR() are POSIX-1
#     if not available, define DISABLE_STAT=1, slight performance reduction
#
#  MAKE_SHARED_LIB: "make lib" will make both shared and static wgrib2 libraries
#      The shared library is for use by pywgrib2 (Python).  In theory, the shared 
#      library used by programs link in the wgrib2 library. However, there is
#      no versioning of the library, most users cannot put the library in the
#      default LD_LIBRARY_PATH. So it is not recommended.
#      Support for this option depends on compiler.
#
#  USE_NAMES:  select table for names of WMO defined variables, can be NCEP, ECMWF, DWD (alpha)
#
# on NCEP AIX circa 2012
# export CC=/usr/vacpp/bin/xlc_r
# export CPP=/usr/bin/cpp
# export FC=xlf_r
#
# for AOCC (AMD Optimizing C Compiler, version of clang)
# export CC=clang
# export FC=flang
# export COMP_SYS=clang_linux
#
# for classic intel on linux
#   icc does not compile jasper, needs gcc
# export CC=icc
# export FC=ifort
# export COMP_SYS=intel_linux
#
# for oneAPI intel on linux
#   icx does not compile jasper, needs gcc
# export CC=icx
# export FC=ifx
# export COMP_SYS=oneapi_linux
#
# Main development platform: linux with gcc/gfortran
#
# export CC=gcc
# export FC=gfortran
#
SHELL=/bin/sh
# SHELL=/bin/ksh

# 
# the flags are stored in wgrib2/config.h
#

# if USE_NETCDF3=1, then USE_NETCDF4 must 0
#
# USE_NETCDF3=0
USE_NETCDF3=0
#
USE_NETCDF4=1
# USE_NETCDF4=compile
# USE_NETCDF4=system
#USE_NETCDF4=${NETCDF_INCLUDES}:${NETCDF_LIBRARIES}
# USE_HDF5 is only active if USE_NETCDF4 != 0
#USE_HDF5=0
USE_HDF5=compile
#USE_HDF5=system
#USE_HDF5=${HDF5_INCLUDES}:${HDF5_LIBRARIES}
USE_REGEX=1
USE_TIGGE=1
USE_MYSQL=0
USE_IPOLATES=3
USE_SPECTRAL=1
USE_UDF=0
USE_OPENMP=1
USE_PROJ4=1
USE_WMO_VALIDATION=0
USE_NAMES=NCEP
MAKE_FTN_API=1
DISABLE_ALARM=0
DISABLE_STAT=0
MAKE_SHARED_LIB=1

USE_G2CLIB=0
USE_PNG=1
USE_JASPER=1
USE_OPENJPEG=0
USE_AEC=0

# Add any customization comments, appears in help and config pages
BUILD_COMMENTS=stock build

# test if cmake is available
ifeq (, $(shell which cmake 2>/dev/null))
  HAS_CMAKE:=0
else
  HAS_CMAKE:=1
endif

# often environment variable FC=fortran compiler, is set to f77, needs f95+ compiler

cwd:=${CURDIR}
lib:=${cwd}/lib
tmp:=${cwd}/tmp
export TMPDIR=${tmp}
wLDFLAGS:=-L${lib}
a:=$(shell mkdir -p ${lib})
a:=$(shell mkdir -p ${tmp})
wCPPFLAGS:=${CPPFLAGS} -I${cwd}/include
# netcdf4 should be compiled without OpenMP
netcdf3CPPFLAGS:=${CPPFLAGS} -I${cwd}/include
netcdf4CPPFLAGS:=${CPPFLAGS} -I${cwd}/include
hdf5CFLAGS:=${CPPFLAGS}
wFFLAGS:=${FFLAGS}
FTN_REAL8:=
PIC_option:=
USE_ZLIB:=0

a:=$(shell mkdir -p ${lib})
a:=$(shell mkdir -p ${cwd}/include)

CONFIG_H=${cwd}/wgrib2/config.h
a:=$(shell echo "/* config.h */" > ${CONFIG_H})
a:=$(shell echo '! config.h' > ip2lib_d/config.h)
CPPFLAGS=
FFLAGS=

# check if flag are ok

ifndef USE_NETCDF4
  USE_NETCDF4:=0
endif
ifeq ($(USE_NETCDF4),1)
  USE_NETCDF4:=compile
endif
ifndef USE_HDF5
  USE_HDF5=compile
endif

ifeq ($(USE_G2CLIB),1)
  ifeq ($(USE_PNG),0)
    $(error ERROR, USE_G2CLIB = 1 requires USE_PNG = 0)
  endif
  ifeq ($(USE_JASPER),0)
    $(error ERROR, USE_G2CLIB = 1 requires USE_JASPER = 0)
  endif
endif
ifeq ($(USE_SPECTRAL),1)
  ifneq ($(USE_IPOLATES),3)
     $(error ERROR, USE_SPECTRAL = 1 requires USE_IPOLATES = 3)
  endif
endif
ifeq ($(USE_NETCDF3),1)
  ifneq ($(USE_NETCDF4),0)
     $(error ERROR, compile with NETCDF3 or NETCDF4, not both: USE_NETCDF4=${USE_NETCDF4})
  endif
endif
ifeq ($(USE_JASPER),1)
  ifeq ($(USE_OPENJPEG),1)
    $(error ERROR, USE_JASPER = 1 or USE_OPENJPEG = 1 for jpeg2000 support, not both)
  endif
endif

ifeq ($(HAS_CMAKE),0)
  ifeq ($(USE_AEC),1)
    $(error ERROR, Need cmake for AEC support, install cmake or set USE_AEC=0 in grib2/makefile)
  endif
  ifeq ($(USE_OPENJPEG),1)
    $(error ERROR, Need cmake for OpenJPEG support, install cmake or set USE_OPENJPEG=0 in grib2/makefile)
  endif
endif

H:=\#
ifeq ($(USE_REGEX),1)
   a:=$(shell echo '$Hdefine USE_REGEX' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_REGEX' >> ${CONFIG_H})
endif

ifeq ($(USE_TIGGE),1)
   a:=$(shell echo '$Hdefine USE_TIGGE' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_TIGGE' >> ${CONFIG_H})
endif

ifeq ($(DISABLE_ALARM),1)
   a:=$(shell echo '$Hdefine DISABLE_ALARM' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine DISABLE_ALARM' >> ${CONFIG_H})
endif

ifeq ($(DISABLE_STAT),1)
   a:=$(shell echo '$Hdefine DISABLE_STAT' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine DISABLE_STAT' >> ${CONFIG_H})
endif

ifeq (${USE_NAMES},DWD)
   a:=$(shell echo '$Hdefine USE_NAMES DWD1' >> ${CONFIG_H})
else
   a:=$(shell echo '$Hdefine USE_NAMES ${USE_NAMES}' >> ${CONFIG_H})
endif

need_ftn=0
ifeq ($(MAKE_FTN_API),1)
   need_ftn=1
endif
ifneq ($(USE_IPOLATES),0)
   need_ftn=1
endif
ifeq ($(need_ftn),1)
   ifeq ($(findstring f77,$(notdir $(FC))),f77)
#      $(error need fortran-95+TR-15581/fortran-2003 compiler, not ${FC})
      $(error FC=?   compile directions: https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/compile_questions.html)
   endif
endif


ifeq ($(USE_UDF),1)
   a:=$(shell echo '$Hdefine USE_UDF' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_UDF' >> ${CONFIG_H})
endif

ifeq ($(USE_IPOLATES),1)
#   for HWRF iplib:=${lib}/libipolate_hwrf.a
#   for HWRF wLDFLAGS+=-lipolate_hwrf
   ipdir:=${cwd}/iplib.v3.0.0
   iplib:=${lib}/libipolate.a
   wLDFLAGS+=-lipolate
else ifeq ($(USE_IPOLATES),3)
   ipdir:=${cwd}/ip2lib_d
   iplib=${lib}/libip2_d.a
   wLDFLAGS+=-lip2_d
   a:=$(shell echo '$Hdefine USE_SPECTRAL '"${USE_SPECTRAL}" > ${ipdir}/config.h)
endif

ifneq ($(USE_IPOLATES),0)
a:=$(shell echo '$Hdefine IPOLATES_LIB '\"`basename ${ipdir}`\" >> ${CONFIG_H})
endif
a:=$(shell echo '$Hdefine USE_IPOLATES ${USE_IPOLATES}' >> ${CONFIG_H})

# C compile and load commmands
# wCPPFLAGS has the directory of the includes 
# wLDFLAGS has the directory/name of the library

ifeq ($(findstring opencc,$(notdir $(CC))),opencc)
   wCPPFLAGS+=-O3 -Wall -opencc
   netcdf3CPPFLAGS+=-O3 -Wall -opencc
   netcdf4CPPFLAGS+=-O3 -Wall -opencc
   hdf5CFLAGS+=-O1 -Wall -opencc
endif
ifeq ($(findstring xlc_r,$(notdir $(CC))),xlc_r)
   wCPPFLAGS+=-O3
   netcdf3CPPFLAGS+=-O3
   netcdf4CPPFLAGS+=-O3
   hdf5CFLAGS+=-O2
endif

# new method of configure the options


# if $COMP_SYS is undefined, identify system

ifndef COMP_SYS
   system=$(shell uname -s)
   ifeq (${system},Linux)
      ifeq ($(findstring gcc,$(notdir $(CC))),gcc)
         COMP_SYS=gnu_linux
      endif
      ifeq ($(findstring g95,$(notdir $(FC))),g95)
         COMP_SYS=gnu_linux_g95
      endif
      ifeq ($(findstring clang,$(notdir $(CC))),clang)
         COMP_SYS=clang_linux
      endif
      ifeq ($(findstring icc,$(notdir $(CC))),icc)
         COMP_SYS=intel_linux
      endif
      ifeq ($(findstring icx,$(notdir $(CC))),icx)
         COMP_SYS=oneapi_linux
      endif
      ifeq ($(findstring nvc,$(notdir $(CC))),nvc)
         COMP_SYS=nvidia_linux
      endif
   endif
   ifeq ($(findstring CYGWIN,$(system)),CYGWIN)
      ifeq ($(findstring gcc,$(notdir $(CC))),gcc)
         COMP_SYS=cygwin_win
      endif
   endif
   ifeq ($(findstring BSD,$(system)),BSD)
      ifeq ($(findstring gcc,$(notdir $(CC))),gcc)
         COMP_SYS=gnu_linux
      endif
   endif
   ifeq (${system},Darwin)
      ifeq ($(findstring gcc,$(notdir $(CC))),gcc)
         COMP_SYS=gnu_mac
      endif
   endif
endif

CCjasper:=${CC}

ifeq (${COMP_SYS},gnu_linux)
   wCPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
#  would like to use following line but jasper 1.900.1.14 will not compile :(
#  wCPPFLAGS+=-Werror=implicit-function-declaration -Werror=implicit-int
   netcdf3CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   netcdf4CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   hdf5CFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -O1
   ifeq ($(need_ftn),1)
      wLDFLAGS+=-lgfortran
      wCPPFLAGS+=-DGFORTRAN
      wFFLAGS+=-c -O3
      FTN_REAL8:="-fdefault-real-8 -fdefault-double-8"
      FTN_LEGACY:="-std=legacy -Wno-argument-mismatch"
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-fopenmp
      wFFLAGS+=-fopenmp
   endif
   ifeq ($(MAKE_SHARED_LIB),1)
      a:=$(shell echo '$Hdefine MAKE_SHARED_LIB' >> ${CONFIG_H})
      PIC_option:=-fPIC
      wCPPFLAGS+=-fPIC -g
      wFFLAGS+=-fPIC -g
      netcdf3CPPFLAGS+=-fPIC -g
      netcdf4CPPFLAGS+=-fPIC -g
      hdf5CFLAGS+=-fPIC -g
      MAKE_SHARED_LIB:=gnu_linux
   endif
endif

ifeq (${COMP_SYS},cygwin_win)
   wCPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
#  would like to use following line but jasper 1.900.1.14 will not compile :(
#  #  wCPPFLAGS+=-Werror=implicit-function-declaration -Werror=implicit-int
   netcdf3CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   netcdf4CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   hdf5CFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -O1
   ifeq ($(need_ftn),1)
      wLDFLAGS+=-lgfortran
      wCPPFLAGS+=-DGFORTRAN
      wFFLAGS+=-c -O3
      FTN_REAL8:="-fdefault-real-8 -fdefault-double-8"
      FTN_LEGACY:="-std=legacy -Wno-argument-mismatch"
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-fopenmp
      wFFLAGS+=-fopenmp
   endif
   ifeq ($(MAKE_SHARED_LIB),1)
      ifeq ($(USE_JASPER),1)
         $(error incompatible options: MAKE_SHARED_LIB=1, and USE_JASPER=1 with cygwin)
      endif
      ifeq ($(USE_OPENJEPG),1)
         $(error incompatible options: MAKE_SHARED_LIB=1, and USE_OPENJPEG=1 with cygwin)
      endif
      ifeq ($(USE_PNG),1)
         $(error incompatible options: MAKE_SHARED_LIB=1, and USE_PNG=1 on cygwin)
      endif
      a:=$(shell echo '$Hdefine MAKE_SHARED_LIB' >> ${CONFIG_H})
      PIC_option:=-fPIC
      wCPPFLAGS+=-fPIC -g
      wFFLAGS+=-fPIC -g
      netcdf3CPPFLAGS+=-fPIC -g
      netcdf4CPPFLAGS+=-fPIC -g
      hdf5CFLAGS+=-fPIC -g
      MAKE_SHARED_LIB:=cygwin_win
   endif
endif

ifeq (${COMP_SYS},gnu_mac)
   wCPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
#  would like to use following line but jasper 1.900.1.14 will not compile :(
#  #  wCPPFLAGS+=-Werror=implicit-function-declaration -Werror=implicit-int
   netcdf3CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   netcdf4CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   hdf5CFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -O1
   ifeq ($(need_ftn),1)
      wLDFLAGS+=-lgfortran
      wCPPFLAGS+=-DGFORTRAN
      wFFLAGS+=-c -O3
      FTN_REAL8:="-fdefault-real-8 -fdefault-double-8"
      FTN_LEGACY:="-std=legacy -Wno-argument-mismatch"
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-fopenmp
      wFFLAGS+=-fopenmp
   endif
   ifeq ($(MAKE_SHARED_LIB),1)
      a:=$(shell echo '$Hdefine MAKE_SHARED_LIB' >> ${CONFIG_H})
      PIC_option:=-fPIC
      wCPPFLAGS+=-fPIC -g
      wFFLAGS+=-fPIC -g
      netcdf3CPPFLAGS+=-fPIC -g
      netcdf4CPPFLAGS+=-fPIC -g
      hdf5CFLAGS+=-fPIC -g
      MAKE_SHARED_LIB:=gnu_mac
   endif
endif

ifeq (${COMP_SYS},gnu_linux_g95)
   wCPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3 -ftree-vectorizer-verbose=3
#  would like to use following line but jasper 1.900.1.14 will not compile :(
#  #  wCPPFLAGS+=-Werror=implicit-function-declaration -Werror=implicit-int
   netcdf3CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   netcdf4CPPFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -Werror=format-security -O3
   hdf5CFLAGS+=-Wall -Wmissing-prototypes -Wold-style-definition -O1
   ifeq ($(need_ftn),1)
      libf95:=$(shell $(FC) -print-file-name=libf95.a)
      ifeq "$(libf95)" ""
          $(error ERROR, g95 missing? $(FC))
      endif
      wLDFLAGS+=-L$(dir ${libf95}) -lf95
      wCPPFLAGS+=-DG95
      wFFLAGS+=-c -O2
      FTN_REAL8:="-r8"
   endif
endif

# for clang/flang from AOCC
ifeq (${COMP_SYS},clang_linux)
   wCPPFLAGS+=-O3 -fvectorize
#   wCPPFLAGS+=-march=znver3
   netcdf3CPPFLAGS+=-O3
   netcdf4CPPFLAGS+=-O3
   hdf5CFLAGS+=-O2
   ifeq ($(need_ftn),1)
      wCPPFLAGS+=-DFLANG
      wLDFLAGS+=-lflang -lflangrti -lpgmath
      wFFLAGS+=-c -O3
#      wFFLAGS+=-march=znver3
      FTN_REAL8:="-r8"
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-fopenmp
      wFFLAGS+=-mp
   endif
   ifeq ($(MAKE_SHARED_LIB),1)
      a:=$(shell echo '$Hdefine MAKE_SHARED_LIB' >> ${CONFIG_H})
      PIC_option:=-fPIC
      wCPPFLAGS+=-fPIC -g
      wFFLAGS+=-fPIC -g
      netcdf3CPPFLAGS+=-fPIC -g
      netcdf4CPPFLAGS+=-fPIC -g
      hdf5CFLAGS+=-fPIC -g
      MAKE_SHARED_LIB:=clang_linux
   endif
endif

# intel's oneAPI icx/ifx compilers
ifeq (${COMP_SYS},oneapi_linux)
   wCPPFLAGS+=-O3 -fvectorize -fp-model=strict
   netcdf3CPPFLAGS+=-O3 -fp-model=strict
   netcdf4CPPFLAGS+=-O3 -fp-model=strict
   hdf5CFLAGS+=-O2 -fp-model=strict
   ifeq ($(USE_JASPER),1)
      CCjasper:=gcc
      a:=$(shell echo '$Hdefine CC_jasper '\"${CCjasper}\" >> ${CONFIG_H})
   endif
   ifeq ($(need_ftn),1)
      wCPPFLAGS+=-DIFORT
      wLDFLAGS+=-lifcore
      wFFLAGS+=-c -O3 -heap-arrays 32  -fp-model=strict
      FTN_REAL8:="-r8"
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-qopenmp
      wFFLAGS+=-qopenmp
   endif
   ifeq ($(MAKE_SHARED_LIB),1)
      a:=$(shell echo '$Hdefine MAKE_SHARED_LIB' >> ${CONFIG_H})
      PIC_option:=-fPIC
      wCPPFLAGS+=-fPIC -g
      wFFLAGS+=-fPIC -g
      netcdf3CPPFLAGS+=-fPIC -g
      netcdf4CPPFLAGS+=-fPIC -g
      hdf5CFLAGS+=-fPIC -g
      MAKE_SHARED_LIB:=clang_linux
   endif
endif

# intel icc/ifort compilers, now the classic compilers
ifeq (${COMP_SYS},intel_linux)
   wCPPFLAGS+=-O2 -fp-model=precise
   netcdf3CPPFLAGS+=-O2 -fp-model=precise
   netcdf4CPPFLAGS+=-O2  -fp-model=precise
   hdf5CFLAGS+=-O2 -fp-model=precise
   ifeq ($(USE_JASPER),1)
      CCjasper:=gcc
      a:=$(shell echo '$Hdefine CC_jasper '\"${CCjasper}\" >> ${CONFIG_H})
   endif
   ifeq ($(need_ftn),1)
      wCPPFLAGS+=-DIFORT -cxxlib
      wLDFLAGS+=-lifcore -lc -limf -lintlc
      wFFLAGS+=-c -O2 -nofor-main  -cxxlib -heap-arrays 32 -fp-model=precise
      FTN_REAL8:="-r8"
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-qopenmp
      wFFLAGS+=-qopenmp
   endif
endif

ifeq (${COMP_SYS},nvidia_linux)
   ifeq ($(USE_AEC),1)
     $(error ERROR, nvc does not make AEC correctly, USE_AEC=0 in makefile)
   endif
   wCPPFLAGS+=-O2
   netcdf3CPPFLAGS+=-O2
   netcdf4CPPFLAGS+=-O2
   hdf5CFLAGS+=-O2
   ifeq ($(need_ftn),1)
      wCPPFLAGS+=-DNVFORTRAN
      wLDFLAGS+=-fortranlibs
      wFFLAGS+=-c -O2
      FTN_REAL8:="-r8"
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-fopenmp
      wFFLAGS+=-fopenmp
   endif
   ifeq ($(MAKE_SHARED_LIB),1)
      a:=$(shell echo '$Hdefine MAKE_SHARED_LIB' >> ${CONFIG_H})
      PIC_option:=-fPIC
      wCPPFLAGS+=-fPIC -g
      wFFLAGS+=-fPIC -g
      netcdf3CPPFLAGS+=-fPIC -g
      netcdf4CPPFLAGS+=-fPIC -g
      hdf5CFLAGS+=-fPIC -g
      MAKE_SHARED_LIB:=nvidia_linux
   endif
endif

ifeq (${COMP_SYS},solaris_studio)
   wCPPFLAGS+=???
   netcdf3CPPFLAGS+=???
   netcdf4CPPFLAGS+=???
   hdf5CFLAGS+=???
   ifeq ($(need_ftn),1)
      wLDFLAGS+=???
      wCPPFLAGS+=-DSOLARIS
      wFFLAGS+=???
      FTN_REAL8:=???
   endif
   ifeq ($(USE_OPENMP),1)
      a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
      wCPPFLAGS+=-xopenmp
      wFFLAGS+=-xopenmp
   endif
endif


# this is for old style setup .. change to new style when have access to machine
ifeq ($(need_ftn),1)

   ifndef FC
     $(error ERROR, configuration requires fortran90 compiler which is set by environement variable FC)
   endif

# for open64 fortran - personal system
   ifeq ($(findstring openf95,$(notdir $(FC))),openf95)
      wLDFLAGS+=/export/cpc-lw-webisuzak/wd51we/opt/x86_open64-4.5.1/lib/gcc-lib/x86_64-open64-linux/4.5.1/libfortran.a
      wLDFLAGS+=/export/cpc-lw-webisuzak/wd51we/opt/x86_open64-4.5.1/lib/gcc-lib/x86_64-open64-linux/4.5.1/libffio.a
      wCPPFLAGS+=-DOPENF95
      wFFLAGS+=-O2
      FTN_REAL8:="-r8"
   endif

# NCEP CCS:
   ifeq ($(findstring xlf_r,$(notdir $(FC))),xlf_r)
      wLDFLAGS+=-L/usr/lib - -lxlf90_r
      wCPPFLAGS+=-DXLF
      wFFLAGS+=-O2
      FTN_REAL8:="-qrealsize=8"
   endif

   ifeq ($(wFFLAGS),"")
      $(error ERROR, C and fortran pair compiler is not recognized)
   endif
endif

# write out compilation flags
a:=$(shell echo '$Hdefine CPPFLAGS '\"${wCPPFLAGS}\" >> ${CONFIG_H})
a:=$(shell echo '$Hdefine FFLAGS '\"${wFFLAGS}\" >> ${CONFIG_H})

# grib2c library
# g2clib is required if USE_G2CLIB, USE_PNG or USE_JASPER
# USE_G2CLIB and USE_JASPER implies USE_PNG
#   

g2cdir:=${cwd}/g2clib-1.4.0
ifeq ($(USE_G2CLIB),1)
   g2clib:=${lib}/libgrib2c.a
   wLDFLAGS+=-lgrib2c
   wCPPFLAGS+=-I${g2cdir}
   a:=$(shell echo '$Hdefine USE_G2CLIB' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_G2CLIB' >> ${CONFIG_H})
endif

# gctpc library
gctpcdir:=${cwd}/wgrib2/gctpc
#gctpcsrc:=gctpc20a.tgz
gctpclib:=${lib}/libgeo.a
wLDFLAGS+=-lgeo
# wCPPFLAGS+=-I${gctpc}/source

# proj4 library
proj4dir:=${cwd}/proj-4.8.0
ifeq ($(USE_PROJ4),1)
   proj4src:=${cwd}/proj-4.8.0.tar.gz
   proj4lib:=${lib}/libproj.a
   wLDFLAGS+=-lproj
#   wCPPFLAGS+=-I${proj4dir}/src
   a:=$(shell echo '$Hdefine USE_PROJ4' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_PROJ4' >> ${CONFIG_H})
endif

# Jasper

jasperdir=${cwd}/jasper-1.900.1
ifeq ($(USE_JASPER),1)
   jsrc=jasper-1.900.1-14ubuntu3.2.debian.tgz
   jlib=${lib}/libjasper.a
   wLDFLAGS+=-ljasper
   wCPPFLAGS+=-I${jasperdir}/src/libjasper/include
   a:=$(shell echo '$Hdefine USE_JASPER' >> ${CONFIG_H})
endif

# OpenJPEG
openjpegdir=${cwd}/openjpeg-2.5.0
ifeq ($(USE_OPENJPEG),1)
   ojsrc:=openjpeg-2.5.0.tar.gz
   a:=$(shell echo "$Hdefine USE_OPENJPEG \"${ojsrc}\"" >> ${CONFIG_H})
   ojlib:=${lib}/libopenjp2.a
   wLDFLAGS+=-lopenjp2
endif

# AEC

aecdir=${cwd}/libaec-master
ifeq ($(USE_AEC),1)
	aecsrc:=libaec-1.0.6.tar.gz
	aeclib:=${lib}/libaec.a
   wLDFLAGS+=-laec
   a:=$(shell echo '$Hdefine USE_AEC '\"${aecsrc}\" >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_AEC' >> ${CONFIG_H})
endif

# SPECTRAL (optional used by ip2_d)
spectraldir:=sp_v2.0.2_d
ifeq ($(USE_SPECTRAL),1)
   spectrallib:=${lib}/libsp_v2.0.2_d.a
   wLDFLAGS+=-lsp_v2.0.2_d
   a:=$(shell echo '$Hdefine USE_SPECTRAL 1' >> ${CONFIG_H})
endif

netcdf3dir:=${cwd}/netcdf-3.6.3
ifeq ($(USE_NETCDF3),1)
   netcdf3src=netcdf-3.6.3.tar.gz
   netcdf3lib:=${lib}/libnetcdf.a
   wLDFLAGS+=-lnetcdf
#   wCPPFLAGS+=-I$n/libsrc
   a:=$(shell echo '$Hdefine USE_NETCDF3' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_NETCDF3' >> ${CONFIG_H})
endif

ifneq ($(USE_NETCDF4),0)
   ifeq ($(USE_NETCDF4),compile)
      netcdf4dir:=${cwd}/netcdf-c-4.9.0
      netcdf4src=netcdf-c-4.9.0.tar.gz
      netcdf4lib:=${lib}/libnetcdf.a
      a:=$(shell echo "$Hdefine USE_NETCDF4 \"${netcdf4src}\"" >> ${CONFIG_H})
   else ifeq ($(USE_NETCDF4),system)
      a:=$(shell echo "$Hdefine USE_NETCDF4 \"system\"" >> ${CONFIG_H})
   else ifeq ($(findstring :,$(USE_NETCDF4)),:)
      a:=$(subst :, ,$(USE_NETCDF4))
      wCPPFLAGS+=-I$(word 1,$a)
      wLDFLAGS+=-L$(word 2,$a)
      a:=$(shell echo "$Hdefine USE_NETCDF4 \"ext_lib=${USE_NETCDF4}\"" >> ${CONFIG_H})
   else
      $(error ERROR, bad USE_NETCDF4 in makefile")
   endif
   ifeq ($(USE_HDF5),compile)
      hdf5dir=${cwd}/hdf5-1.12.2
      hdf5src=${cwd}/hdf5-1.12.2.tar.gz
      hdf5lib:=${lib}/libhdf5.a
      a:=$(shell echo "$Hdefine USE_HDF5 \"${hdf5src}\"" >> ${CONFIG_H})
   else ifeq ($(USE_HDF5),system)
      a:=$(shell echo "$Hdefine USE_HDF5 \"system\"" >> ${CONFIG_H})
   else ifeq ($(findstring :,$(USE_HDF5)),:)
      a:=$(subst :, ,$(USE_HDF5))
      wCPPFLAGS+=-I$(word 1,$a)
      wLDFLAGS+=-L$(word 2,$a)
      a:=$(shell echo "$Hdefine USE_HDF5 \"ext_lib=${USE_HDF5}\"" >> ${CONFIG_H})
   else
      $(error ERROR, bad USE_HDF5 in makefile")
   endif
   wLDFLAGS+=-lnetcdf -lhdf5_hl -lhdf5 -ldl
endif

ifeq ($(USE_MYSQL),1)
   wCPPFLAGS+=`mysql_config --cflags`
   wLDFLAGS+=`mysql_config --libs`
   a:=$(shell echo '$Hdefine USE_MYSQL' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_MYSQL' >> ${CONFIG_H})
endif

# OPENMP .. only select configurations

ifeq ($(USE_OPENMP),1)
   ifeq ($(findstring opencc,$(notdir $(CC))),opencc)
      ifeq ($(findstring openf95,$(notdir $(FC))),openf95)
	 a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
	 wCPPFLAGS+=-fopenmp
	 wFFLAGS+=-fopenmp
      endif
   endif
   ifeq ($(findstring xlc_r,$(notdir $(CC))),xlc_r)
      ifeq ($(findstring xlf_r,$(notdir $(FC))),xlf_r)
	 a:=$(shell echo '$Hdefine USE_OPENMP' >> ${CONFIG_H})
	 wCPPFLAGS+=-qsmp=omp
	 wFFLAGS+=-qsmp=omp
      endif
   endif
endif

# save fortran and C compiler names in config.h file

ifeq ($(findstring gcc,$(notdir $(CC))),gcc)
   a:=$(shell echo '$Hdefine CC '\"`${CC} --version | head -n 1`\" >> ${CONFIG_H})
else ifeq ($(findstring icc,$(notdir $(CC))),icc)
   a:=$(shell echo '$Hdefine CC '\"`${CC} --version | head -n 1`\" >> ${CONFIG_H})
else ifeq ($(findstring icx,$(notdir $(CC))),icx)
   a:=$(shell echo '$Hdefine CC '\"`${CC} --version | head -n 1`\" >> ${CONFIG_H})
else
   a:=$(shell echo '$Hdefine CC '\"${CC}\" >> ${CONFIG_H})
endif


ifeq ($(need_ftn),1)
  ifeq ($(findstring gfortran,$(notdir $(FC))),gfortran)
    a:=$(shell echo '$Hdefine FORTRAN '\"`${FC} --version | head -n 1`\" >> ${CONFIG_H})
  else ifeq ($(findstring ifort,$(notdir $(FC))),ifort)
    a:=$(shell echo '$Hdefine FORTRAN '\"`${FC} --version | head -n 1`\" >> ${CONFIG_H})
  else ifeq ($(findstring ifx,$(notdir $(FC))),ifx)
    a:=$(shell echo '$Hdefine FORTRAN '\"`${FC} --version | head -n 1`\" >> ${CONFIG_H})
  else ifeq ($(findstring nvfortran,$(notdir $(FC))),nvfortran)
    a:=$(shell echo '$Hdefine FORTRAN '\"`${FC} --version | head -n 2 | tail -n 1`\" >> ${CONFIG_H})
  else
    a:=$(shell echo '$Hdefine FORTRAN '\"${FC}\" >> ${CONFIG_H})
  endif
endif

a:=$(shell echo '$Hdefine BUILD_COMMENTS '"\"${BUILD_COMMENTS}\"" >> ${CONFIG_H})

ifeq ($(USE_JASPER),1)
   USE_ZLIB:=1
endif
ifeq ($(USE_OPENJPEG),1)
   USE_ZLIB:=1
endif
ifeq ($(USE_PNG),1)
   USE_ZLIB:=1
endif
ifneq ($(USE_NETCDF4),0)
   USE_ZLIB:=1
endif


# png 

pngdir=${cwd}/libpng-1.2.59
ifeq ($(USE_PNG),1)
   pngsrc=${cwd}/libpng-1.2.59.tar.gz
   pnglib=${lib}/libpng12.a
   wLDFLAGS+=-lpng12
   a:=$(shell echo '$Hdefine USE_PNG' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine USE_PNG' >> ${CONFIG_H})
endif

# zlib, needed by png, jasper, openjpeg

zdir=${cwd}/zlib-1.2.12
ifeq ($(USE_ZLIB),1)
   zsrc=${cwd}/zlib-1.2.12.tar.gz
   zlib=${lib}/libz.a
   wLDFLAGS+=-lz
endif

# WMO Validation testing mode
ifeq ($(USE_WMO_VALIDATION),1)
   a:=$(shell echo '$Hdefine WMO_VALIDATION' >> ${CONFIG_H})
else
   a:=$(shell echo '//$Hdefine WMO_VALIDATION' >> ${CONFIG_H})
endif

wLDFLAGS+=-lm
# WNE 4-2020 wCPPFLAGS+=-I/usr/include ${CPPFLAGS}
wCPPFLAGS+=${CPPFLAGS}

# -----------------------------------------------------

# check if make is GNU make else use gmake
make_is_gnu:=$(word 1,$(shell make -v))
ifeq ($(make_is_gnu),GNU)
   MAKE:=make
else
   MAKE:=gmake
endif


w=wgrib2
prog=$w/wgrib2

all:	${netcdf4src} ${hdf5src} ${prog} aux_progs/gmerge aux_progs/smallest_grib2 aux_progs/smallest_4


${prog}:        $w/*.c $w/*.h ${jlib} ${aeclib} ${netcdf3lib} ${pnglib} ${hdf5lib} ${g2clib} ${netcdf4lib} ${iplib} ${spectrallib} ${gctpclib} ${proj4lib} ${ojlib}
	cd "$w" && export LDFLAGS="${wLDFLAGS}" && export CPPFLAGS="${wCPPFLAGS}" && ${MAKE}

fast:        $w/*.c $w/*.h ${jlib} ${aeclib} ${netcdf3lib} ${pnglib} ${hdf5lib} ${g2clib} ${netcdf4lib} ${iplib} ${spectrallib} ${gctpclib} ${proj4lib} ${ojlib}
	cd "$w" && export LDFLAGS="${wLDFLAGS}" && export CPPFLAGS="${wCPPFLAGS}" && ${MAKE} fast

lib:        $w/*.c $w/*.h ${jlib} ${aeclib} ${netcdf3lib} ${pnglib} ${hdf5lib} ${g2clib} ${netcdf4lib} ${iplib} ${spectrallib} ${gctpclib} ${proj4lib} ${ojlib}
	cd "$w" && export LDFLAGS="${wLDFLAGS}" && export CPPFLAGS="${wCPPFLAGS}" && export FFLAGS="${wFFLAGS}" && ${MAKE} lib
	cp wgrib2/libwgrib2.a ${lib}/libwgrib2x.a
	touch ${lib}/wgrib2api.mod && rm ${lib}/wgrib2api.*
ifeq ($(MAKE_FTN_API),1)
#       compile ftn_api
	export CPPFLAGS="${wCPPFLAGS}" && export FFLAGS="${wFFLAGS}" && cd ftn_api && ${MAKE}
	cp ${cwd}/ftn_api/wgrib2api.mod ${lib}/
	cp ${cwd}/ftn_api/wgrib2lowapi.mod ${lib}/
	cp ${cwd}/ftn_api/libwgrib2_api.a ${lib}/
endif	
#       make libraries
	export CPPFLAGS="${wCPPFLAGS}" && export FFLAGS="${wFFLAGS}" && cd c_api && ${MAKE}
	cd lib && export CPPFLAGS="${wCPPFLAGS}" && export LDFLAGS="${wLDFLAGS}" && export MAKE_SHARED_LIB="${MAKE_SHARED_LIB}" && ${MAKE}

${jasperdir}:
	cp ${jsrc}  tmpj.tar.gz
	gunzip -n -f tmpj.tar.gz
	tar -xvf tmpj.tar
	rm tmpj.tar

${jlib}:	${jasperdir} ${zlib}
	cd ${jasperdir} && export CC=${CCjasper} && export CFLAGS=${PIC_option}  && ./configure --without-x --disable-libjpeg --disable-opengl --prefix=${cwd} && ${MAKE} -j 1 check install

${openjpegdir}:
	cp ${ojsrc} tmpoj.tar.gz
	gunzip -n -f tmpoj.tar.gz
	tar -xvf tmpoj.tar
	rm tmpoj.tar

${ojlib}:	${openjpegdir} ${zlib} ${pnglib}
	cd ${openjpegdir} && touch build && rm -r build && mkdir build
	#cd ${openjpegdir}/build && cmake .. -DZLIB_INCLUDE_DIR=${cwd}/include -DZLIB_LIBRARY=${zlib} -DPNG_LIBRARY=${pnglib} -DPNG_PNG_INCLUDE_DIR=${cwd}/include -DCMAKE_C_COMPILER=${CC} -DCMAKE_POSITION_INDEPENDENT_CODE:bool=ON -BUILD_THIRDPARTY:BOOL=ON && make
	cd ${openjpegdir}/build && cmake .. -DZLIB_INCLUDE_DIR=${cwd}/include -DZLIB_LIBRARY=${zlib} -DPNG_LIBRARY=${pnglib} -DPNG_PNG_INCLUDE_DIR=${cwd}/include -DCMAKE_C_COMPILER=${CC} -DCMAKE_POSITION_INDEPENDENT_CODE:bool=ON && make
	cp ${openjpegdir}/build/bin/libopenjp2.a ${lib}/
	cp ${openjpegdir}/src/lib/openjp2/openjpeg.h ${cwd}/include/
	cp ${openjpegdir}/src/lib/openjp2/opj_stdint.h ${cwd}/include/
	cp ${openjpegdir}/build/src/lib/openjp2/opj_config.h ${cwd}/include

${aecdir}:
	cp ${aecsrc} tmpaec.tar.gz
	gunzip -n -f tmpaec.tar.gz
	tar -xvf tmpaec.tar
	rm tmpaec.tar

${aeclib}:	${aecdir}
	cd ${aecdir} ; mkdir build ; cd build ; cmake ..  ; make
	cp ${aecdir}/build/src/libaec.a ${lib}
	cp ${aecdir}/include/libaec.h ${cwd}/include/

${pngdir}:
	cp ${pngsrc} tmpp.tar.gz
	gunzip -n -f tmpp.tar.gz
	tar -xvf tmpp.tar
	rm tmpp.tar

${pnglib}:	${zlib} ${pngdir}
#       for OSX
#	export LDFLAGS="-L${lib}" && cd "${pngdir}" && export CPPFLAGS="${wCPPFLAGS}" && ${MAKE} -f scripts/makefile.darwin
#	for everybody else
#	export LDFLAGS="-L${lib}" && cd "${pngdir}" && export CFLAGS="-DPNG_USER_WIDTH_MAX=200000000L -I${cwd}/include" ${PIC_option}" && ./configure --disable-shared --prefix=${cwd} --exec-prefix=${cwd} && ${MAKE} check install
	export LDFLAGS="-L${lib}" && cd "${pngdir}" && export CPPFLAGS="-DPNG_USER_WIDTH_MAX=200000000L -I${cwd}/include  ${PIC_option}" && ./configure --disable-shared --prefix=${cwd} --exec-prefix=${cwd} && ${MAKE} check install

${zdir}:
	cp ${zsrc} tmpz.tar.gz
	gunzip -f tmpz.tar.gz
	tar -xvf tmpz.tar
	rm tmpz.tar

${zlib}:	${zdir}
	cd ${zdir} && export CFLAGS="${wCPPFLAGS}" && ./configure --prefix=${cwd} --static && ${MAKE} install
	# WNE 3/2023 cd ${zdir} && export CFLAGS="" && ./configure --prefix=${cwd} --static && ${MAKE} install

${g2clib}:	${jlib} ${pnglib} ${zlib}
	cd "${g2cdir}" && export CPPFLAGS="${wCPPFLAGS}" && ${MAKE} && cp libgrib2c.a ${lib}

${gctpcdir}/source/makefile.gctpc:
	cp ${gctpcsrc} tmpgctpc.tar.gz
	gunzip -n -f tmpgctpc.tar.gz
	tar -xvf tmpgctpc.tar
	rm tmpgctpc.tar
	cp makefile.gctpc proj.h cproj.c sominv.c somfor.c ${gctpcdir}/source/

${gctpclib}:
	cd ${gctpcdir}/source && export CPPFLAGS="${wCPPFLAGS}" && ${MAKE} -f makefile.gctpc
	cp ${gctpcdir}/source/libgeo.a ${lib}
	cp ${gctpcdir}/source/include/proj.h ${cwd}/include/

${proj4lib}:
	cp ${proj4src}  tmpproj4.tar.gz
	gunzip -f tmpproj4.tar.gz
	tar -xvf tmpproj4.tar
	rm tmpproj4.tar
	cd ${proj4dir}  && export CFLAGS="${wCPPFLAGS}" &&  ./configure --disable-shared --prefix=${cwd} && ${MAKE} -j 1 check install

${netcdf3dir}:
	cp ${netcdf3src} tmpn.tar.gz
	gunzip -f tmpn.tar.gz
	tar -xvf tmpn.tar
	rm tmpn.tar

${netcdf3lib}:	${netcdf3dir}
	cd ${netcdf3dir} && export CPPFLAGS="${netcdf3CPPFLAGS}" && ./configure --enable-c-only --prefix=${cwd} && ${MAKE} -j 1 check install
	cp ${netcdf3dir}/libsrc/netcdf.h ${cwd}/include

${netcdf4src}:
	$(error ERROR, get netcdf4 source by "wget https://downloads.unidata.ucar.edu/netcdf-c/4.9.0/netcdf-c-4.9.0.tar.gz")

${netcdf4dir}:	${netcdf4src}
	cp ${netcdf4src} tmpn4.tar.gz
	gunzip -n -f tmpn4.tar.gz
	tar -xvf tmpn4.tar
	rm tmpn4.tar

${netcdf4lib}:	${zlib} ${netcdf4dir} ${hdf5lib}
	cd "${netcdf4dir}" && export CPPFLAGS="${netcdf4CPPFLAGS}" && export LDFLAGS="-L${lib}" && export LIBS="-lhdf5 -ldl" && ./configure --disable-dap --enable-netcdf-4 --prefix=${cwd} --disable-shared && ${MAKE} -j 1 install

${hdf5src}:
	$(error ERROR, get hdf5 source by "wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/hdf5-1.12.2.tar.gz" )

${hdf5dir}:	${hdf5src}
	cp ${hdf5src} tmph5.tar.gz
	gunzip -n -f tmph5.tar.gz
	tar -xvf tmph5.tar
	rm tmph5.tar

${hdf5lib}:	${hdf5dir} ${zlib}
#	cd "${hdf5dir}" && export CFLAGS="${hdf5CFLAGS}" && export LDFLAGS="${LDFLAGS}" && ./configure --disable-shared --with-zlib=$z --prefix=${cwd} && ${MAKE} -j 1 all check install
#	cd "${hdf5dir}" && export CFLAGS="${hdf5CFLAGS}" && export LDFLAGS="${LDFLAGS}" && ./configure --disable-shared --with-zlib=${lib} --disable-sharedlib-rpath --prefix=${cwd} --with-default-api-version=v110 && ${MAKE} -j 1 all check install
	cd "${hdf5dir}" && export CFLAGS="${hdf5CFLAGS}" && export LDFLAGS="${LDFLAGS}" && ./configure --disable-shared --with-zlib=${cwd} --disable-sharedlib-rpath --prefix=${cwd} --with-default-api-version=v110 && ${MAKE} -j 1 all check install

${iplib}:
	cd "${ipdir}" && export FFLAGS="${wFFLAGS}" && export FTN_REAL8=${FTN_REAL8}  && ${MAKE} && cp $(notdir ${iplib}) ${iplib}

${spectrallib}:
	cd "${spectraldir}" && export FFLAGS="${wFFLAGS}" && export FTN_REAL8=${FTN_REAL8} && export FTN_LEGACY=${FTN_LEGACY} && ${MAKE} && cp $(notdir ${spectrallib}) ${spectrallib}

aux_progs/gmerge:	aux_progs/gmerge.c		
	cd ${cwd}/aux_progs && ${MAKE} -f gmerge.make

aux_progs/smallest_grib2:	aux_progs/smallest_grib2.c
	cd ${cwd}/aux_progs && ${MAKE} -f smallest_grib2.make

aux_progs/smallest_4:	aux_progs/smallest_4.c
	cd ${cwd}/aux_progs && ${MAKE} -f smallest_4.make

clean:
	cd ${lib} && ${MAKE} clean || true
	cd ${cwd}/ftn_api && ${MAKE} clean || true
	mkdir -p ${tmp} && rm -r ${tmp}
	mkdir -p ${cwd}/bin && rm -r ${cwd}/bin 
	mkdir -p ${cwd}/include && rm -r ${cwd}/include
	mkdir -p ${cwd}/man && rm -r ${cwd}/man
	mkdir -p ${cwd}/share  && rm -r ${cwd}/share
	mkdir -p ${cwd}/tmp && rm -r ${cwd}/tmp
	cd $w && ${MAKE} clean || true
	[ -f ${gctpcdir}/source/makefile.gctpc ] && ( cd ${gctpcdir}/source ; ${MAKE} -f makefile.gctpc clean ) || true
	[ -f ${zdir}/Makefile ] && ( cd ${zdir} ; ${MAKE} clean ) || true
	[ "${pngdir}" != "" -a -f ${pngdir}/Makefile ] && ( cd ${pngdir} ; ${MAKE} clean ) || true
	[ "${jasperdir}" != "" -a -f ${jasperdir}/Makefile ] && ( cd ${jasperdir} ; ${MAKE} clean ) || true
	[ "${openjpegdir}" != "" -a -f ${openjpegdir}/build/Makefile ] && ( cd ${openjpegdir}/build ; ${MAKE} clean ) || true
	[ -d ${aecdir}/cmake ] && rm -r ${aecdir} || true
	[ "${g2dir}" != "" -a -f ${g2cdir}/Makefile ] && ( cd ${g2cdir} ; ${MAKE} clean )  || true
	[ "${ipdir}" != "" -a -f ${ipdir}/Makefile ] && ( cd ${ipdir} ; ${MAKE} clean ) || true
	[ "${proj4dir}" != "" -a -f ${proj4dir}/Makefile ] && ( cd ${proj4dir} ; ${MAKE} clean ) || true
	[ "${netcdf3dir}" != "" -a -f ${netcdf3dir}/Makefile ] && ( cd ${netcdf3dir} ; ${MAKE} clean ) || true
	[ "${netcdf4dir}" != "" -a -f ${netcdf4dir}/Makefile ] && ( cd ${netcdf4dir} ; ${MAKE} clean ) || true
	[ "${hdf5dir}" != "" -a -f ${hdf5dir}/Makefile ] && ( cd ${hdf5dir} ; ${MAKE} clean ) || true
	[ "${spectraldir}" != "" -a -f ${spectraldir}/Makefile ] && ( cd ${spectraldir} ; ${MAKE} clean ) || true
	cd ${cwd}/aux_progs && ${MAKE} clean -f gmerge.make
	cd ${cwd}/aux_progs && ${MAKE} clean -f smallest_grib2.make
	cd ${cwd}/aux_progs && ${MAKE} clean -f smallest_4.make

deep-clean:
	[ -f ${zdir}/Makefile ] && rm -r ${zdir} || true
	[ -f ${pngdir}/Makefile ] && rm -r ${pngdir} || true
	[ -f ${jasperdir}/Makefile ] && rm -r ${jasperdir} || true
	[ -f ${openjpegdir}/build/Makefile ] && rm -r ${openjpegdir} || true
	[ -d ${aecdir}/cmake ] && rm -r ${aecdir} || true
	[ -f ${proj4dir}/Makefile ] && rm -r ${proj4dir} || true
	[ -f ${netcdf3dir}/Makefile ] && rm -r ${netcdf3dir} || true
	[ -f ${netcdf4dir}/Makefile ] && rm -r ${netcdf4dir} || true
	[ -f ${hdf5dir}/Makefile ] && rm -r ${hdf5dir} || true
