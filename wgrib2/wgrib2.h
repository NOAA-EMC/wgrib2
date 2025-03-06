/*
 * public domain 12/2006 wesley ebisuzaki
 *               1/2007 M. Schwarb
 *               5/2016 G. Schnee
 */

#ifndef _WGRIB2_H_
#define _WGRIB2_H_

#include <stdio.h>

#ifndef _CONFIG_H
#include "config.h"
#define _CONFIG_H
#endif

#include "wgrib2_api.h"

#ifndef BUILD_COMMENTS
#define BUILD_COMMENTS "unknown build"
#endif

/* define level of OpenMP support minimum level is v3.1 */

#ifdef USE_OPENMP
#if _OPENMP >= 201307
  #define IS_OPENMP_3_1
#endif
#if _OPENMP >= 201307
  #define IS_OPENMP_4_0
#endif
#if _OPENMP >= 201511
  #define IS_OPENMP_4_5
#endif
#if _OPENMP >= 201811
  #define IS_OPENMP_5_0
#endif
#endif

/* parameters to PNG encode routine */
#define PNG_WIDTH_MAX  100000000
#define PNG_HEIGHT_MAX 100000


/*  1/2007 M. Schwarb unsigned int ndata */

/* max number of regular expressions used, max number of fs (fixed string) matches used */
#define MATCH_MAX 2000
#define GREP_MAX 200
/* maximum number of inv functions that can be added to match_inv */
#define MATCH_EXTRA_FN 10

/* max number of nested if blocks */
#define MAX_IF  10

#define UNDEFINED       9.999e20
#define UNDEFINED_LOW   9.9989e20
#define UNDEFINED_HIGH  9.9991e20
#define UNDEFINED_VAL(x) ((x) >= UNDEFINED_LOW && (x) <= UNDEFINED_HIGH)
#define DEFINED_VAL(x) ((x) < UNDEFINED_LOW || (x) > UNDEFINED_HIGH)
#define UNDEFINED_ANGLE 999.0


/* formatting length for function names for help screen */
#define HELP_NAME_LEN	15
#define N_ARGLIST	10000
struct ARGLIST {int fn; int i_argc;};
#define STRING_SIZE	200
#define INV_STRING_SIZE	600
#define EXT_TABLE_SIZE  (8*1024)

#define N_mem_buffers 30
#define N_RPN_REGS    20

#define DEFAULT_G2CLIB	1		/* use g2clib emulation by default */
// #define DEFAULT_GCTPC	0		/* use gctpc for geolocation */
#define DEFAULT_GCTPC	1		/* use gctpc for geolocation */
#define DEFAULT_PROJ4	1		/* use Proj4 for geolocation */

#define CACHE_LINE_BITS	1024		/* size of cache line in bits for openmp, needs to be a multiple of 8 */
					/* want to prevent false sharing.  Note: x86 cache line = 64 bytes = 512 bits */
					/* but no harm if double this value.  */
					/* if not right but multiple of 8, may be slower */

/* calling arguments for function API */

#define ARG     int mode, unsigned char **sec, float *data, unsigned int ndata, char *inv_out, void **local
#define ARG0	ARG, const char *dum1, const char *dum2, const char *dum3, const char *dum4, const char *dum5, const char *dum6, const char *dum7, const char *dum8
#define ARG1	ARG, const char *arg1, const char *dum2, const char *dum3, const char *dum4, const char *dum5, const char *dum6, const char *dum7, const char *dum8
#define ARG2	ARG, const char *arg1, const char *arg2, const char *dum3, const char *dum4, const char *dum5, const char *dum6, const char *dum7, const char *dum8
#define ARG3	ARG, const char *arg1, const char *arg2, const char *arg3, const char *dum4, const char *dum5, const char *dum6, const char *dum7, const char *dum8
#define ARG4	ARG, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *dum5, const char *dum6, const char *dum7, const char *dum8
#define ARG5	ARG, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *dum6, const char *dum7, const char *dum8
#define ARG6	ARG, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *dum7, const char *dum8
#define ARG7	ARG, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *dum8
#define ARG8	ARG, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8


/* buf = buffer our text out from function */
/* void **local, pointer for static data */

#define call_ARG0(inv_out,local)	        mode, sec, data, ndata, inv_out, local, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#define call_ARG1(inv_out,local,arg1)	        mode, sec, data, ndata, inv_out, local, arg1, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#define call_ARG2(inv_out,local,arg1,arg2)	mode, sec, data, ndata, inv_out, local, arg1, arg2, NULL, NULL, NULL, NULL, NULL, NULL
#define call_ARG3(inv_out,local,arg1,arg2,arg3)	mode, sec, data, ndata, inv_out, local, arg1, arg2, arg3, NULL, NULL, NULL, NULL, NULL
#define call_ARG4(inv_out,local,arg1,arg2,arg3,arg4)	mode, sec, data, ndata, inv_out, local, arg1, arg2, arg3, arg4, NULL, NULL, NULL, NULL
#define call_ARG5(inv_out,local,arg1,arg2,arg3,arg4,arg5) mode, sec, data, ndata, inv_out, local, arg1, arg2, arg3, arg4, arg5, NULL, NULL, NULL
#define call_ARG6(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6) mode, sec, data, ndata, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, NULL, NULL
#define call_ARG7(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6,arg7) mode, sec, data, ndata, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, arg7, NULL
#define call_ARG8(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) mode, sec, data, ndata, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8

