# This is the CMake build file for the wgrib2 project, wgrib2
# subdirectory.
#
# Kyle Gerheiser, Edward Hartnett, Wesley Ebisuzaki
#
# if BUILD_SHARED_LIB
#    ftn_api    -> shared library, interface to fortran
#    wgrib2_lib -> shared  (includes gctpc)
#
# if !BUILD_SHARED_LIB
#    ftn_api    -> static library, interface to fortran
#    wgrib2_lib -> static library includeing gctpc
#
# note: in wgrib2 v3.1.3 or earlier, all the libraries
#  incuding aec, jasper, etc were included with the wgrib2 library
#  in github version, it is different.

message(STATUS "Setting wgrib2 compiler flags...")
if(CMAKE_C_COMPILER_ID MATCHES "^Intel$")
  set(CMAKE_C_FLAGS "-g -traceback -fp-model=precise ${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "-O0 -fp-model=precise ${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_C_COMPILER_ID MATCHES "^IntelLLVM$")
  set(CMAKE_C_FLAGS "-O3 -g -traceback -fp-model=strict ${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g -traceback -fp-model=strict ${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  set(CMAKE_C_FLAGS "-O3 -g -Wall -Werror=format-security ${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g -Wall -Werror=format-security ${CMAKE_C_FLAGS_DEBUG}")
endif()

if(CMAKE_Fortran_COMPILER_ID MATCHES "^(Intel|IntelLLVM)$")
  set(CMAKE_Fortran_FLAGS "-g -traceback ${CMAKE_Fortran_FLAGS}")
  set(CMAKE_Fortran_FLAGS_DEBUG "-O0 -g -traceback ${CMAKE_Fortran_FLAGS_DEBUG}")
elseif(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  set(CMAKE_Fortran_FLAGS "-O2 -g -Wall -Wno-tabs -fbacktrace ${CMAKE_Fortran_FLAGS}")
  set(CMAKE_Fortran_FLAGS_DEBUG "-O0 -ggdb -Wall -fbacktrace ${CMAKE_Fortran_FLAGS_DEBUG}")
endif()

# sets lib_src
set(lib_src AAIG.c AAIGlong.c addtime.c aec_pk.c Aerosol.c Alarm.c
Alias.c Ave_test.c Ave_var.c Bbox.c bitstream.c Box_ave.c
Check_pdt_size.c Checksum.c Cluster.c cname.c codetable_4_230.c
CodeTable.c Code_Values.c Code_Values_JMA.c complex_pk.c Config.c
copy.c crc32.c Cress_lola.c Csv.c Csv_long.c
Cyclic.c Data.c Dump.c Earth.c Else.c End.c
Endif.c Ensemble.c Ens_processing.c Ens_qc.c EOF.c Export_lonlat.c
ExtName.c fatal_error.c Fcst_ave.c ffopen.c Fi.c File.c Fix_CFSv2_fcst.c
Fix_ncep_2.c Fix_ncep_3.c Fix_ncep_4.c Fix_ncep.c Fix_undef.c
FlagTable.c flt2ieee.c flt2ieee_nan.c Flush.c fnlist.c For.c Ftime1.c
Ftime2.c Ftime.c Ftn_api.c gaussian2ij.c Gctpc.c gctpc_ll2xy.c GDT.c
geo.c Geolocation.c get_unixtime.c Grads.c Grib.c Grib_out_irr2.c
Grib_out_irr.c gribtab.c Gribtable_used.c Grid_def.c Grid_id.c
grid_ident.c Gridout.c Help.c Hybrid.c ieee2flt.c ieee2flt_nan.c
ieee_pk.c If_delayed_error.c If_rec.c Import.c Import_grib.c
Import_grib_fs.c Import_lonlat.c Import_netcdf.c init.c int8.c
intpower.c Inv.c Inv_no.c Irr_grids.c itoshort_a.c JMA.c jpeg_pk.c
Last.c lat2ij.c Latlon.c Level.c Limit.c Lola.c Lvl.c Macro.c
manage_inv_out.c Match.c Match_fs.c Match_inv.c Mem_buffer.c Merge.c
Misc.c missing.c mk_gdt.c Model_version_date.c Mod_grib.c
Names.c ncep_grids.c NCEP_norm.c
NCEP_uv.c Ncpu.c Ndate.c Ndates.c Netcdf.c Netcdf_sup.c New_grid.c
new_grid_lambertc.c New_grid_order.c openmp_util.c parse_loop.c
parse_msg.c pdt_len.c Precision.c Prob.c Proj4.c proj4_initialize.c
proj4_transform.c PyInv.c Range.c rdieee_file.c rd_inventory.c
rd_seq_grib.c read_latlon.c Reduced_gaussian_grid.c
Reset_delayed_error.c rotll.c RPN.c save_string.c Scaling_0001.c
Scan.c Sec0.c Sec1.c Sec3.c sec3_grids.c Sec4.c Sec5.c Sec6.c
Sec_len.c Set_byte.c Set_date.c Set_ensm_derived_fcst.c Set_ens_num.c
Set_ftime1.c Set_ftime2.c Set_ftime_mode.c Set_gds.c Set_metadata.c
Set_pdt.c Set_percentile.c Set_prob.c Set_sec_size.c Set_ts_dates.c
setup_user_gribtable.c Set_val.c simple_pk.c Small_grib.c
space_view2ij.c space_view.c SpatialProcessing.c Spectral_bands.c
stagger.c Status.c string2time_unit.c Submsg_uv.c subtime.c Summary.c
swap_buffer.c test_sec.c Tigge.c Time_processing.c Timer.c Tosubmsg.c
Type_reftime.c UDF.c Undefine.c units.c Unix_time.c Unmerge_fcst.c
unpk_0.c unpk.c unpk_complex.c unpk_run_length.c update_sec3.c
update_sec4.c v1_v2_if.c VerfTime.c Warn_old_g2lib.c Waves.c
wgrib2_api.c wgrib2.c Wind_dir.c Wind_speed.c Wind_uv.c Write_sec.c
Wrt_grib.c wrtieee.c wxtext.c
)

include_directories(gctpc/source/include)
add_subdirectory(gctpc)

if(MAKE_FTN_API)
  add_subdirectory(ftn_api)
endif()

if (BUILD_LIB)
  add_subdirectory(c_api)
endif()

# make this an object lib so we can re-use most of object files
add_library(obj_lib OBJECT ${lib_src})
target_compile_definitions(obj_lib PUBLIC ${definitions_list}) 

# Write wgrib2_meta.h. This is installed and contains pre-processor
# defines for other packages using wgrib2.
configure_file (
  "${PROJECT_SOURCE_DIR}/src/wgrib2_meta.h.in"
  "${PROJECT_BINARY_DIR}/src/wgrib2_meta.h"
  )

add_executable(wgrib2_exe wgrib2_main.c)
set_target_properties(wgrib2_exe PROPERTIES OUTPUT_NAME wgrib2)

if(BUILD_LIB)
  if(BUILD_SHARED_LIB)
    add_library(wgrib2_lib SHARED ${lib_src})
    set_property(TARGET wgrib2_lib PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_target_properties(wgrib2_lib PROPERTIES SOVERSION 0)
  else()
    add_library(wgrib2_lib STATIC ${lib_src} $<TARGET_OBJECTS:gctpc>)
    set_property(TARGET wgrib2_lib PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  # library and executable have same name (wgrib2) but different target names
  set_target_properties(wgrib2_lib PROPERTIES OUTPUT_NAME wgrib2)
endif()

if(USE_NETCDF)
  target_link_libraries(obj_lib PUBLIC NetCDF::NetCDF_C)
  target_link_libraries(wgrib2_exe PRIVATE NetCDF::NetCDF_C)
  if(BUILD_LIB)
    target_link_libraries(wgrib2_lib PUBLIC NetCDF::NetCDF_C)
  endif()
endif()

if(USE_OPENMP)
  target_link_libraries(obj_lib PUBLIC OpenMP::OpenMP_C)
  target_link_libraries(wgrib2_exe PRIVATE OpenMP::OpenMP_C)
  if(BUILD_LIB)
    target_link_libraries(wgrib2_lib PUBLIC OpenMP::OpenMP_C)
  endif()
endif()

if(USE_IPOLATES)
  target_link_libraries(obj_lib PUBLIC ip::ip_d)
  target_link_libraries(wgrib2_exe PRIVATE ip::ip_d)
  if(BUILD_LIB)
    target_link_libraries(wgrib2_lib PUBLIC ip::ip_d)
  endif()
endif()

if(USE_G2CLIB)
  target_link_libraries(obj_lib PUBLIC g2c::g2c)
  target_link_libraries(wgrib2_exe PRIVATE g2c::g2c)
  if(BUILD_LIB)
    target_link_libraries(wgrib2_lib PUBLIC g2c::g2c)
  endif()
endif()

if (BUILD_EXTRA)
  target_link_libraries(obj_lib PUBLIC wgrib2_extra)
  target_link_libraries(wgrib2_exe PRIVATE wgrib2_extra)
  if(BUILD_LIB)
    target_link_libraries(wgrib2_lib PRIVATE wgrib2_extra)
  endif()
endif()

# Link to gctpc directly because object libraries do not link transitively
target_link_libraries(wgrib2_exe PRIVATE gctpc -lm)
target_link_libraries(wgrib2_exe PRIVATE obj_lib)

if(BUILD_LIB)
  set(headers wgrib2_api.h wgrib2.h ${CMAKE_BINARY_DIR}/src/wgrib2_meta.h)
  set_target_properties(wgrib2_lib PROPERTIES PUBLIC_HEADER "${headers}")
  target_link_libraries(wgrib2_lib PUBLIC gctpc -lm)
  target_compile_definitions(wgrib2_lib PUBLIC ${definitions_list})
  
  install(
    TARGETS wgrib2_lib
    EXPORT wgrib2_exports
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  
endif()

install(
  TARGETS wgrib2_exe obj_lib
  EXPORT wgrib2_exports
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})


