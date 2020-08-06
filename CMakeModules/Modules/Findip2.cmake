# This module looks for environment variables detailing where IP2 lib is
# If variables are not set, IP2 will be built from external source

if(DEFINED ENV{IP2_LIBd} )
  set(IP2_LIBd $ENV{IP2_LIBd} CACHE STRING "IP2_d Library Location" )
  set(IP2_LIB4 $ENV{IP2_LIB4} CACHE STRING "IP2_4 Library Location" )
  set(IP2_LIB8 $ENV{IP2_LIB8} CACHE STRING "IP2_8 Library Location" )
  set(IP2_INC4 $ENV{IP2_INC4} CACHE STRING "IP2_4 Include Location" )
  set(IP2_INC8 $ENV{IP2_INC8} CACHE STRING "IP2_8 Include Location" )
  set(IP2_INCd $ENV{IP2_INCd} CACHE STRING "IP2_d Include Location" )

  set(name "ip2")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIBd})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4" "d" "8")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(ip2_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
    
      add_library(${lib_name}::${lib_name}_${kind} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${ip2_path_${kind}}
        INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ip2
  REQUIRED_VARS ip2_path_d)