#define init_ARG0(inv_out,local)	        -1, NULL, NULL, 0, inv_out, local, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#define init_ARG1(inv_out,local,arg1)	        -1, NULL, NULL, 0, inv_out, local, arg1, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#define init_ARG2(inv_out,local,arg1,arg2)	-1, NULL, NULL, 0, inv_out, local, arg1, arg2, NULL, NULL, NULL, NULL, NULL, NULL
#define init_ARG3(inv_out,local,arg1,arg2,arg3)	-1, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, NULL, NULL, NULL, NULL, NULL
#define init_ARG4(inv_out,local,arg1,arg2,arg3,arg4)	-1, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, NULL, NULL, NULL, NULL
#define init_ARG5(inv_out,local,arg1,arg2,arg3,arg4,arg5) -1, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, NULL, NULL, NULL
#define init_ARG6(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6) -1, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, NULL, NULL
#define init_ARG7(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6,arg7) -1, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, arg7, NULL
#define init_ARG8(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) -1, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8

#define fin_ARG0(inv_out,local)	        -2, NULL, NULL, 0, inv_out, local, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#define fin_ARG1(inv_out,local,arg1)	        -2, NULL, NULL, 0, inv_out, local, arg1, NULL, NULL, NULL, NULL, NULL, NULL, NULL
#define fin_ARG2(inv_out,local,arg1,arg2)	-2, NULL, NULL, 0, inv_out, local, arg1, arg2, NULL, NULL, NULL, NULL, NULL, NULL
#define fin_ARG3(inv_out,local,arg1,arg2,arg3)	-2, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, NULL, NULL, NULL, NULL, NULL
#define fin_ARG4(inv_out,local,arg1,arg2,arg3,arg4)	-2, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, NULL, NULL, NULL, NULL
#define fin_ARG5(inv_out,local,arg1,arg2,arg3,arg4,arg5) -2, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, NULL, NULL, NULL
#define fin_ARG6(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6) -2, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, NULL, NULL
#define fin_ARG7(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6,arg7) -2, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, arg7, NULL
#define fin_ARG8(inv_out,local,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) -2, NULL, NULL, 0, inv_out, local, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8

/* delayed errors */
#define DELAYED_NONERROR_END		1
#define DELAYED_PDT_SIZE_ERR		2
#define DELAYED_LOCAL_GRIBTABLE_ERR	4
#define DELAYED_GRID_SIZE_ERR		8
#define DELAYED_FTIME_ERR		16
#define DELAYED_MISC			32

enum input_dev_type {DISK, PIPE, MEM, NOT_OPEN};
enum input_type {inv_mode, dump_mode, all_mode};
enum output_order_type {raw,wesn,wens};
enum output_grib_type {jpeg,ieee_packing,simple,complex1,complex2,complex3,aec};
enum wind_rotation_type {grid, earth, undefined};
enum geolocation_type {proj4, gctpc, internal, external, not_used};
enum new_grid_format_type {bin, ieee, grib};

struct seq_file {
    enum input_dev_type file_type;
    FILE *cfile;
    int n_mem_buffer;
    /* the rest of these values are used by rd_grib2_msg for sequential reads */
    unsigned char unget_buf[10];
    int unget_cnt;
    long int buffer_size;
    unsigned char *buffer;
    char filename[STRING_SIZE];
    long pos;
};



const char *output_order_name(void);			/* returns text string of output order */

/* maximum length of an inventory line */
#define INV_BUFFER	100000
/* maximum length of a name */
#define NAMELEN		50

/* maximum size of PDT in update_pdt.c */
#define SET_PDT_SIZE  3000
#define SET_GDT_SIZE  3000
int update_sec3(unsigned char **sec, unsigned char *sec3);
int update_sec4(unsigned char **sec, unsigned char *sec4);

#define ONES	(~ (int) 0)

struct full_date {
   int year, month, day, hour, minute, second;
};

struct gribtable_s {
  int disc;   /* Section 0 Discipline                                */
  int mtab_set;    /* Section 1 Master Tables Version Number used by set_var      */
  int mtab_low;    /* Section 1 Master Tables Version Number low range of tables  */
  int mtab_high;   /* Section 1 Master Tables Version Number high range of tables */
  int cntr;   /* Section 1 originating centre, used for local tables */
  int ltab;   /* Section 1 Local Tables Version Number               */
  int pcat;   /* Section 4 Template 4.0 Parameter category           */
  int pnum;   /* Section 4 Template 4.0 Parameter number             */
  const char *name;
  const char *desc;
  const char *unit;
};

