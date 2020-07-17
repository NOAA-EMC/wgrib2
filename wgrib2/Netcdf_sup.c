/******************************************************************************************

vsm: test compilation with undefined USE_NETCDF...

 This file is part of wgrib2 and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

 Edition 2008.02.18

 Sergey Varlamov
 Kristian Nilssen
 Wesley Ebisuzaki
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
//#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "wgrib2nc.h"

//#define DEBUG_NC

#if defined USE_NETCDF3 || defined USE_NETCDF4

/*
 * Find UTC "seconds since 1970-01-01 00:00:00.0 0:00"
 */
double get_unixtime(int year, int month, int day, int hour,
                    int minute, int second, int * err_code)
{
  struct tm t, *gmt_tm;
  time_t local_t, gmt_t;
  *err_code = 0;
  t.tm_sec = second;
  t.tm_min = minute;
  t.tm_hour = hour;
  t.tm_mday = day;
  t.tm_mon = month - 1;
  t.tm_year = year - 1900;
  t.tm_isdst = 0;
/*
   vsm: for int(4) type max valid date range for mktime is
   1902-2037 or 1970-2037 depending on C library implementation.
*/
  if (sizeof(time_t) <= 4 && (year > 2037 || year < 1902))
  {
    *err_code = 1;
    return 0;
  }
  local_t = mktime(&t);
  /* Simple check that mktime realization returns "normal" expected values,
  start of Epoch = 1970.01.01 */
  if (year < 1970 && local_t >= 0)
  {
    *err_code = 2;
    return 0;
  }
  if (year > 1970 && local_t <= 0)
  {
    *err_code = 3;
    return 0;
  }
  gmt_tm = gmtime(&local_t);
  gmt_t = mktime(gmt_tm);
  return ((double)(local_t + (local_t-gmt_t)));
}
/*
 * Create time string (UTC) from time
 */
char * get_unixdate(double utime, char * date_str)
{
  struct tm *gmt;
  time_t gmt_t;

  gmt_t=(time_t)utime;
  gmt = gmtime(&gmt_t);
  sprintf(date_str,"%.4d.%.2d.%.2d %.2d:%.2d:%.2d UTC",
  gmt->tm_year+1900,gmt->tm_mon+1,gmt->tm_mday,gmt->tm_hour,gmt->tm_min,gmt->tm_sec);
  return date_str;
}


/*
 * check if strings match
 */
int match_str(const char *s, const char *match) {
   while (*match) {
      if (*match++ != *s++) return 0;
   }
   return 1;
}
 

/*
* makes units COARDS compliant: C -> Celsius, g -> gram, prob -> 1, gpm -> m
*/

void fix_units(char *s, int n)
{
// seems to be risky incrementing the pointer s...
// modify to more safe having time. vsm
  char tmp[n+2], *p;

  tmp[0] = '+';
  strncpy(tmp+1, s, n);
  p = tmp+1;
  *s = 0;

  while (*p)
  {
    if (toupper((unsigned char) *p) == 'C' && !isalpha((unsigned char) p[1]) && !isalpha((unsigned char) p[-1]))
    {
      *s++ = 'C';
      *s++ = 'e';
      *s++ = 'l';
      *s++ = 's';
      *s++ = 'i';
      *s++ = 'u';
      *s++ = 's';
      p++;
    }
    else if (toupper((unsigned char) *p) == 'G' && !isalpha((unsigned char) p[1]) && !isalpha( (unsigned char) p[-1]))
    {
      *s++ = 'g';
      *s++ = 'r';
      *s++ = 'a';
      *s++ = 'm';
      p++;
    }
    else if (toupper((unsigned char) *p) == '%') 
    {
      *s++ = 'p';
      *s++ = 'e';
      *s++ = 'r';
      *s++ = 'c';
      *s++ = 'e';
      *s++ = 'n';
      *s++ = 't';
      p++;
    }
    else if (toupper((unsigned char) *p) == 'P' && toupper((unsigned char) p[1]) == 'R' && toupper((unsigned char) p[2]) == 'O' && toupper((unsigned char) p[3]) == 'B')
    {
      *s++ = '1';
      p += 4;
    }
    else if (toupper((unsigned char) *p) == 'G' && toupper((unsigned char) p[1]) == 'P' && toupper((unsigned char) p[2]) == 'M' && !isalpha((unsigned char) p[-1]) && !isalpha((unsigned char) p[3]) )
    {
      *s++ = 'm';
      p += 3;
    }
    else
    {
      *s++ = *p++;
    }
  }
  *s = 0;
}

