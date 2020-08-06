# This module looks for environment variables detailing where GFSIO lib is
# If variables are not set, GFSIO will be built from external source
if(DEFINED ENV{GFSIO_LIB4} )
  set(GFSIO_LIB4 $ENV{GFSIO_LIB4} CACHE STRING "GFSIO_4 Library Location" )
  set(GFSIO_INC4 $ENV{GFSIO_INC4} CACHE STRING "GFSIO_4 Include Location" )

  set(name "gfsio")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB4})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(gfsio_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
    
      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${gfsio_path_${kind}} 
        INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gfsio
  REQUIRED_VARS gfsio_path_4)
