# This is the CMake file for the test directory in the wgrib2
# project. 
#
# Ed Hartnett, Alyson Stahl 3/27/24

# Run each shell test.
function(shell_test name)
  # Copy the test scripts.
  file(COPY "${CMAKE_SOURCE_DIR}/tests/${name}.sh"
    DESTINATION ${CMAKE_BINARY_DIR}/tests
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
  # Add the shell script as a test.
  add_test(NAME ${name}.sh COMMAND bash ${name}.sh)
endfunction()

# Some very small test files may be committed to the repo. This
# function copies such a data file to the build directory.
function(copy_test_data name)
  message(STATUS "Copying test file ${name}")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/${name}"
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endfunction()


# Some test files are large and are kept on the NOAA EMC FTP
# site. This function is used to download such test data. It takes two
# arguments, the URL and the file to be downloaded.
function(PULL_DATA THE_URL THE_FILE)
  # If the TEST_FILE_DIR was specified, look for our test data files
  # there before FTPing them. Developers can keep all test files on
  # their machines, and save the time of downloading them every time.
  if(NOT ${TEST_FILE_DIR} STREQUAL ".")
    if (EXISTS ${TEST_FILE_DIR}/${THE_FILE})
      message(STATUS "Copying file ${TEST_FILE_DIR}/${THE_FILE} to test data directory.")
      FILE(COPY ${TEST_FILE_DIR}/${THE_FILE}
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data)
    endif()
  endif()
  if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/data/${THE_FILE}")
    message(STATUS "Downloading file ${THE_URL}/${THE_FILE} to test data directory.")    
    file(DOWNLOAD
      ${THE_URL}/${THE_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/data/${THE_FILE}
      SHOW_PROGRESS
      STATUS status
      INACTIVITY_TIMEOUT 30
      )
    list(GET status 0 status_num)
    if(NOT status_num EQUAL 0 OR NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/data/${THE_FILE}")
      message(FATAL_ERROR "Could not download ${THE_FILE}")
    endif()
  endif()
endfunction()

# Does the user want to get extra test files from the FTP site, and
# run extra tests on them?
if(FTP_TEST_FILES)
  # This is the FTP site.
  set(G2_FTP_URL "https://ftp.emc.ncep.noaa.gov/static_files/public/NCEPLIBS-g2")

  # These are the test data files.
  # These are the test data files.
  set(FTP_FILES WW3_Regional_US_West_Coast_20220718_0000.grib2
    WW3_Regional_US_East_Coast_20220717_0600.grib2
    gep19.t00z.pgrb2a.0p50_bcf144 geavg.t00z.pgrb2a.0p50_mecomf144
    gec00.t00z.pgrb2a.0p50.f144 gegfs.t00z.pgrb2a.0p50.f144
    gegfs.t00z.pgrb2a.0p50_mef144 gep19.t00z.pgrb2a.0p50.f144
    gdas.t12z.pgrb2.1p00.anl.grib2 blend.t19z.core.f001.co.grib2
    aqm.t12z.max_8hr_o3.227.grib2 seaice.t00z.grb.grib2)

  # User may also ask for large test file.
  if(FTP_LARGE_TEST_FILES)
    set(FTP_FILES ${FTP_FILES} fv3lam.t00z.prslev.f000.grib2)
  endif()

  # User may also ask for extra large test files.
  if(FTP_EXTRA_TEST_FILES)
    set(FTP_FILES ${FTP_FILES} rrfs.t18z.prslev.f000.grib2)
  endif()
  message(STATUS "Getting these files from FTP: ${FTP_FILES}.")      
  
  # Get each of the test data files.
  foreach(THE_FILE IN LISTS FTP_FILES) 
    PULL_DATA(${G2_FTP_URL} ${THE_FILE})
  endforeach()

endif()

# Copy test data file into buiild directory for testing.
copy_test_data(gdaswave.t00z.wcoast.0p16.f000.grib2)
copy_test_data(ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv)
copy_test_data(ref_npts_gdaswave.t00z.wcoast.0p16.f000.grib2.txt)
copy_test_data(ref_simple_packing.grib2)
copy_test_data(gdas.t12z.pgrb2.1p00.anl.75r.grib2)
copy_test_data(ref_wind.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv)
copy_test_data(ref_uv.gdas.t12z.pgrb2.1p00.anl.75r.grib2.inv)
copy_test_data(ref_grid.gdaswave.t00z.wcoast.0p16.f000.grib2.txt)
copy_test_data(ref_celsius.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
copy_test_data(ref_fahrenheit.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
copy_test_data(ref_rh.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
copy_test_data(ref_tmp_windspeed.txt)
copy_test_data(ref_rpn.windspeed.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
copy_test_data(ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2)
copy_test_data(ref_sec0.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
copy_test_data(ref_sec_len.gdaswave.t00z.wcoast.0p16.f000.grib2.txt)
copy_test_data(ref_sec_len.simple_packing.grib2.txt)
copy_test_data(ref_simple_packing.grib2.spread.txt)
copy_test_data(ref_new_grid_gdt_32769.grib2)
copy_test_data(png_4bits.png)
copy_test_data(large_png.grb2)


# Run these shell tests.
shell_test(run_wgrib2_tests)
shell_test(run_wgrib2_rpn_tests)

if (USE_PNG)
  shell_test(run_wgrib2_png_tests)
endif()

if (USE_NETCDF)
  shell_test(run_wgrib2_netcdf4_tests)
  shell_test(run_wgrib2_netcdf3_tests)
  copy_test_data(ref_ncdump4.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
  copy_test_data(ref_tablenc.gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
  copy_test_data(nctab.table)
endif()
if (USE_IPOLATES)
  shell_test(run_ipolates_tests)
  copy_test_data(ref_new_grid_gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
  copy_test_data(ref_new_grid_earth_gdas.t12z.pgrb2.1p00.anl.75r.grib2.txt)
  copy_test_data(ref_new_grid_ncep_2.txt)
  copy_test_data(ref_new_grid_ncep_3.txt)
  copy_test_data(ref_new_grid_ncep_4.txt)
  copy_test_data(ref_new_grid_ncep_45.txt)
  copy_test_data(ref_new_grid_ncep_98.txt)
  copy_test_data(ref_new_grid_ncep_126.txt)
  copy_test_data(ref_new_grid_ncep_127.txt)
  copy_test_data(ref_new_grid_ncep_128.txt)
  copy_test_data(ref_new_grid_ncep_129.txt)
  copy_test_data(ref_new_grid_ncep_170.txt)
  copy_test_data(ref_new_grid_ncep_173.txt)
  copy_test_data(ref_new_grid_ncep_184.txt)
  copy_test_data(ref_new_grid_ncep_194.txt)
  copy_test_data(ref_new_grid_ncep_221.txt)
  copy_test_data(ref_new_grid_ncep_230.txt)
  copy_test_data(ref_new_grid_ncep_242.txt)
  copy_test_data(ref_new_grid_ncep_249.txt)
  copy_test_data(ref_new_grid_ncep_t62.txt)
  copy_test_data(ref_new_grid_ncep_t126.txt)
  copy_test_data(ref_new_grid_ncep_t170.txt)
  copy_test_data(ref_new_grid_ncep_t190.txt)
  copy_test_data(ref_new_grid_ncep_t254.txt)
  copy_test_data(ref_new_grid_ncep_t382.txt)
  copy_test_data(ref_new_grid_ncep_t574.txt)
  copy_test_data(ref_new_grid_ncep_t1148.txt)
  copy_test_data(ref_new_grid_ncep_t1534.txt)
  copy_test_data(ref_mercator_grid.txt)
  copy_test_data(ref_hrrr_grid.txt)
  copy_test_data(ref_nam12_grid.txt)
  copy_test_data(ref_new_grid_ncep_130.txt)
  copy_test_data(ref_new_grid_ncep_132.txt)
  copy_test_data(ref_new_grid_ncep_187.txt)
  copy_test_data(ref_new_grid_ncep_197.txt)
  copy_test_data(ref_new_grid_ncep_236.txt)
endif()

if (USE_AEC)
  shell_test(run_wgrib2_aec_tests)
endif()
if (USE_JASPER OR USE_OPENJPEG)
  shell_test(run_jpeg_tests)
  copy_test_data(ref_jpeg2simple.txt)
endif()

if (MAKE_FTN_API)
  shell_test(test_ftn_api)
endif()
if (BUILD_LIB)
  shell_test(test_lib)
endif()
if (BUILD_SHARED_LIB)
  shell_test(test_shared_lib)
endif()


if (FTP_TEST_FILES)
  copy_test_data(ref_WW3_Regional_US_West_Coast_20220718_0000.grib2.inv)
  copy_test_data(ref_merge_fcst.aqm.t12z.max_8hr_o3.227.grib2.txt)
  copy_test_data(ref_HGT.500_mb.ENS=low-res_ctl.2023043000_2023050600.asc)
  shell_test(run_wgrib2_ftp_tests)
  if (FTP_LARGE_TEST_FILES)
    copy_test_data(ref_fv3lam.t00z.prslev.f000.grib2.inv)
    shell_test(run_ftp_large_tests)
  endif()
  if (FTP_EXTRA_TEST_FILES)
    copy_test_data(ref_rrfs.t18z.prslev.f000.grib2.inv)
    shell_test(run_ftp_extra_large_tests)
  endif()
endif()
