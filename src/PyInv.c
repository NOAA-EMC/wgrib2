#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

extern const char *item_deliminator;
extern unsigned int npnts, nx_, ny_;
/*
 * function to dump GRIB metadata as Python dictionary
 * public domain 8/2020 George Trojan
 */

/*
 * HEADER:200:pyinv:inv:0:miscelaneous metadata for pywgrib2_XXX (experimental)
 */
int f_pyinv(ARG0)
{
    if (mode < 0) return 0;

    f_ext_name(call_ARG0(inv_out, NULL));
    strcat(inv_out, item_deliminator);
    inv_out += strlen(inv_out);

    f_lev(call_ARG0(inv_out, NULL));
    strcat(inv_out, item_deliminator);
    inv_out += strlen(inv_out);
    
    f_ftime(call_ARG0(inv_out, NULL));
    strcat(inv_out, item_deliminator);
    inv_out += strlen(inv_out);
    
    inv_out += sprintf(inv_out, "pyinv={");
    /* Section 0 */
    {
        inv_out += sprintf(inv_out, "'discipline':%d", GB2_Discipline(sec));
    }
    /* Section 1 */
    {
        int year, month, day, hour, minute, second;
        const char *string;
        char tmp[20];
        int ctr = GB2_Center(sec);
        int subctr = GB2_Subcenter(sec);
        switch (ctr) {
#include "code_table0.dat"
        }
        inv_out += sprintf(inv_out, ",'centre':'%d - %s'", ctr, string);
        string = NULL;
        if (ctr == 7) {
            switch (subctr) {
#include "ncep_tableC.dat"
            }
        }
        if (string == NULL) {
            inv_out += sprintf(inv_out, ",'subcentre':'%d'", subctr);
        } else {
            inv_out += sprintf(inv_out, ",'subcentre':'%s'", string);
        }
        inv_out += sprintf(inv_out, ",'mastertab':%d", GB2_MasterTable(sec));
        inv_out += sprintf(inv_out, ",'localtab':%d", GB2_LocalTable(sec));
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
        inv_out += sprintf(inv_out, ",'reftime':'%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d'",
                           year, month, day, hour, minute, second); 
    }
    /* Section 3 */
    {
        int n, ierr, gdtnum;
        int gdtmpl[200];
        int gdtlen = sizeof(gdtmpl) / sizeof(gdtmpl[0]);
        inv_out += sprintf(inv_out, ",'npts':%u,'nx':%u,'ny':%u", npnts, nx_, ny_);
        ierr = mk_gdt(sec, &gdtnum, gdtmpl, &gdtlen);
        if (ierr == 0) {
            inv_out += sprintf(inv_out, ",'gdtnum':%d,'gdtmpl':[%d", gdtnum, gdtmpl[0]);
            for (n = 1; n < gdtlen; n++) {
                inv_out += sprintf(inv_out, ",%d", gdtmpl[n]);
            }
            inv_out += sprintf(inv_out, "]");
        }
    }
    /* Section 4 */
    {
        int year, month, day, hour, minute, second;
        int level_type1, level_type2;
        float val1, val2;
        int undef_val1, undef_val2;
        char desc[STRING_SIZE], units[STRING_SIZE];
        getName(sec, 2, NULL, NULL, desc, units); 
        inv_out += sprintf(inv_out, ",'long_name':'%s'", desc);
        inv_out += sprintf(inv_out, ",'units':'%s'", units);
        inv_out += sprintf(inv_out, ",'pdt':%d", code_table_4_0(sec));
        inv_out += sprintf(inv_out, ",'parmcat':%d", GB2_ParmCat(sec));
        inv_out += sprintf(inv_out, ",'parmnum':%d", GB2_ParmNum(sec));
        if (start_ft(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            inv_out += sprintf(inv_out, ",'start_ft':'%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d'",
                               year, month, day, hour, minute, second);
        }
        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            inv_out += sprintf(inv_out, ",'end_ft':'%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d'",
                               year, month, day, hour, minute, second);
        }
        fixed_surfaces(sec, &level_type1, &val1, &undef_val1,
                       &level_type2, &val2, &undef_val2);
        inv_out += sprintf(inv_out, ",'bot_level_code':%d", level_type1);
        if (undef_val1 == 0) {
            inv_out += sprintf(inv_out, ",'bot_level_value':%lg", val1);
        }
        inv_out += sprintf(inv_out, ",'top_level_code':%d", level_type2);
        if (undef_val2 == 0) {
            inv_out += sprintf(inv_out, ",'top_level_value':%lg", val2);
        }
    }
    inv_out += sprintf(inv_out, "}");
    return 0;
}
