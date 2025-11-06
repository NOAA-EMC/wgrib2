/** @file
 * @brief GRIB table definitions for various meteorological centers.
 * 
 * Now have two types of grib table:
 * 
 * Primary
 *      has the names for WMO defined variables
 *      has the names for its locally defined variables
 *      as of 1/2021, only ncep and ecmwf can be primary
 * 
 * Secondary
 *      has the names for its locally defined variables
 *      The WMO defined variables are specified by the primary grib tables
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2004 | J. Hyvätti | Initial 
 * 9/13/2004 | J. Hyvätti | Manually converted from gribtab.  I would say this is as easy to
 *                          update as the colon-separated file, and we do not need separate
 *                          compilation step from tab form to C form.
 * 9/14/2004 | J. Hyvätti | Converted automatically wgrib-beta parameter names with
 *                          g2lib-1.0.3/params.f conversion table (as amended by Wesley
 *                          Ebisuzaki).
 * 1/7/2021 | W. Ebisuzaki | Added ECMWF grib table
 * @author Jaakko Hyvätti <jaakko.hyvatti@foreca.com> @date 2004
 */

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

/** Struct for NCEP GRIB table. */
struct gribtable_s NCEP_gribtable[] = {
#include "gribtables/ncep/gribtable.dat"
    /* END MARKER */
    { -1, -1, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL }
};

/** Struct for ECMWF GRIB table. */
struct gribtable_s ECMWF_gribtable[] = {
#include "gribtables/ecmwf/ECMWF_gribtable.dat"
    /* END MARKER */
    { -1, -1, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL }
};

/** Struct for DWD1 GRIB table. */
struct gribtable_s DWD1_gribtable[] = {
#include "gribtables/dwd/dwd_gribtable.dat"
    /* END MARKER */
    { -1, -1, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL }
};

/** Struct for local GRIB tables. */
struct gribtable_s local_gribtable[] = {
#include "gribtables/ndfd/NDFD_gribtable.dat"
#include "gribtables/mrms/MRMS_gribtable.dat"
#include "gribtables/bom/BOM_gribtable.dat"
#include "gribtables/kma/KMA_gribtable.dat"
#include "gribtables/misc/misc_gribtable.dat"
#include "gribtables/usaf/USAF_gribtable.dat"
#include "gribtables/nesdis/NESDIS_gribtable.dat"
    /* END MARKER */
    { -1, -1, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL }
};

#ifdef USE_TIGGE

/** Struct for TIGGE GRIB table. */
struct gribtable_s tigge_gribtable[] = {
#include "gribtables/tigge/tigge_gribtable.dat"
    /* END MARKER */
    { -1, -1, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL }
};

#endif
