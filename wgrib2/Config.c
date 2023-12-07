#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "wgrib2.h"
#ifdef USE_JASPER
#include <jasper/jasper.h>
#endif
#include "grb2.h"
#include "fnlist.h"

#if defined USE_NETCDF3 || defined USE_NETCDF4
#include <netcdf.h>
#endif

/*
 * Config.c  just prints out the configuration
 *
 * 3/2009 public domain Wesley Ebisuzaki
 */

extern const char *default_vectors[];
extern int version_ftime;
extern int names;
extern const char *set_options;
/*
 * HEADER:100:config:misc:0:shows the configuration
 */
int f_config(ARG0) {

    char *filename;
    FILE *input;
    int i;

    inv_out[0] = 0;
    strcat(inv_out, "wgrib2 " WGRIB2_VERSION "\n    " BUILD_COMMENTS "\n\n");

    inv_out += strlen(inv_out);
    sprintf(inv_out,"Compiled on %s %s\n\n",__TIME__,__DATE__);

#if defined USE_NETCDF3 || defined USE_NETCDF4
    strcat(inv_out, "Netcdf package: ");
    strcat(inv_out,  nc_inq_libvers());
    strcat(inv_out, " is installed\n");
#else
    strcat(inv_out, "Netcdf package is not installed\n");
#endif
#if defined USE_NETCDF4
    strcat(inv_out, "netcdf4: " USE_NETCDF4 "\n");
    strcat(inv_out, "hdf5: " USE_HDF5 "\n");
#endif
#ifdef USE_AEC
    strcat(inv_out, USE_AEC " is installed\n" );
#endif
#ifdef USE_JASPER
    strcat(inv_out, "Jasper ");
    strcat(inv_out, jas_getversion());
    strcat(inv_out, " is installed\n");
    inv_out += strlen(inv_out);
//    sprintf(inv_out,"JAS_VERSION_MAJOR=%d\n", JAS_VERSION_MAJOR);
#endif
#ifdef USE_OPENJPEG
    strcat(inv_out,USE_OPENJPEG " is installed\n");
    inv_out += strlen(inv_out);
#endif

#ifdef USE_MYSQL
    strcat(inv_out, "mysql package is installed\n");
#else
    strcat(inv_out, "mysql package is not installed\n");
#endif


#ifdef USE_REGEX
    strcat(inv_out, "regex package is installed\n");
#else
    strcat(inv_out, "regex package is not installed\n");
#endif
#ifdef DISABLE_STAT
    strcat(inv_out, "flush_mode=1\n");
#else
    strcat(inv_out, "flush_mode determined by stat()\n");
#endif

#ifdef USE_TIGGE
    strcat(inv_out, "tigge package is installed\n");
#else
    strcat(inv_out, "tigge package is not installed\n");
#endif

#if USE_IPOLATES > 0
    inv_out += strlen(inv_out);
    sprintf(inv_out, "IPOLATES " IPOLATES_LIB " (option %d) is installed", USE_IPOLATES);
#if USE_SPECTRAL > 0
    strcat(inv_out, " with spectral interpolation");
#endif
    strcat(inv_out, ", default vectors:\n");
#else
    strcat(inv_out, "interpolation package is not installed, default vectors:\n");
#endif

    i = 0;
    while (default_vectors[i] != NULL) {
	if ( (i % 15 == 14)) strcat(inv_out, "\n    ");
        strcat(inv_out, default_vectors[i]);
        strcat(inv_out, i & 1 ? " " : "/");
        i++;
    }
    strcat(inv_out, "\n");

strcat(inv_out, "Geolocation library status (by search order)\n");

#if (DEFAULT_GCTPC == 1)
    strcat(inv_out, "  gctpc geolocation (-lambert azimuthal equal area non-spherical) is enabled by default\n"); 
#else
    strcat(inv_out, "  gctpc geolocation (-lambert azimuthal equal area non-spherical) is disabled by default\n");
#endif

#ifdef USE_PROJ4
  #if (DEFAULT_PROJ4 == 1)
    strcat(inv_out, "  Proj4 geolocation is enabled by default\n");
  #else
    strcat(inv_out, "  Proj4 geolocation is disabled by default\n");
  #endif
#endif

strcat(inv_out, "  spherical geolocation is enabled\n"); 


#ifdef USE_UDF
    strcat(inv_out, "UDF package is installed\n");
#else
    strcat(inv_out, "UDF package is not installed\n");
#endif

    inv_out += strlen(inv_out);
    sprintf(inv_out,"version ftime=%d\n", version_ftime);

#ifdef N_ARGLIST
    inv_out += strlen(inv_out);
    sprintf(inv_out, "maximum number of arguments on command line: %d\n",
	N_ARGLIST);
#else
    inv_out += strlen(inv_out);
    sprintf(inv_out, "maximum number of arguments on command line: limited by shell/OS\n");
#endif

#ifdef USE_REGEX
    inv_out += strlen(inv_out);
    sprintf(inv_out, "maximum number of -match,-not,-if, and -not_if options: %d\n", MATCH_MAX);
#endif
    inv_out += strlen(inv_out);
    sprintf(inv_out, "maximum number of -match_fs,-not_fs,-if_fs, and -not_if_fs options: %d\n", MATCH_MAX);
    inv_out += strlen(inv_out);
    sprintf(inv_out, "maximum number of -fgrep, -egrep, -fgrep_v, -egrep_v options: %d\n", GREP_MAX);
    inv_out += strlen(inv_out);
    sprintf(inv_out, "RPN registers: 0..%d\n", N_RPN_REGS-1);
    inv_out += strlen(inv_out);
    sprintf(inv_out, "memory files: @mem:0, @mem:1 .. @mem:%d\n",N_mem_buffers-1);
    inv_out += strlen(inv_out);
    sprintf(inv_out, "stdout buffer length: %d\n", INV_BUFFER);

#if (DEFAULT_G2CLIB == 0)
    strcat(inv_out, "default decoding: WMO standard\n");
#endif
#if (DEFAULT_G2CLIB == 1)
    strcat(inv_out, "default decoding: g2clib emulation\n");
#endif
#if (DEFAULT_G2CLIB == 2)
    strcat(inv_out, "default decoding: g2clib\n");
#endif

#ifdef USE_G2CLIB
    strcat(inv_out, "g2clib decoders are installed\n");
#else
    strcat(inv_out, "g2clib decoders are not installed\n");
#endif

    strcat(inv_out,"Supported decoding: simple, complex, rle, ieee");
#ifdef USE_PNG
    strcat(inv_out, ", png");
#endif
#if defined USE_JASPER || defined USE_OPENJPEG
    strcat(inv_out, ", jpeg2000");
#endif
#ifdef USE_AEC
    strcat(inv_out, ", CCSDS AEC");
#endif
    strcat(inv_out, "\n");

    strcat(inv_out,"Supported encoding: simple, complex, ieee");
#if defined USE_JASPER || defined USE_OPENJPEG
    strcat(inv_out, ", jpeg2000");
#endif
#ifdef USE_AEC
    strcat(inv_out, ", CCSDS AEC");
#endif
    strcat(inv_out, "\n");

    filename = getenv("GRIB2TABLE");
    if (filename == NULL) filename = getenv("grib2table");
    if (filename == NULL) filename = "grib2table";
    input = fopen(filename,"r");
    inv_out += strlen(inv_out);
    sprintf(inv_out, "user gribtable: %s\n", input == NULL ? "(none)" : filename);
    if (input) fclose(input);

#if USE_NAMES == NCEP
    sprintf(inv_out,"default WMO names: NCEP\n");
#endif
#if USE_NAMES == ECMWF
    sprintf(inv_out,"default WMO names: ECMWF\n");
#endif
#if USE_NAMES == DWD1
    sprintf(inv_out,"default WMO names: DWD\n");
#endif
    inv_out += strlen(inv_out);

#ifdef CC
    strcat(inv_out,"C compiler: " CC "\n");
    strcat(inv_out,"  CPPFLAGS=" CPPFLAGS "\n");
#endif

#ifdef FORTRAN
    strcat(inv_out,"Fortran compiler: " FORTRAN "\n");
    strcat(inv_out,"  FFLAGS=" FFLAGS "\n");
#endif
#ifdef USE_OPENMP
    strcat(inv_out,"OpenMP: control number of threads with environment variable OMP_NUM_THREADS\n");
#else
    strcat(inv_out,"OpenMP: not used\n");
#endif
    inv_out += strlen(inv_out);

    sprintf(inv_out, "INT_MAX:   %d\n", INT_MAX);
    inv_out += strlen(inv_out);
    sprintf(inv_out, "ULONG_MAX: %lu\n", ULONG_MAX);
    inv_out += strlen(inv_out);
    sprintf(inv_out, "-set options: %s\n", set_options);

    return 1;
}

const char *wgrib2api_info(void) {
  return WGRIB2_VERSION "  " BUILD_COMMENTS " " CC;
}
