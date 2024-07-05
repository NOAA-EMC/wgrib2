# This is the CMake file for the wgrib utility of the
# NCEPLIBS-grib_util project.
#
# Mark Potts, Kyle Gerheiser

add_compile_definitions("DEF_T62_NCEP_TABLE=opn" "FAST_GRIBTAB" "P_TABLE_FIRST")

set(c_src wgrib.c)

set(exe_name wgrib)
add_executable(${exe_name} ${c_src})

install(TARGETS ${exe_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# If doxygen documentation is enabled, build it.
#if(ENABLE_DOCS)
#  add_subdirectory(docs)  
#endif()
