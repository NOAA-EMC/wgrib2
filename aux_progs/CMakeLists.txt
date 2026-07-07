
add_executable(gmerge gmerge.c rd_msg.c uint8.c aux_progs.h)

add_executable(smallest_grib2 smallest_grib2.c uint8.c aux_progs.h)

add_executable(smallest_4 smallest_4.c uint8.c aux_progs.h)

install(
  TARGETS gmerge smallest_grib2 smallest_4
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
