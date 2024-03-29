#
#  Find libaec
#
#  LIBAEC_FOUND        - if false, do not try to use libaec
#  LIBAEC_INCLUDE_DIRS - the libaec include directories
#  LIBAEC_LIBRARIES    - the libraries to link against to use libaec
#
#  This file is part of NCEPLIBS-g2c. Distributed under the LGPL v3.0.

find_package(PkgConfig QUIET)
pkg_check_modules(LIBAEC_PKGCONF QUIET libaec aec)
set(LIBAEC_VERSION ${LIBAEC_PKGCONF_VERSION})

find_path(LIBAEC_INCLUDE_DIRS
  NAMES libaec.h
  HINTS ${LIBAEC_PKGCONF_INCLUDEDIR} ${LIBAEC_PKGCONF_INCLUDE_DIRS}
)

find_library(LIBAEC_LIBRARIES
  NAMES libaec aec
  HINTS ${LIBAEC_PKGCONF_LIBDIR} ${LIBAEC_PKGCONF_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libaec
    FOUND_VAR LIBAEC_FOUND
    REQUIRED_VARS LIBAEC_LIBRARIES LIBAEC_INCLUDE_DIRS
    VERSION_VAR LIBAEC_VERSION
)

mark_as_advanced(LIBAEC_INCLUDE_DIRS LIBAEC_LIBRARIES)