/* Ens_processing.c: ens_processing grid allocation
 * need to save grids for ens_proceesing
 * first allocation  npts * ENS_PROCESSING_NGRID0
 * next realloc ENS_PROCESSING_NGRID_FACTER * old_allocation
 */
#define ENS_PROCESSING_NGRID0 		8
#define ENS_PROCESSING_NGRID_FACTOR 	2

void init_globals(void);
double Int_Power(double x, int y);

int int4(unsigned const char *);
int int4_comp(unsigned const char *);
int int2(unsigned const char *);
int int1(unsigned const char *);
int int_n(unsigned const char *p, int n);
int is_missing(unsigned const char *p, int len);
unsigned int uint_n(unsigned const char *p, int n);
unsigned int uint4(unsigned const char *);
unsigned int uint4_missing(unsigned const char *);
unsigned int uint2(unsigned const char *);
unsigned long int uint8(unsigned const char *);
float scaled2flt(int scale_factor, int scale_value);
double scaled2dbl(int scale_factor, int scale_value);
int flt2scaled(int scale_factor, float value);
void scaled_char(int scale_factor, int scale_value, unsigned char *p);
int best_scaled_value(double val, int *scale_factor, int *scale_value);
void uint8_char(unsigned long int i, unsigned char *p);
void uint_char(unsigned int i, unsigned char *p);
void int_char(int i, unsigned char *p);
void uint2_char(unsigned int i, unsigned char *p);
void int2_char(int i, unsigned char *p);
void int1_char(int i, unsigned char *p);
void itoshort_a(char *string, int i);
char *nx_str(unsigned int nx);
char *ny_str(unsigned int ny);
int string2time_unit(char *string);
int sub_angle(unsigned const char *p);
int is_uint(const char *p);

float ieee2flt(unsigned char *ieee);
float ieee2flt_nan(unsigned char *ieee);
int rdieee_file(float *array, unsigned int n, int header, struct seq_file *input);

FILE *ffopen(const char *filename, const char *mode);
int ffclose(FILE *file);
int mk_file_persistent(const char *filename);
int mk_file_transient(const char *filename);
int rewind_file(const char *filename);
int ffclose_finished(void);
void status_ffopen(void);
void set_io_buffer_size(int n);
 
unsigned char *seek_grib2(FILE *file, long int *pos, unsigned long int *len_grib,
        unsigned char *buffer, unsigned int buf_len, long int *n_bytes);

unsigned char *rd_grib2_msg(unsigned char **sec, FILE *input, long int *pos, unsigned long int *len, int *num_submsgs);
unsigned char *rd_grib2_msg_seq(unsigned char **sec, FILE *input, long int *pos, unsigned long int *len, int *num_submsgs);

int parse_1st_msg(unsigned char **sec);
int parse_next_msg(unsigned char **sec);

int fopen_file(struct seq_file *file, const char *filename, const char *open_mode);
void fclose_file(struct seq_file *file);
int fseek_file(struct seq_file *file, long position, int whence);
long ftell_file(struct seq_file *file); 
size_t fread_file(void *ptr, size_t size, size_t nmemb, struct seq_file *file);
size_t fwrite_file(const void *ptr, size_t size, size_t nmemb, struct seq_file *file);
int fgetc_file(struct seq_file *file);
char *fgets_file(char *s, int size, struct seq_file *file);
void fflush_file(struct seq_file *file);


unsigned char *rd_grib2_msg_seq_file(unsigned char **sec, struct seq_file *input, long int *pos,
        unsigned long int *len, int *num_submsgs);
 
unsigned int missing_points(unsigned char *bitmap, unsigned int n);

void BDS_unpack(float *flt, unsigned char *bits, unsigned char *bitmap,
        int n_bits, int n, double ref, double scale);

void setup_user_gribtable(void);
int getName(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *unit);
int getName_all(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *unit, int *mset, int *mlow, int *mhigh);

int rd_inventory(int *rec_num, int *submsg, long int *pos, struct seq_file *);
int get_nxny(unsigned char **sec, int *nx, int *ny, unsigned int *npnts, int *res, int *scan);
int get_nxny_(unsigned char **sec, unsigned int *nx, unsigned int *ny, unsigned int *npnts, int *res, int *scan);

