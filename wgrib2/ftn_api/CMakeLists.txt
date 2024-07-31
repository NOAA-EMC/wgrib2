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

add_library(wgrib2_api ${fortran_src} ${c_src})

set(module_dir "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
set_target_properties(wgrib2_api PROPERTIES Fortran_MODULE_DIRECTORY ${module_dir})

target_include_directories(wgrib2_api
    PUBLIC $<BUILD_INTERFACE:${module_dir}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(wgrib2_api PUBLIC wgrib2_lib)  

install(DIRECTORY ${module_dir} DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
  TARGETS wgrib2_api
  EXPORT wgrib2_exports
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

