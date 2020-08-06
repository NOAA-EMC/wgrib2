# This module looks for environment variables detailing where NEMSIO lib is
# If variables are not set, NEMSIO will be built from external source
if(DEFINED ENV{NEMSIO_LIB} )
  set(NEMSIO_LIB $ENV{NEMSIO_LIB} CACHE STRING "NEMSIO Library Location" )
  set(NEMSIO_INC $ENV{NEMSIO_INC} CACHE STRING "NEMSIO Include Location" )

  set(name "nemsio")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB})
  set(version ${CMAKE_MATCH_1})

  set(versioned_lib_name ${name}_${version})

  if(EXISTS ${${uppercase_name}_LIB} )
    get_filename_component(lib_dir ${${uppercase_name}_LIB} DIRECTORY)
    find_library(nemsio_path NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
  
    add_library(${name} STATIC IMPORTED)
    set_target_properties(${name} PROPERTIES
      IMPORTED_LOCATION ${nemsio_path}
      INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC})
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nemsio
  REQUIRED_VARS nemsio_path)
