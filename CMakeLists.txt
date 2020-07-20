cmake_minimum_required(VERSION 3.12)
project(wgrib2 VERSION 2.9.0 LANGUAGES Fortran C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/Modules")

#set default install path if not provided
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
      "${CMAKE_BINARY_DIR}/install"
      CACHE PATH "default install path" FORCE)
endif()

option(USE_NETCDF3 "Use NetCDF-3?" off)
option(USE_NETCDF4 "Use NetCDF-4?" on)
option(USE_REGEX "Use Regex?" on)
option(USE_TIGGE "Use tigge?" on)
option(USE_MYSQL "Use MySQL?" off)
set(USE_IPOLATES "3" CACHE STRING "")
option(USE_SPECTRAL "Use SP lib?" on)
option(USE_UDF "Use UDF?" off)
option(USE_OPENMP "Use OpenMP?" off)
option(USE_PROJ4 "Use Proj4?" off)
option(USE_WMO_VALIDATION "Use WMO Validation?" off)
option(DISABLE_TIMEZONE "Disable Timezone?" off)
option(MAKE_FTN_API "Make Fortran API?" on)
option(DISABLE_ALARM "Disable Alarm?" off)

option(USE_G2CLIB "Use g2c lib?" off)
option(USE_PNG "Use PNG?" on)
option(USE_JASPER "Use Jasper?" on)
option(USE_AEC "Use AEC?" off)


if(USE_G2CLIB) 
    if(not USE_PNG) 
        message(FATAL_ERROR "If USE_G2CLIB is on, USE_PNG must be off")
    endif()

    if(not USE_JASPER) 
        message(FATAL_ERROR "If USE_G2CLIB is on, USE_JASPER must be off")
    endif()
endif()


if(USE_SPECTRAL)
    if(NOT USE_IPOLATES EQUAL 3)
        message(FATAL_ERROR "If USE_SPECTRAL is on, USE_IPOLATES must be 3")
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
  message(FATAL_ERROR "USE_IPOLATES=1 not supported")
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
endif()

if(USE_JASPER)
  list(APPEND definitions_list -DUSE_JASPER)
  find_package(Jasper REQUIRED)
endif()

if(USE_OPENMP)
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

add_subdirectory(wgrib2)
add_subdirectory(aux_progs)

if(MAKE_FTN_API)
  add_subdirectory(ftn_api)
endif()



### Package config
include(CMakePackageConfigHelpers)
set(CONFIG_INSTALL_DESTINATION lib/cmake/${PROJECT_NAME})

export(EXPORT wgrib2_exports
  NAMESPACE wgrib2}::
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
