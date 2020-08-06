# This module looks for environment variables detailing where G2TMPL lib is
# If variables are not set, G2TMPL will be built from external source
if(DEFINED ENV{G2TMPL_LIB} )
  set(G2TMPL_LIB $ENV{G2TMPL_LIB} CACHE STRING "G2TMPL Library Location" )
  set(G2TMPL_INC $ENV{G2TMPL_INC} CACHE STRING "G2TMPL Include Location" )

  set(name "g2tmpl")
  string(TOUPPER ${name} uppercase_name)

  string(REGEX MATCH "(v[0-9]+\\.[0-9]+\\.[0-9]+)" _ ${${uppercase_name}_LIB})
  set(version ${CMAKE_MATCH_1})

  set(lib_name ${name})
  set(versioned_lib_name ${name}_${version})

  if(EXISTS ${${uppercase_name}_LIB} )
    get_filename_component(lib_dir ${${uppercase_name}_LIB} DIRECTORY)
    find_library(g2tmpl_path NAMES ${versioned_lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)

    add_library(${lib_name} STATIC IMPORTED)
    set_target_properties(${lib_name} PROPERTIES
      IMPORTED_LOCATION ${g2tmpl_path}
      INTERFACE_INCLUDE_DIRECTORIES ${${uppercase_name}_INC})
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(g2tmpl
  REQUIRED_VARS g2tmpl_path)
