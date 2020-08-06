# This module looks for environment variables detailing where W3EMC lib is
# If variables are not set, W3EMC will be built from external source

if(DEFINED ENV{W3EMC_LIBd} )
  set(W3EMC_VER $ENV{W3EMC_VER} CACHE STRING "W3EMC Version")
  set(W3EMC_LIB4 $ENV{W3EMC_LIB4} CACHE STRING "W3EMC_4 Library Location")
  set(W3EMC_LIB8 $ENV{W3EMC_LIB8} CACHE STRING "W3EMC_8 Library Location")
  set(W3EMC_LIBd $ENV{W3EMC_LIBd} CACHE STRING "W3EMC_d Library Location")
  set(W3EMC_INC4 $ENV{W3EMC_INC4} CACHE STRING "W3EMC_4 Include Location")
  set(W3EMC_INC8 $ENV{W3EMC_INC8} CACHE STRING "W3EMC_8 Include Location")
  set(W3EMC_INCd $ENV{W3EMC_INCd} CACHE STRING "W3EMC_d Include Location")

  set(name "w3emc")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIBd})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4" "d" "8")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if((EXISTS ${${uppercase_name}_LIB${kind}}) AND (EXISTS ${${uppercase_name}_INC${kind}}))
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(w3emc_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
    
      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${w3emc_path_${kind}}
        INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(w3emc
  REQUIRED_VARS w3emc_path_d)
