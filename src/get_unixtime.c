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

//#define DEBUG_NC

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wgrib2.h"
#include "fnlist.h"


/*
 * Find UTC "seconds since 1970-01-01 00:00:00.0 0:00"
 */
double get_unixtime(int year, int month, int day, int hour,
                    int minute, int second, int *err_code)
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
  if (year >= 1970 && local_t <= 0)
  {
    *err_code = 3;
    return 0;
  }
  gmt_tm = gmtime(&local_t);
  gmt_t = mktime(gmt_tm);
  return ((double)(local_t + (local_t-gmt_t)));
}
