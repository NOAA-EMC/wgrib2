
# sets lib_src
include("source-files.cmake")

set(callable_src
    wgrib2.c
    fatal_error.c
    wgrib2_api.c
)   

add_subdirectory(gctpc)
# make this an object lib so we can re-use most of object files
# The only files that differ are ${callable_src} which are compiled
# with -DCALLABLE_WGRIB2
add_library(obj_lib OBJECT ${lib_src})
target_compile_definitions(obj_lib PUBLIC ${definitions_list}) 

# with -DCALLABLE_WGRIB2 for the lib
if(BUILD_SHARED_LIB)
  add_library(wgrib2_lib SHARED $<TARGET_OBJECTS:obj_lib> $<TARGET_OBJECTS:gctpc> ${callable_src})
else()
  add_library(wgrib2_lib STATIC $<TARGET_OBJECTS:obj_lib> $<TARGET_OBJECTS:gctpc> ${callable_src})
endif()

# library and executable have same name (wgrib2) but different target names
set_target_properties(wgrib2_lib PROPERTIES OUTPUT_NAME wgrib2)

target_compile_definitions(wgrib2_lib PRIVATE CALLABLE_WGRIB2)

# without -DCALLABLE_WGRIB2 for the executable
add_executable(wgrib2_exe $<TARGET_OBJECTS:obj_lib> ${callable_src})
set_target_properties(wgrib2_exe PROPERTIES OUTPUT_NAME wgrib2)

if(USE_NETCDF4)
  target_link_libraries(obj_lib PUBLIC NetCDF::NetCDF_C)
  target_link_libraries(wgrib2_exe PRIVATE NetCDF::NetCDF_C)
  target_link_libraries(wgrib2_lib PUBLIC NetCDF::NetCDF_C)
endif()

if(USE_JASPER)
  target_include_directories(obj_lib PUBLIC ${JASPER_INCLUDE_DIR})
  target_link_libraries(obj_lib PUBLIC ${JASPER_LIBRARIES})
  target_link_libraries(wgrib2_exe PRIVATE ${JASPER_LIBRARIES})
  target_link_libraries(wgrib2_lib PUBLIC ${JASPER_LIBRARIES})
endif()

if(USE_PNG)
  target_link_libraries(obj_lib PUBLIC PNG::PNG)
  target_link_libraries(wgrib2_exe PRIVATE PNG::PNG)
  target_link_libraries(wgrib2_lib PUBLIC PNG::PNG)
endif()

if(OpenMP_FOUND)
  target_link_libraries(obj_lib PUBLIC OpenMP::OpenMP_C)
  target_link_libraries(wgrib2_exe PRIVATE OpenMP::OpenMP_C)
  target_link_libraries(wgrib2_lib PUBLIC OpenMP::OpenMP_C)
endif()

if(USE_SPECTRAL)
endif()

if(USE_G2CLIB)
endif()

if(USE_IPOLATES EQUAL 3)
  target_link_libraries(obj_lib PUBLIC ip2::ip2_d)
  target_link_libraries(wgrib2_exe PUBLIC ip2::ip2_d)
  target_link_libraries(wgrib2_lib PUBLIC ip2::ip2_d)

  # Link to the Fortran runtime library for each compiler if using ip2.
  # The wgrib2 exectuable is created using the C compiler and
  # doesn't link the necessary Fortran library required for ip2.
  if(CMAKE_Fortran_COMPILER_ID MATCHES "^(Intel)$")
    target_link_libraries(wgrib2_exe PRIVATE "-lifcore")
  elseif(CMAKE_Fortran_COMPILER_ID MATCHES "^(GNU)$")
    target_link_libraries(wgrib2_exe PRIVATE "-lgfortran")
  endif()
  
endif()

target_link_libraries(obj_lib PUBLIC gctpc)
target_link_libraries(wgrib2_exe PRIVATE gctpc -lm)
target_link_libraries(wgrib2_lib PUBLIC -lm)

set(headers wgrib2_api.h wgrib2.h)

set_target_properties(wgrib2_lib PROPERTIES PUBLIC_HEADER "${headers}")

install(
  TARGETS wgrib2_lib wgrib2_exe
  EXPORT wgrib2_exports
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)





