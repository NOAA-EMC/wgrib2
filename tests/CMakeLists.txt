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
  set(FTP_FILES WW3_Regional_US_West_Coast_20220718_0000.grib2)

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
copy_test_data(ref_WW3_Regional_US_West_Coast_20220718_0000.grib2.inv)
copy_test_data(gdas.t12z.pgrb2.1p00.anl.75r)
copy_test_data(ref_wind.grb)
copy_test_data(ref_uv.grb)

# Run these shell tests.
shell_test(run_wgrib2_tests)

if (FTP_TEST_FILES)
  shell_test(run_wgrib2_ftp_tests)
endif()
