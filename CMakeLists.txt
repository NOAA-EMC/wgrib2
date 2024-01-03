# This is the make cmake file for wgrib2.
#
# Kyle Gerheiser, Edward Hartnett
cmake_minimum_required(VERSION 3.12)
project(wgrib2 VERSION 3.2.0 LANGUAGES Fortran C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set default install path if not provided.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "${CMAKE_BINARY_DIR}/install"
    CACHE PATH "default install path" FORCE)
endif()

include(GNUInstallDirs)

# Set compiler flags.
if(CMAKE_C_COMPILER_ID MATCHES "^(Intel)$")
  set(CMAKE_C_FLAGS "-g -traceback ${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "-O0")
elseif(CMAKE_C_COMPILER_ID MATCHES "^(GNU)$")
  set(CMAKE_C_FLAGS "-g ${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "-O0")
endif()

if(CMAKE_Fortran_COMPILER_ID MATCHES "^(Intel)$")
  set(CMAKE_Fortran_FLAGS "-g -traceback ${CMAKE_Fortran_FLAGS}")
  set(CMAKE_Fortran_FLAGS_DEBUG "-O0")
elseif(CMAKE_Fortran_COMPILER_ID MATCHES "^(GNU)$")
  set(CMAKE_Fortran_FLAGS "-g -fbacktrace ${CMAKE_Fortran_FLAGS}")
  set(CMAKE_Fortran_FLAGS_DEBUG "-ggdb -O0")
endif()

# Handle build options.
option(USE_NETCDF3 "Use NetCDF-3?" off)
option(USE_NETCDF4 "Use NetCDF-4?" off)
option(USE_REGEX "Use Regex?" on)
option(USE_TIGGE "Use tigge?" on)
option(USE_MYSQL "Use MySQL?" off)
set(USE_IPOLATES "0" CACHE STRING "")
option(USE_SPECTRAL "Use SP lib?" off)
option(USE_UDF "Use UDF?" off)
option(OPENMP "Use OpenMP?" off)
option(USE_PROJ4 "Use Proj4?" off)
option(USE_WMO_VALIDATION "Use WMO Validation?" off)
option(DISABLE_TIMEZONE "Disable Timezone?" off)
option(DISABLE_ALARM "Disable Alarm?" off)

option(USE_G2CLIB "Use g2c lib?" off)
option(USE_PNG "Use PNG?" on)
option(USE_JASPER "Use Jasper?" on)
option(USE_AEC "Use AEC?" off)

option(BUILD_LIB "Build wgrib2 library?" on)
option(BUILD_SHARED_LIB "Build shared library?" off)

if(USE_G2CLIB) 
  if(USE_PNG) 
    message(FATAL_ERROR "If USE_G2CLIB is on, USE_PNG must be off")
  endif()

  if(USE_JASPER) 
    message(FATAL_ERROR "If USE_G2CLIB is on, USE_JASPER must be off")
  endif()
endif()

if(USE_NETCDF3 AND USE_NETCDF4)
  message(FATAL_ERROR "USE_NETCDF3 OR USE_NetCDF4, not both")
endif()

if(USE_REGEX) 
  list(APPEND definitions_list -DUSE_REGEX)
endif()

if(USE_TIGGE)
  list(APPEND  -DUSE_TIGGE)
endif()

if(DISABLE_ALARM)
  list(APPEND definitions_list -DDISABLE_ALARM)
endif()

if(DISABLE_TIMEZONE)
  list(APPEND definitions_list -DDISABLE_TIMEZONE)
endif()

if(USE_UDF)
  list(APPEND definitions_list -DUSE_UDF)
endif()

if(USE_IPOLATES EQUAL 1)
  find_package(ip CONFIG REQUIRED)
  find_package(sp CONFIG REQUIRED)
elseif(USE_IPOLATES EQUAL 3)
  find_package(ip2 CONFIG REQUIRED)
  list(APPEND definitions_list -DUSE_SPECTRAL=1)
endif()

if(NOT USE_IPOLATES EQUAL 0)
  list(APPEND definitions_list -DIPOLATES_LIB="ipolates_lib")
endif()

list(APPEND definitions_list -DUSE_IPOLATES=${USE_IPOLATES})

if(USE_NETCDF4)
  find_package(NetCDF MODULE REQUIRED COMPONENTS C)
  list(APPEND definitions_list -DUSE_NETCDF4 -DUSE_HDF5 -DHDF5="hdf5")
endif()

if(USE_JASPER)
  list(APPEND definitions_list -DUSE_JASPER)
  find_package(Jasper REQUIRED)
  if(JASPER_VERSION_STRING VERSION_GREATER_EQUAL "1.900.25")
    list(APPEND definitions_list -DJAS_VERSION_MAJOR=2)
  endif()
endif()

if(OPENMP)
  list(APPEND definitions_list -DUSE_OPENMP)
  find_package(OpenMP)
endif()

if(USE_G2CLIB) 
  list(APPEND definitions_list -DUSE_G2CLIB)
endif()

if(USE_PROJ4)
  list(APPEND definitions_list -DUSE_PROJ4)
endif()

if(USE_PNG)
  find_package(PNG REQUIRED)
  list(APPEND definitions_list -DUSE_PNG)
endif()

if(USE_WMO_VALIDATION)
  list(APPEND definitions_list -DUSE_WMO_VALIDATION)
endif()

# Build these subdirectories.
add_subdirectory(wgrib2)
add_subdirectory(aux_progs)

# Build the wgrib2 library if desired.
if(BUILD_LIB)
  add_subdirectory(ftn_api)

  ### Package config
  include(CMakePackageConfigHelpers)
  set(CONFIG_INSTALL_DESTINATION lib/cmake/${PROJECT_NAME})

  # No need for config file if library isn't built
  export(EXPORT wgrib2_exports
    NAMESPACE wgrib2::
    FILE wgrib2-targets.cmake)

  configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/PackageConfig.cmake.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DESTINATION})
  install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    DESTINATION ${CONFIG_INSTALL_DESTINATION})

  write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)
  install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${CONFIG_INSTALL_DESTINATION})

  install(EXPORT wgrib2_exports
    NAMESPACE wgrib2::
    FILE wgrib2-targets.cmake
    DESTINATION ${CONFIG_INSTALL_DESTINATION})
endif()