int wrtieee(float *array, unsigned int n, int header, struct seq_file *out);
int flt2ieee(float x, unsigned char *ieee);
int flt2ieee_nan(float x, unsigned char *ieee);
int check_datecode(int year, int month, int day);
int check_time(int year, int month, int day, int hour, int minute, int second);
int add_time(int *year, int *month, int *day, int *hour, int *minute, int *second, int dtime, int unit);
int Add_time(struct full_date *date, int dtime, int unit);
int add_dt(int *year, int *month, int *day, int *hour, int *minute, int *second, int dtime, int unit);
int sub_dt(int *year, int *month, int *day, int *hour, int *minute, int *second, int dtime, int unit);
int sub_time(int year1, int month1, int day1, int hour1, int minute1, int second1, int year0, int month0, int day0, int hour0, int minute0, int second0, int *dtime, int *unit);
int jday(int year,int month, int day);
int num_days_in_month(int year, int month);
int verftime(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second);
int Verf_time(unsigned char **sec, struct full_date *date);
int start_ft(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second);
int reftime(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second);
int Ref_time(unsigned char **sec, struct full_date *date);

int cmp_time(int year0, int month0, int day0, int hour0, int minute0, int second0, int year1, int month1, int day1, int hour1, int minute1, int second1);
int Cmp_time(struct full_date *date0, struct full_date *date1);


int is_match(const char *string);
int is_match_fs(const char *string);
int is_egrep(const char *s);
int is_fgrep(const char *s);
const char *nc_strstr(const char *s, const char *t);
int match_inv(int type_datecode, ARG0);

int code_table_0_0(unsigned char **sec);
unsigned char *code_table_0_0_location(unsigned char **sec);
int code_table_1_0(unsigned char **sec);
unsigned char *code_table_1_0_location(unsigned char **sec);
int code_table_1_1(unsigned char **sec);
unsigned char *code_table_1_1_location(unsigned char **sec);
int code_table_1_2(unsigned char **sec);
unsigned char *code_table_1_2_location(unsigned char **sec);
int code_table_1_3(unsigned char **sec);
unsigned char *code_table_1_3_location(unsigned char **sec);
int code_table_1_4(unsigned char **sec);
unsigned char *code_table_1_4_location(unsigned char **sec);
int code_table_1_5(unsigned char **sec);
unsigned char *code_table_1_5_location(unsigned char **sec);
int code_table_1_6(unsigned char **sec);
unsigned char *code_table_1_6_location(unsigned char **sec);

int code_table_3_0(unsigned char **sec);
int code_table_3_1(unsigned char **sec);
int code_table_3_2(unsigned char **sec);
unsigned char *code_table_3_2_location(unsigned char **sec);
int code_table_3_3(unsigned char **sec);
int code_table_3_6(unsigned char **sec);
int code_table_3_7(unsigned char **sec);
int code_table_3_8(unsigned char **sec);
int code_table_3_11(unsigned char **sec);
int code_table_3_15(unsigned char **sec);
unsigned char *code_table_3_15_location(unsigned char **sec);
int code_table_3_20(unsigned char **sec);
unsigned char *code_table_3_20_location(unsigned char **sec);
int code_table_3_21(unsigned char **sec);

int code_table_4_0(unsigned char **sec);
int code_table_4_1(unsigned char **sec);
unsigned char *code_table_4_1_location(unsigned char **sec);
int code_table_4_2(unsigned char **sec);
unsigned char *code_table_4_2_location(unsigned char **sec);
int code_table_4_3(unsigned char **sec);
unsigned char *code_table_4_3_location(unsigned char **sec);
int code_table_4_4(unsigned char **sec);
unsigned char *code_table_4_4_location(unsigned char **sec);
int code_table_4_4_not_used(unsigned char **sec);
int code_table_4_5a(unsigned char **sec);
unsigned char *code_table_4_5a_location(unsigned char **sec);
int code_table_4_5b(unsigned char **sec);
unsigned char *code_table_4_5b_location(unsigned char **sec);
int code_table_4_6(unsigned char **sec);
unsigned char *code_table_4_6_location(unsigned char **sec);
int code_table_4_7(unsigned char **sec);
unsigned char *code_table_4_7_location(unsigned char **sec);
int code_table_4_8(unsigned char **sec);
unsigned char *code_table_4_8_location(unsigned char **sec);
int code_table_4_9(unsigned char **sec);
unsigned char *code_table_4_9_location(unsigned char **sec);
int code_table_4_10(unsigned char **sec);
unsigned char *code_table_4_10_location(unsigned char **sec);
int code_table_4_11(unsigned char **sec);
unsigned char *code_table_4_11_location(unsigned char **sec);
int code_table_4_15(unsigned char **sec);
unsigned char *code_table_4_15_location(unsigned char **sec);
int code_table_4_16(unsigned char **sec);
unsigned char *code_table_4_16_location(unsigned char **sec);
int code_table_4_57(unsigned char **sec);
unsigned char *code_table_4_57_location(unsigned char **sec);
int code_table_4_91(unsigned char **sec);
unsigned char *code_table_4_91_location(unsigned char **sec);
int code_table_4_91b(unsigned char **sec);
unsigned char *code_table_4_91b_location(unsigned char **sec);
int prt_code_table_4_91(int type_of_intervale, double val1, double val2, char *inv_out);
int scan_code_table_4_91(int *type_of_interval, double *val1, double *val2, const char *string);

