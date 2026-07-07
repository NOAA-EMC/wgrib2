/** @file
 * @brief Header file for the wgrib2 C API. Based on wgrib2api.
 * 
 * This include file requires C99 because it uses __VA_ARGS__
 * @author Public Domain: Wesley Ebisuzaki @date 3/2018
 */

int grb2_mk_inv(char *grb, char *inv);

#define grb2_UNDEFINED       9.999e20   /**< Undefined value - if bitmap. */
#define grb2_UNDEFINED_LOW   9.9989e20  /**< Floor of undefined value.*/
#define grb2_UNDEFINED_HIGH  9.9991e20  /**< Ceiling of undefined value.*/

/** Check if a value is undefined. */
#define grb2_UNDEFINED_VAL(x) ((x) >= grb2_UNDEFINED_LOW && (x) <= grb2_UNDEFINED_HIGH)

/** Check if a value is defined. */
#define grb2_DEFINED_VAL(x) ((x) < grb2_UNDEFINED_LOW || (x) > grb2_UNDEFINED_HIGH)

#define N_CMDS		60 /**< Maximum number of commands. */
#define CMD_LEN		300 /**< Maximum length of command string. */

/* for variable number of arguments, requires C99 */
// #if __STDC_VERSION__ >= 199001L
#define grb2_inq(...) grb2_inqVA(__VA_ARGS__, NULL) /**< For variable number of arguments in grb2_inqVA(). Requires C99. */
#define grb2_wrt(...) grb2_wrtVA(__VA_ARGS__, NULL) /**< For variable number of arguments in grb2_wrtVA(). Requires C99. */
// #endif

long long int grb2_inqVA(const char *grb, const char *inv, unsigned int options, ...);
int grb2_wrtVA(const char *grb, const char *template, int msgno, float *data, unsigned int ndata, ...);


/* options: grb2_inqVA */
#define SEQUENTIAL 1 /**< Option for grb2_inqVA(). Same as '-end' command-line option. */
#define DATA       2 /**< Option for grb2_inqVA(). Same as '-rpn_sto 19' command-line option. */
#define LATLON     4 /**< Option for grb2_inqVA(). Same as '-rpn rcl_lon:sto_17:rcl_lat:sto_18' command-line option. */
#define LONLAT     4 /**< Option for grb2_inqVA(). Same as LATLON option.  */
#define WENS       8 /**< Option for grb2_inqVA(). Same as '-order we:ns' command-line option. Cannot be used with LATLON/LONLAT or RAW_ORDER options. */
#define RAW_ORDER 16 /**< Option for grb2_inqVA(). Same as '-order raw' command-line option. Cannot be used with LATLON/LONLAT or WENS options. */
#define META      32 /**< Option for grb2_inqVA(). Same as '-S -last0 @mem:18' command-line option. */
#define GRIDMETA  64 /**< Option for grb2_inqVA(). Same as '-grid -last0 @mem:17' command-line option. */
#define REGEX    128 /**< Option for grb2_inqVA(). If selected, the '-egrep' command-line option is used, otherwise, '-fgrep' is used. */

int grb2_get_data(float *data, int ndata);
int grb2_get_lonlat(float *lon, float *lat, int ndata);
int grb2_size_meta(void);
int grb2_size_gridmeta(void);
int grb2_get_meta(unsigned char *meta, int nbytes) ;
int grb2_get_gridmeta(unsigned char *meta, int nbytes);

void wgrib2_init_cmds(void);
int wgrib2_add_cmd(const char *string);
int wgrib2_cmd(void);
int wgrib2_list_cmd(void);


void fatal_error(const char *fmt, ...);

