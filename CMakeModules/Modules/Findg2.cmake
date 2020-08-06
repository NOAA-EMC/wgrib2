# This module looks for environment variables detailing where G2 lib is
# If variables are not set, G2 will be built from external source
if(DEFINED ENV{G2_LIBd})
  set(G2_LIBd $ENV{G2_LIBd} CACHE STRING "G2_d Library Location" )
  set(G2_LIB4 $ENV{G2_LIB4} CACHE STRING "G2_4 Library Location" )
  set(G2_INC4 $ENV{G2_INC4} CACHE STRING "G2_4 Include Location" )
  set(G2_INCd $ENV{G2_INCd} CACHE STRING "G2_d Include Location" )

  set(name "g2")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIBd})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4" "d")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(g2_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
    
      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${g2_path_${kind}}
        INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(g2
  REQUIRED_VARS g2_path_d)