int code_table_4_230(unsigned char **sec);
unsigned char *code_table_4_230_location(unsigned char **sec);
struct codetable_4_230 {int no; char *name;};

int code_table_4_233(unsigned char **sec);
unsigned char *code_table_4_233_location(unsigned char **sec);
int code_table_4_235(unsigned char **sec);
unsigned char *code_table_4_235_location(unsigned char **sec);
int code_table_4_240(unsigned char **sec);
unsigned char *code_table_4_240_location(unsigned char **sec);
int code_table_4_241(unsigned char **sec);
unsigned char *code_table_4_241_location(unsigned char **sec);
int code_table_4_242(unsigned char **sec);
unsigned char *code_table_4_242_location(unsigned char **sec);


int code_table_5_0(unsigned char **sec);
int code_table_5_1(unsigned char **sec);
int code_table_5_4(unsigned char **sec);
int code_table_5_5(unsigned char **sec);
int code_table_5_6(unsigned char **sec);
int code_table_5_7(unsigned char **sec);
int code_table_6_0(unsigned char **sec);
int number_of_coordinate_values_after_template(unsigned char **sec);
int number_of_forecasts_in_the_ensemble(unsigned char **sec);
unsigned char *number_of_forecasts_in_the_ensemble_location(unsigned char **sec);
int perturbation_number(unsigned char **sec);
unsigned char *perturbation_number_location(unsigned char **sec);
int forecast_time_in_units(unsigned char **sec);
unsigned char *forecast_time_in_units_location(unsigned char **sec, int *size);
void fixed_surfaces(unsigned char **sec, int *type1, float *surface1,
        int *undef_val1, int *type2, float *surface2, int *undef_val2);
int background_generating_process_identifier(unsigned char **sec);
unsigned char *background_generating_process_identifier_location(unsigned char **sec);
int analysis_or_forecast_generating_process_identifier(unsigned char **sec);
unsigned char *analysis_or_forecast_generating_process_identifier_location(unsigned char **sec);
int observation_generating_process_identifier(unsigned char **sec);
unsigned char *observation_generating_process_identifier_location(unsigned char **sec);
int hours_of_observational_data_cutoff_after_reference_time(unsigned char **sec);
unsigned char *hours_of_observational_data_cutoff_after_reference_time_location(unsigned char **sec);
int minutes_of_observational_data_cutoff_after_reference_time(unsigned char **sec);
unsigned char *minutes_of_observational_data_cutoff_after_reference_time_location(unsigned char **sec);
int sub_missing_values(unsigned char **sec, float *missing1, float *missing2);
unsigned char *stat_proc_verf_time_location(unsigned char **sec);
int stat_proc_n_time_ranges_index(unsigned char **sec);
int stat_proc_verf_time(unsigned char **sec, int *year, int *month, int *day, int *hour, int *minute, int *second);
unsigned char *year_of_model_version_date_location(unsigned char **sec);
int percentile_value(unsigned char **sec);
unsigned char *percentile_value_location(unsigned char **sec);
int number_of_mode(unsigned char **sec);
int mode_number(unsigned char **sec);
int number_of_following_distribution_parameters_np(unsigned char **sec);
unsigned char *number_of_following_distribution_parameters_np_location(unsigned char **sec);

int smallest_pdt_len(int pdt);
int pdt_len(unsigned char **sec, int pdt);
int type_of_post_processing(unsigned char **sec);
int cluster_identifier(unsigned char **sec);
unsigned char *cluster_identifier_location(unsigned char **sec);
int number_of_clusters(unsigned char **sec);
unsigned char *number_of_clusters_location(unsigned char **sec);
int number_of_forecasts_in_the_cluster(unsigned char **sec);
unsigned char *number_of_forecasts_in_the_cluster_location(unsigned char **sec);
unsigned char *list_of_nc_ensemble_forecast_numbers_location(unsigned char **sec);
int number_of_contributing_spectral_bands(unsigned char **sec);
unsigned char *number_of_contributing_spectral_bands_location(unsigned char **sec);
int number_of_categories(unsigned char **sec);
unsigned char *number_of_categories_location(unsigned char **sec);
int number_of_partitions(unsigned char **sec);
unsigned char *number_of_partitions_location(unsigned char **sec);

int flag_table_3_3(unsigned char **sec);
int set_flag_table_3_3(unsigned char **sec, unsigned int flag);
unsigned char *flag_table_3_3_location(unsigned char **sec);

