
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
target_link_libraries(obj_lib PRIVATE gctpc)

# with -DCALLABLE_WGRIB2 for the lib
add_library(wgrib2_lib $<TARGET_OBJECTS:obj_lib> ${callable_src})
set_target_properties(wgrib2_lib PROPERTIES OUTPUT_NAME wgrib2)
target_compile_definitions(wgrib2_lib PRIVATE CALLABLE_WGRIB2)
target_link_libraries(wgrib2_lib PUBLIC gctpc)

# without -DCALLABLE_WGRIB2 for the executable
add_executable(wgrib2_exe $<TARGET_OBJECTS:obj_lib> ${callable_src})
set_target_properties(wgrib2_exe PROPERTIES OUTPUT_NAME wgrib2)
target_link_libraries(wgrib2_exe PRIVATE gctpc)

if(USE_NETCDF4)
  target_link_libraries(wgrib2_lib PUBLIC NetCDF::NetCDF_C)
  target_link_libraries(wgrib2_exe PUBLIC NetCDF::NetCDF_C)
endif()

if(USE_JASPER)
  target_include_directories(obj_lib PUBLIC ${JASPER_INCLUDE_DIR})
  target_link_libraries(wgrib2_exe PRIVATE ${JASPER_LIBRARIES})
  target_link_libraries(wgrib2_lib PUBLIC ${JASPER_LIBRARIES})
endif()

if(USE_PNG)
  target_link_libraries(obj_lib PUBLIC PNG::PNG)
  target_link_libraries(wgrib2_exe PRIVATE PNG::PNG)
  target_link_libraries(wgrib2_lib PUBLIC PNG::PNG)
endif()

if(OpenMP_FOUND)
  target_link_libraries(wgrib2_exe PRIVATE OpenMP::OpenMP_C)
endif()

if(USE_SPECTRAL)
endif()

if(USE_G2CLIB)
endif()

if(USE_IPOLATES EQUAL 3)
  target_link_libraries(obj_lib PUBLIC ip2::ip2_d)
  target_link_libraries(wgrib2_exe PUBLIC ip2::ip2_d)
  target_link_libraries(wgrib2_lib PUBLIC ip2::ip2_d)
endif()


install(
  TARGETS wgrib2_lib wgrib2_exe
  EXPORT wgrib2_exports
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)





