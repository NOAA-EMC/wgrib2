/******************************************************************************************

 This file is part of wgrib2 and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

 First version: Copyright (C) 2007 Kristian Nilssen
 1st release version 3/2007, changes by Wesley Ebisuzaki
 versions 05/2007 - 06/2009 Sergey Varlamov, vsm@jamstec.go.jp
 Version 8 June 2009
  minor update 1/2011 replace new_GDS by GDS_change_no, WNE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <time.h>
#include <ctype.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "wgrib2nc.h"

//#define DEBUG_NC

#if defined USE_NETCDF3 || defined USE_NETCDF4
#include <netcdf.h>

// change by Sander Hulst 5/2011, for netcdf-4 library compiled without netcdf-4/hdf format 
#if defined USE_NETCDF3
#undef NC_NETCDF4
#endif

/* defined in Netcdf_sup.c */
extern g2nc_4Dlt nc_4Dlt[G2NC_NUM_4DLT];
extern g2nc_table * nc_table;
extern int nc_grads_compatible;
extern int nc_pack;
extern float nc_pack_offset;
extern float nc_pack_scale;
extern float nc_valid_min;
extern float nc_valid_max;
extern int nc_nlev;
extern double nc_date0;
extern int    nc_date0_type;  /* 1 for absolute, -1 for relative (alignment only) */
extern double nc_dt;          /* -1 will be used for variable (undefined) step */
extern int nc4;

/* defined in wgrib2.c */
extern int file_append;
extern int decode, latlon;
extern double *lat, *lon;
extern int nx, ny;
// extern unsigned in npnts;
extern enum output_order_type output_order;
extern int mode, GDS_change_no;

//int level2(int mode, int type1, float value1, int type2, float value2, int center, int subcenter,
//           char *inv_out);
static void get_nc_dims( int ncid, char * fname,
                  int * time_dim, int * time_var, int * time_ind,
                  double * verf_utime,  double * date0, int date0_type,
                  double * time_step, int *time_step_type,
                  int * y_dim, int * x_dim,
                  int * nlev, int * lev_dim, int * lev_var,
                  int * lev_type, int * lev_ind, int * lev_step,
                  int * dim_latlon, int *nx, int *ny, int* grid_template);
static int check_nc_latlon( int ncid, int y_dim, int x_dim, int dim_latlon );
static void create_nc_dims(int ncid, g2nc_4Dlt * lt_4D,
                  int * time_dim, int * time_var, int time_ind,
                  double time_step, int time_step_type,
                  double date0, double verf_utime, double ref_utime,
                  int * y_dim, int * x_dim,
                  int nlev, int * lev_dim, int * lev_var, int * lev_type,
                  int * lev_ind, int * lev_step,
                  g2nc_table * nc_table, int dim_latlon,
                  int grid_template, double dx, double dy);
static int get_nc_time_ind(int ncid, double verf_utime, int time_var, int time_dim,
                  int * time_ind, double * last_verf_time,
                  double * tm_step, int time_step_type, double date0 );
static int update_nc_ref_time(int ncid, double verf_utime, double ref_utime,
                       int time_var, int time_step_type, double time_step);
static int update_nc_lev( int ncid, g2nc_4Dlt * lt_4D,
                   float lev_val, int nlev,  int lev_dim, int lev_var,
                   int * lev_ind, int * lev_step, int * lev_type);
/******************************************************************************************/
static void netcdf_command(int status)
{
  if (status != NC_NOERR) fatal_error("netcdf error %s", nc_strerror(status));
}

/******************************************************************************************/
static void netcdf_command_plus(int status, const char * message)
{
  if (status != NC_NOERR)
  {
   fprintf(stderr,"\n*** wgrib2:netcdf: %s ***\n",message);
   fatal_error("netcdf error %s", nc_strerror(status));
  }
}

/******************************************************************************************/
static int g2nc_type2pack(int type)
{
  switch (type)
  {
    case NC_BYTE:
      return G2NC_PACK_BYTE;
    case NC_SHORT:
      return G2NC_PACK_SHORT;
    case NC_FLOAT:
      return G2NC_PACK_FLOAT;
    case NC_DOUBLE:
      return G2NC_PACK_DOUBLE;
    default:
      return G2NC_PACK_UNDEF;
  }
}
static int g2nc_pack2type(int pack)
{
  switch (pack)
  {
    case G2NC_PACK_BYTE:
      return NC_BYTE;
    case G2NC_PACK_SHORT:
      return NC_SHORT;
    case G2NC_PACK_FLOAT:
      return NC_FLOAT;
    case G2NC_PACK_DOUBLE:
      return NC_DOUBLE;
    case G2NC_PACK_UNDEF:  /*default - float*/
      return NC_FLOAT;
    default:
      return NC_NAT;
  }
}
/*_FillValue for different data types, could be used for packing (short,byte), max value*/
static short       sfill_value = G2NC_FILL_VALUE_SHORT;
static signed char bfill_value = G2NC_FILL_VALUE_BYTE;
static float       ffill_value = G2NC_FILL_VALUE_FLOAT;
// static double      dfill_value = G2NC_FILL_VALUE_DOUBLE;

/******************************************************************************************
 *
 * HEADER:100:netcdf:output:1:write netcdf data to X
 *
 *  As it seems, now wgrib2 options are activated in order
 *  as they are found on the command line.
 *  In this module all options-dependant actions are done when mode > 0,
 *  it guaranty that all global variables were assigned
 *
 */
