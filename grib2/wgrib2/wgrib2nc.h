/******************************************************************************************
 This file is part of wgrib2 and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

 Edition 2008.02.18  Sergey Varlamov
*/
//#include "wgrib2.h" /* UNDEFINED only */

#ifndef _MAX_PATH
#define _MAX_PATH   256 /* max. length of full pathname */
#endif
#define G2NC_MAX_NLEV 1000
/*
   max number of entries in GRIB2 to NETCDF conversion table,
   security only to avoid infinite read-reallocate cycle
 */
#define G2NC_MAX_VARS 1000
/*
   list of standard level types that could be clearly written as 4D in netcdf.
   smlt - 'supported multy-level types' of vertical coordinate.
   sname is used as variable name so must be different from all
   other dimension and physical variables written into the netcdf file.
   Selected from Level.c
   Or use code_table_3_15? It includes more candidates that are 'reserved' in Levels.c:
    case 110: string="Geometric height [m]"; break;
    case 111: string="Eta coordinate  []"; break;
    case 112: string="Geopotential height [gpm]"; break;
*/
#define G2NC_TM_TOLERANCE  1e-1    /* assumed accuracy for time difference estimations */
#define G2NC_TM_IND_GAP    50      /* max number of undefined time steps before entered new data field  */
#define G2NC_LV_TOLERANCE  1e-3    /* const for z-dim test */

#define G2NC_PACK_UNDEF  0   /* in version 2008.02.18 mean default float */
#define G2NC_PACK_BYTE   1
#define G2NC_PACK_SHORT  2
#define G2NC_PACK_FLOAT  3
#define G2NC_PACK_DOUBLE 4

/*_FillValue for different data types, could be used for packing (short,byte), max value*/
#define G2NC_FILL_VALUE_BYTE   127
#define G2NC_FILL_VALUE_SHORT  32767
#define G2NC_FILL_VALUE_FLOAT  UNDEFINED
#define G2NC_FILL_VALUE_DOUBLE UNDEFINED

typedef struct {
    int type;
    const char * sname;
    const char * lname;
    const char * units;
    float scale;      /* multiply on it, example: 0.01 to change Pa to gPa or mb */
} g2nc_4Dlt;

#define G2NC_NUM_4DLT 6

/*
   user defined table of GRIB2 to NETCDF conversion parameters
   It would be read from user file provided with '-nc_table' option
   Format:  wgib2_name:[wgrib2_level|*]:nc_name[:[short|byte]:min:max]
   wgib2_name, wgrib2_level are as returned by wgrib2 in inventory;
   '*' used as wgrib2_level will apply for all levels not given explicitly,
   packing information is optional but it overwrites common packing
   if given with the '-nc_pack' option. Absence of packing info means no packing.
   Both zero min and max values activate 'auto' packing when these values are defined
   from the first entered field. When packing, fitting is checked, error if do not fit.
 */
typedef struct {
    char * wgrib2_name;
    char * wgrib2_level;
    char * nc_name;
    int    nc_pack;       /* One of 0, NC_BYTE, NC_SHORT or NC_FLOAT. In last case check the valid_range only */
    float  nc_offset;     /* Used to check valid_range of values and pack data to short or to byte, replace "bad" by _FillValue */
    float  nc_scale;      /* Used to check valid_range of values and pack data to short or to byte, replace "bad" by _FillValue */
    float  nc_valid_min;  /* Used to check valid_range of values for float data, replace "bad" by _FillValue */
    float  nc_valid_max;  /* Used to check valid_range of values for float data, replace "bad" by _FillValue */
    int    nc_deflate;    /* Netcdf-4 only for by-variable deflating; default deflation level is 1 (1-9 is OK) */
    int    ignore;
} g2nc_conv;

typedef struct {     /* default */
  int         used;  /* 0, if was used and is linked to one or more 'local' descriptors, could re-initialized */
  int         grads; /* 0, does check netcdf structure for: 1) fixed time step; 2) no byte packing? */
  int         endianness; /* Netcdf-4 only, could be done by-variable but applied to all; 0 for native, 1 for little, 2 for big endian */
  int         nlev;  /* -1, number of vertical levels if defined in the table */
  g2nc_4Dlt * lt;    /* NULL, 4D level type description if defined in the table */
  float     * lv;    /* NULL, 4D level values to use if defined in the table */
  int         nvc;   /* 0, counter of variable conversion entries 'vc' in this table */
  g2nc_conv * vc;    /* NULL, variables conversion parameters */
} g2nc_table;

/*
   Function declarations (defined in the Netcdf_sup.c)
   that would not be added to the fnlist.h by fnlist.sh script
*/
double get_unixtime(int year, int month, int day, int hour,
                    int minute, int second, int * err_code);
char * get_unixdate(double utime, char * date_str);
int match_str(const char *s, const char *match);
void fix_units(char *s, int n);
int get_nc_conv_table(const char * name, const char * level,
                      const g2nc_table * nc_table);
int free_nc_table( g2nc_table * nc_table );

