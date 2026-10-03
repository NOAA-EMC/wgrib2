# This is the CMake build file for the wgrib2 project, extra 
# subdirectory.
#
# It optionally builds all user-contributed (non-operational) 
# codes if BUILD_EXTRA=ON.

set(src Mysql.c Mysql_dump.c Mysql_speed.c)

include_directories(../src/)

add_library(wgrib2_extra OBJECT ${src})
target_compile_definitions(wgrib2_extra PUBLIC ${definitions_list}) 

# Optional MySQL support: detect and define USE_MYSQL if available
# This is intentionally NOT REQUIRED; build proceeds without MySQL.
find_package(MySQL QUIET)

if (MYSQL_FOUND)
  message(STATUS "MySQL found: enabling USE_MYSQL for wgrib2_extra")
  target_compile_definitions(wgrib2_extra PUBLIC USE_MYSQL)

  if (DEFINED MYSQL_INCLUDE_DIRS)
    target_include_directories(wgrib2_extra PUBLIC ${MYSQL_INCLUDE_DIRS})
  elseif(DEFINED MYSQL_INCLUDE_DIR)
    target_include_directories(wgrib2_extra PUBLIC ${MYSQL_INCLUDE_DIR})
  endif()

  if (DEFINED MYSQL_LIBRARIES)
    target_link_libraries(wgrib2_extra PUBLIC ${MYSQL_LIBRARIES})
  elseif(DEFINED MYSQL_LIBRARY)
    target_link_libraries(wgrib2_extra PUBLIC ${MYSQL_LIBRARY})
  endif()
else()
  message(STATUS "MySQL not found: building wgrib2_extra without MySQL support")
endif()

if(USE_OPENMP)
  target_link_libraries(wgrib2_extra PUBLIC OpenMP::OpenMP_C)
endif()

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
