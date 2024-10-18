# This is the CMake file for the ftn_api directory in the wgrib2
# project. This will only build if MAKE_FTN_API=ON and 
# BUILD_LIB=ON
#
# Kyle Gerheiser

set(fortran_src
  wgrib2api.f90
  wgrib2lowapi.f90
  )

set(c_src
  fort_wgrib2.c
  )

if (BUILD_SHARED_LIB)
   add_library(wgrib2_ftn_api SHARED ${fortran_src} ${c_src})
else()
   add_library(wgrib2_ftn_api STATIC ${fortran_src} ${c_src})
   set_property(TARGET wgrib2_ftn_api PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

set(module_dir "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
set_target_properties(wgrib2_ftn_api PROPERTIES Fortran_MODULE_DIRECTORY ${module_dir})

target_include_directories(wgrib2_ftn_api
    PUBLIC $<BUILD_INTERFACE:${module_dir}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(wgrib2_ftn_api PUBLIC wgrib2_lib)  

install(DIRECTORY ${module_dir} DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
  TARGETS wgrib2_ftn_api
  EXPORT wgrib2_exports
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

