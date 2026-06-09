#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cnames.h"
#include "pds4.h"
#include "grib.h"

/* cnames.c 				Wesley Ebisuzaki
 *
 * returns strings with either variable name or comment field
 * v1.4 4/98
 * reanalysis can use process 180 and subcenter 0
 *
 * Add DWD tables 2, 201, 202, 203      Helmut P. Frank, DWD, FE13
 *                                      Thu Aug 23 09:28:34 GMT 2001
 * add DWD tables 204, 205              H. Frank, 10-19-2005
 * LAMI => DWD				11/2008 Davide Sacchetti 
 * add JRA-55 table 200
 */


extern const  struct ParmTable parm_table_ncep_opn[256];
extern const  struct ParmTable parm_table_ncep_reanal[256];
extern const  struct ParmTable parm_table_nceptab_128[256];
extern const  struct ParmTable parm_table_nceptab_129[256];
extern const  struct ParmTable parm_table_nceptab_130[256];
extern const  struct ParmTable parm_table_nceptab_131[256];
extern const  struct ParmTable parm_table_nceptab_133[256];
extern const  struct ParmTable parm_table_nceptab_140[256];
extern const  struct ParmTable parm_table_nceptab_141[256];
extern const  struct ParmTable parm_table_mdl_nceptab[256];

extern const  struct ParmTable parm_table_ecmwf_128[256];
extern const  struct ParmTable parm_table_ecmwf_129[256];
extern const  struct ParmTable parm_table_ecmwf_130[256];
extern const  struct ParmTable parm_table_ecmwf_131[256];
extern const  struct ParmTable parm_table_ecmwf_132[256];
extern const  struct ParmTable parm_table_ecmwf_133[256];
extern const  struct ParmTable parm_table_ecmwf_140[256];
extern const  struct ParmTable parm_table_ecmwf_150[256];
extern const  struct ParmTable parm_table_ecmwf_151[256];
extern const  struct ParmTable parm_table_ecmwf_160[256];
extern const  struct ParmTable parm_table_ecmwf_162[256];
extern const  struct ParmTable parm_table_ecmwf_170[256];
extern const  struct ParmTable parm_table_ecmwf_171[256];
extern const  struct ParmTable parm_table_ecmwf_172[256];
extern const  struct ParmTable parm_table_ecmwf_173[256];
extern const  struct ParmTable parm_table_ecmwf_174[256];
extern const  struct ParmTable parm_table_ecmwf_180[256];
extern const  struct ParmTable parm_table_ecmwf_190[256];
extern const  struct ParmTable parm_table_ecmwf_200[256];
extern const  struct ParmTable parm_table_ecmwf_210[256];
extern const  struct ParmTable parm_table_ecmwf_211[256];
extern const  struct ParmTable parm_table_ecmwf_228[256];
extern struct ParmTable parm_table_user[256];
extern const  struct ParmTable parm_table_dwd_002[256];
extern const  struct ParmTable parm_table_dwd_201[256];
extern const  struct ParmTable parm_table_dwd_202[256];
extern const  struct ParmTable parm_table_dwd_203[256];
extern const  struct ParmTable parm_table_dwd_204[256];
extern const  struct ParmTable parm_table_dwd_205[256];
extern const  struct ParmTable parm_table_cptec_254[256];
extern const  struct ParmTable parm_table_jra55_200[256];

extern enum Def_NCEP_Table def_ncep_table;
extern int cmc_eq_ncep;

/*
 * returns pointer to the parameter table
 */