int flag_table_3_4(unsigned char **sec);
int set_flag_table_3_4(unsigned char **sec, unsigned int flag);
unsigned char *flag_table_3_4_location(unsigned char **sec);
int flag_table_3_5(unsigned char **sec);
unsigned char *flag_table_3_5_location(unsigned char **sec);
int flag_table_3_9(unsigned char **sec);
int flag_table_3_10(unsigned char **sec);

unsigned int pds_fcst_time(unsigned char **sec);
float *ij2p(unsigned int i, unsigned j, int scan_mode, unsigned int nx, unsigned int ny, float *data);
int to_we_ns_scan(float *data, int scan, unsigned int npnts, int nx, int ny, int save_translation);
int to_we_sn_scan(float *data, int scan, unsigned int npnts, int nx, int ny, int save_translation);
int get_latlon(unsigned char **sec, double **lon, double **lat);
void fatal_error(const char *fmt, ...);
#define fatal_error_i  fatal_error
#define fatal_error_ii  fatal_error
#define fatal_error_u fatal_error
#define fatal_error_lu fatal_error
#define fatal_error_li fatal_error
#define fatal_error_uu fatal_error
#define fatal_error_ss fatal_error
void set_mode(int new_mode);
int latlon_0(unsigned char **sec);
int new_gds(unsigned char **sec);
double gord(int n, double x);
double *gauss2lats(int nlat, double *ylat);
int closest_init(unsigned char **sec);
void closest_free(void);
//vsm: group of lat-lon related functions, float->double
long int closest( unsigned char **sec, double plat, double plon);
int regular2ll(unsigned char **sec, double **lat, double **lon);
int rot_regular2ll(unsigned char **sec, double **lat, double **lon);
int rot_regular2ij(unsigned char **sec, double **lat, double **lon, int n);
int polar2ll(unsigned char **sec, double **lat, double **lon);
int gauss2ll(unsigned char **sec, double **lat, double **lon);
int lambert2ll(unsigned char **sec, double **lat, double **lon);
int mercator2ll(unsigned char **sec, double **lat, double **lon);
int space_view2ll(unsigned char **sec, double **lat, double **lon);
int cubed_sphere2ll(unsigned char **sec, double **lat, double **lon);
int irr_grid2ll(unsigned char **sec, double **lat, double **lon);
int stagger(unsigned char **sec, unsigned int assumed_npnts, double *x, double *y);


void flist2bitstream(float *list, unsigned char *bitstream, unsigned int ndata, int nbits);


// void netcdf_command(int status);


double radius_earth(unsigned char **sec);
int axes_earth(unsigned char **sec, double *major , double *minor, int *is_spherical);

int unpk_grib(unsigned char **sec, float *data);
int set_order(unsigned char **sec, enum output_order_type order);
int swap_buffer(unsigned char *buffer, unsigned int n);
int wrt_sec(unsigned const char *sec0, unsigned const char *sec1, unsigned const char *sec2,
    unsigned const char *sec3, unsigned const char *sec4, unsigned const char *sec5,
    unsigned const char *sec6, unsigned const char *sec7, struct seq_file *file);
int scaling(unsigned char **sec, double *base, int *decimal, int *binary, int *nbits);
unsigned char *mk_bms(float *data, unsigned int *ndata);

int g2c_dec_png(unsigned char *pngbuf, int *width, int *height, unsigned char *cout);
int ieee_grib_out(unsigned char **sec, float *data, unsigned int ndata, struct seq_file *out);
int jpeg_grib_out(unsigned char **sec, float *data, unsigned int ndata, 
    int nx, int ny, int use_scale, int dec_scale, int bin_scale, FILE *out);
int jpeg2000_grib_out(unsigned char **sec, float *data, unsigned int ndata, int nx, int ny, 
    int use_scale, int dec_scale, int bin_scale, int wanted_bits, int max_bits, struct seq_file *out);
int aec_grib_out(unsigned char ** sec, float *data, unsigned int ndata, int use_scale, int dec_scale, 
    int bin_scale, int wanted_bits, int max_bits, struct seq_file *out);
int grib_out(unsigned char **sec, float *data, unsigned int ndata, FILE *out);
int complex_grib_out(unsigned char **sec, float *data, unsigned int ndata,
 int use_scale, int dec_scale, int bin_scale, int wanted_bits, int max_bits,
int packing_mode, int use_bitmap, struct seq_file *out);
int new_pdt(unsigned char **sec, unsigned char *new_sec4, int pdt, int len, int copy_metadata, char *misc_arg);

int grib_wrt(unsigned char **sec, float *data, unsigned int ndata, unsigned int nx, unsigned int ny, int use_scale, int dec_scale,
        int bin_scale, int wanted_bits, int max_bits, enum output_grib_type grib_type, struct seq_file *out);
int simple_grib_out(unsigned char **sec, float *data, unsigned int ndata, int use_scale,
   int dec_scale, int bin_scale, int wanted_bits, int max_bits, struct seq_file *out);
