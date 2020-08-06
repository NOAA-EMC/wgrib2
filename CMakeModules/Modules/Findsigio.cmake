# This module looks for environment variables detailing where SIGIO lib is
# If variables are not set, SIGIO will be built from external source
if(DEFINED ENV{SIGIO_LIB4} )
  set(SIGIO_LIB4 $ENV{SIGIO_LIB4} CACHE STRING "SIGIO_4 Library Location" )
  set(SIGIO_INC4 $ENV{SIGIO_INC4} CACHE STRING "SIGIO_4 Include Location" )

  set(name "sigio")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB4})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(sigio_path NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)

      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${sigio_path}
        INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sigio
  REQUIRED_VARS sigio_path)