int f_netcdf(ARG1)
{

  // define a struct to store local info across iterations...
  typedef struct {
    int initialized;
    int ncid;
    int time_dim;
    int time_var;
    int y_dim;
    int x_dim;
    int lev_dim;
    int lev_var;
    char* ncfile;       /* name of file */
    int time_ind;       /* max written to netcdf time step index, 0,1,...*/
    double verf_utime;  /* last written to file verftime value */
    int grads_compatible;
    double time_step;   /* used to check that step is const with '-nc_grads_compatible' option */
    int time_step_type; /* 0 for 'auto' and 1 for 'user' command-line defined value */
    double date0;       /* possible initial or relative user-defined date */
    int date0_type;     /* zero for automatic; 1 for absolute, -1 for relative (alignment only), depends from the command line option only! */
//    int file_append;   /* is real 'appending' going or it is first call and file is created? */
    int lev_type;      /* only one type allowed to be 4D data */
    int lev_ind;       /* max written to netcdf z-dim index, 0,1,...*/
    int lev_step;      /* to check that levels are going monotonically, save sign only */
    int nc_nlev;       /* max value of z-dim as when file was created */
    int nc_pack;       /* using specified packing */
    float nc_pack_scale, nc_pack_offset; /* packing options */
    float nc_valid_min, nc_valid_max;    /* test and packing options, secondary */
    g2nc_table *nc_table; /* user-defined conversion table */
    int free_nc_table;       /* as same table could be shared by differnt instances of netcdf - does free memory here? */
    int nx;
    int ny;
    int nid;           /* total number of fields written or updated in the netcdf file */
    int dim_latlon;    /* lat and lon coordinates dimension, 1 for COARDS or 2 for CF-1.0 conversion */
    int grid_template; /* need for not-mixing in one netcdf file of different grids with same nx,ny: excotic, but...*/
    int nc4;           /* write NetCDF4 version file */
    int endianness;    /* choice for NetCDF4 version file, 0,1,2 - native,little,big */
  } local_struct;
  local_struct* save;

  g2nc_4Dlt * lt_4D = NULL;
  g2nc_conv * nc_vc = NULL; /* variable conversion parameters */
  int varid, i, ok;
  unsigned int j;
  char varname_buf[_MAX_PATH],level_buf[_MAX_PATH], tmpname[_MAX_PATH];
  char name[_MAX_PATH], desc[_MAX_PATH], unit[_MAX_PATH];
  int time_ind;      /* current time step index to write in netcdf */
  int level_ind=-1;   /* current z-dim index to write in netcdf */
  size_t start[4];   /* record start location in 4D = {TIME0, LEV0, LAT0, LON0}; */
  size_t count[4];   /* record size in 4D = {TIMES, LEVS, LATS, LONS}; */
  int dimids[4],test_dimids[4];   //lev_types[2];

  size_t chunks[4];
  int shuffle, deflate, deflate_level;

  int year, month, day, hour, minute, second, err_code;
  int grid_template, ndims, ind_nct;
  double verf_utime, tm_step, dx, dy, ref_utime;
  int level_type1 = -1, level_type2 = -1;
  int undef_val1, undef_val2;
//  int center, subcenter;
  float level_val1=0, level_val2, dz, dz1; //level_val2=0,lev_vals[2], test_val1, test_val2;
  float scale_factor, add_offset, valid_min, valid_max;
  double max=0, min=0, range=0;
  nc_type var_type;
  int var_pack;
  float* fdata;
  short* sdata;
  signed char* bdata;
  const char *str;
  float dd;
  unsigned char *gds;

  if (mode == -1)
  {    //initialization
    *local = save = (local_struct *) malloc(sizeof(local_struct));
    if (!save) fatal_error("netcdf: %s","error doing malloc of save");
    save->ncfile = (char*) malloc(strlen(arg1)+1);
    if (!(save->ncfile)) fatal_error("netcdf: %s","error doing malloc of ncfile");
    strcpy(save->ncfile, arg1);
    decode = latlon = 1;
    save->initialized = 0; /* not yet */
    save->ncid = -1; /* do not created or open */
    save->nid = 0;
    /* initial time settings */
    save->time_step = nc_dt;    /* 0 if do not initialized, -1 if variable (undefined) */
    /* time_step_type == 0 for 'auto' and 1 for 'user' command-line defined value */
    if (nc_dt > G2NC_TM_TOLERANCE)
      save->time_step_type = 1;
    else
      save->time_step_type = 0; /* 0 for 'auto' and 1 for 'user' command-line defined value */

    save->date0 = nc_date0;            /* possible initial or relative user-defined date */
    save->date0_type = nc_date0_type;  /* 0 for automatic; 1 for absolute, -1 for relative (alignment only) */

    save->nc_pack = nc_pack;
    save->nc_pack_scale = nc_pack_scale;
    save->nc_pack_offset = nc_pack_offset;
    save->nc_valid_min = nc_valid_min;
    save->nc_valid_max = nc_valid_max;
    save->grid_template=-1;
    save->dim_latlon=0;
    save->lev_ind=-1;
    save->time_ind=-1;
    {
      const char * vlib;
      float nclib_ver;
      vlib = nc_inq_libvers();
      sscanf(vlib,"%f",&nclib_ver);

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: library version defined as = %f\n",nclib_ver);
fprintf(stderr,"netcdf: long version = %s\n",vlib);
#endif
      save->nc4 = 0;
      if(nc4)
      {
#ifdef NC_NETCDF4
        if(nclib_ver >= 4.0) save->nc4 = 1;
        else
        {
          fatal_error(
"netcdf: -nc4 option require wgrib2 to be compiled with NetCDF-4/HDF5/zlib libraries,\n was compiled with libnetcdf version %s",
          vlib);
        }
#else
        fatal_error(
"netcdf: -nc4 option require wgrib2 to be compiled with NetCDF-4/HDF5/zlib libraries,\n was compiled with libnetcdf version %s",
          vlib);
#endif
      }
    }
    if ( nc_table )
    {
      if ( nc_table->grads >= 0)  save->grads_compatible = nc_table->grads;
      else                        save->grads_compatible = nc_grads_compatible;
      if ( nc_table->nlev >= 0)   save->nc_nlev = nc_table->nlev;
      else                        save->nc_nlev = nc_nlev;
      if ( nc_table->used )       save->free_nc_table = 0;
      else                        save->free_nc_table = 1;
      if ( nc_table->endianness >= 0) save->endianness = nc_table->endianness;
      else                            save->endianness = 0;

      nc_table->used = 1;
      save->nc_table = nc_table;

      if (save->grads_compatible && nc_table->nlev > 2 && nc_table->lv)
      {
        dz1 = nc_table->lv[1]-nc_table->lv[0];
        for(i = 2; i < nc_table->nlev; i++)
        {
          dz = nc_table->lv[i]-nc_table->lv[i-1];
          if ( dz*dz1 <= 0 )
          {
            fatal_error(
            "netcdf: -nc_grads require monotonic order of $levs in -nc_table for %s",
            save->ncfile);
          }
        }
      }
    }
    else
    {
      save->grads_compatible = nc_grads_compatible;
      save->nc_nlev = nc_nlev;
      save->nc_table = NULL;
      save->free_nc_table = 0;
      save->endianness=0;
    }
//    save->file_append = 0;
    if (file_append)
    { /* try to open existing file for modifications,
         if open fails - create new file */
      ok = nc_open (save->ncfile, NC_WRITE|NC_SHARE, &(save->ncid));
      if (ok == NC_NOERR)
      { /* Locate dimensions and corresponding variables;
         * check does existing netcdf file structure is consistent
         * with the wgrib2-defined structure? Fatal_error if not.
         */
        get_nc_dims(save->ncid, save->ncfile,
          &(save->time_dim), &(save->time_var),
          &(save->time_ind), &(save->verf_utime),
          &(save->date0), save->date0_type,
          &(save->time_step), &(save->time_step_type),
          &(save->y_dim), &(save->x_dim),
          &(save->nc_nlev), &(save->lev_dim), &(save->lev_var),
          &(save->lev_type), &(save->lev_ind), &(save->lev_step),
          &(save->dim_latlon), &(save->nx), &(save->ny),
          &(save->grid_template));

        save->initialized=1;
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: initialized from file, nlev=%d, file=%s\n",
save->nc_nlev,save->ncfile);
#endif
      }
    }

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: initialization done, nc_file=%s\n",save->ncfile);
fprintf(stderr,"netcdf: nlev=%d, ncid=%d, initialized=%d\n",
save->nc_nlev,save->ncid,save->initialized);
#endif
  }
  else if (mode == -2)
  {  // cleanup
    save = (local_struct *) *local;
    if ( save->nc_table && !save->nid )
    {
      fprintf(stderr,
      "netcdf: WARNING! No data satisfying your selection criterias for %s!\n",save->ncfile);
      fprintf(stderr,"Check grib2 file, grep selection criteria(s) and -nc_table file\n");
    }
    else
    {
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: were added/updated %d fields, file %s\n",save->nid,save->ncfile);
#endif
    }
    if (save->ncid >=0) netcdf_command( nc_close(save->ncid));
    free(save->ncfile);
    if (save->free_nc_table)
      free_nc_table(save->nc_table);
    free(save);
  }
  else if (mode >= 0)
  {  // mode 0,1,2 for decoding a message.
    if (output_order != wesn && output_order != wens) fatal_error("netcdf: only works in we:sn order","");
    save = (local_struct *) *local;

//  if (new_GDS && save->initialized && save->nid ) fatal_error("netcdf: only one grid type at a time","");
    if ((GDS_change_no != 1) && save->initialized && save->nid ) fatal_error("netcdf: only one grid type at a time","");

    // Specify dimension of lat-lon coordinates (1 - COARDS, 2 - CF-1.0)
    dx = 1;
    dy = 1;
    grid_template = code_table_3_1(sec);
    if (save->initialized)
    {
      if (save->grid_template != grid_template)
      {
        fatal_error("netcdf: grid templates differs from this in netcd file %s",save->ncfile);
      }
    }
    else
    {
      if (grid_template == 0 || grid_template == 10 || grid_template == 40)
      {
        // COARDS convention only allows lat-lon, mercator and gaussian
        save->dim_latlon = 1; //"COARDS", rectangular non-rotated grids
      }
      else if (grid_template == 20 || grid_template == 30)
      {
        // CF convention allows much more...
        save->dim_latlon = 2; //"CF-1.0"; include here also rotated grids when becomes possible
        gds = sec[3];
        if (grid_template == 20)
        {
          dy = fabs(GDS_Polar_dy(gds));
          dx = fabs(GDS_Polar_dx(gds));
//      lov = GDS_Polar_lov(gds);
//      lad = GDS_Polar_lad(gds);
        }
        else if (grid_template == 30)
        {
          dy = fabs(GDS_Lambert_dy(gds));
          dx = fabs(GDS_Lambert_dx(gds));
//      lov = GDS_Lambert_Lov(gds);
//      lad = GDS_Lambert_Lad(gds); //etc.
        }
      }
      else {
        fprintf(stderr,"netcdf: doesn't support yet grid_template %d\n",grid_template);
        // fatal_error_i("netcdf: doesn't support grid_template %d",grid_template);
        save->dim_latlon = 2; //"CF-1.0"; include here also rotated grids when becomes possible
      }
      save->grid_template=grid_template;
    }
    if (save->dim_latlon > 1 && save->grads_compatible)
      fatal_error_i("netcdf: non-COARDS netCDF is not GrADS v1.9b4 compatible,\n"
                    "        unsupported grid_template=%d,\n"
                    "        but possibly could be open by GrADS with valid ctl (data description) file",
                    grid_template);

    // To avoid problems in future: used occasionally are ndata (main), npnts, nx and ny (extern).
    // For the eligible grid_templates ndata==npnts==nx*ny; does it needs to check it once more?

    /*
    What variable, levels we got?
    f_lev for some templates return 0 (OK) and do not write nothing to level_buf
    */
    level_buf[0]=0;
    // err_code = f_lev(mode, sec, data, ndata, level_buf, local);
    err_code = f_lev(call_ARG0(level_buf, NULL));

//    if (strcmp(level_buf, "reserved")==0) return(0);
    if (err_code || strlen(level_buf)==0 || strcmp(level_buf, "reserved")==0)
      fatal_error("netcdf: undefined or unsupported (reserved?) level: %s",level_buf);

    // WNE ok=getName(sec, mode, NULL, name, desc, unit);
    ok=getExtName(sec, mode, NULL, name, desc, unit,".","_");

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: Start processing of %s at %s\n", name, level_buf);
#endif

    if ( save->nc_nlev ) ndims = 4;  /* suppose 4D shape */
    else                 ndims = 3;  /* 3D shape */

    /* First check the 'ignore' keyword in the user defined vc (var conversion directives).
     * Look for the "exact" matching of name and level
     * in the user defined conversion table, then for 'any' level (*) */
    ind_nct = get_nc_conv_table(name, level_buf, save->nc_table);
    if ( ind_nct >= 0 ) nc_vc = save->nc_table->vc + ind_nct;
    else                nc_vc = NULL;
    if ( nc_vc )
    {
      if ( nc_vc->ignore )
      {
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: Variable/level rejected by user, found ignore keyword in nc_table\n");
#endif
        return(0);
      }
      /*
      Found exactly the given variable on givel level: special level,
      special name, remove from candidates for treating as 4D
      */
      ndims = 3;
    }
    else
    {  /* search this variable for 'any' level value '*' */
      i = get_nc_conv_table(name, "*", save->nc_table);
      if ( i >= 0 )
        if ( save->nc_table->vc[i].ignore )
        {
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: Variable at all levels (*) rejected by user, found ignore keyword in nc_table\n");
#endif
          return(0);
        }
    }
//    center = GB2_Center(sec);
//    subcenter = GB2_Subcenter(sec);
    err_code = verftime(sec, &year, &month, &day, &hour, &minute, &second);
    if(err_code) fatal_error("netcdf: can not get verftime","");

    verf_utime = get_unixtime(year, month, day, hour, minute, second, &err_code);
    if(err_code) fatal_error("netcdf: time [sec] is out of range for this OS","");

    i = reftime(sec, &year, &month, &day, &hour, &minute, &second);
    ref_utime = get_unixtime(year, month, day, hour, minute, second, &err_code);
    if (err_code) fatal_error("netcdf: time out of range for presentation in sec on this system","");

    time_ind=-1;
    if (verf_utime+G2NC_TM_TOLERANCE < save->date0 )
    { /* time is before date0 */
      if (save->date0_type > 0 )   return 0; /* skip silently */
      else if (save->initialized ) fatal_error("netcdf: input time before initial in file %s",save->ncfile);
    }
    if (save->time_step_type &&                   /* time_step is user def and */
       (save->date0_type || save->initialized))   /* date0 is already defined */
    {
      time_ind = (int)((verf_utime - save->date0)/save->time_step+G2NC_TM_TOLERANCE);
      if (fabs(save->date0+save->time_step*time_ind-verf_utime) > G2NC_TM_TOLERANCE)
      {
//        if (save->date0_type ) { /*not auto type, +-1*/
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: skip as do not time-aligned data, ind=%.2lf\n",
(verf_utime - save->date0)/save->time_step);
#endif
           return 0; /* non-integer time index, skip silently */
//        }
//        else fatal_error("netcdf: input time do not time-aligned to initial and time step in file %s",save->ncfile);
      }
      /* Check for too large gap in data */
      if (( save->initialized && abs(save->time_ind-time_ind) > G2NC_TM_IND_GAP ) ||
          (!save->initialized && save->date0_type > 0 && time_ind > G2NC_TM_IND_GAP ))
      {
        fatal_error("netcdf: too large gap in time index %s",save->ncfile);
      }
    }

    if ( !save->initialized )
    {
      save->nx = nx;
      save->ny = ny;
    }
    /* Do next check for each new field added to the netcdf file */
    if (nx != save->nx || ny != save->ny )
    {
      fprintf(stderr,"netcdf: error processing %s at %s\n", name, level_buf);
      fprintf(stderr,"netcdf: existing grid (nx*ny) %d x %d, new grid %d x %d\n",
       save->nx,save->ny,nx,ny);
      fatal_error("netcdf: grid size must be same for all variables","");
    }
    /* work around z-dim:
     * is this data eligible for treating as 4D?
     * which level it is etc. Used code from Level.c */
    if ( save->nc_table ) lt_4D = save->nc_table->lt;   /* still could be NULL */
    else lt_4D = NULL;

    fixed_surfaces(sec,&level_type1,&level_val1,&undef_val1,&level_type2,&level_val2,&undef_val2);
