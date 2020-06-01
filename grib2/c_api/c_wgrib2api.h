/* include file for wgrib2 C api
 * 3/2018 Public Domain Wesley Ebisuzaki
 *
 * this include file requires C99 because it uses __VA_ARGS__
 */

#include "wgrib2_api.h"

/* these are based on wgrib2api */

int grb2_mk_inv(char *grb, char *inv);

#define grb2_UNDEFINED       9.999e20
#define grb2_UNDEFINED_LOW   9.9989e20
#define grb2_UNDEFINED_HIGH  9.9991e20
#define grb2_UNDEFINED_VAL(x) ((x) >= grb2_UNDEFINED_LOW && (x) <= grb2_UNDEFINED_HIGH)
#define grb2_DEFINED_VAL(x) ((x) < grb2_UNDEFINED_LOW || (x) > grb2_UNDEFINED_HIGH)

#define N_CMDS		60
#define CMD_LEN		300

/* for variable number of arguments, requires C99 */
// #if __STDC_VERSION__ >= 199001L
#define grb2_inq(...) grb2_inqVA(__VA_ARGS__, NULL)
#define grb2_wrt(...) grb2_wrtVA(__VA_ARGS__, NULL)
// #endif

long long int grb2_inqVA(const char *grb, const char *inv, unsigned int options, ...);
int grb2_wrtVA(const char *grb, const char *template, int msgno, float *data, unsigned int ndata, ...);


/* options: grb2_inqVA */
#define SEQUENTIAL 1
#define DATA       2
#define LATLON     4
#define LONLAT     4
#define WENS       8
#define RAW_ORDER 16
#define META      32 
#define GRIDMETA  64 
#define REGEX    128

int grb2_get_data(float *data, int ndata);
int grb2_get_lonlat(float *lon, float *lat, int ndata);
int grb2_size_meta(void);
int grb2_size_gridmeta(void);

void wgrib2_init_cmds(void);
int wgrib2_add_cmd(const char *string);
int wgrib2_cmd(void);
int wgrib2_list_cmd(void);


void fatal_error(const char *fmt, const char *string);