int mk_sec5and7(float *data, unsigned int n, unsigned char **sec5, unsigned char **sec7,
        int use_scale, int dec_scale, int bin_scale, int wanted_bits, int max_bits);

unsigned char *sec3_lola(int nx, double x0, double dx, int ny, double y0, double dy, unsigned char **old_sec);
unsigned char *sec3_lc(double lov, double lad, double latin1, double latin2, int proj, 
	int nx, double x0, double dx, int ny, double y0, double dy, unsigned char **old_sec);
unsigned char *sec3_polar_stereo(double lov, double lad, int proj, int nx, double x0, double dx, 
	int ny, double y0, double dy, unsigned char **old_sec);
unsigned char *sec3_mercator(double lad, int nx, double x0, double dx, double xn, int ny, 
        double y0, double dy, double yn, unsigned char **old_sec);
unsigned char *sec3_gaussian(int nx, double x0, double dx, int ny, double y0, unsigned char **old_sec);
unsigned char *sec3_rot_ll(int nx, double x0, double dx, int ny, double y0, double dy,
        double sp_lon, double sp_lat, double sp_rot, unsigned char **old_sec);

int small_grib(unsigned char **sec, int mode, float *data, double *lon, double *lat, unsigned int ndata,
        int ix0, int ix1, int iy0, int iy1, struct seq_file *out);
int small_domain(unsigned char **sec, double lonW, double lonE, double latS, double latN, int *ix0, int *ix1, int *iy0, int *iy1);
int cyclic(unsigned char **sec);
int undo_output_order(float *data, float *data_old_order, unsigned int npnts);

int prt_stat_tr(int mode, unsigned char **sec, char *inv_out, unsigned char *p, int n_inner);
int wrt_time(int unit, int value, char *inv_out);
int get_time(unsigned char *p, int *year, int *month, int *day, int *hour, int *minute, int *second);
int Get_time(unsigned char *p, struct full_date *date);
int save_time(int year, int month, int day, int hour, int minute, int second, unsigned char *p);
int Save_time(struct full_date *date, unsigned char *p);

int copy_sec(unsigned char **sec, unsigned char **clone_sec);
int free_sec(unsigned char **clone_sec);
int init_sec(unsigned char **clone_sec);
int copy_data(float *data, unsigned int ndata, float **clone_data);
int free_data(float *clone_data);

int same_sec0(unsigned char **sec_a, unsigned char **sec_b);
int same_sec0_not_var(int mode, unsigned char **sec_a, unsigned char **sec_b);
int same_sec1(unsigned char **sec_a, unsigned char **sec_b);
int same_sec1_not_time(int mode, unsigned char **sec_a, unsigned char **sec_b);
int same_sec1_not_var(int mode, unsigned char **sec_a, unsigned char **sec_b);
int same_sec2(unsigned char **sec_a, unsigned char **sec_b);
int same_sec3(unsigned char **sec_a, unsigned char **sec_b);
int same_sec4(unsigned char **sec_a, unsigned char **sec_b);
int same_sec4_not_time(int mode, unsigned char **sec_a, unsigned char **sec_b);
int same_sec4_diff_ave_period(unsigned char **sec_a, unsigned char **sec_b);
int same_sec4_for_merge(int mode, unsigned char **sec_a, unsigned char **sec_b);
int same_sec4_but_ensemble(int mode, unsigned char **sec_a, unsigned char **sec_b);
int same_sec4_not_var(int mode, unsigned char **sec_a, unsigned char **sec_b);
int same_sec4_unmerge_fcst(int mode, unsigned char **sec_a, unsigned char **sec_b);


void unpk_0(float *flt, unsigned char *bits0, unsigned char *bitmap0,
        int n_bits, unsigned int n, double ref0, double bin_scale, double dec_scale);

int fix_ncep_2(unsigned char **sec);
int fix_ncep_3(unsigned char **sec);
int fix_ncep_4(unsigned char **sec);
int fix_undef(unsigned char **sec);

int check_pdt_size(unsigned char **sec);

const char *wgrib2api_info(void);

// units.c
int a2time_range(const char * string);
const char *time_range2a(int tr);
int normalize_time_range(int *tr, int *val);
void simple_time_range(int *tr, int *val);
int a2code_4_10(const char *string);
const char *code_4_10_name(int code_4_10);
int a2anl_fcst(const char *string);

unsigned int cksum(unsigned char const *buf, size_t length);
void rd_bitstream(unsigned char *p, int offset, int *u, int n_bits, unsigned int n);
void rd_bitstream_flt(unsigned char *p, int offset, float *u, int n_bits, unsigned int n);
void add_bitstream(int t, int n_bits);
void add_many_bitstream(int *t, unsigned int n, int n_bits);
void init_bitstream(unsigned char *new_bitstream);
void finish_bitstream(void);