//    level_type1 = sec[4][22];
//    level_type2 = sec[4][28];
//    level_type1 = code_table_4_5a(sec);
//    level_type2 = code_table_4_5b(sec);
//    level_val1 = scaled2flt(INT1(sec[4][23]), int4(sec[4] + 24));
//    level_val2 = scaled2flt(INT1(sec[4][29]), int4(sec[4] + 30));

    if ( ndims == 4 )
    {
      /* search level_type1 index in nc_4Dlt; if find - treat data as 4D */
      if (level_type2 == 255)
      {//must be undefined, now only single-level data are supported as 4D
        if ( lt_4D )
        {
          if (lt_4D->type == level_type1 ) level_val1 *= lt_4D->scale;
          else lt_4D = NULL;
        }
        else
        { /* look in global conversion table */
          for (i=0; i < G2NC_NUM_4DLT; i++)
          {
            if (nc_4Dlt[i].type == level_type1)
            {
              level_val1 *= nc_4Dlt[i].scale;
              lt_4D = (g2nc_4Dlt *) &(nc_4Dlt[i]);
              break;
            }
          }
        }
      }
    }
    if ( !lt_4D ) ndims = 3; /* process data as 3D */

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: ndims=%d, levt1=%d, levt2=%d level_val1*scale=%g\n",
ndims,level_type1,level_type2, level_val1);
#endif

    if ( ndims == 4 )
    {
      if ( save->nc_table )
      {
        if( save->nc_table->lv )
        { /* user provided list of levels, check there */
          /* level value test criteria dd */
          if (fabs(level_val1) < G2NC_LV_TOLERANCE) dd = G2NC_LV_TOLERANCE;
          else dd = fabs(level_val1)*G2NC_LV_TOLERANCE;

          level_ind = -1;
          for (i = 0; i < save->nc_table->nlev; i++)
          {
            if ( fabs(save->nc_table->lv[i]-level_val1) < dd )
            {
              level_ind = i;
              break;
            }
          }
          if ( level_ind < 0 )
          {
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: skip this field as not at one of user-defined levels: lev=%g\n",level_val1);
#endif
//          fatal_error("netcdf: 4D data are not on user-defined level","");
            return 0;
          }
        }
      }
      /* look for user-defined name for variables at 'any' level, only 4D data*/
      if ( !nc_vc )
      {
        ind_nct = get_nc_conv_table(name,"*",save->nc_table);
        if ( ind_nct >= 0 ) nc_vc = save->nc_table->vc + ind_nct;
      }
    }

    if ( nc_vc )
    {
      /* it is user responsibility to provide legal variable names */
      strcpy(varname_buf,nc_vc->nc_name);
    }
    else
    {

      if (mode > 0) fprintf(stderr,"initial netcdf name=%s\n", name);
      /* code to make variable CF compliant [a-zA-Z][a-zA-Z0-9_]* , note netcdf allows - and leading _ */
      tmpname[0] = 0;
//      if (isdigit(name[0])) strcpy(tmpname, "N");  // no names starting with digit
      if (! isalpha((unsigned char) name[0])) strcpy(tmpname, "N");  // names must start with a-z or A-Z
      strcat(tmpname, name);

      if ( ndims == 3 )
      {
        strcat(tmpname, "_");
        strcat(tmpname, level_buf);
      }

      // get rid of illegal characters for netcdf names

      i = j = 0;
      while (tmpname[i]) {
	
	if (tmpname[i] == ' ') { i++; continue; }

        if (match_str(tmpname+i, ">=")) {
	    varname_buf[j++] = '_'; 
	    varname_buf[j++] = 'G'; 
	    varname_buf[j++] = 'E'; 
	    varname_buf[j++] = '_'; 
	    i+=2; 
	}
        else if (match_str(tmpname+i, ">")) {
	    varname_buf[j++] = '_'; 
	    varname_buf[j++] = 'G'; 
	    varname_buf[j++] = 'T'; 
	    varname_buf[j++] = '_'; 
	    i+=1; 
	}
        else if (match_str(tmpname+i, "<=")) {
	    varname_buf[j++] = '_'; 
	    varname_buf[j++] = 'L'; 
	    varname_buf[j++] = 'E'; 
	    varname_buf[j++] = '_'; 
	    i+=2; 
	}
        else if (match_str(tmpname+i, "<")) {
	    varname_buf[j++] = '_'; 
	    varname_buf[j++] = 'L'; 
	    varname_buf[j++] = 'T'; 
	    varname_buf[j++] = '_'; 
	    i+=1; 
	}
        else if (match_str(tmpname+i, "=")) {
	    varname_buf[j++] = '_'; 
	    varname_buf[j++] = 'E'; 
	    varname_buf[j++] = 'Q'; 
	    varname_buf[j++] = '_'; 
	    i+=1; 
	}

	/* -ve sign */
        else if (match_str(tmpname+i, "-")) {
	    varname_buf[j++] = 'M'; 
	    i+=1; 
	}

	/* +ve sign */
        else if (match_str(tmpname+i, "+")) {
	    varname_buf[j++] = 'P'; 
	    i+=1; 
	}

	/* period */
        else if (match_str(tmpname+i, ".")) {
	    /* if period number, then replace with a D */
	    if (isdigit((unsigned char) tmpname[i+1]))  varname_buf[j++] = 'D';
	    else varname_buf[j++] = '_'; 
	    i+=1; 
	}

	else if (isalnum((unsigned char) tmpname[i])) { 
	    varname_buf[j++] = tmpname[i++];
	}
	else { 
	    varname_buf[j++] = '_'; 
	    i++;
	}
      }
      varname_buf[j] = 0;

      /* get rid of double underscores */
      j = i = 0;
      while (varname_buf[i]) {
	if (varname_buf[i] != '_' || varname_buf[i+1] != '_') varname_buf[j++] = varname_buf[i];
	i++;
      }
      varname_buf[j] = 0;

    }

    if ( !save->initialized )
    { /* first record in new file */
      if (save->date0_type <= 0)
      {/* undef or alignment requested (was checked earlier) */
        time_ind = 0; /* first record */
        save->date0 = verf_utime;
      }
      else
      {
        if (save->time_step_type == 0)
        {/* step undefined (auto), but defined date0 */
          if (fabs(save->date0-verf_utime) > G2NC_TM_TOLERANCE )
          { /* first time is not user-defined start time date0 */
            time_ind = 1; /* pass first and write data as the second record */
            save->time_step = verf_utime - save->date0;
          }
        }
        else
          time_ind = (int)((verf_utime - save->date0)/(save->time_step)+G2NC_TM_TOLERANCE);
      }
//    else it was already defined (know date0 and step)
      save->time_ind = time_ind;
      save->verf_utime = verf_utime;
      // define NEW nc file, assume nx and ny of the 1st message hold for all messages...
    // get some info for grid_mapping
#ifdef NC_NETCDF4
      if (save->nc4)
        netcdf_command( nc_create(save->ncfile, NC_NETCDF4, &(save->ncid)) );
      else
        netcdf_command( nc_create(save->ncfile, NC_CLASSIC_MODEL, &(save->ncid)) );
#else
      netcdf_command( nc_create(save->ncfile, 0, &(save->ncid)) );
#endif
      create_nc_dims(save->ncid, lt_4D,
          &(save->time_dim), &(save->time_var), save->time_ind,
          save->time_step, save->time_step_type,
          save->date0, save->verf_utime, ref_utime,
          &(save->y_dim), &(save->x_dim),
          save->nc_nlev, &(save->lev_dim), &(save->lev_var),
          &(save->lev_type), &(save->lev_ind), &(save->lev_step),
          save->nc_table, save->dim_latlon,
          save->grid_template, dx, dy);

      save->initialized = 1;
    }
    else
    { /* check data in open netcdf file */
      tm_step = save->time_step;
      time_ind = get_nc_time_ind(save->ncid, verf_utime,
                    save->time_var, save->time_dim, &(save->time_ind),
                    &(save->verf_utime), &tm_step,
                    save->time_step_type, save->date0 );

      if(time_ind < 0) fatal_error("netcdf: unresolved timing problem","");
      if (save->grads_compatible)
      {
        if( tm_step > G2NC_TM_TOLERANCE )
        {
          if( save->time_step > G2NC_TM_TOLERANCE )
          {
            if (fabs(save->time_step-tm_step) > G2NC_TM_TOLERANCE)
            {
              fatal_error("netcdf: variable time stepping is not GrADS v1.9b4 compatible","");
            }
          }
          else save->time_step = tm_step;
        }
      }

      i = update_nc_ref_time(save->ncid, verf_utime, ref_utime,
                           save->time_var, save->time_step_type, tm_step);
   /*
    * Check does existing lats lons in netcdf file are same
    * as this of new field?
    */
      i = check_nc_latlon(save->ncid, save->y_dim, save->x_dim, save->dim_latlon);
    }
//vsm: seems now next code is not used at all, even for debugging...
/*
    // f_ftime(mode, sec, data, ndata, ftime_buf, local);
    ftime_buf[0] = 0;
    f_ftime(call_ARG0(ftime_buf, NULL));

    sprintf(vt_buf,"%.4d%.2d%.2d%.2d", year,month,day,hour);
    p = sec[1];
    sprintf(btime_buf,"%.4d%.2d%.2d%.2d", (p[12]<<8)+p[13], p[14],p[15],p[16]);
*/

    //netcdf is open or created and defined, data do not rejected, go ahead...

    /* Set chunking, shuffle, and deflate. */
    shuffle = 1;
    deflate = 1;
    deflate_level = 1;

    dimids[0] = save->time_dim;
    chunks[0] = 1;
    if ( ndims == 4 )
    {
      level_ind = update_nc_lev( save->ncid, lt_4D,
                  level_val1, save->nc_nlev, save->lev_dim, save->lev_var,
                  &(save->lev_ind), &(save->lev_step),
                  &(save->lev_type) );

      if ( level_ind < 0 )
      {
        fatal_error("netcdf: 4D z-dim indexing error %s",save->ncfile);
      }
      dimids[1] = save->lev_dim;
      dimids[2] = save->y_dim;
      dimids[3] = save->x_dim;
      chunks[1] = 1;
      chunks[2] = ny;
      chunks[3] = nx;
    }
    else
    {
      dimids[1] = save->y_dim;
      dimids[2] = save->x_dim;
      chunks[1] = ny;
      chunks[2] = nx;
    }
// fprintf(stderr,"netcdf: varname=(%s) >> nc_inq_varid\n", varname_buf);
    ok = nc_inq_varid (save->ncid, varname_buf, &varid);
