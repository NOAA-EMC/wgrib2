# This module looks for environment variables detailing where LANDSFCUTIL lib is
# If variables are not set, LANDSFCUTIL will be built from external source 

if(DEFINED ENV{LANDSFCUTIL_LIB4} )
  set(LANDSFCUTIL_VER $ENV{LANDSFCUTIL_VER} CACHE STRING "LANDSFCUTIL Version")
  set(LANDSFCUTIL_LIB4 $ENV{LANDSFCUTIL_LIB4} CACHE STRING "LANDSFCUTIL_4 Library Location")
  set(LANDSFCUTIL_LIBd $ENV{LANDSFCUTIL_LIBd} CACHE STRING "LANDSFCUTIL_d Library Location")
  set(LANDSFCUTIL_INC4 $ENV{LANDSFCUTIL_INC4} CACHE STRING "LANDSFCUTIL_4 Include Location")
  set(LANDSFCUTIL_INCd $ENV{LANDSFCUTIL_INCd} CACHE STRING "LANDSFCUTIL_d Include Location")

  set(name "landsfcutil")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB4})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4" "d")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(landsfcutil_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
    
      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${landsfcutil_path_${kind}}
        INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(landsfcutil
  REQUIRED_VARS landsfcutil_path_4)
