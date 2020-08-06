# This module looks for the wgrib2 library and Fortran modules in WGRIB2_ROOT
if(DEFINED ENV{WGRIB2_ROOT})
  set(WGRIB2_ROOT $ENV{WGRIB2_ROOT})
endif()

find_path (WGRIB2_INCLUDES
  wgrib2api.mod
  HINTS ${WGRIB2_ROOT}/include)

find_library (WGRIB2_LIBRARIES
  names libwgrib2.a
  HINTS ${WGRIB2_ROOT}/lib ${WGRIB2_ROOT}/lib64)

if(EXISTS ${WGRIB2_INCLUDES} AND EXISTS ${WGRIB2_LIBRARIES})
  message(STATUS "Found WGRIB2: include directory ${WGRIB2_INCLUDES}, library ${WGRIB2_LIBRARIES}")
else()
  message(STATUS "Unable to locate WGRIB2 library and/or Fortran modules")
endif()

mark_as_advanced (WGRIB2_INCLUDES WGRIB2_LIBRARIES)

add_library(wgrib2 UNKNOWN IMPORTED)
set_target_properties(wgrib2 PROPERTIES
  IMPORTED_LOCATION "${WGRIB2_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${WGRIB2_INCLUDES}"
  INTERFACE_LINK_LIBRARIES "${WGRIB2_LIBRARIES}")