//    if (ok != NC_NOERR) {
//	fprintf(stderr,"netcdf: bad variable name=%s\n", varname_buf);
//    }
    var_pack = G2NC_PACK_UNDEF;
    scale_factor = 0.;
    if (ok != NC_NOERR)
    {
      varid = -1; /* new variable, not in file yet */
      if ( nc_vc )
      {
        var_pack = nc_vc->nc_pack;
        scale_factor = nc_vc->nc_scale;
        add_offset = nc_vc->nc_offset;
        valid_min = nc_vc->nc_valid_min;
        valid_max = nc_vc->nc_valid_max;
      }
      else if (save->nc_pack)
      {
        var_pack = save->nc_pack;
        scale_factor = save->nc_pack_scale;
        add_offset = save->nc_pack_offset;
        valid_min = save->nc_valid_min;
        valid_max = save->nc_valid_max;
      }
    }
    else
    {  /* same variable found in open netcdf file */
      netcdf_command( nc_inq_vartype (save->ncid, varid, &var_type) );
      /* Check existing variable definition in netcdf with requested */
      if ( nc_vc ) var_pack = nc_vc->nc_pack;
      else if (nc_pack) var_pack = save->nc_pack;
//      if ( var_type == NC_FLOAT ) var_type = 0;  //Jan 2007, vsm
      if ( var_pack != G2NC_PACK_UNDEF && var_type != g2nc_pack2type(var_pack) )
      {
        fatal_error("netcdf: variable %s exists with other packing size",varname_buf);
      }
      netcdf_command( nc_inq_varndims (save->ncid, varid, &i) );
      if ( i != ndims )
      {
        fprintf(stderr,"netcdf: ndims=%d in file, new ndims=%d\n",i,ndims);
        fatal_error("netcdf: variable %s exists having different dimension",varname_buf);
      }
      test_dimids[0]=test_dimids[1]=test_dimids[2]=test_dimids[3]=-1;
      netcdf_command( nc_inq_vardimid (save->ncid, varid, test_dimids) );
      for ( i = 0; i < ndims; i++ )
      {
        if (test_dimids[i] != dimids[i])
        {
          fatal_error("netcdf: variable %s exists with different dimension shape",varname_buf);
        }
      }
      /* packing information is taken from the open netcdf file */
      if ( var_type == NC_BYTE || var_type == NC_SHORT )
      {
        ok = nc_get_att_float(save->ncid, varid, "scale_factor", &scale_factor);
        if (ok == NC_NOERR)
        {
          ok = nc_get_att_float(save->ncid, varid, "add_offset", &add_offset);
          if (ok == NC_NOERR) var_pack = g2nc_type2pack(var_type);
        }
        if (var_pack != g2nc_type2pack(var_type))
          fatal_error("netcdf: wrong or absent packing info for SHORT or BYTE var=%s,\n"
                      "        invalid netCDF file",varname_buf);
      }
      else if (var_type == NC_FLOAT)
      {
        // next attributes COULD present
        ok = nc_get_att_float(save->ncid, varid, "valid_min", &valid_min);
        if (ok == NC_NOERR)
        {
          ok = nc_get_att_float(save->ncid, varid, "valid_max", &valid_max);
          if (ok == NC_NOERR) var_pack = G2NC_PACK_FLOAT; // will check the valid_range
        }
      }
    }
    if (var_pack == G2NC_PACK_BYTE && save->grads_compatible)
      fatal_error("netcdf: byte packing is not GrADS v1.9b4 compatible, var=%s,\n"
                  "        but such netCDF file still could be open by GrADS\n"
                  "        with valid ctl (data description) file", varname_buf);

//    if ( var_pack ) {
      // Find max-min for checking of packing parameters
      // vsm: From Jan 2007 version values that do not fit to provided valid_range
      // are replaced by UNDEFINED
    if ( scale_factor == 0. && (var_pack == G2NC_PACK_BYTE || var_pack == G2NC_PACK_SHORT) )
    {
      max = min = ok = 0;
      for (j = 0; j < ndata; j++)
      {
        if (!UNDEFINED_VAL(data[j]))
        {
          if (ok)
          {
            max = max > data[j] ? max : data[j];
            min = min < data[j] ? min : data[j];
          }
          else
          {
            ok = 1;
            max = min = data[j];
          }
        }
      }
    }
    if (varid < 0)
    { /* new variable, not in file yet */

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: add new var=%s to output file %s\n",varname_buf,save->ncfile);
#endif
      netcdf_command( nc_redef(save->ncid) );

      /* try to apply provided packing to all new variables */
      if ( var_pack == G2NC_PACK_BYTE )
      {
        netcdf_command( nc_def_var (save->ncid, varname_buf, NC_BYTE, ndims, dimids, &varid) );
        netcdf_command( nc_put_att_schar(save->ncid, varid, "_FillValue", NC_BYTE, 1, &bfill_value) );
      }
      else if ( var_pack == G2NC_PACK_SHORT )
      {
        netcdf_command( nc_def_var (save->ncid, varname_buf, NC_SHORT, ndims, dimids, &varid) );
        netcdf_command( nc_put_att_short(save->ncid, varid, "_FillValue", NC_SHORT, 1, &sfill_value) );
      }
      else
      { /* default case of unpacked float variables */
        netcdf_command( nc_def_var (save->ncid, varname_buf, NC_FLOAT, ndims, dimids, &varid) );
        netcdf_command( nc_put_att_float(save->ncid, varid, "_FillValue", NC_FLOAT, 1, &ffill_value) );
      }
//vsm: which 'short name' is correct for the NetCDF COARDS compliant file? 'name' or 'varname_buf'?
//      netcdf_command( nc_put_att_text(save->ncid, varid, "short_name", strlen(name), name) );
      netcdf_command( nc_put_att_text(save->ncid, varid, "short_name", strlen(varname_buf), varname_buf) );
      netcdf_command( nc_put_att_text(save->ncid, varid, "long_name", strlen(desc), desc) );
/*
      lev_types[0]=level_type1;
      lev_types[1]=level_type2;
      netcdf_command( nc_put_att_int(save->ncid, varid, "grib2_lt", NC_INT, 2, lev_types) );
*/
      if ( ndims == 4 )
      {
        netcdf_command( nc_put_att_text(save->ncid, varid, "level",
           strlen(lt_4D->lname), lt_4D->lname) );
      }
      else
      {
        netcdf_command( nc_put_att_text(save->ncid, varid, "level", strlen(level_buf), level_buf) );
/*
As an option, could get extended description from f_lev...
        lev_vals[0]=level_val1;
        lev_vals[1]=level_val2;
        netcdf_command( nc_put_att_float(save->ncid, varid, "grib2_lv", NC_FLOAT, 2, lev_vals) );
        ok = level2(mode, level_type1, level_val1, level_type2, level_val2,
                center, subcenter, desc);
        if ( strcmp(desc, level_buf) )
          netcdf_command( nc_put_att_text(save->ncid, varid, "level_description",
                          strlen(desc), desc) );
*/
      }
      fix_units(unit,sizeof(unit));
      netcdf_command( nc_put_att_text(save->ncid, varid, "units", strlen(unit), unit) );

      if (save->dim_latlon > 1 )
      {
        str="longitude latitude";
        netcdf_command( nc_put_att_text(save->ncid, varid, "coordinates", strlen(str), str) );
      }
      if ( var_pack == G2NC_PACK_BYTE || var_pack == G2NC_PACK_SHORT )
      {
        if ( scale_factor == 0.)
        {
          /* Auto-packing, center near 0 as signed char or short are used.
             Try secure scaling for all next input fields: extend the range on +-10%... */
          range = (max-min)*1.2;
          add_offset = (float) (min+max)*0.5;
          if ( var_pack == G2NC_PACK_BYTE )
            scale_factor = (float) (range/(bfill_value - 2))*0.5;
          else if ( var_pack == G2NC_PACK_SHORT )
            scale_factor = (float) (range/(sfill_value - 2))*0.5;
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: auto-packing, min=%lf, max=%lf, offset=%f, scale=%f, var=%s\n",
min, max, add_offset, scale_factor, varname_buf);
#endif
        }
        netcdf_command( nc_put_att_float(save->ncid, varid, "scale_factor", NC_FLOAT, 1, &scale_factor) );
        netcdf_command( nc_put_att_float(save->ncid, varid, "add_offset", NC_FLOAT, 1, &add_offset) );
      }
      else if ( var_pack == G2NC_PACK_FLOAT )
      {
        netcdf_command( nc_put_att_float(save->ncid, varid, "valid_min", NC_FLOAT, 1, &valid_min) );
        netcdf_command( nc_put_att_float(save->ncid, varid, "valid_max", NC_FLOAT, 1, &valid_max) );
      }
#ifdef NC_NETCDF4
      if (save->nc4)
      {
        netcdf_command( nc_def_var_chunking(save->ncid, varid, NC_CHUNKED, chunks) );
        netcdf_command( nc_def_var_deflate( save->ncid, varid, shuffle, deflate, deflate_level) );
//        netcdf_command( nc_var_set_cache(save->ncid, varid, nx*ny);
//        netcdf_command( nc_def_var_endian(save->ncid, varid, endian) );
//        NC_ENDIAN_NATIVE,NC_ENDIAN_LITTLE,NC_ENDIAN_BIG
      }
#endif
      netcdf_command( nc_enddef(save->ncid) );
    }

    // specify where to write new data portion - record size and location in 4D or 3D
    start[0] = time_ind; count[0] = 1;
    if ( ndims == 4 )
    {
      start[1] = level_ind; count[1] = 1;
      start[2] = 0;         count[2] = ny;
      start[3] = 0;         count[3] = nx;
    }
    else
    {
      start[1] = 0;  count[1] = ny;
      start[2] = 0;  count[2] = nx;
    }
    // pack data and write one time step
    if ( var_pack == G2NC_PACK_BYTE || var_pack == G2NC_PACK_SHORT )
    { /* get packing attributes from file */
      netcdf_command( nc_get_att_float(save->ncid, varid, "scale_factor", &scale_factor) );
      netcdf_command( nc_get_att_float(save->ncid, varid, "add_offset", &add_offset) );
      if ( fabs(scale_factor) < 1e-20) fatal_error("netcdf: zero packing scale factor %s",varname_buf);
/* vsm: From version Jan 2007 replace data out of range by UNDEFINED
      // stop if data do not fit to with packing parameters:
      test_val1 = (min - add_offset) / scale_factor;
      test_val2 = (max - add_offset) / scale_factor;
      if ( var_pack == G2NC_PACK_BYTE )
      {
        if ( test_val1 <= -bfill_value || test_val2 >= bfill_value )
        {
          fprintf(stderr,"values: %lf %lf %f %f %f %f %d %d\n",
          min,max,test_val1,test_val2,add_offset,scale_factor,(int)(-bfill_value),(int)bfill_value );
          fatal_error("netcdf: values do not fit byte packing: %s",varname_buf);
        }
*/
      if ( var_pack == G2NC_PACK_BYTE )
      {
        valid_min = -(bfill_value-2)*scale_factor + add_offset;
        valid_max =  (bfill_value-2)*scale_factor + add_offset;
        bdata = (signed char*) malloc(sizeof(signed char) * (size_t) ndata);
        if (!bdata) fatal_error("netcdf: %s","error doing malloc of bdata");
        for (j = 0; j < ndata; j++)
        {
          bdata[j] = (UNDEFINED_VAL(data[j]) || data[j] < valid_min || data[j] > valid_max ) ?
             bfill_value : (data[j] - add_offset) / scale_factor;
        }
        netcdf_command( nc_put_vara_schar(save->ncid, varid, start, count, bdata) );
        free(bdata);
      }
      else if (var_pack == G2NC_PACK_SHORT )
      {
/*
        if ( test_val1 <= -sfill_value || test_val2 >= sfill_value )
        {
          fprintf(stderr,"values: %lf %lf %f %f %f %f %d %d\n",
          min,max,test_val1,test_val2,add_offset,scale_factor,(int)(-sfill_value),(int)sfill_value );
          fatal_error("netcdf: values do not fit short packing: %s",varname_buf);
        }
*/
        valid_min = -(sfill_value-2)*scale_factor + add_offset;
        valid_max =  (sfill_value-2)*scale_factor + add_offset;
        sdata = (short*) malloc(sizeof(short) * (size_t) ndata);
        if (!sdata) fatal_error("netcdf: %s","error doing malloc of sdata");
        for (j = 0; j < ndata; j++)
        {
          sdata[j] = (UNDEFINED_VAL(data[j]) || data[j] < valid_min || data[j] > valid_max ) ?
            sfill_value : (data[j] - add_offset) / scale_factor;
        }
        netcdf_command( nc_put_vara_short(save->ncid, varid, start, count, sdata) );
        free(sdata);
      }
    }
    else
    { /* not packed float values */
      if (var_pack == G2NC_PACK_FLOAT)
      { // check valid_range
        fdata = (float*) malloc(sizeof(float) * (size_t) ndata);
        if (!fdata) fatal_error("netcdf: %s","error doing malloc of fdata");
        for (j = 0; j < ndata; j++)
        {
          fdata[j] = (UNDEFINED_VAL(data[j]) || data[j] < valid_min || data[j] > valid_max ) ?
            ffill_value : data[j];
        }
        netcdf_command( nc_put_vara_float(save->ncid, varid, start, count, fdata) );
        free(fdata);
      }
      else
      {
        netcdf_command( nc_put_vara_float(save->ncid, varid, start, count, data) );
      }
    }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf: added var=%s to output file %s\n",varname_buf,save->ncfile);
