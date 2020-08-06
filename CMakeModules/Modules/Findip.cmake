# This module looks for environment variables detailing where IP lib is
# If variables are not set, IP will be built from external source

if(DEFINED ENV{IP_LIBd} )
  set(IP_LIBd $ENV{IP_LIBd} CACHE STRING "IP_d Library Location" )
  set(IP_LIB4 $ENV{IP_LIB4} CACHE STRING "IP_4 Library Location" )
  set(IP_LIB8 $ENV{IP_LIB8} CACHE STRING "IP_8 Library Location" )
  set(IP_INC4 $ENV{IP_INC4} CACHE STRING "IP_4 Include Location" )
  set(IP_INC8 $ENV{IP_INC8} CACHE STRING "IP_8 Include Location" )
  set(IP_INCd $ENV{IP_INCd} CACHE STRING "IP_d Include Location" )

  set(name "ip")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIBd})
  set(version ${CMAKE_MATCH_1})

  set(kinds "4" "d" "8")
  foreach(kind ${kinds})
    set(lib_name ${name}_${kind})
    set(versioned_lib_name ${name}_${version}_${kind})

    if(EXISTS ${${uppercase_name}_LIB${kind}} )
      get_filename_component(lib_dir ${${uppercase_name}_LIB${kind}} DIRECTORY)
      find_library(ip_path_${kind} NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
    
      add_library(${lib_name} STATIC IMPORTED)
      set_target_properties(${lib_name} PROPERTIES
        IMPORTED_LOCATION ${ip_path_${kind}}
        INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC${kind}})
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ip
  REQUIRED_VARS ip_path_d)
