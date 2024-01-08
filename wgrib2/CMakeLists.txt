# This is the cmake file for the wgrib2 directory of wgrib2.
#
# Kyle Gerheiser, Edward Hartnett

# These are the source files.
set(lib_src AAIG.c AAIGlong.c Aerosol.c Alarm.c Alias.c Ave_test.c
Ave_var.c Bbox.c Box_ave.c Checksum.c CodeTable.c Code_Values.c
Config.c Cress_lola.c Csv.c Csv_long.c Cyclic.c Data.c Dump.c EOF.c
Earth.c End.c Ens_processing.c Ensemble.c ExtName.c Fcst_ave.c Fi.c
File.c Fix_CFSv2_fcst.c Fix_ncep.c Fix_ncep_2.c Fix_ncep_3.c
Fix_ncep_4.c FlagTable.c Flush.c For.c Ftime.c Ftime1.c Ftime2.c
Ftn_api.c GDT.c Gctpc.c Grads.c Grib.c Grib_out_irr.c Grib_out_irr2.c
Grid_def.c Grid_id.c Gridout.c Help.c Hybrid.c If_rec.c Import.c
Import_grib.c Import_netcdf.c Inv.c Inv_no.c Irr_grids.c JMA.c Last.c
Latlon.c Level.c Limit.c Lola.c Macro.c Match.c Match_fs.c
Mem_buffer.c Merge.c Misc.c Mod_grib.c Model_version_date.c Mysql.c
Mysql_dump.c Mysql_speed.c NCEP_norm.c NCEP_uv.c Ncpu.c Ndate.c
Ndates.c Netcdf.c Netcdf_sup.c New_grid.c Precision.c Prob.c Proj4.c
RPN.c Range.c Reduced_gaussian_grid.c Scaling_0001.c Scan.c Sec0.c
Sec1.c Sec3.c Sec4.c Sec5.c Sec6.c Sec_len.c Set_byte.c Set_date.c
Set_ens_num.c Set_ensm_derived_fcst.c Set_ftime1.c Set_ftime2.c
Set_metadata.c Set_pdt.c Set_percentile.c Set_prob.c Set_sec_size.c
Set_ts_dates.c Set_val.c Small_grib.c SpatialProcessing.c
Spectral_bands.c Status.c Submsg_uv.c Summary.c Tigge.c
Time_processing.c Tosubmsg.c UDF.c Undefine.c Unix_time.c VerfTime.c
Warn_old_g2lib.c Waves.c Wind_dir.c Wind_speed.c Wrt_grib.c addtime.c
aec_pk.c bitstream.c cname.c codetable_4_230.c complex_pk.c copy.c
crc32.c dec_png_clone.c enc_jpeg2000_clone.c ffopen.c flt2ieee.c
flt2ieee_nan.c fnlist.c gctpc_ll2xy.c geo.c gribtab.c grid_ident.c
ieee2flt.c ieee2flt_nan.c ieee_pk.c init.c int8.c intpower.c
itoshort_a.c jpeg_pk.c lat2ij.c manage_inv_out.c missing.c mk_gdt.c
mk_kgds.c ncep_grids.c new_grid_lambertc.c openmp_util.c parse_loop.c
parse_msg.c proj4_initialize.c proj4_transform.c
rd_inventory.c rd_seq_grib.c rdieee_file.c rotll.c save_string.c
sec3_grids.c setup_user_gribtable.c simple_pk.c space_view.c
space_view2ij.c stagger.c subtime.c swap_buffer.c test_sec.c units.c
unpk.c unpk_0.c unpk_complex.c unpk_run_length.c update_sec4.c
wrtieee.c wxtext.c )

# prod_def_temp_size.c 

set(callable_src wgrib2.c fatal_error.c wgrib2_api.c)

# This subdirectory no longer present!
add_subdirectory(gctpc)

# make this an object lib so we can re-use most of object files
# The only files that differ are ${callable_src} which are compiled
# with -DCALLABLE_WGRIB2
add_library(obj_lib OBJECT ${lib_src})
target_compile_definitions(obj_lib PUBLIC ${definitions_list}) 

if(BUILD_LIB)
  # with -DCALLABLE_WGRIB2 for the lib
  if(BUILD_SHARED_LIB)
    add_library(wgrib2_lib SHARED ${lib_src} $<TARGET_OBJECTS:gctpc> ${callable_src})
  else()
    add_library(wgrib2_lib STATIC ${lib_src} $<TARGET_OBJECTS:gctpc> ${callable_src})
  endif()

  # library and executable have same name (wgrib2) but different target names
  set_target_properties(wgrib2_lib PROPERTIES OUTPUT_NAME wgrib2)
  target_compile_definitions(wgrib2_lib PRIVATE CALLABLE_WGRIB2 USE_REGEX)
endif()

# without -DCALLABLE_WGRIB2 for the executable
add_executable(wgrib2_exe ${callable_src})
set_target_properties(wgrib2_exe PROPERTIES OUTPUT_NAME wgrib2)

if(USE_NETCDF4)
  target_link_libraries(obj_lib PUBLIC NetCDF::NetCDF_C)
endif()

if(USE_JASPER)
  target_include_directories(obj_lib PUBLIC ${JASPER_INCLUDE_DIR})
  target_link_libraries(obj_lib PUBLIC ${JASPER_LIBRARIES})
  message(STATUS "JASPER_INCLUDE_DIR ${JASPER_INCLUDE_DIR} JASPER_LIBRARIES ${JASPER_LIBRARIES}")
endif()

if(USE_PNG)
  target_link_libraries(obj_lib PUBLIC PNG::PNG)
endif()

if(OpenMP_C_FOUND)
  target_link_libraries(obj_lib PUBLIC OpenMP::OpenMP_C)
endif()

if(USE_G2CLIB)
endif()

if(USE_IPOLATES EQUAL 3)
  target_link_libraries(obj_lib PUBLIC ip2::ip2_d)

  # Link to the Fortran runtime library for each compiler if using ip2.
  # The wgrib2 exectuable is created using the C compiler and
  # doesn't link the necessary Fortran library required for ip2.
  if(CMAKE_Fortran_COMPILER_ID MATCHES "^(Intel)$")
    target_link_libraries(wgrib2_exe PRIVATE "-lifcore")
  elseif(CMAKE_Fortran_COMPILER_ID MATCHES "^(GNU)$")
    target_link_libraries(wgrib2_exe PRIVATE "-lgfortran")
  endif()
  
endif()

target_link_libraries(obj_lib PUBLIC gctpc -lm)

# Link to gctpc directly because oobject libraries do not link transitively
target_link_libraries(wgrib2_exe PRIVATE gctpc)
target_link_libraries(wgrib2_exe PRIVATE obj_lib)

if(BUILD_LIB)
  set(headers wgrib2_api.h wgrib2.h)
  target_link_libraries(wgrib2_lib PUBLIC gctpc)
  set_target_properties(wgrib2_lib PROPERTIES PUBLIC_HEADER "${headers}")

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