fprintf(stderr,"netcdf: varid=%d, time_ind=%d, lev_ind=%d, pack=%d\n",
varid,time_ind,level_ind,var_pack);
#endif
    save->nid++;
  }
  return 0;
}

/******************************************************************************************/

static void get_nc_dims( int ncid, char * fname,
                  int * time_dim, int * time_var, int * time_ind,
                  double * verf_utime,  double * date0, int date0_type,
                  double * time_step, int *time_step_type,
                  int * y_dim, int * x_dim,
                  int * nlev, int * lev_dim, int * lev_var,
                  int * lev_type, int * lev_ind, int * lev_step,
                  int * dim_latlon, int *nx, int *ny, int* grid_template)
{
 /*
  * Get dimension information from existing netcdf file
  * Sergey Varlamov
  */
  double tmp_val;
  size_t nyy, nxx, ntt, nzz;
  int nco_err, ok, i, lat_var, lon_var;
  float * test_lv, dd, fill_value;
  const float test_d=1e-6;
  char *lev_name, *attr;
  const char *string;
  size_t start[2], count[2];

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: step in get_nc_dims\n");
#endif

  nco_err=0;
  nzz = 0;
  /* locate dimensions and corresponding variables with fixed names */
  string="-append: existing netcdf file has wgrib2-inconsistent structure. Check input, remove or change file name.";
  netcdf_command_plus( nc_inq_dimid(ncid, "time", time_dim), string );
  netcdf_command_plus( nc_inq_varid(ncid, "time", time_var), string );
  /* get the number of records written so far */
  netcdf_command( nc_inq_dimlen(ncid, *time_dim, &ntt) );
  netcdf_command_plus( nc_get_att_double(ncid, *time_var, "time_step", &tmp_val), string );
  netcdf_command_plus( nc_inq_attlen (ncid, *time_var, "time_step_setting", count), string );
  if (*count < 4) fatal_error("netcdf:get_nc_dims: %s","small time_step_setting  attribute size in existing netcdf file");
  attr = (char*) malloc((*count + 2)*sizeof(char));
  if (!attr) fatal_error("netcdf:get_nc_dims: %s","error doing malloc of attr");
  netcdf_command( nc_get_att_text(ncid, *time_var, "time_step_setting", attr) );
  attr[4]=0;
  if ( strcmp(attr,"user")==0 )
  { /* time step in file is surely defined */
    if (*time_step_type && fabs(*time_step-tmp_val) > G2NC_TM_TOLERANCE)
    {
      fatal_error("netcdf:get_nc_dims: new user-defined time_step conflicts this in netcdf file","");
    }
    *time_step_type=1;
  }
  else if ( strcmp(attr,"auto")==0 )
  {
    if (*time_step_type)
    { /* user define new */
      if(ntt > 1 && fabs(*time_step-tmp_val) > G2NC_TM_TOLERANCE)
      {
        fatal_error("netcdf:get_nc_dims: new user-defined time_step conflicts with this in netcdf file","");
      }
      else
      {
        tmp_val = *time_step; /* take new value */
        netcdf_command( nc_put_att_double(ncid, *time_var, "time_step", NC_DOUBLE, 1, &tmp_val) );
      }
      /* re-define attribute value to "user" */
      netcdf_command( nc_put_att_text(ncid, *time_var, "time_step_setting", 4, "user") );
    }
  }
  else fatal_error("netcdf:get_nc_dims: undefined time_step_setting attribute value: %s",attr);
  *time_step = tmp_val; /* could still be undefined */
  free(attr);
  /* In an existing file first time value must be defined */
  if (ntt <= 0)
  {
    fprintf(stderr,"%s\n",string);
    fatal_error("netcdf:get_nc_dims: time variable is undefined (size==0), %s", fname);
  }
  start[0] = 0;
  netcdf_command( nc_get_var1_double(ncid, *time_var, start, &tmp_val));
  if (date0_type > 0)
  { /*new user defined, get old and compare */
    if (fabs(tmp_val-*date0) > G2NC_TM_TOLERANCE)
    {
      fprintf(stderr,"%s\n",string);
      fatal_error("netcdf:get_nc_dims: user date0 is not same as this in file, %s", fname);
    }
  }
  else if (date0_type < 0)
  { /* user requested alignment */
    /* time_step must be defined on input or in file, do not check for zero */
    i=(int)((*date0-tmp_val)/(*time_step)+G2NC_TM_TOLERANCE);
    if (fabs((*time_step)*i+tmp_val-*date0) > G2NC_TM_TOLERANCE )
    {
      fprintf(stderr,"%s\n",string);
      fatal_error("netcdf:get_nc_dims: user date0,dt are not aligned with date0 in file, %s", fname);
    }
  }
  *date0=tmp_val;
  *time_ind = -1; /* undefined */
  *verf_utime = 0;
  if (ntt > 0)
  { /* if file is not empty - save last time index, get time value and step */
    *time_ind = ntt-1;
    start[0] = *time_ind;
    netcdf_command( nc_get_var1_double(ncid, *time_var, start, verf_utime) );
  }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:get_nc_dims: time(dim,var,nt,ind,val)=%d %d %lu %d %.1lf\n",
*time_dim,*time_var,(unsigned long)ntt,*time_ind,*verf_utime);
{
  char tmp_str[50];
  fprintf(stderr,"netcdf:get_nc_dims: date=%s\n",get_unixdate(*verf_utime, tmp_str));
}
#endif
  //  else nco_err+=1;
  /* get some global attributes */
  netcdf_command( nc_get_att_int (ncid, NC_GLOBAL, "GRIB2_grid_template", grid_template) );
  netcdf_command( nc_inq_varid (ncid, "latitude", &lat_var) );
  netcdf_command( nc_inq_varid (ncid, "longitude", &lon_var) );
  netcdf_command( nc_inq_varndims (ncid, lat_var, dim_latlon) );
  netcdf_command( nc_inq_varndims (ncid, lon_var, &i) );

  if (i != *dim_latlon || *dim_latlon > 2 )
  { /* How about dimension == 0 (scalar) ? */
    fprintf(stderr,"netcdf: processing file %s\n", fname);
    fatal_error_i("netcdf:get_nc_dims: unsupported dimension of lat-lon variables: %d", *dim_latlon);
  }
  if (*dim_latlon == 1)
  {
    // expect lat and lon dimension variables
    netcdf_command( nc_inq_dimid (ncid, "latitude",  y_dim) );
    netcdf_command( nc_inq_dimid (ncid, "longitude", x_dim) );
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:get_nc_dims: lon=%d lat=%d\n",*x_dim,*y_dim);
#endif
  }
  else
  {
    // expect x and y dimension + latitude and longitude variables
    netcdf_command( nc_inq_dimid (ncid, "y", y_dim) );
    netcdf_command( nc_inq_dimid (ncid, "x", x_dim) );
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:get_nc_dims: X=%d Y=%d\n",*x_dim,*y_dim);
#endif
  }
  netcdf_command( nc_inq_dimlen (ncid, *y_dim, &nyy) );
  netcdf_command( nc_inq_dimlen (ncid, *x_dim, &nxx) );
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:get_nc_dims: nx=%lu ny=%lu\n",
(unsigned long)nxx,(unsigned long)nyy);
#endif

  *nx=nxx;
  *ny=nyy;

  *lev_ind = -1; /* undefined */
  *lev_step = 0; /* undefined */
  if (*nlev > 0)
  {
    netcdf_command( nc_inq_ndims (ncid, &i) );
    if (i < 4)
    {
      fprintf(stderr,"requested 4D output to existing %dD netcdf file\n",i);
      nco_err+=32;
    }
    else
    { /* find which is z-dimension?*/
      *lev_dim=-1;
      for (i=0;i<4;i++)
      {
        if (*time_dim == i)continue;
        if (*y_dim == i)continue;
        if (*x_dim == i)continue;
        *lev_dim = i;
        break;
      }
      if (*lev_dim >= 0)
      {
        netcdf_command( nc_inq_dimlen (ncid, *lev_dim, &nzz) );
        if (nzz >= *nlev)
        { /* get lev_name, lev_var and lev_type from file */
          lev_name = (char*) malloc(NC_MAX_NAME+1);
          if (!lev_name) fatal_error("netcdf:get_nc_dims: %s",
            "error doing malloc of lev_name");
          netcdf_command( nc_inq_dimname (ncid, *lev_dim, lev_name) );
          /* Look for the same-named variable.
             It could be still default undefined (lev_type==-1)
             -this seems to be normal  */
          netcdf_command( nc_inq_varid (ncid, lev_name, lev_var) );
          netcdf_command( nc_get_att_int(ncid, *lev_var, "lev_type", lev_type) );

          test_lv = (float*) malloc(nzz*sizeof(float));
          if (!test_lv) fatal_error("netcdf:get_nc_dims: %s",
            "error doing malloc of test_lv for z_lev");

          start[0] = 0; count[0] = nzz;
          netcdf_command( nc_get_vara_float(ncid, *lev_var, start, count, test_lv) );

          ok = nc_get_att_float(ncid, *lev_var, "_FillValue",  &fill_value);
          if (ok != NC_NOERR) fill_value = NC_FILL_FLOAT;

          dd = fabs(fill_value)*test_d; /* differnce at least in 6th valid digit */

          *lev_ind = nzz-1;  /* suppose all are defined */
          for (i = 0; i < nzz; i++)
          {
            if ( fabs(test_lv[i]-fill_value) <= dd )
            {
              *lev_ind = i-1;
              break;
            }
          }
          if ( *lev_ind > 0)
          {
            if (test_lv[1] > test_lv[0] ) *lev_step = 1; /*level increases */
            else *lev_step = -1;
          }
          free(test_lv);

#ifdef DEBUG_NC
fprintf(stderr,"netcdf:get_nc_dims: z-dim=%d %d %lu %d %s\n",
*lev_dim,*lev_var,(unsigned long)nzz,*lev_type,lev_name);
#endif
          /* Increse nlev to max value defined for existing file */
          *nlev = nzz;
          free(lev_name);
        }
        else
        {
          fprintf(stderr,"requested nz=%d exceeds this in existing netcdf file: %lu\n",
          *nlev, (unsigned long)nzz);
          nco_err+=64;
        }
      }
      else
      {
        fprintf(stderr,"z-dim get ID error\n");
        nco_err+=128;
      }
    }
  }
  if (nco_err)
  {
    fprintf(stderr,"*** Error appending data to existing netcdf file ***\n");
    fprintf(stderr,"*** Existing dimensions [t,z,y,x]: [%lu, %lu, %lu, %lu] ***\n",
    (unsigned long)ntt,(unsigned long)nzz,(unsigned long)nyy,(unsigned long)nxx);
    fatal_error_i("netcdf:get_nc_dims: dimension error: %d",nco_err);
  }
}

