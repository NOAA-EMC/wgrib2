# note: this CMakelist.txt is for building the gctpc library
#   wgrib2 now includes the library into the source compile
#   so the wgrib2 library now include gctpc.  This makes it
#   easier for users who want to use the wgrib2 library.

set(src
  alberfor.c
  alberinv.c
  alconfor.c
  alconinv.c
  azimfor.c
  aziminv.c
  cproj.c
  eqconfor.c
  eqconinv.c
  equifor.c
  equiinv.c
  for_init.c
  gctp.c
  gnomfor.c
  gnominv.c
  goodfor.c
  goodinv.c
  gvnspfor.c
  gvnspinv.c
  hamfor.c
  haminv.c
  imolwfor.c
  imolwinv.c
  inv_init.c
  lamazfor.c
  lamazinv.c
  lamccfor.c
  lamccinv.c
  make.com
  merfor.c
  merinv.c
  millfor.c
  millinv.c
  molwfor.c
  molwinv.c
  nad1927.dat
  nad1983.dat
  nad27sp
  nad83sp
  obleqfor.c
  obleqinv.c
  omerfor.c
  omerinv.c
  orthfor.c
  orthinv.c
  paksz.c
  polyfor.c
  polyinv.c
  psfor.c
  psinv.c
  report.c
  robfor.c
  robinv.c
  sinfor.c
  sininv.c
  somfor.c
  sominv.c
  sphdz.c
  sterfor.c
  sterinv.c
  stplnfor.c
  stplninv.c
  tmfor.c
  tminv.c
  untfz.c
  utmfor.c
  utminv.c
  vandgfor.c
  vandginv.c
  wivfor.c
  wivinv.c
  wviifor.c
  wviiinv.c
  )

# if want shared library, had to make separate library
# otherwise include gctpc in libwgrib2.a

add_library(gctpc OBJECT ${src})
if (BUILD_LIB)
  set_property(TARGET gctpc PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(gctpc PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)
  
install(
  TARGETS gctpc
  EXPORT wgrib2_exports
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
