macro (setGaea)

  message("Setting flags and paths for Cray")
  option(FIND_HDF5 "Try to Find HDF5 libraries" OFF)
  option(FIND_HDF5_HL "Try to Find HDF5 libraries" ON)
  set(HDF5_USE_STATIC_LIBRARIES "ON" CACHE INTERNAL "HDF5_Static" )

  set(HOST_FLAG "-xCORE-AVX2" CACHE INTERNAL "Host Flag") # for Haswell (C4)
  set(MKL_FLAG "-mkl" CACHE INTERNAL "MKL Flag" )
  set(GSI_Intel_Platform_FLAGS "-DPOUND_FOR_STRINGIFY -fp-model strict -assume byterecl -convert big_endian -implicitnone -D_REAL8_ -traceback ${HOST_FLAG} ${MKL_FLAG} ${OpenMP_Fortran_FLAGS} ${MPI_Fortran_COMPILE_FLAGS} -O3" CACHE INTERNAL "")
  set(ENKF_Platform_FLAGS "-O3 -fp-model strict -convert big_endian -assume byterecl -implicitnone  -DGFS -D_REAL8_ -traceback ${HOST_FLAG} ${MKL_FLAG} ${MPI3FLAG} ${OpenMP_Fortran_FLAGS} " CACHE INTERNAL "")
  set(GSI_LDFLAGS "${MKL_FLAG} ${OpenMP_Fortran_FLAGS}" CACHE INTERNAL "")
  set(BUILD_CORELIBS "OFF" )
endmacro()
