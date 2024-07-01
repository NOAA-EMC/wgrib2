#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wgrib2.h"
#include "grb2.h"

extern struct gribtable_s NCEP_gribtable[], ECMWF_gribtable[], DWD1_gribtable[], local_gribtable[];
extern struct gribtable_s *user_gribtable;

static struct gribtable_s *search_gribtable(struct gribtable_s *gribtable, unsigned char **sec);

#ifdef USE_TIGGE
extern int tigge;
extern struct gribtable_s tigge_gribtable[];
#endif
/*
 * get the name information    2006 Public Domain  Wesley Ebisuzaki
 *
 * if inv_out, name, desc, unit == NULL, not used
 *
 * v1.0 Wesley Ebisuzaki 2006
 * v1.1 Wesley Ebisuzaki 4/2007 netcdf support
 * v1.2 Wesley Ebisuzaki 4/2007 multiple table support
 * v1.3 Wesley Ebisuzaki 6/2011 make parameter cat >= 192 local
 * v1.4 Wesley Ebisuzaki 2/2012 fixed search_gribtab for local tables
 * v1.5 Wesley Ebisuzaki 4/2013 gribtab -> gribtable, added user_gribtable
 * v1.6 Wesley Ebisuzaki 1/2021 use NCEP or ECMWF names as default, local table handled epar
 */

extern int names;

int getName_all(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *unit, int *mset, int *mlow, int *mhigh) {

    int discipline, center, mastertab, localtab, parmcat, parmnum;
    int pdt, use_local_table;
    struct gribtable_s *p;
    const char *p_unit;

    p = NULL;

    /* check USER grib table first */
    if (user_gribtable != NULL) p = search_gribtable(user_gribtable, sec);

    /* use either NCEP or ECMWF tables */
    /* TIGGE is consider an extension of NCEP */
    /* if NCEP, include TIGGE defintions as before, otherwise ignore TIGGE */

    if (p == NULL && names == ECMWF) p = search_gribtable(ECMWF_gribtable, sec);
    if (p == NULL && names == DWD1) p = search_gribtable(DWD1_gribtable, sec);
    if (p == NULL && names == NCEP) {
#ifdef USE_TIGGE
        if (tigge && p == NULL) p = search_gribtable(tigge_gribtable, sec);		/* tigge is default table */
#endif
        if (p == NULL) p = search_gribtable(NCEP_gribtable, sec);
#ifdef USE_TIGGE
        /* if undefined and a tigge file */
        if (p == NULL && !tigge && (code_table_1_3(sec) == 4 || code_table_1_3(sec) == 5)) p = search_gribtable(tigge_gribtable, sec);
#endif
    }

    /* check local tables */
    if (p == NULL) {
	discipline = GB2_Discipline(sec);
        center = GB2_Center(sec);
        mastertab = GB2_MasterTable(sec);
        localtab = GB2_LocalTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);
        use_local_table = (mastertab == 255) ? 1 : 0;
        if ((parmnum >= 192 && parmnum <= 254) || (parmcat >= 192 && parmcat <= 254)
            || (discipline >= 192 && discipline <= 254) ) use_local_table = 1;

        if (use_local_table) {
	   if (center == ECMWF && names != ECMWF) p = search_gribtable(ECMWF_gribtable, sec);
	   else if (center == DWD1 && names != DWD1) p = search_gribtable(DWD1_gribtable, sec);
	   else if (center == DWD2 && names != DWD1) p = search_gribtable(DWD1_gribtable, sec);
	   else if (center == NCEP && names != NCEP) p = search_gribtable(NCEP_gribtable, sec);
	   else p = search_gribtable(local_gribtable, sec);
	}
    }

    p_unit = "unit";
    if (p) {
        p_unit = p->unit;
        pdt = code_table_4_0(sec);
	if (pdt == 5 || pdt == 9) p_unit = "prob";
    }

    if (p) {
        if (name) strcpy(name, p->name);
	if (desc) strcpy(desc, p->desc);
	if (unit) strcpy(unit, p_unit);

	if (inv_out) {
	    sprintf(inv_out, "%s", p->name);
	    inv_out += strlen(inv_out);
            if (mode) sprintf(inv_out," %s [%s]", p->desc, p_unit);
        }
	*mset = p->mtab_set;
	*mlow = p->mtab_low;
	*mhigh = p->mtab_high;
    }
    else {
        discipline = GB2_Discipline(sec);
        center = GB2_Center(sec);
        mastertab = GB2_MasterTable(sec);
        localtab = GB2_LocalTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);
	*mset = 0;
	*mlow = 0;
	*mhigh = 255;

        if (name) sprintf(name,"var%d_%d_%d",discipline,parmcat,parmnum);
	if (desc) strcpy(desc,"desc");
	if (unit) strcpy(unit,p_unit);

	if (inv_out) {
	    if ((parmnum >= 192 && parmnum <= 254) || (parmcat >= 192 && parmcat <= 254)
		      || (discipline >= 192 && discipline <= 254) ) {
	        sprintf(inv_out,"var discipline=%d center=%d local_table=%d parmcat=%d parm=%d",
                  discipline, center, localtab, parmcat, parmnum);
	    }
	    else {
                sprintf(inv_out,"var discipline=%d master_table=%d parmcat=%d parm=%d", 
                  discipline, mastertab, parmcat, parmnum);
            }
	}
    }

    return 0;
}

int getName(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *unit) {
   int mset, mlow, mhigh;

   return getName_all(sec, mode, inv_out, name, desc, unit, &mset, &mlow, &mhigh);
}

/*
 * search the grib table
 */

static struct gribtable_s *search_gribtable(struct gribtable_s *p, unsigned char **sec) {

    int discipline, center, mastertab, localtab, parmcat, parmnum;
    int use_local_table;
    static int count = 0;

    if (p == NULL) return NULL;

    discipline = GB2_Discipline(sec);
    center = GB2_Center(sec);
    mastertab = GB2_MasterTable(sec);
    localtab = GB2_LocalTable(sec);
    parmcat = GB2_ParmCat(sec);
    parmnum = GB2_ParmNum(sec);

    use_local_table = (mastertab == 255) ? 1 : 0;
    if ((parmnum >= 192 && parmnum <= 254) || (parmcat >= 192 && parmcat <= 254)
	|| (discipline >= 192 && discipline <= 254) ) use_local_table = 1;
   
    if (use_local_table == 1 && localtab == 0) {
	if (count++ < 6 ) fprintf(stderr,"**** ERROR: local table = 0 is not allowed, set to 1 ***\n");
	localtab = 1;
    }
    if (use_local_table == 1 && localtab == 255) {
	fatal_error("local gribtable is undefined (255)","");
    }

    if (! use_local_table) {
        for (; p->disc >= 0; p++) {
            if (discipline == p->disc && (mastertab >= p->mtab_low) && (mastertab <= p->mtab_high) &&
                parmcat == p->pcat && parmnum == p->pnum) {
                return p;
            }
        }
    }
    else {
//	printf(">> cname local find: disc %d center %d localtab %d pcat %d pnum %d\n", discipline, center, localtab, parmcat, parmnum);
        for (; p->disc >= 0; p++) {
            if (discipline == p->disc && center == p->cntr && localtab == p->ltab && 
                parmcat == p->pcat && parmnum == p->pnum) {
                return p;
	    }
	}
    }
    return NULL;
}