/******************************************************************************************/
static int check_nc_latlon( int ncid, int y_dim, int x_dim, int dim_latlon )
{
 /*
  * Check does existing lat and lot definition in open netcdf file
  * is consistent  with the new coming data?
  *
  * Sergey Varlamov
  */
  int i, nco_err;
  size_t nyy, nxx;
  double * test_ll, dd, dtmp;
  const double test_d=1e-6;
  size_t start[2], count[2];
  int lat_var, lon_var;

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: step in check_nc_latlon\n");
#endif

  nco_err=0;
  /* locate dimensions and corresponding variables with fixed names */

  if (dim_latlon == 1) i = nx > ny ? nx:ny;
  else i = nx*ny;

  test_ll = (double*) malloc(i*sizeof(double));
  if (!test_ll) fatal_error("netcdf:check_nc_latlon: %s","error doing malloc of test_ll");

  // expect lat and lon dimension variables
  netcdf_command( nc_inq_dimlen (ncid, x_dim, &nxx) );
  netcdf_command( nc_inq_dimlen (ncid, y_dim, &nyy) );
  netcdf_command( nc_inq_varid (ncid, "longitude", &lon_var) );
  netcdf_command( nc_inq_varid (ncid, "latitude", &lat_var) );

#ifdef DEBUG_NC
fprintf(stderr,"netcdf:check_nc_latlon: x,lon,nx,nxx,y,lat,ny,nyy=%d %d %d %lu %d %d %d %lu\n",
x_dim,lon_var,nx,(unsigned long)nxx,y_dim,lat_var,ny,(unsigned long)nyy);
#endif

  if (nyy != ny || nxx != nx){
    nco_err+=2;
  }
  else if (dim_latlon == 1)
  {
      start[0] = 0; count[0] = nx;
      netcdf_command( nc_get_vara_double(ncid, lon_var, start, count, test_ll) );
      if (nx > 1) dd = 0.01*fabs(lon[1]-lon[0]);
      else        dd = test_d*fabs(lon[0]);
      for (i=0; i<nx ; i++)
      {
        dtmp = fabs(lon[i]-test_ll[i]);
	while (dtmp > 180.0) dtmp -= 360.0;
        if(fabs(dtmp) > dd )
        {
          fprintf(stderr,"different grid (longitude) in existing netcdf!\n");
          nco_err+=4;
          break;
        }
      }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:check_nc_latlon: longitude=%lf %lf\n",
test_ll[0],test_ll[nx-1]);
#endif
      start[0] = 0; count[0] = ny;
      netcdf_command( nc_get_vara_double(ncid, lat_var, start, count, test_ll) );
      /* use 1% of first step value as criteria */
      if (ny > 1) dd = 0.01*fabs(lat[nx]-lat[0]); /* 'nx' here is not error, wgrib2 specifics! */
      else        dd = test_d*fabs(lat[0]);      /* assumed tolerance */
      for (i=0; i<ny ; i++)
      {
        if(fabs(lat[i*nx]-test_ll[i]) > dd )
        {
          fprintf(stderr,"different grid (latitude) in existing netcdf!\n");
          nco_err+=8;
          break;
        }
      }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:check_nc_latlon: latitude=%lf %lf\n",
test_ll[0],test_ll[ny-1]);
#endif
  }
  else
  {
      start[0] = 0; count[0] = ny;
      start[1] = 0; count[1] = nx;
      netcdf_command( nc_get_vara_double(ncid, lat_var, start, count, test_ll) );
      /* use 1% of first step value as criteria */
      if (ny > 1) dd = 0.01*fabs(lat[nx]-lat[0]); /* 'nx' here is not error, wgrib2 specifics! */
      else        dd = test_d*fabs(lat[0]);      /* assumed tolerance */
      for (i=0; i<nx*ny ; i++)
      {
        if(fabs(lat[i]-test_ll[i]) > dd )
        {
          fprintf(stderr,"different grid (latitude) in existing netcdf!\n");
          nco_err+=16;
          break;
        }
      }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:check_nc_latlon: latitude=%lf %lf\n",
test_ll[0],test_ll[ny-1]);
#endif
      start[0] = 0; count[0] = ny;
      start[1] = 0; count[1] = nx;
      netcdf_command( nc_get_vara_double(ncid, lon_var, start, count, test_ll) );
      if (nx > 1) dd = 0.01*fabs(lon[1]-lon[0]);
      else        dd = test_d*fabs(lon[0]);
      for (i=0; i<nx*ny ; i++)
      {
        dtmp = fabs(lon[i]-test_ll[i]);
	while (dtmp > 180.0) dtmp -= 360.0;
        if(fabs(dtmp) > dd )
        {
          fprintf(stderr,"different grid (longitude) in existing netcdf!\n");
          nco_err+=32;
          break;
        }
      }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:check_nc_latlon: longitude=%lf %lf\n",
test_ll[0],test_ll[nx-1]);
#endif
  }
  free(test_ll);

  if (nco_err)
  {
    fprintf(stderr,"*** Error appending data to existing netcdf file ***\n");
    fprintf(stderr,"*** Existing vise input dimensions [y,x]: [%lu, %lu]:[%d, %d] ***\n",
    (unsigned long)nyy,(unsigned long)nxx,ny,nx);
    fatal_error_i("netcdf:check_nc_latlon: dimension error: %d",nco_err);
  }
  return nco_err;
}

/******************************************************************************************/

