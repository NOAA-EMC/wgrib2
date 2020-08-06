# This module looks for environment variables detailing where NEMSIOGFS lib is
# If variables are not set, NEMSIOGFS will be built from external source
if(DEFINED ENV{NEMSIOGFS_LIB} )
  set(NEMSIOGFS_LIB $ENV{NEMSIOGFS_LIB} CACHE STRING "NEMSIOGFS Library Location" )
  set(NEMSIOGFS_INC $ENV{NEMSIOGFS_INC} CACHE STRING "NEMSIOGFS Include Location" )

  set(name "nemsiogfs")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB})
  set(version ${CMAKE_MATCH_1})

  set(versioned_lib_name ${name}_${version})

  if(EXISTS ${${uppercase_name}_LIB${kind}} )
  get_filename_component(lib_dir ${${uppercase_name}_LIB} DIRECTORY)
    find_library(nemsiogfs_path NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
  
    add_library(${name} STATIC IMPORTED)
    set_target_properties(${name} PROPERTIES
      IMPORTED_LOCATION ${nemsiogfs_path}
      INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC})
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nemsiogfs
  REQUIRED_VARS nemsiogfs_path)
