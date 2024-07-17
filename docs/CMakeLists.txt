# This is the CMake file for building the docs directory of wgrib2.
#
# Ed Hartnett 7/17/24

IF(ENABLE_DOCS)

  # Create doxyfile.
  SET(abs_top_srcdir "${CMAKE_SOURCE_DIR}")
  SET(abs_top_builddir "${CMAKE_BINARY_DIR}")
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API Documentation with Doxygen" VERBATIM)

ENDIF(ENABLE_DOCS)

