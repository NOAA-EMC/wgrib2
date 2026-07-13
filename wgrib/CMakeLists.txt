# This is the CMake file for the wgrib utility of the
# NCEPLIBS-grib_util project.
#
# Mark Potts, Kyle Gerheiser

add_compile_definitions("DEF_T62_NCEP_TABLE=opn" "FAST_GRIBTAB" "P_TABLE_FIRST")

set(c_src BDS_NValues.c	  ectable_132.c  ectable_210.c	nceptab_130.c
BDSunpk.c	  ectable_133.c  ectable_211.c	nceptab_131.c
cnames.c	  ectable_140.c  ectable_228.c	nceptab_133.c
cptectable_254.c  ectable_150.c  ensemble.c	nceptab_140.c
dwdtable_002.c	  ectable_151.c  flt2ieee.c	nceptab_141.c
dwdtable_201.c	  ectable_160.c  gds_grid.c	nceptable_opn.c
dwdtable_202.c	  ectable_162.c  gribtable.c	nceptable_reanal.c
dwdtable_203.c	  ectable_170.c  ibm2flt.c	PDS_date.c
dwdtable_204.c	  ectable_171.c  intpower.c	PDStimes.c
dwdtable_205.c	  ectable_172.c  jra55_200.c	readgrib.c
ec_ext.c	  ectable_173.c  levels.c	seekgrib.c
ectable_128.c	  ectable_174.c  mdl_nceptab.c	wgrib_main.c
ectable_129.c	  ectable_180.c  missing.c	wrtieee.c
ectable_130.c	  ectable_190.c  nceptab_128.c
ectable_131.c	  ectable_200.c  nceptab_129.c)

set(exe_name wgrib)
add_executable(${exe_name} ${c_src})

install(TARGETS ${exe_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# If doxygen documentation is enabled, build it.
#if(ENABLE_DOCS)
#  add_subdirectory(docs)  
#endif()
