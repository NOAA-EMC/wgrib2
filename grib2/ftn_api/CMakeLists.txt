

set(fortran_src
  wgrib2api.f90
  wgrib2lowapi.f90
  )

set(c_src
  fort_wgrib2.c
  )


add_library(wgrib2_api ${fortran_src} ${c_src})

set(module_dir "${CMAKE_CURRENT_BINARY_DIR}/include")
set_target_properties(wgrib2_api PROPERTIES Fortran_MODULE_DIRECTORY ${module_dir})



target_include_directories(wgrib2_api
    PUBLIC $<BUILD_INTERFACE:${module_dir}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)


install(DIRECTORY ${module_dir} DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
  TARGETS wgrib2_api
  EXPORT wgrib2_exports
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