int get_nc_conv_table(const char * name, const char * level,
                      const g2nc_table * nc_table)
{
  int i;
  if ( nc_table )
    if ( nc_table->nvc )
      for (i=0; i < nc_table->nvc; i++)
        if ( strcmp(nc_table->vc[i].wgrib2_name, name) == 0 &&
             strcmp(nc_table->vc[i].wgrib2_level, level) == 0 ) return i;
  return -1;
}

int free_nc_table( g2nc_table * nc_table )
{
  // cleanup
  int i;
  if ( nc_table == NULL ) return 0;
  if ( nc_table->nvc > 0 && nc_table->vc )
  {
    for (i=0; i < nc_table->nvc; i++)
    {
      free(nc_table->vc[i].wgrib2_name);
      free(nc_table->vc[i].wgrib2_level);
      free(nc_table->vc[i].nc_name);
    }
  }
  free(nc_table->vc);
  free(nc_table->lt);
  free(nc_table->lv);
  free(nc_table);
  nc_table = NULL;
#ifdef DEBUG_NC
fprintf(stderr,"nc_table: cleaned-up...\n");
#endif
  return 0;
}

g2nc_4Dlt nc_4Dlt[G2NC_NUM_4DLT] = {
  { 20,"klevel","K level","K",1.,},             //"%g K level",
  {100,"plevel","pressure level","mb",0.01,},   //"%g hPa",
  {104,"slevel","sigma level","level",1.,},     //"%g sigma level",
  {105,"hlevel","hybrid level","level",1.,},    //"%g hybrid level",
  {107,"kilevel","K isentropic level","K",1.,}, //"%g K isentropic level",
  {160,"depth","ocean depth","m",1.,},          //"%g m below sea level",
};

g2nc_table * nc_table = NULL; /* table undefined */
/*
 * HEADER:100:nc_grads:setup:0:require netcdf file to be grads v1.9b4 compatible (fixed time step only)
 */
int nc_grads_compatible = 0;

int f_nc_grads(ARG0)
{
  if (mode == -1) nc_grads_compatible = 1;
  return 0;
}

/*
 * HEADER:100:no_nc_grads:setup:0:netcdf file may be not grads v1.9b4 compatible, variable time step
 */
int f_no_nc_grads(ARG0)
{
  if (mode == -1) nc_grads_compatible = 0;
  return 0;
}

/*
 * HEADER:100:nc_pack:setup:1:pack/check limits of all NEW input variables, X=min:max[:byte|short|float]
 *
 * NEW means that if some variable was already defined in the netcdf file and now is appended to it (-append mode)
 * initially defined packing parameters are used. min and max are used to estimate appropriate offset and scaling,
 * if both are zero - automatic scaling goes.
 */
int nc_pack = 0;
float nc_pack_offset = 0.;
float nc_pack_scale = 1.;
float nc_valid_min = 0.;
float nc_valid_max = 0.;

int f_nc_pack(ARG1)
{
  char * pack_to=NULL;
  float range;
  int i;
  if (mode == -1)
  {
//fprintf(stderr,"nc_pack: get arg: %s\n", arg1);
    pack_to = (char*) malloc((strlen(arg1)+12)*sizeof(char));
    if (pack_to == NULL) fatal_error("nc_pack: error allocating tmp string","");
    i = sscanf(arg1,"%g:%g:%s", &nc_valid_min, &nc_valid_max, pack_to);
    if ( i < 2)
    {
      fatal_error("nc_pack: bad value, expect min:max[:byte|short|float(default)], found %s", arg1);
//    if (sscanf(arg1,"%g:%g:%s", &nc_pack_offset, &nc_pack_scale, pack_to) != 3){
//      fatal_error("netcdf: bad nc_pack, expect offset:scale:[byte|short], found %s", arg1);
    }
    range = (nc_valid_max - nc_valid_min);
    if ( i == 2 ) strcpy(pack_to,"float");

    if (strcmp(pack_to,"byte")==0 || strcmp(pack_to,"BYTE")==0 || strcmp(pack_to,"Byte")==0)
      nc_pack = G2NC_PACK_BYTE;
    else if (strcmp(pack_to,"short")==0 || strcmp(pack_to,"SHORT")==0 || strcmp(pack_to,"Short")==0)
      nc_pack = G2NC_PACK_SHORT;
    else if (strcmp(pack_to,"float")==0 || strcmp(pack_to,"FLOAT")==0 || strcmp(pack_to,"Float")==0)
      nc_pack = G2NC_PACK_FLOAT; /* will check valid_range */
    else
      fatal_error("nc_pack: bad value, expect min:max:byte|short|float, found %s", arg1);
    if ( nc_pack == G2NC_PACK_FLOAT && fabs(range) < 1e-20 ) fatal_error(
      "nc_pack: small valid_range specified, expected min:max[:float], min<max, found %s", arg1);

    /* Center near 0 as signed char or short are used. */
    if (nc_pack != G2NC_PACK_FLOAT)
    {
      nc_pack_offset = (float)(nc_valid_max+nc_valid_min)*0.5;
      if ( nc_pack == G2NC_PACK_BYTE )
        nc_pack_scale = (float) (range/(G2NC_FILL_VALUE_BYTE - 2))*0.5;
      else if ( nc_pack == G2NC_PACK_SHORT )
        nc_pack_scale = (float) (range/(G2NC_FILL_VALUE_SHORT - 2))*0.5;
    }
    free(pack_to);

//fprintf(stderr,"nc_pack: nc_pack=%d\n", nc_pack);

  }
  return 0;
}