static void create_nc_dims(int ncid, g2nc_4Dlt * lt_4D,
                  int * time_dim, int * time_var, int time_ind,
                  double time_step, int time_step_type,
                  double date0, double verf_utime, double ref_utime,
                  int * y_dim, int * x_dim,
                  int nlev, int * lev_dim, int * lev_var, int * lev_type,
                  int * lev_ind, int * lev_step,
                  g2nc_table * nc_table, int dim_latlon,
                  int grid_template, double dx, double dy)
{
 /*
  * Define dimensions and dimension variables for new created netcdf file.
  * Modif. by Sergey Varlamov:
  *   added vertical dimension (with possible tmp name and undefined values),
  *   If nlev <= 0 vertical dimension is not looked for and is left undefined.
  *   Else its parameters are defined.
  *   May 7, 2007 - January 30, 2008
  */
  double ttime;
  int ref_time_type, i, j;
  double * test_ll;
  size_t start[2], count[2];
  int dimids[2];
  const char * str, * name, * lname, * units;
  g2nc_4Dlt * g2nc_lt;
  float * g2nc_lv;
  int lat_var, lon_var, y_var, x_var;
  char ref_date[50];

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: in create_nc_dims\n");
#endif
  if (nc_table)
  {
    g2nc_lt = nc_table->lt;
    g2nc_lv = nc_table->lv;
  }
  else
  {
    g2nc_lt = NULL;
    g2nc_lv = NULL;
  }
  // define NEW nc file, assume nx and ny of the 1st message hold for all messages...
  str="";
  if (dim_latlon == 1)
    str = "COARDS";
  else if (dim_latlon == 2)
    str = "CF-1.0";
  else
    fatal_error("netcdf:create_nc_dims: %s","unsupported lat-lon dimension");

//  netcdf_command( nc_redef(ncid) );

  netcdf_command( nc_put_att_text(ncid, NC_GLOBAL, "Conventions", strlen(str), str) );
  str = "created by wgrib2";
  netcdf_command( nc_put_att_text(ncid, NC_GLOBAL, "History", strlen(str), str) );
  netcdf_command( nc_put_att_int (ncid, NC_GLOBAL, "GRIB2_grid_template",
                            NC_INT, 1, &grid_template) );

  // create coordinate variables...
  if (dim_latlon == 1)
  {
    netcdf_command( nc_def_dim (ncid, "latitude", ny, y_dim) );
    netcdf_command( nc_def_dim (ncid, "longitude", nx, x_dim) );
    dimids[0] = *y_dim;
    netcdf_command( nc_def_var (ncid, "latitude", NC_DOUBLE, 1, dimids, &lat_var) );
    dimids[0] = *x_dim;
    netcdf_command( nc_def_var (ncid, "longitude", NC_DOUBLE, 1, dimids, &lon_var) );
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:create_nc_dims: latitude(dim,var,ny)=%d %d %d\n",*y_dim,lat_var,ny);
fprintf(stderr,"netcdf:create_nc_dims: longitude(dim,var,nx)=%d %d %d\n",*x_dim,lon_var,nx);
#endif
  }
  else
  {

    netcdf_command( nc_def_dim (ncid, "y", ny, y_dim) );
    netcdf_command( nc_def_dim (ncid, "x", nx, x_dim) );
    dimids[0] = *y_dim;
    netcdf_command( nc_def_var (ncid, "y", NC_DOUBLE, 1, dimids, &y_var) );
    str = "y coordinate of projection";
//    str = "projection_y_coordinate";    //it is standard_name,
                                        //but could require grid_mapping definition that is
                                        //projection-dependant; could do it later;
                                        //do not need lat-lon values in this case as these would be
                                        //computed in processing software (CF convention)
    netcdf_command( nc_put_att_text(ncid, y_var, "long_name", strlen(str), str) );
    str = "projection_y_coordinate";
    netcdf_command( nc_put_att_text(ncid, y_var, "standard_name", strlen(str), str) );
    dimids[0] = *x_dim;
    netcdf_command( nc_def_var (ncid, "x", NC_DOUBLE, 1, dimids, &x_var) );
    str = "x coordinate of projection";
    netcdf_command( nc_put_att_text(ncid, x_var, "long_name", strlen(str), str) );
    str = "projection_x_coordinate";
    netcdf_command( nc_put_att_text(ncid, x_var, "standard_name", strlen(str), str) );
    str = "m";
    netcdf_command( nc_put_att_text(ncid, y_var, "units", strlen(str), str) );
    netcdf_command( nc_put_att_text(ncid, x_var, "units", strlen(str), str) );
    netcdf_command( nc_put_att_double(ncid, x_var, "grid_spacing", NC_DOUBLE, 1, &dx) );
    netcdf_command( nc_put_att_double(ncid, y_var, "grid_spacing", NC_DOUBLE, 1, &dy) );
    dimids[0] = *y_dim;
    dimids[1] = *x_dim;
    netcdf_command( nc_def_var (ncid, "latitude", NC_DOUBLE, 2, dimids, &lat_var) );
    netcdf_command( nc_def_var (ncid, "longitude", NC_DOUBLE, 2, dimids, &lon_var) );
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:create_nc_dims: y(dim,var,ny)=%d %d %d\n",*y_dim,y_var,ny);
fprintf(stderr,"netcdf:create_nc_dims: x(dim,var,nx)=%d %d %d\n",*x_dim,x_var,nx);
fprintf(stderr,"netcdf:create_nc_var (2D): latitude(xdim,ydim,var)=%d %d %d\n",*y_dim,*x_dim,lat_var);
fprintf(stderr,"netcdf:create_nc_var (2D): longitude(xdim,ydim,var)=%d %d %d\n",*y_dim,*x_dim,lon_var);
#endif
  }
  str = "degrees_north";
  netcdf_command( nc_put_att_text(ncid, lat_var, "units", strlen(str), str) );
  str = "degrees_east";
  netcdf_command( nc_put_att_text(ncid, lon_var, "units", strlen(str), str) );
  str = "latitude";
  netcdf_command( nc_put_att_text(ncid, lat_var, "long_name", strlen(str), str) );
  str = "longitude";
  netcdf_command( nc_put_att_text(ncid, lon_var, "long_name", strlen(str), str) );

  /* time settings */
  netcdf_command( nc_def_dim (ncid, "time", NC_UNLIMITED, time_dim) );
  dimids[0] = *time_dim;
  netcdf_command( nc_def_var (ncid, "time", NC_DOUBLE, 1, dimids, time_var) );
// wne 9/2011  netcdf_command( nc_put_att_double(ncid, *time_var, "_FillValue", NC_DOUBLE, 1, &dfill_value) );
  str = "seconds since 1970-01-01 00:00:00.0 0:00";
  netcdf_command( nc_put_att_text(ncid, *time_var, "units", strlen(str), str) );
  str = "verification time generated by wgrib2 function verftime()";
  netcdf_command( nc_put_att_text(ncid, *time_var, "long_name", strlen(str), str) );

  /* nc_ref_time is reference time value,
     nc_ref_time_type is the reference time type:
     0 - undefined
     1 - analyses, all for the same reference date, could be succeded by forecasts
     2 - analyses, but for different reference date/time (time serie)
     3 - forecasts from the same reference date/time
     For the type 0 or 2 nc_ref_time keeps first field reference date/time
  */
  if (fabs(ref_utime - verf_utime) < G2NC_TM_TOLERANCE )
  {
    ref_time_type = 1;
    str = "analyses, reference date is fixed";
  }
  else
  {
    ref_time_type = 3;
    str = "forecast or accumulated, reference date is fixed";
  }
  netcdf_command( nc_put_att_double(ncid, *time_var, "reference_time", NC_DOUBLE, 1, &ref_utime) );
  netcdf_command( nc_put_att_int(ncid, *time_var,    "reference_time_type", NC_INT, 1, &ref_time_type) );
  get_unixdate(ref_utime, ref_date);
  netcdf_command( nc_put_att_text(ncid, *time_var,   "reference_date", strlen(ref_date), ref_date) );
  netcdf_command( nc_put_att_text(ncid, *time_var,   "reference_time_description", strlen(str), str) );
  /* write time step attributes, could be -1 (undefined) for first or single time step, then update */
  if ( time_step_type )
  {
    str="user";
  }
  else
  {
    str="auto";
  }
  netcdf_command( nc_put_att_text(ncid, *time_var, "time_step_setting", strlen(str), str) );
  netcdf_command( nc_put_att_double(ncid, *time_var,  "time_step", NC_DOUBLE, 1, &time_step) );

#ifdef DEBUG_NC
fprintf(stderr,"netcdf:create_nc_dims: time(dim,var)=%d %d, size unlimited\n",*time_dim,*time_var);
fprintf(stderr,"netcdf:create_nc_dims: time_step=%.1lf time_step_type=%d (%s)\n",
time_step,time_step_type,str);
#endif

  if (nlev > 0)
  {
    if (g2nc_lt)
    {
      name = g2nc_lt->sname;
      *lev_type = g2nc_lt->type;
      units = g2nc_lt->units;
      lname = g2nc_lt->lname;
    }
    else
    {
      if (lt_4D)
      {
        name =lt_4D->sname;
        *lev_type=lt_4D->type;
        units = lt_4D->units;
        lname = lt_4D->lname;
      }
      else
      { /* use tmp name, rename when eligible data will arrive... */
        name ="level";
        *lev_type=-1;
        units = "undef";
        lname = units;
      }
    }
    netcdf_command( nc_def_dim (ncid, name, nlev, lev_dim) );
    dimids[0] = *lev_dim;
    netcdf_command( nc_def_var (ncid, name, NC_FLOAT, 1, dimids, lev_var) );
    netcdf_command( nc_put_att_int(ncid, *lev_var, "lev_type", NC_INT, 1, lev_type) );
    netcdf_command( nc_put_att_text(ncid, *lev_var, "units", strlen(units), units) );
    netcdf_command( nc_put_att_text(ncid, *lev_var, "long_name", strlen(lname), lname) );
    // next force the return of "_FillValue" for non-written z-dim request:
    netcdf_command( nc_put_att_float(ncid, *lev_var, "_FillValue", NC_FLOAT, 1, &ffill_value) );

#ifdef DEBUG_NC
fprintf(stderr,"netcdf:create_nc_dims: z(dim,var,nz)=%d %d %d type=%d\n",
*lev_dim,*lev_var,nlev,*lev_type);
#endif
  }
  netcdf_command( nc_enddef(ncid) );

  // populate coordinate variables...
  start[0] = 0;
  netcdf_command( nc_put_var1_double(ncid, *time_var, start, &date0) );
#ifdef DEBUG_NC
  ttime = date0;
#endif
  if ( time_ind == 1 )
  {
    start[0] = 1;
    netcdf_command( nc_put_var1_double(ncid, *time_var, start, &verf_utime) );
#ifdef DEBUG_NC
    ttime = verf_utime;
#endif
  }
  else if (time_ind > 1)
  {
    for (j=1; j <= time_ind; j++)
    {
      start[0] = j;
      ttime = date0 + (time_step)*j;
      netcdf_command( nc_put_var1_double(ncid, *time_var, start, &ttime) );
    }
  }
  else if (time_ind < 0)
  {
   fatal_error("netcdf:create_nc_dims: %s","negative time index");
  }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:create_nc_dims: time_ind=%d, date0=%.1lf, date_last=%.1lf\n",
time_ind, date0, ttime);
#endif

  i = nx > ny ? nx : ny;
  test_ll = (double*) malloc(i*sizeof(double));
  if (!test_ll) fatal_error("netcdf:create_nc_dims: %s",
    "error doing malloc of test_ll");
  if (dim_latlon == 1)
  {
    for (i=0; i<ny ; i++) test_ll[i] = lat[i*nx];
    start[0] = 0; count[0] = ny;
    netcdf_command( nc_put_vara_double(ncid, lat_var, start, count, test_ll) );

    // not monotonic increasing, removed 3/2014 WNE for (i=0; i<nx; i++) test_ll[i] = lon[i];
    dx = lon[1] > lon[0] ? lon[1] - lon[0] : 360.0 + lon[1] - lon[0];
    test_ll[0] = lon[0] >= 180.0 ? lon[0] - 360.0 : lon[0];
    for (i = 1; i < nx; i++) test_ll[i] = test_ll[0] + i*dx;
    start[0] = 0; count[0] = nx;
    netcdf_command( nc_put_vara_double(ncid, lon_var, start, count, test_ll) );
  }
  else
  {
    for (i=0; i<ny ; i++) test_ll[i] = dy*i;
    start[0] = 0; count[0] = ny;
    netcdf_command( nc_put_vara_double(ncid, y_var, start, count, test_ll) );

    for (i=0; i<nx; i++) test_ll[i] = dx*i;
    start[0] = 0; count[0] = nx;
    netcdf_command( nc_put_vara_double(ncid, x_var, start, count, test_ll) );

    start[0] = 0;  count[0] = ny;
    start[1] = 0;  count[1] = nx;
    netcdf_command( nc_put_vara_double(ncid, lat_var, start, count, lat) );
    netcdf_command( nc_put_vara_double(ncid, lon_var, start, count, lon) );
  }
  free(test_ll);
  *lev_ind = -1;
  *lev_step = 0;

  if (nlev > 0 && g2nc_lv)
  {
    *lev_ind = nlev-1;
    start[0] = 0; count[0] = nlev;
    netcdf_command( nc_put_vara_float(ncid, *lev_var, start, count, g2nc_lv) );
    if ( *lev_ind > 0)
    {
      if (g2nc_lv[1] > g2nc_lv[0] ) *lev_step = 1; /*level increases */
      else *lev_step = -1;
    }
  }
}

