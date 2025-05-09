# This is the CMake build file for the wgrib2 project, extra 
# subdirectory.
#
# It optionally builds all user-contributed (non-operational) 
# codes if BUILD_EXTRA=ON.

set(src Mysql.c Mysql_dump.c Mysql_speed.c Csv.c Csv_long.c
PyInv.c
)

include_directories(../wgrib2/)

add_library(wgrib2_extra OBJECT ${src})
target_compile_definitions(wgrib2_extra PUBLIC ${definitions_list}) 

if (BUILD_LIB)
  set_property(TARGET wgrib2_extra PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(wgrib2_extra PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)
  
#install(FILES "fnlist_extra.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  TARGETS wgrib2_extra
  EXPORT wgrib2_exports
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})