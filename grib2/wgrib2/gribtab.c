/*
 This file gribtab.c is a part of wgrib2
 copyright 2004 Jaakko Hyvätti

    gribtab.c is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    gribtab.c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


/*
2004-09-13
Jaakko Hyvätti <jaakko.hyvatti@foreca.com>
Manually converted from gribtab.  I would say this is as easy to
update as the colon-separated file, and we do not need separate
compilation step from tab form to C form.

2004-09-14
Jaakko Hyvätti <jaakko.hyvatti@foreca.com>
Converted automatically wgrib-beta parameter names with
g2lib-1.0.3/params.f conversion table (as amended by Wesley
Ebisuzaki).

*/

#include "wgrib2.h"

/*
  int disc;   Section 0 Discipline
  int mtab_set;   Section 1 Master Tables Version Number
  int mtab_low;   Section 1 Master Tables Version Number
  int mtab_high;   Section 1 Master Tables Version Number
  int cntr;   Section 1 originating centre, used for local tables
  int ltab;   Section 1 Local Tables Version Number
  int pcat;   Section 4 Template 4.0 Parameter category
  int pnum;   Section 4 Template 4.0 Parameter number
  const char *name;
  const char *desc;
  const char *unit;
*/

struct gribtable_s gribtable[] = {

#include "gribtable.dat"
#include "misc_gribtable.dat"
#include "NDFD_gribtable.dat"
#include "MRMS_gribtable.dat"
#include "BOM_gribtable.dat"

  /* END MARKER */
  { -1, -1, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL }

};

#ifdef USE_TIGGE

struct gribtable_s tigge_gribtable[] = {

#include "tigge_gribtable.dat"
  /* END MARKER */
  { -1, -1, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL }

};

#endif