/*
 * HEADER:100:no_nc_pack:setup:0:no packing in netcdf for NEW variables
 *
 */
int f_no_nc_pack(ARG0)
{
  if (mode == -1)
  {
    nc_pack = 0;
    nc_pack_scale = 1.;
    nc_pack_offset = 0.;
    nc_valid_min = 0.;
    nc_valid_max = 0.;
  }
  return 0;
}

/*
 * HEADER:100:nc_nlev:setup:1:netcdf, X = max LEV dimension for {TIME,LEV,LAT,LON} data
 */
int nc_nlev = 0;

int f_nc_nlev(ARG1)
{
  if (mode == -1)
  {
    if (sscanf(arg1,"%d", &nc_nlev) != 1)
    {
      fatal_error("netcdf: bad nc_nlev %s", arg1);
    }
    if (nc_nlev > G2NC_MAX_NLEV)
    {
      fatal_error("nc_nlev: value exceeds G2NC_MAX_NLEV, %s", arg1);
    }
  }
  return 0;
}

/*
 * HEADER:100:nc_table:setup:1:X is conversion_to_netcdf_table file name
 */
int f_nc_table(ARG1)
{
  char   input[_MAX_PATH], on[_MAX_PATH], lv[_MAX_PATH],
         nn[_MAX_PATH], pk[_MAX_PATH], defl[_MAX_PATH], ctmp[_MAX_PATH];
  char * prd;
  FILE * fl;
  int i, ir, ierr, itmp;
  g2nc_conv * old_nct;
  char *tptr;
  float min, max, range, ftmp;

  if (mode == -1)
  {
    /* avoid 'lost memory' */
    if ( nc_table )
      if ( !nc_table->used)
        fatal_error("nc_table: second table name entered when first was not used: %s", arg1);

    fl = fopen(arg1,"r");
    if(fl == NULL)
      fatal_error("nc_table: can not open file for reading: %s", arg1);

    /* Deconnect from allocated memory, it is mapped in some 'local' structure,
     * and allocate new instance
     */
    nc_table = (g2nc_table*)malloc(sizeof(g2nc_table));
    if ( nc_table == NULL )
      fatal_error("nc_table: error allocating new table: %s", arg1);

    nc_table->vc = NULL;
    nc_table->lt = NULL;
    nc_table->lv = NULL;
    nc_table->nvc = 0;
    nc_table->used = 0;
    nc_table->nlev = -1;
    nc_table->grads = -1;
    nc_table->endianness = -1;

    ierr = 0;

    while(!feof(fl))
    {/* read strings including \n symbol */
      prd = fgets(input, _MAX_PATH, fl);
      if(prd == NULL) continue;
      if(strlen(input) < 2)continue;
      /* format:

wgib2_name:wgrib2_level|*:nc_name|ignore[:ignore|no|float|deflate{0-9}|[short|byte:min:max]][:deflate{0-9}]

         or/and special instructions that overcome command-line options
         for the netcdf file they are applied:

           # 4D level type description: grib2_type:user_short_name:user:long_name:user_units:scale_to_user_units
           $lev_type 100:plevel:pressure level:mb:0.01   #JMA MSM model upper layer (p) data; scale: Pa->mb
           $nlev 3   #number of vertical levels for 4D variables
           $levs 1000 500 100  #vertical level values in user_units, at least nlev;
                 50 10
           $endianness native|little-endian|big-endian

         Length of input strings must be less then _MAX_PATH or 256 symbols,
         input on multiple lines is supported.
         The "deflate{0-9}" directive is supported only if netCDF-4 type new file is created;
         deflate0 means "no compression", other values define compression level. Default is 1.
       */
//
//    next do not work as expected, parse string 'manually', it's C...
//    ir = sscanf(input," %[^:]s:%[^:]s%[^:\n]s%[^:\n]s:%g:%g",
//                     on,    lv,    nn,      pk,    &min, &max);
      ir=0;
      min=max=0;
      prd = input;
      i = sscanf(prd," %[^:]s",on);
      if (i < 1) continue;
      if (on[0]=='#' || on[0]=='\n') continue;  /* comment or line of spaces only, pass */
      if (on[0]=='$')
      {
        /* specification of vertical level for 4D variables in grib2 to netcdf conversion */
        if (strncmp(on,"$nlev",5) == 0 || strncmp(nn,"$NLEV",5) == 0)
        {
          itmp = -1;
          i = sscanf(prd," %*s %d",&itmp);
          if ( i < 1 || itmp < 0 || itmp > G2NC_MAX_NLEV)
          {
            fprintf(stderr,"nc_table: large value or error in $nlev definition string:\n%s\n", prd);
            ierr = -1;
            break;
          }
          if (nc_table->nlev >= 0)
          {
            fprintf(stderr,"nc_table: found dublicate $nlev definition, was %d, new:\n%s\n",
            nc_table->nlev,prd);
            ierr = -2;
            break;
          }
          nc_table->nlev=itmp;
        }
        else if (strncmp(on,"$grads",6) == 0 || strncmp(nn,"$GRADS",6) == 0)
        {
          itmp = -1;
          i = sscanf(prd," %*s %d",&itmp);
          if ( i < 1 || itmp < 0 || itmp > 1)
          {
            fprintf(stderr,"nc_table: use 0 or 1 in $grads definition string:\n%s\n", prd);
            ierr = -1;
            break;
          }
          if (nc_table->grads >= 0)
          {
            fprintf(stderr,"nc_table: found dublicate $grads definition, was %d, new:\n%s\n",
            nc_table->grads,prd);
            ierr = -2;
            break;
          }
          nc_table->grads=itmp;
        }
        else if (strncmp(on,"$lev_type",9) == 0 || strncmp(nn,"$LEV_TYPE",9) == 0)
        {
          /* parse input string */
          i = sscanf(prd," %*s %d:",&itmp); /* type */
          if ( i >= 1 )
          {
            ir++;
            i = strcspn( prd, ":");
            if (i > 0 && i < strlen(prd) )
            {
              prd += (i+1);
              i = sscanf(prd," %[^: ]s",nn); /* sname, no spaces */
              if (i >= 1 )
              {
                ir++;
                i = strcspn( prd, ":");
                if (i > 0 && i < strlen(prd) )
                {
                  prd += (i+1);
                  i = sscanf(prd," %[^:]s",lv); /* lname */
                  if (i >= 1 )
                  {
                    ir++;
                    i = strcspn( prd, ":");
                    if (i > 0 && i < strlen(prd) )
                    {
                      prd += (i+1);
                      i = sscanf(prd," %[^:#\n]s",pk); /* units could be last */
                      if (i >= 1 )
                      {
                        ir++;
                        i = strcspn( prd, ":");
                        if (i > 0 && i < strlen(prd) )
                        {
                          prd += (i+1);
                          i = sscanf(prd," %g",&ftmp); /* scale */
                          if (i >= 1 )
                          {
                            ir++;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          if ( ir < 4 )
          {
            ierr = -3;
            fprintf(stderr,"nc_table: error %d in $lev_type definition string:\n%s\n", ir,on);
            break;
          }
          if ( nc_table->lt != NULL )
          {
            fprintf(stderr,"nc_table: found dublicate $lev_type definition:\n%s\n",on);
            ierr = -4;
            break;
          }
          nc_table->lt = (g2nc_4Dlt*)malloc(sizeof(g2nc_4Dlt));
          if ( ir > 4 )nc_table->lt->scale = ftmp;
          else         nc_table->lt->scale = 1;
          nc_table->lt->type = itmp;


          tptr = (char *) malloc((strlen(nn)+1)*sizeof(char));
	  if (tptr) strcpy(tptr,nn);
          else ierr = 1;
          nc_table->lt->sname = tptr;

/* old version
          nc_table->lt->sname = (char *)malloc((strlen(nn)+1)*sizeof(char));
          if (nc_table->lt->sname) strcpy(nc_table->lt->sname,nn);
          else ierr = 1;
*/

          tptr = (char *) malloc((strlen(lv)+1)*sizeof(char));
          if (tptr) strcpy(tptr,lv);
          else ierr = 2;
          nc_table->lt->lname = lv;

/* old version
          nc_table->lt->lname = (char *)malloc((strlen(lv)+1)*sizeof(char));
          if (nc_table->lt->lname) strcpy(nc_table->lt->lname,lv);
          else ierr = 2;
*/

          tptr = (char *) malloc((strlen(pk)+1)*sizeof(char));
          if (tptr) strcpy(tptr,pk);
          else ierr = 3;
          nc_table->lt->units = tptr;

/* old version
          nc_table->lt->units = (char *)malloc((strlen(pk)+1)*sizeof(char));
          if (nc_table->lt->units) strcpy(nc_table->lt->units,pk);
          else ierr = 3;
 */

          if ( ierr ) break;
        }
        else if (strncmp(on,"$levs",5) == 0 || strncmp(nn,"$LEVS",5) == 0)
        {
          if ( nc_table->lv )
          {
            fprintf(stderr,"nc_table: found dublicate $levs definition:\n%s\n",on);
            ierr = -5;
            break;
          }
          if (nc_table->nlev > 0)
          {
            nc_table->lv = (float *)malloc(nc_table->nlev*sizeof(float));
            if ( nc_table->lv == NULL )
            {
              ierr = 4;
              break;
            }
          }
          else
          {
            fprintf(stderr,"nc_table: found $levs directive for undef $nlev:\n%s\n",prd);
            ierr = -6;
            break;
          }
          i = strcspn( prd, "$");
          prd += (i+4+1);

          for (i=0; i < nc_table->nlev; i++)
          {
            search_lev: ir = strcspn(prd,"+-.0123456789");
            itmp = strcspn(prd,"#"); /* inline comment */
            if ( ir < 0 || ir >= strlen(prd) || itmp < ir )
            { /* read next line */
              prd = fgets(input, _MAX_PATH, fl);
              if (prd == NULL )
              {
                fprintf(stderr,"nc_table: error reading multy line $levs \n");
                ierr = -7;
                break;
              }
              goto search_lev;
            }
            itmp = strcspn(prd,":"); /* second important fields separator before number */
            if (itmp <= ir )
            {
              fprintf(stderr,"nc_table: error entering $levs, check that there are $nlev values defined\n");
              ierr = -8;
              break;
            }
            prd += ir;
            ir = sscanf(prd,"%g",&nc_table->lv[i]);
            if ( ir < 1 )
            {
              fprintf(stderr,"nc_table: $levs formatted input error\n");
              ierr = -9;
              break;
            }
            ir = strcspn(prd," ,;:\n");
            if(ir <= strlen(prd) ) prd += ir;
            else
            {
              fprintf(stderr,"nc_table: $levs parsing error, do no found fields separator\n");
              ierr = -10;
              break;
            }
          }
          if ( ierr ) break;
        }
        else if (strncmp(on,"$endianness",11) == 0 || strncmp(nn,"$ENDIANNESS",11) == 0)
        {
          *ctmp = 0;
          itmp = 0;
          i = sscanf(prd," %*s %[^ #\n]s",ctmp); /* could be last, no spaces */
          if ( i >= 1){
            if (strcmp(ctmp,"native")==0 || strcmp(ctmp,"NATIVE")==0 || strcmp(ctmp,"Native")==0)
            {
              nc_table->endianness = 0;
            }
            else if (strcmp(ctmp,"little-endian")==0 || strcmp(ctmp,"LITTLE-ENDIAN")==0)
            {
              nc_table->endianness = 1;
            }
            else if (strcmp(ctmp,"big-endian")==0 || strcmp(ctmp,"BIG-ENDIAN")==0)
            {
              nc_table->endianness = 2;
            }
            else
            {
              itmp = 1;
            }
          }
          if ( i < 1 || itmp == 1)
          {
            fprintf(stderr,"nc_table: use native, little-endian or big-endian strings in $endianness definition:\n%s\n", prd);
            ierr = -1;
            break;
          }
          /* dublicates do not checked, last found value would be used for endianness */
        }
        else
        {
          fprintf(stderr,"nc_table: found unrecognized directive:\n%s\n", prd);
          ierr = -11;
          break;
        }
        continue;
      }
/********************************************************************
      grib2 to netcdf variable convertion rule string parsing
********************************************************************/
      ir++;
      i = strcspn( prd, ":");
      if (i > 0 && i < strlen(prd) )
      {
        prd += (i+1);
        i = sscanf(prd," %[^:]s",lv);
        if (i >= 1 )
        {
          ir++;
          i = strcspn( prd, ":");
          if (i > 0 && i < strlen(prd) )
          {
            prd += (i+1);
            i = sscanf(prd," %[^: #\n]s",nn); /* could be last, no spaces */
            if (i >= 1 )
            {
              ir++;
              i = strcspn( prd, ":");
              if (i > 0 && i < strlen(prd) )
              {
                prd += (i+1);
                i = sscanf(prd," %[^: #\n]s",pk);  /* could be last, no spaces */
                if (i >= 1 )
                {
                  ir++;
                  i = strcspn( prd, ":");
                  if (i > 0 && i < strlen(prd) )
                  {
                    prd += (i+1);
                    i = strcspn( prd, ":");
                    if (i > 0 && i < strlen(prd) ) //one more separator found, min:max
                    {
//                      i = sscanf(prd," %g: %g",&offset, &scale);
                      i = sscanf(prd," %g: %g",&min, &max);
                      if (i >= 2 )
                      {
                        ir += i;
                        i = strcspn( prd, ":");
                        if (i > 0 && i < strlen(prd) )
                        {
                          prd += (i+1);
                          i = strcspn( prd, ":");
                          if (i > 0 && i < strlen(prd) )
                          {
                            prd += (i+1);
                            i = sscanf(prd," %[^: #\n]s",defl);  /* must be last, no spaces */
                            ir += i;
                          }
                        }
                      }
                    }
                    else /* no separators more, single last input string*/
                    {
                      i = sscanf(prd," %[^: #\n]s",defl);  /* could be last, no spaces */
                      ir += i;
                    }
                  }
                }
              }
            }
          }
        }
      }
      if ( ir < 3 )
      {
        /* not eligible string, issue warning and pass */
//        fprintf(stderr,"nc_table: badly formatted string, ignore: %s",input);
//        continue;
        fprintf(stderr,"nc_table: badly formatted string:\n %s",input);
        ierr = -21;
        break;
      }
      if ( nc_table->nvc > G2NC_MAX_VARS )
      {
        fprintf(stderr,"nc_table: nvc exceed G2NC_MAX_VARS: %d %d\n",
        nc_table->nvc, G2NC_MAX_VARS);
        ierr = -22;
        break;
      }
      i = nc_table->nvc;
      nc_table->nvc++;
      old_nct = nc_table->vc;

      nc_table->vc = (g2nc_conv*) realloc((void*)old_nct, nc_table->nvc*sizeof(g2nc_conv));
      nc_table->vc[i].ignore = 0;
      nc_table->vc[i].nc_pack = 0;
      nc_table->vc[i].nc_offset = 0.;
      nc_table->vc[i].nc_scale = 1.;
      nc_table->vc[i].nc_valid_min = 0.;
      nc_table->vc[i].nc_valid_max = 0.;
      nc_table->vc[i].nc_deflate = 1;
      nc_table->vc[i].wgrib2_name = (char *)malloc((strlen(on)+1)*sizeof(char));
      if (nc_table->vc[i].wgrib2_name)
        strcpy(nc_table->vc[i].wgrib2_name,on);
      else
      {
        ierr = 21;
        break;
      }
      nc_table->vc[i].wgrib2_level = (char *)malloc((strlen(lv)+1)*sizeof(char));
      if (nc_table->vc[i].wgrib2_level)
        strcpy(nc_table->vc[i].wgrib2_level,lv);
      else
      {
        ierr = 22;
        break;
      }
      nc_table->vc[i].nc_name = (char *)malloc((strlen(nn)+1)*sizeof(char));
      if (nc_table->vc[i].nc_name)
      {
        strcpy(nc_table->vc[i].nc_name,nn);
        if (strcmp(nn,"ignore")==0 || strcmp(nn,"IGNORE")==0 || strcmp(nn,"Ignore")==0)
          nc_table->vc[i].ignore = 1;
      }
      else
      {
        ierr = 23;
        break;
      }
      if (ir < 4)continue;

      range = (max - min);
      if (strcmp(pk,"byte")==0 || strcmp(pk,"BYTE")==0 || strcmp(pk,"Byte")==0)
      {
        nc_table->vc[i].nc_pack = G2NC_PACK_BYTE;
        nc_table->vc[i].nc_scale = (float) (range/(G2NC_FILL_VALUE_BYTE - 2))*0.5;
      }
      else if (strcmp(pk,"short")==0 || strcmp(pk,"SHORT")==0 || strcmp(pk,"Short")==0)
      {
        nc_table->vc[i].nc_pack = G2NC_PACK_SHORT;
        nc_table->vc[i].nc_scale = (float) (range/(G2NC_FILL_VALUE_SHORT - 2))*0.5;
      }
      else if (strcmp(pk,"float")==0 || strcmp(pk,"FLOAT")==0 || strcmp(pk,"Float")==0)
      {
        nc_table->vc[i].nc_pack = G2NC_PACK_FLOAT;
      }
      else if (strcmp(pk,"ignore")==0 || strcmp(pk,"IGNORE")==0 || strcmp(pk,"Ignore")==0)
      {
        nc_table->vc[i].ignore = 1;
        continue;
      }
      else if (strcmp(pk,"no") == 0 || strcmp(pk,"NO") == 0 )
      {
        continue;
      }
      else if (strncmp(pk,"deflate",7)==0 || strncmp(pk,"DEFLATE",7)==0 || strncmp(pk,"Deflate",7)==0)
      {
        if ( strlen(pk) > 7 && isdigit((int)pk[7]))
        {
          nc_table->vc[i].nc_deflate = (int)pk[7];
          continue;
        }
        else
        {
          fprintf(stderr,"nc_table: unsupported deflate definition, ignored: %s in %s",pk,input);
          continue;  /* not eligible string, warning and pass */
        }
      }
      else
      {
        fprintf(stderr,"nc_table: unsupported packing type, ignored: %s in %s",pk,input);
        continue;  /* not eligible string, warning and pass */
      }
      if (nc_table->vc[i].nc_pack == G2NC_PACK_FLOAT)
      {
        if (fabs(range) < 1e-20)
        {
          fprintf(stderr,"nc_table: small valid_range specified, expected ...:float:{min}:{max}, min<max: %s",input);
          ierr = 23;
          break;
        }
      }
      else  nc_table->vc[i].nc_offset = (float) (min+max)*0.5;

      nc_table->vc[i].nc_valid_min=min;
      nc_table->vc[i].nc_valid_max=max;

      if (strncmp(defl,"deflate",7)==0 || strncmp(defl,"DEFLATE",7)==0 || strncmp(defl,"Deflate",7)==0)
      {
        if ( strlen(defl) > 7 && isdigit((int)defl[7]))
        {
          nc_table->vc[i].nc_deflate = (int)defl[7];
          continue;
        }
        else
        {
          fprintf(stderr,"nc_table: unsupported deflate definition, ignored: %s in %s",defl,input);
          continue;  /* not eligible string, warning and pass */
        }
      }
    }
    fclose(fl);
    if( ierr )
    {
      if ( ierr > 0 ) fprintf(stderr,"nc_table: allocation error\n");
      fatal_error("nc_table: fatal error parsing file %s", arg1);
    }

#ifdef DEBUG_NC
fprintf(stderr, "nc_table: total found: %d entries\n",nc_table->nvc);
for (i=0; i < nc_table->nvc; i++)
{
  fprintf(stderr, "%2d) %s:%s:%s:%d:%d:%f:%f:%f:%f:%d\n",i,
    nc_table->vc[i].wgrib2_name,
    nc_table->vc[i].wgrib2_level,
    nc_table->vc[i].nc_name,
    nc_table->vc[i].ignore,
    nc_table->vc[i].nc_pack,
    nc_table->vc[i].nc_valid_min,
    nc_table->vc[i].nc_valid_max,
    nc_table->vc[i].nc_offset,
    nc_table->vc[i].nc_scale,
    nc_table->vc[i].nc_deflate);
}
fprintf(stderr, "nc_table: $endianness=%d\n",nc_table->endianness);
if (nc_table->nlev >= 0)
{
  fprintf(stderr, "nc_table: $nlev=%d\n",nc_table->nlev);
  if (nc_table->lv)
    for (i=0; i < nc_table->nlev; i++)
      fprintf(stderr, "lev(%d)=%g\n",i,nc_table->lv[i]);
}
if( nc_table->lt )
{
  fprintf(stderr, "nc_table: found $lev_type directive:\n");
  fprintf(stderr, "$lev_type = %d:%s:%s:%s:%g\n",
    nc_table->lt->type,  nc_table->lt->sname,
    nc_table->lt->lname, nc_table->lt->units,
    nc_table->lt->scale);
}
#endif
  }
  return 0;
}

/*
 * HEADER:100:no_nc_table:setup:0:disable previously defined conversion_to_netcdf_table
 */
int f_no_nc_table(ARG0)
{
  if (mode == -1)
  {
    if (nc_table)
    {
      if (nc_table->used)
      { /* disconnect from allocated memory, it is mapped in some 'local' structure */
       nc_table = NULL;
      }
      else fatal_error("no_nc_table: disabling not used -nc_table, is it your intention?","");
    }
  }
  return 0;
}
/*
 * HEADER:100:nc_time:setup:1:netcdf, [[-]yyyymmddhhnnss]:[dt{s[ec]|m[in]|h[our]|d[ay]}], [-] is for time alignment only
 */
//nc_time option [+|-]{yyyymmddhhnn}:{dt}[mn|hr|dy]
double nc_date0 = 0;      /* undefined value... */
int    nc_date0_type = 0; /* undefined; 1 for absolute, -1 for relative (alignment only) */
double nc_dt = 0;         /* not initialized; -1 will be used for variable (undefined) step */

int f_nc_time(ARG1)
{
  char chr;
  const char *p;
  int year,month,day,hour,minute,second,dt_val,dt_conv,err_code;
  char dt_type[25];

  if (mode == -1)
  {

    if (strlen(arg1) == 0) return 0;
    /* default initialization, clear any previously defined values */
    nc_date0_type = 0;
    nc_date0 = 0;
    nc_dt = 0;

    sscanf(arg1,"%c",&chr);

    p=arg1;
    if (chr==':')
    { /* separator symbol is first, time step only, any long string*/
      p++;
      goto get_time_step;
    }
    if (strlen(p) < 14)
    { /* time step only */
      goto get_time_step;
    }
    /* include date */
    nc_date0_type = 1;    /*absolute */
    if (chr=='-')
    {
      p++;
      nc_date0_type = -1; /* relative*/
    }
    else if (chr == '+')
    {
      p++;
    }
    if (sscanf(p,"%4d%2d%2d%2d%2d%2d",&year,&month,&day,&hour,&minute,&second) != 6)
      fatal_error("nc_time: bad format of yyyymmddhhnnss value in nc_time: %s", arg1);

    if(year<0 || year>9999 || month<1 || month>12 || day<1 || day>31 ||
       hour <0 || hour>23 || minute < 0 || minute >59 || second < 0 || second >59)
      fatal_error("nc_time: bad value of yyyymmddhhnnss value in nc_time: %s", arg1);

#ifdef DEBUG_NC
printf("nc_time: date0_type=%d date0=%d.%d.%d %d:%d:%d\n",nc_date0_type,year,month,day,hour,minute,second);
#endif
    nc_date0 = get_unixtime(year, month, day, hour, minute, second, &err_code);
    if(err_code)
      fatal_error("nc_time: time [sec] is out of range for this OS","");

    p+=14;
    if (strlen(p) <= 1)
    {
      if (nc_date0_type > 0) return 0;
      fatal_error("nc_time: negative yyyymmddhhnnss (relative date) need time step in -nc_time option: %s", arg1);
    }
    p++; /*pass separator symbol ':' or other*/

get_time_step:

    if (sscanf(p,"%d%s",&dt_val,dt_type) != 2)
      fatal_error("nc_time: bad time step in nc_time: %s", p);

    if( strlen(dt_type) == 0)
      fatal_error("nc_time: undef time step type in nc_time: %s", arg1);

    p = dt_type;
    dt_conv=0;
    if (*p=='s' || *p=='S')
     dt_conv=1;
    else if (*p=='m' || *p=='M')
     dt_conv=60;
    else if (*p=='h' || *p=='H')
     dt_conv=3600;
    else if (*p=='d' || *p=='D')
     dt_conv=86400;
    else
      fatal_error("nc_time: unsupported time step type in nc_time: %s", p);

//    if (align_only && dt_conv > 86400*30) // unclear behavior for monthly data...
//      fatal_error("netcdf: unsupported large time increment in nc_time %s", arg1);
    nc_dt = (double)dt_val*(double)dt_conv;  // convert to seconds

#ifdef DEBUG_NC
printf("nc_time: time_step_value=%d, type=%s, val_sec=%.1lf\n",dt_val,dt_type,nc_dt);
#endif
  }
  return 0;
}
/*
 * HEADER:100:no_nc_time:setup:0:netcdf, disable previously defined initial or relative date and time step
 */
int f_no_nc_time(ARG0)
{
  if (mode == -1)
  {
    nc_date0 = 0;      /* undefined value... */
    nc_date0_type = 0; /* undefined; 1 for absolute, -1 for relative (alignment only) */
    nc_dt = 0;         /* not initialized; -1 will be used for variable (undefined) step */
  }
  return 0;
}
/*
 * HEADER:100:nc4:setup:0:use netcdf4 (compressed, controlled endianness etc)
 */
int nc4 = 0;

int f_nc4(ARG0)
{
  if (mode == -1) nc4 = 1;
  return 0;
}
/*
 * HEADER:100:nc3:setup:0:use netcdf3 (classic)
 */
int f_nc3(ARG0)
{
  if (mode == -1) nc4 = 0;
  return 0;
}

#else

int f_nc_nlev(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_pack(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_table(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_table(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_pack(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_grads(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_grads(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_time(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_time(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc4(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc3(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}

#endif



