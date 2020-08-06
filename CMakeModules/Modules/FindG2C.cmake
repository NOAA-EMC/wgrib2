# This module looks for environment variables detailing where G2 lib is
# If variables are not set, G2 will be built from external source
if(DEFINED ENV{G2C_LIB})
  set(G2C_LIB $ENV{G2C_LIB} CACHE STRING "G2C Library Location")
  set(G2C_VER $ENV{G2C_VER} CACHE STRING "G2C Version")
endif()