/******************************************************************************************/
static int update_nc_ref_time(int ncid, double verf_utime, double ref_utime,
                       int time_var, int time_step_type, double time_step)
{
/*
  Update time attributes if necessary and possible;
  nc_ref_time is reference time value,
  nc_ref_time_type is the reference time type:
     0 - undefined
     1 - analyses, all for the same reference date, could be succeded by forecasts
     2 - analyses for different reference date/time (time serie)
     3 - forecasts from the same reference date/time
     For the type 0 or 2 nc_ref_time keeps first input field reference date/time,
     could be misleading if not date0 field is coming first...
  Sergey Varlamov
*/
  int ok, update_rt, update_ts;
  double nc_ref_time, nc_time_step;
  int nc_ref_time_type;
  const char * str;
  char ref_date[50];

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: step in update_nc_ref_time\n");
#endif
  str = "";
  update_rt = 1;
  nc_ref_time=0;
  ok = nc_get_att_double(ncid, time_var, "reference_time", &nc_ref_time);
  if (ok != NC_NOERR) update_rt = 0; /* attribute do not exists, could be with update of file from older wgrib2 version */

  nc_ref_time_type=0;
  ok = nc_get_att_int(ncid, time_var, "reference_time_type", &nc_ref_time_type);
  if (ok != NC_NOERR || nc_ref_time_type <= 0) update_rt = 0;

  update_ts = 0;
  if (time_step_type == 0)
  { /* try to update only for "auto" case, skip if step is user-defined */
    nc_time_step=-1;  /* variable step, do not update */
    ok = nc_get_att_double(ncid, time_var, "time_step", &nc_time_step);
    if (ok == NC_NOERR && nc_time_step > -G2NC_TM_TOLERANCE) update_ts = 1;
  }
#ifdef DEBUG_NC
fprintf(stderr,
"netcdf:update_nc_ref_time: ref_time(f,c)=%.1lf %.1lf ref_time_type=%d time_step(f,c)=%.1lf %.1lf\n",
nc_ref_time, ref_utime,nc_ref_time_type,nc_time_step,time_step);
fprintf(stderr,"netcdf:update_nc_ref_time: update_rt=%d update_ts=%d\n",update_rt, update_ts);
#endif
  if ( !update_rt && !update_ts ) return 1;

  if (update_rt)
  {/* exists, value is defined - check it against new and,
      if it is not consistent - modify type */
    update_rt = 0;
    /* string for type 0 */
    str = "kind of product unclear, reference date is variable, min found reference date is given";
    switch (nc_ref_time_type)
    {
    case 1: /* 1 - analyses, all for the same date, could be succeded by forecasts */
      if ( fabs(nc_ref_time - ref_utime) > G2NC_TM_TOLERANCE &&
           fabs(ref_utime - verf_utime) < G2NC_TM_TOLERANCE )
      { /* change to analyses for different ref_time */
        nc_ref_time_type = 2;
        str = "analyses, reference date is variable, min found reference date is given";
        update_rt = 1;
      }
      else if (fabs(nc_ref_time - ref_utime) < G2NC_TM_TOLERANCE &&
               fabs(ref_utime - verf_utime) > G2NC_TM_TOLERANCE)
      { /* possibly it is first forecast after analyses */
        nc_ref_time_type = 3;
        str = "forecasts or accumulated (including analyses), reference date is fixed"; /*including analyses*/
        update_rt = 1;
      }
      else if (fabs(nc_ref_time - ref_utime) > G2NC_TM_TOLERANCE &&
               fabs(ref_utime - verf_utime) > G2NC_TM_TOLERANCE)
      { /* forecast for different reference time */
        nc_ref_time_type = 0;
        update_rt = 1;
      }
      break;
    case 2: /*2 - analyses, but for different reference date/time */
      if (fabs(ref_utime - verf_utime) > G2NC_TM_TOLERANCE)
      { /* mix, change type to undefined */
        nc_ref_time_type = 0;
        update_rt = 1;
      }
      break;
    case 3: /* 3 - forecasts from the same reference date/time */
      if (fabs(nc_ref_time - ref_utime) > G2NC_TM_TOLERANCE)
      { /* mix, change type to undefined */
        nc_ref_time_type = 0;
        update_rt = 1;
      }
      break;
    }
  }
  if (update_ts )
  {
    update_ts = 0;
    if (fabs(nc_time_step) < G2NC_TM_TOLERANCE && time_step > G2NC_TM_TOLERANCE)
    { /* attribute value is uninitialized (0), get valid value */
      nc_time_step = time_step;
      update_ts = 1;
    }
    else if (nc_time_step > G2NC_TM_TOLERANCE && time_step > G2NC_TM_TOLERANCE &&
             fabs(nc_time_step - time_step) > G2NC_TM_TOLERANCE)
    {
      nc_time_step = -1; /* variable time step, mark it as undefined */
      update_ts = 1;
    }
  }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:update_nc_ref_time: update_rt=%d update_ts=%d\n",update_rt, update_ts);
#endif
  if (update_rt || update_ts)
  {
    netcdf_command( nc_redef(ncid) );
    if (update_rt)
    {
      if (ref_utime < nc_ref_time) /* try to keep in file latest data reference time */
      {
        netcdf_command( nc_put_att_double(ncid, time_var, "reference_time", NC_DOUBLE, 1, &ref_utime) );
        get_unixdate(ref_utime, ref_date);
        netcdf_command( nc_put_att_text(ncid, time_var, "reference_date", strlen(ref_date), ref_date) );
      }
      netcdf_command( nc_put_att_int(ncid, time_var, "reference_time_type", NC_INT, 1, &nc_ref_time_type) );
      netcdf_command( nc_put_att_text(ncid, time_var, "reference_time_description", strlen(str), str) );
    }
    if (update_ts)
    {
      netcdf_command( nc_put_att_double(ncid, time_var, "time_step", NC_DOUBLE, 1, &nc_time_step) );
    }
    netcdf_command( nc_enddef(ncid) );
  }
  return 0;
}

/******************************************************************************************/
static int get_nc_time_ind(int ncid, double verf_utime, int time_var, int time_dim,
                    int * time_ind, double * last_verf_time,
                    double * time_step, int time_step_type, double date0 )
{
 /*
  * Compare provided verf_utime with this in open netcdf file and return
  * corresponding index for time dimension. If new value exceeds existing -
  * extend UNLIMITED time dimension writing new entry point(s).
  * Negative value is for errors.
  * Sergey Varlamov
  */
  int t_ind, i, j;
  double *wtime, ttime;
  size_t index[1], start[1], count[1], ntt;

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: step in get_nc_time_ind\n");
#endif
  if (*time_ind < 0)
  { /* time dim is undefined yet */
    *last_verf_time = verf_utime-1;
    *time_ind = -1;
  }
  if(fabs(verf_utime - (*last_verf_time)) < G2NC_TM_TOLERANCE)
  {
    return (*time_ind); /* same as before, already defined */
  }
  /*
  Get the number of records written so far
  */
  netcdf_command( nc_inq_dimlen (ncid, time_dim, &ntt) );
  if (time_step_type)
  { /* user-defined, fixed, non-zero */
    i = (int)((verf_utime - date0)/(*time_step) + G2NC_TM_TOLERANCE);
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:get_nc_time_ind: fixed time step, time_ind(r,i)=%.2lf %d ntt=%lu\n",
(verf_utime - date0)/(*time_step), i, ntt);
#endif
    if (fabs(date0+(*time_step)*i-verf_utime) > G2NC_TM_TOLERANCE)
    {
      return (-1); /* non-integer index */
    }
    /*
    Extend file if needed
    */
    if (i > ntt-1)
    { /* generate new time values */
      for (j=ntt-1; j <= i; j++)
      {
        index[0] = j;
        ttime = date0 + (*time_step)*j;
        netcdf_command( nc_put_var1_double(ncid, time_var, index, &ttime) );
      }
    }
    *time_ind = i;
    *last_verf_time = verf_utime;
    return i;
  }
  /* auto time stepping; could be irregular time-spaced data */
  index[0] = ntt-1;
  netcdf_command( nc_get_var1_double(ncid, time_var, index, &ttime) );

  if(verf_utime > ttime + G2NC_TM_TOLERANCE)
  {/* write new time entry point */
    t_ind = ntt;
    *time_ind = ntt;
    *time_step = verf_utime - ttime;
    *last_verf_time = verf_utime;
    index[0] = ntt;
    netcdf_command( nc_put_var1_double(ncid, time_var, index, last_verf_time) );
  }
  else
  { /* verf_utime <= ttime */
    t_ind = -1;
    wtime = (double *) malloc(ntt*sizeof(double));
    if (!wtime) fatal_error("netcdf:get_nc_time_ind: %s",
        "error doing malloc of wtime");

    start[0] = 0; count[0] = ntt;
    netcdf_command( nc_get_vara_double(ncid, time_var, start, count, wtime) );

    for (i = ntt-1; i >= 0; i--)
    {
      if (fabs(wtime[i]-verf_utime) < G2NC_TM_TOLERANCE)
      {
        t_ind = i;
        break;
      }
    }
    free(wtime);
    if (t_ind >= 0)
    {
      *time_ind = t_ind;
      *last_verf_time = verf_utime;
    }
  }
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:get_nc_time_ind: variable time step, time_ind=%d ntt=%lu\n",
t_ind, ntt);
#endif

  return t_ind;
}

/******************************************************************************************/
static int update_nc_lev( int ncid, g2nc_4Dlt * lt_4D,
                   float lev_val, int nlev,  int lev_dim, int lev_var,
                   int * lev_ind, int * lev_step, int * lev_type)
{
 /*
  * Return index of defined z-dim or -1
  * If still undefined - redefine z_lev axes parameters - name and attributes
  * Sergey Varlamov
  */
  size_t index[1], start[1], count[1];
  int level_ind, i;
  float dd, * test_lev;

#ifdef DEBUG_NC
fprintf(stderr,"netcdf: step in update_nc_lev\n");
#endif

  if (*lev_type < 0)
  { /* was not fixed yet in netcdf */
    *lev_type = lt_4D->type;
    netcdf_command( nc_redef(ncid) );
    netcdf_command( nc_rename_dim (ncid, lev_dim, lt_4D->sname ) );
    netcdf_command( nc_rename_var (ncid, lev_var, lt_4D->sname ) );
    netcdf_command( nc_put_att_int(ncid, lev_var, "lev_type",NC_INT, 1, lev_type) );
    netcdf_command( nc_put_att_text(ncid, lev_var, "units",
            strlen(lt_4D->units), lt_4D->units) );
    netcdf_command( nc_put_att_text(ncid, lev_var, "long_name",
            strlen(lt_4D->lname), lt_4D->lname) );
    netcdf_command( nc_enddef(ncid) );
#ifdef DEBUG_NC
fprintf(stderr,"netcdf:update_nc_lev: first defined 4D, lev_type=%d lev=%g\n",
lt_4D->type,lev_val);
#endif
  }

  /* look z-dim info in the netcdf */
  level_ind = -1;

  test_lev = (float*) malloc(nlev*sizeof(float));
  if (!test_lev) fatal_error("netcdf:get_nc_lev_ind: %s",
    "error doing malloc of test_lev");

  start[0] = 0; count[0] = nlev;
  netcdf_command( nc_get_vara_float(ncid, lev_var, start, count, test_lev) );

  if (fabs(lev_val) < G2NC_LV_TOLERANCE) dd = G2NC_LV_TOLERANCE;
  else dd = fabs(lev_val)*G2NC_LV_TOLERANCE;

  for (i = 0; i < nlev; i++)
  {
    /* undefined return test_lev = _FillValue */
    if ( fabs(test_lev[i]-lev_val) < dd )
    {
      level_ind = i;
      break;
    }
  }
  if ( level_ind < 0 )
  {
   /* do not found this level in netcdf;
    * check that levels change monotonically and add new level value
    */
    level_ind = *lev_ind;  /*index of previous defined value */
    if(level_ind >= nlev-1)
    {
      fatal_error_i(
      "netcdf:get_nc_lev_ind: more vertical levels found then were defined, max=%d",
      nlev);
    }
    *lev_ind += 1;
    if (level_ind >= 0)
    {
      if ( *lev_step )
      {
        if ( (*lev_step)*(lev_val - test_lev[level_ind]) < 0. )
          fatal_error("netcdf:update_nc_lev: %s",
          "non-monotonic levels order in grib2, use -nc_table $levs to fix");
      }
      else
      {
        if (lev_val > test_lev[level_ind] ) *lev_step = 1; /*level increases */
        else *lev_step = -1;
      }
    }
    level_ind = *lev_ind;
    index[0] = level_ind;
    netcdf_command( nc_put_var1_float(ncid, lev_var, index, &lev_val) );
  }
  else if ( level_ind > 0 && *lev_step == 0)
  { /* save levels direction info*/
    if (test_lev[1] > test_lev[0] ) *lev_step = 1; /* increasing order */
    else *lev_step = -1;
  }
  free(test_lev);
  return level_ind;
}

#else

int f_netcdf(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
#endif

