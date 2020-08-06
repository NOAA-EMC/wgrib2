# This module looks for environment variables detailing where BACIO lib is
# If variables are not set, BACIO will be built from external source
if(DEFINED ENV{BACIO_LIB4})
  set(BACIO_LIB4 $ENV{BACIO_LIB4} CACHE STRING "BACIO_4 Library Location")
  set(BACIO_LIB8 $ENV{BACIO_LIB8} CACHE STRING "BACIO_8 Library Location")
  set(BACIO_LIBd $ENV{BACIO_LIBd} CACHE STRING "BACIO_d Library Location")

  set(name "bacio")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB4})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4" "8" "d")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(bacio_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${bacio_path_${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(bacio
  REQUIRED_VARS bacio_path_4)
  
