# This is the main CMake build file for the wgrib2 project.
#
# Kyle Gerheiser, Edward Hartnett, Wesley Ebisuzaki
cmake_minimum_required(VERSION 3.15)

# Read the current version number from file VERSION.
file(STRINGS "VERSION" pVersion)

# Set up project with version number from VERSION file.
project(wgrib2 VERSION ${pVersion} LANGUAGES Fortran C)

# Handle user build options.
option(USE_NETCDF3 "Use NetCDF-3?" off)
option(USE_NETCDF4 "Use NetCDF-4?" off)
option(USE_HDF5 "Use HDF5?" off)
option(USE_REGEX "Use Regex?" on)
option(USE_TIGGE "Use tigge?" on)
option(USE_MYSQL "Use MySQL?" off)
option(USE_IPOLATES "Use Ipolates" on)
option(USE_UDF "Use UDF?" off)
option(USE_OPENMP "Use OpenMP?" on)
option(USE_PROJ4 "Use Proj4?" off)
option(USE_WMO_VALIDATION "Use WMO Validation?" off)
option(DISABLE_TIMEZONE "Disable posix Timezone?" off)
option(DISABLE_ALARM "Disable Alarm?" off)
set(USE_NAMES "NCEP")
option(USE_G2CLIB "Use g2c lib?" off)
option(USE_PNG "Use PNG?" off)
option(USE_JASPER "Use Jasper?" off)
option(USE_OPENJPEG "Use OpenJPEG?" off)
option(USE_AEC "Use AEC?" off)
option(FTP_TEST_FILES "Fetch and test with files on FTP site." OFF)
option(FTP_LARGE_TEST_FILES "Fetch and test with very large files on FTP site." OFF)
option(FTP_EXTRA_TEST_FILES "Fetch even more large files from FTP and test them." OFF)
# MAKE_FTN_API should only be on when building library
option(MAKE_FTN_API "add ftn api?" off)
option(DISABLE_STAT "disable posix feature" off)
set(BUILD_COMMENTS "stock build")
option(BUILD_LIB "Build wgrib2 library?" on)
option(BUILD_SHARED_LIB "Build shared library?" off)
option(BUILD_WGRIB "Build wgrib code?" off)

# Developers can use this option to specify a local directory which
# holds the test files. They will be copied instead of fetching the
# files via FTP.
SET(TEST_FILE_DIR "." CACHE STRING "Check this directory for test files before using FTP.")
message(STATUS "Finding test data files in directory ${TEST_FILE_DIR}.")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set default install path if not provided.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "${CMAKE_BINARY_DIR}/install"
    CACHE PATH "default install path" FORCE)
endif()

include(GNUInstallDirs)

message(STATUS "Setting compiler flags...")
if(CMAKE_C_COMPILER_ID MATCHES "^(Intel)$")
  set(CMAKE_C_FLAGS "-g -traceback ${CMAKE_C_FLAGS} -DIFORT")
  set(CMAKE_C_FLAGS_DEBUG "-O0")
elseif(CMAKE_C_COMPILER_ID MATCHES "^(GNU)$")
  set(CMAKE_C_FLAGS "-g ${CMAKE_C_FLAGS} -DGFORTRAN")
  set(CMAKE_C_FLAGS_DEBUG "-O0")
endif()

if(CMAKE_Fortran_COMPILER_ID MATCHES "^(Intel)$")
  set(CMAKE_Fortran_FLAGS "-g -traceback ${CMAKE_Fortran_FLAGS}")
  set(CMAKE_Fortran_FLAGS_DEBUG "-O0")
elseif(CMAKE_Fortran_COMPILER_ID MATCHES "^(GNU)$")
  set(CMAKE_Fortran_FLAGS "-g -fbacktrace ${CMAKE_Fortran_FLAGS}")
  set(CMAKE_Fortran_FLAGS_DEBUG "-ggdb -O0")
endif()

message(STATUS "Checking if user wants to use NCEPLIBS-g2c...")
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

# If user wants to use NCEPLIBS-ip, find it and the sp library.
message(STATUS "Checking if the user want to use NCEPLIBS-ip...")
if(USE_IPOLATES)
  find_package(ip CONFIG REQUIRED)
  list(APPEND definitions_list -DIPOLATES_LIB="ipolates_lib_d")
  list(APPEND definitions_list -DUSE_IPOLATES)
endif()

message(STATUS "Checking if the user want to use NetCDF...")
if(USE_NETCDF4)
  find_package(NetCDF MODULE REQUIRED COMPONENTS C)
  list(APPEND definitions_list -DUSE_NETCDF4 -DUSE_HDF5 -DHDF5="hdf5")
endif()

message(STATUS "Checking if the user wants to use Jasper...")
if(USE_JASPER)
  list(APPEND definitions_list -DUSE_JASPER)
  find_package(Jasper REQUIRED)
  if(JASPER_VERSION_STRING VERSION_GREATER_EQUAL "1.900.25")
    list(APPEND definitions_list -DJAS_VERSION_MAJOR=2)
  endif()
endif()

message(STATUS "Checking if the user want to use OpenMP...")
if(USE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
     target_link_libraries(MyTarget PUBLIC OpenMP::OpenMP_CXX)
  endif()
#  if(OpenMP_Fortran_FOUND)
#     target_link_libraries(MyTarget PUBLIC OpenMP::OpenMP_Fortran)
#  endif()
endif()

message(STATUS "Checking of the user wants to use PNG...")
if(USE_PNG)
  find_package(ZLIB REQUIRED)
  find_package(PNG REQUIRED)
endif()

# Find required packages to use AEC
message(STATUS "Checking of the user wants to use AEC...")
if(USE_AEC)
  find_package(libaec 1.0.6 REQUIRED)
endif()

# write config.h
message(STATUS "Writing config.h...")
configure_file("${PROJECT_SOURCE_DIR}/wgrib2/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "Adding wgrib2, aux_probs subdirectories...")
add_subdirectory(wgrib2)
add_subdirectory(aux_progs)

if(BUILD_LIB)
  message(STATUS "Adding wgrib2 library build...")
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

if (BUILD_WGRIB)
  add_subdirectory(wgrib)
endif()

# Turn on unit testing.
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
