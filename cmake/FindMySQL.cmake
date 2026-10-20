# FindMySQL.cmake
#
# Find the MySQL / MariaDB C client library with mariadb_config or
# mysql_config. Defines the imported target MySQL::MySQL.
#
# The system tools in /usr/bin are searched first so that the same client
# library as for the system "mysql" command is used, even if other
# environments (conda) are on CMAKE_PREFIX_PATH. Set MYSQL_CONFIG to use a
# different mariadb_config / mysql_config.

find_program(MYSQL_CONFIG NAMES mariadb_config mysql_config
  PATHS /usr/bin /usr/local/bin NO_DEFAULT_PATH)
find_program(MYSQL_CONFIG NAMES mariadb_config mysql_config)

if(MYSQL_CONFIG)
  execute_process(COMMAND ${MYSQL_CONFIG} --include
    OUTPUT_VARIABLE _mysql_include OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${MYSQL_CONFIG} --libs
    OUTPUT_VARIABLE _mysql_libs OUTPUT_STRIP_TRAILING_WHITESPACE)
  separate_arguments(_mysql_include UNIX_COMMAND "${_mysql_include}")
  separate_arguments(_mysql_libs UNIX_COMMAND "${_mysql_libs}")
  set(MYSQL_INCLUDE_DIRS "")
  foreach(_flag IN LISTS _mysql_include)
    if(_flag MATCHES "^-I(.+)")
      list(APPEND MYSQL_INCLUDE_DIRS "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  set(MYSQL_LIBRARIES ${_mysql_libs})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQL DEFAULT_MSG MYSQL_CONFIG MYSQL_INCLUDE_DIRS MYSQL_LIBRARIES)

if(MYSQL_FOUND AND NOT TARGET MySQL::MySQL)
  add_library(MySQL::MySQL INTERFACE IMPORTED)
  set_target_properties(MySQL::MySQL PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${MYSQL_LIBRARIES}")
endif()
