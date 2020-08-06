# This module looks for environment variables detailing where IP lib is
# If variables are not set, IP will be built from external source

if(DEFINED ENV{POST_LIB} )
  set(POST_LIB $ENV{POST_LIB} CACHE STRING "POST Library Location" )
  set(POST_INC $ENV{POST_INC} CACHE STRING "POST Include Location" )
  set(POST_SRC $ENV{POST_SRC} CACHE STRING "POST Include Location" )
else()
  set(POST_VER 8.0.0)
  find_library( POST_LIB
    NAMES libip_v${POST_VER}.a
    HINTS
      ${NCEPLIBS_INSTALL_DIR}/lib
    )
  if(${NCEPLIBS_INSTALL_DIR})
    set(POST_INC ${NCEPLIBS_INSTALL_DIR}/include CACHE STRING "POST Include Location" )
  else()
    set(POST_INC ${CMAKE_INSTALL_PREFIX}/include CACHE STRING "POST Include Location" )
  endif()
endif()