int unpk_complex(unsigned char **sec, float *data, unsigned int ndata);
int unpk_run_length(unsigned char **sec, float *data, unsigned int ndata);

int latlon_init(unsigned char **sec, unsigned int nx, unsigned int ny);
long int latlon_closest(unsigned char **sec, double plat, double plon);
int gaussian_init(unsigned char **sec, unsigned int nx, unsigned int ny);
long int gaussian_closest(unsigned char **sec, double plat, double plon);
int space_view_init(unsigned char **sec);
long int space_view_closest(unsigned char **sec, double plat, double plon);

int mk_kgds(unsigned char **sec, int *kgds);
int mk_gdt(unsigned char **sec, int *igdtnum, int *igdttmpl, int *igdtleni);
void ncep_grids(const char **arg1, const char **arg2, const char **arg3);

int parse_loop(const char *string, int *start, int *end, int *step);
int getExtName(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *unit);

int mk_WxKeys(unsigned char **sec);
const char *WxLabel(float f);

int min_max_array(float *data, unsigned int n, float *min, float *max);
int min_max_array_all_defined(float *data, unsigned int n, float *min, float *max);
int int_min_max_array(int *data, unsigned int n, int *min, int *max);
int int_min_array(int *data, unsigned int n);
int int_max_array(int *data, unsigned int n);
int delta(int *data, unsigned int n, int *min, int *max, int *first_val);

int new_grid_lambertc(int nx, int ny, double ref_lon, double ref_lat,
    double true_lat1, double true_lat2, double stand_lon, double stand_lat,
    double r_maj, double r_min, double dx,  double dy,
    double *lon_0, double *lat_0);

/* from gctpc not declared */
long init(long ipr,long jpr,char *efile,char *pfile);

int gctpc_get_latlon(unsigned char **sec, double **lon, double **lat);

int gctpc_ll2xy_init(unsigned char **sec, double *grid_lon, double *grid_lat);
int gctpc_ll2xy(int n, double *lon, double *lat, double *x, double *y);
int gctpc_ll2i(int n, double *lon, double *lat, unsigned int *ipnt);

int proj4_get_latlon(unsigned char **sec, double **lon, double **lat);

void init_mem_buffers(void);
void free_mem_buffer(int n);
unsigned char *new_mem_buffer(int n, size_t size);
unsigned char *realloc_mem_buffer(int n, size_t inc);
size_t fwrite_mem(const void *ptr, size_t size, size_t nmemb, int n);
size_t fread_mem(void *ptr, size_t size, size_t nmemb, int n);
int fseek_mem(int n, long position, int whence);
long ftell_mem(int n);
char *fgets_mem(char *s, int size, int n);

void err_bin(int error);
void err_string(int error);
char *save_string(char *string);

/* manage_inv_buffer */
void init_inv_out(void);
void new_inv_out(void);
void repeat_inv_out(void);
char *base_inv_out(void);

int parse_level1(unsigned char **sec, const char *string, int *table_4_5, int *scale_factor, int *scale_value);

/* old or modern if blocks */
enum fntype {inv, output, inv_output, misc, setup, If, Else, Elseif, Endif, Null};
int init_check_v1_v2(void);
int check_v1_v2(enum fntype type, const char *name);
int is_v1_v2(void);
void v1_if(void);
void v1_else(void);
void v1_elseif(void);
void v1_endif(void);
unsigned int read_latlon(const char *arg, double **lon, double **lat);

int check_pdt_size(unsigned char **sec);
double get_unixtime(int year, int month, int day, int hour, int minute, int second, int * err_code);

int JMA_Nb(unsigned char **sec);
int JMA_Nr(unsigned char **sec);

int set_metadata_string(ARG1);
#ifdef USE_IPOLATES
void ipolates_grib2_single_field(int *interpol, int *ipopt, int *gdt_in, int *gdttmpl_in, int *gdttmpl_size_in,
  int *gdt_out, int *gdttmpl_out, int *gdttmpl_size_out, int *mi, int *mo, int *km,
  int *ibi, unsigned char *bitmap, double *data_in, int *n_out, double *rlat, double *rlon,
   int *ibo, unsigned char *bitmap_out, double *data_out, int *iret);

void ipolatev_grib2_single_field(int *interpol, int *ipopt, int *gdt_in, int *gdttmpl_in, int *gdttmpl_size_in,
  int *gdt_out, int *gdttmpl_out, int *gdttmpl_size_out, int *mi, int *mo, int *km,
  int *ibi, unsigned char *bitmap, double *u_in, double *v_in, int *n_out, double *rlat, double *rlon,
   double *crot, double *srot, int *ibo, unsigned char *bitmap_out,
   double *u_out, double *v_out, int *iret);

void use_ncep_post_arakawa(void);
#endif /* USE_IPOLATES */



#endif /* _WGRIB2_H_ */
