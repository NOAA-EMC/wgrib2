# This module looks for environment variables detailing where BUFR lib is
# If variables are not set, BUFR will be built from external source
if(DEFINED ENV{BUFR_LIB4} )
  set(BUFR_VER $ENV{BUFR_VER} CACHE STRING "BUFR Version")
  set(BUFR_LIB4 $ENV{BUFR_LIB4} CACHE STRING "BUFR_4 Library Location")
  set(BUFR_LIB8 $ENV{BUFR_LIB8} CACHE STRING "BUFR_8 Library Location")
  set(BUFR_LIBd $ENV{BUFR_LIBd} CACHE STRING "BUFR_d Library Location")

  set(name "bufr")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB4})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4" "8" "d")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(bufr_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
    
      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${bufr_path_${kind}})
    endif()
  endforeach()
  
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(bufr
  REQUIRED_VARS bufr_path_4)