static const struct ParmTable *Parm_Table(unsigned char *pds) {

    int i, center, subcenter, ptable, process;
    static int missing_count = 0, reanal_opn_count = 0;

    center = PDS_Center(pds);
    subcenter = PDS_Subcenter(pds);
    ptable = PDS_Vsn(pds);

    /* CMC (54) tables look like NCEP tables */
    if (center == CMC && cmc_eq_ncep) center = NMC;

#ifdef P_TABLE_FIRST
    i = setup_user_table(center, subcenter, ptable);
    if (i == 1) return &parm_table_user[0];
#endif
    /* figure out if NCEP opn or reanalysis */
    if (center == NMC && ptable <= 3) {
	if (subcenter == 1) return &parm_table_ncep_reanal[0];
	if (subcenter == 14) return &parm_table_mdl_nceptab[0];
        process = PDS_Model(pds);
	if (subcenter != 0 || (process != 80 && process != 180) || 
		(ptable != 1 && ptable != 2)) 
            return &parm_table_ncep_opn[0];

	/* at this point could be either the opn or reanalysis table */
	if (def_ncep_table == opn_nowarn) return &parm_table_ncep_opn[0];
	if (def_ncep_table == rean_nowarn) return &parm_table_ncep_reanal[0];
        if (reanal_opn_count++ == 0) {
	    fprintf(stderr, "Using NCEP %s table, see -ncep_opn, -ncep_rean options\n",
               (def_ncep_table == opn) ?  "opn" : "reanalysis");
	}
        return (def_ncep_table == opn) ?  &parm_table_ncep_opn[0] 
		: &parm_table_ncep_reanal[0];
    }

    if (center == NMC) {
        if (ptable == 128) return &parm_table_nceptab_128[0];
        if (ptable == 129) return &parm_table_nceptab_129[0];
        if (ptable == 130) return &parm_table_nceptab_130[0];
        if (ptable == 131) return &parm_table_nceptab_131[0];
        if (ptable == 132) return &parm_table_ncep_reanal[0];
        if (ptable == 133) return &parm_table_nceptab_133[0];
        if (ptable == 140) return &parm_table_nceptab_140[0];
        if (ptable == 141) return &parm_table_nceptab_141[0];
    }
    if (center == ECMWF) {
        if (ptable == 128) return &parm_table_ecmwf_128[0];
        if (ptable == 129) return &parm_table_ecmwf_129[0];
        if (ptable == 130) return &parm_table_ecmwf_130[0];
        if (ptable == 131) return &parm_table_ecmwf_131[0];
        if (ptable == 132) return &parm_table_ecmwf_132[0];
        if (ptable == 133) return &parm_table_ecmwf_133[0];
        if (ptable == 140) return &parm_table_ecmwf_140[0];
        if (ptable == 150) return &parm_table_ecmwf_150[0];
        if (ptable == 151) return &parm_table_ecmwf_151[0];
        if (ptable == 160) return &parm_table_ecmwf_160[0];
        if (ptable == 162) return &parm_table_ecmwf_162[0];
        if (ptable == 170) return &parm_table_ecmwf_170[0];
        if (ptable == 171) return &parm_table_ecmwf_171[0];
        if (ptable == 172) return &parm_table_ecmwf_172[0];
        if (ptable == 173) return &parm_table_ecmwf_173[0];
        if (ptable == 174) return &parm_table_ecmwf_174[0];
        if (ptable == 180) return &parm_table_ecmwf_180[0];
        if (ptable == 190) return &parm_table_ecmwf_190[0];
        if (ptable == 200) return &parm_table_ecmwf_200[0];
        if (ptable == 210) return &parm_table_ecmwf_210[0];
        if (ptable == 211) return &parm_table_ecmwf_211[0];
        if (ptable == 228) return &parm_table_ecmwf_228[0];
    }
    /* if (center == DWD || center == CHM || center == LAMI) { */
    if (center == DWD || center == CHM) {
        if (ptable ==   2) return &parm_table_dwd_002[0];
        if (ptable == 201) return &parm_table_dwd_201[0];
        if (ptable == 202) return &parm_table_dwd_202[0];
        if (ptable == 203) return &parm_table_dwd_203[0];
        if (ptable == 204) return &parm_table_dwd_204[0];
        if (ptable == 205) return &parm_table_dwd_205[0];
    }
    if (center == CPTEC) {
	if (ptable == 254) return &parm_table_cptec_254[0];
    }
    if (center == JMA) {
	if (ptable == 200) return &parm_table_jra55_200[0];
    }


#ifndef P_TABLE_FIRST
    i = setup_user_table(center, subcenter, ptable);
    if (i == 1) return &parm_table_user[0];
#endif

    if ((ptable > 3 || (PDS_PARAM(pds)) > 127) && missing_count++ == 0) {
	fprintf(stderr,
            "\nUndefined parameter table (center %d-%d table %d), using NCEP-opn\n",
            center, subcenter, ptable);
    }
    return &parm_table_ncep_opn[0];
}

/*
 * return name field of PDS_PARAM(pds)
 */

char *k5toa(unsigned char *pds) {

    return (Parm_Table(pds) + PDS_PARAM(pds))->name;
}

/*
 * return comment field of the PDS_PARAM(pds)
 */

char *k5_comments(unsigned char *pds) {

    return (Parm_Table(pds) + PDS_PARAM(pds))->comment;
}
