# This module attempts to find the CRTM library and will set it up to build if it cannot be found
#  
if(DEFINED ENV{CRTM_LIB})
  set(CRTM_VER $ENV{CRTM_VER} CACHE STRING "CRTM Version")
  set(CRTM_LIB $ENV{CRTM_LIB} CACHE STRING "CRTM Library Location")
  set(CRTM_INC $ENV{CRTM_INC} CACHE STRING "CRTM Library Location")

  set(name "crtm")
  string(TOUPPER ${name} uppercase_name)
  
  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB})
  set(version ${CMAKE_MATCH_1})

  set(versioned_lib_name ${name}_${version})

  get_filename_component(lib_dir ${${uppercase_name}_LIB} DIRECTORY)
  find_library(crtm_path NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
  
  add_library(${name} STATIC IMPORTED)
  set_target_properties(${name} PROPERTIES
    IMPORTED_LOCATION ${crtm_path}
    INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(crtm
  REQUIRED_VARS crtm_path)
