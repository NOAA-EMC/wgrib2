#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "CodeTable4_4.h"
/*
 * some routines that involve Sec4
 *
 * Public Domain 2006: Wesley Ebisuzaki
 * 1/2007  cleanup M Schwarb
 * 7/2009 bug fix (buffer overflow) Reinoud Bokhorst
 */

/*
 * HEADER:400:Sec4:inv:0:Sec 4 values (Product definition section)
 */
int f_Sec4(ARG0) {
    int pdtsize;
    if (mode >= 0) {
	pdtsize =  prod_def_temp_size(sec);
	sprintf(inv_out,"Sec4 len=%u #vert coordinate=%u Product Defn Template=4.%d size=%d free=%d", 
          uint4(&(sec[4][0])), uint2(&(sec[4][5])),GB2_ProdDefTemplateNo(sec),
	  pdtsize, uint4(&(sec[4][0])) - pdtsize - 8*uint2(&(sec[4][5])));
	inv_out += strlen(inv_out);
    }
    return 0;
}

/*
 * HEADER:400:processid:inv:0:process id (locally defined)
 */
int f_processid(ARG0) {
    int i;
    const char *space;


    if (mode >= 0) {
        space = "";
	i = observation_generating_process_identifier(sec);
	if (i >= 0) {
	    sprintf(inv_out,"%sobservation generating process=%d", space, i);
	    inv_out += strlen(inv_out);
	    space = " ";
	}

        i = background_generating_process_identifier(sec);
	if (i >= 0) {
	    sprintf(inv_out,"%sbackground generating process=%d", space, i);
	    inv_out += strlen(inv_out);
	    space = " ";
	}

        i = analysis_or_forecast_generating_process_identifier(sec);
	if (i >= 0) {
	    sprintf(inv_out,"%sforecast generating process=%d", space, i);
	    inv_out += strlen(inv_out);
	    space = " ";
	}
    }
    return 0;
}


/*
 * HEADER:400:0xSec:inv:1:Hex dump of section X (0..8)
 */
int f_0xSec(ARG1) {
    int i, sec_no;
    unsigned int j, len;
    double tot;
    unsigned char *s;

    if (mode >= 0) {
        sec_no = i = atoi(arg1);
	if (sec_no < 0 || sec_no > 8) fatal_error_i("0xSec: bad section number %d", sec_no);
        if ( (s = sec[sec_no]) == NULL) {
            sprintf(inv_out,"sec%d is missing", sec_no);
            return 0;
        }
        if (sec_no == 0) { 
	    len = GB2_Sec0_size;
	}
	else if (sec_no == 8) {
	    len = GB2_Sec8_size;
        }
        else { 
            len = uint4(&(sec[sec_no][0]));
        }

	/* Calculate number of bytes to print */

	if (mode == 0) tot = 2.0 * len;
	else if (mode == 1) tot = 3.0 * len;
	else {		// mode >= 2
	    j = len;
	    i = 0;
	    while (j) {
		j /= 10;
		i++;
	    }
	    // i = maximum number of digits in address
	    tot = len*i			// maximum of characters for address
		  + 4*len		// blank : 2 digits
		  + 4*(len % 15);	// new line (2 for windows) + two blanks
        }

	if (tot >= INV_BUFFER - 1000) {	// 1000 is precaution
	    sprintf(inv_out,"Sec%d=too long to print",sec_no);
	    return 0;
	}

        if (mode == 0) sprintf(inv_out,"Sec%d(1..%u)=0x",sec_no,len);
	else sprintf(inv_out,"Sec%d(1..%u)=",sec_no,len);
	inv_out += strlen(inv_out);

	if (mode == 0) {
            while (len--) {
         	sprintf(inv_out,"%.2x", *s++);
		inv_out += strlen(inv_out);
            }
	}
	else if (mode == 1) {
            for (j = 1; j <= len; j++) {
		sprintf(inv_out," %.2x", *s++);
		inv_out += strlen(inv_out);
	    }
	}
	else if (mode >= 2) {
            for (j = 1; j <= len; j++) {
		sprintf(inv_out,"%u:%.2x", j, *s++);
		inv_out += strlen(inv_out);
		sprintf(inv_out,j % 15 == 0 ? "\n  " : " ");
		inv_out += strlen(inv_out);
	    }
	}
    }
    return 0;
}

/*
 * HEADER:400:var:inv:0:short variable name
 */

int f_var(ARG0) {
    if (mode >= 0) {
        getName(sec, mode, inv_out, NULL, NULL, NULL);
	inv_out += strlen(inv_out);
    }
    return 0;
}

/*
 * HEADER:400:varX:inv:0:raw variable name - discipline mastertab localtab center parmcat parmnum
 */

int f_varX(ARG0) {

    int discipline, center,mastertab,localtab,parmcat,parmnum;

    if (mode >= 0) {
        discipline = GB2_Discipline(sec);
        center = GB2_Center(sec);
        mastertab = GB2_MasterTable(sec);
        localtab = GB2_LocalTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);
	if (mode == 0) {
            if (parmnum == 255) {
                sprintf(inv_out,"missing");
                return 0;
	    }
	    sprintf(inv_out,"var%d_%d_%d_%d_%d_%d", discipline, mastertab, localtab, center, parmcat, parmnum);
        }
	else if (mode > 0) {
	    if (parmnum == 255) {
                sprintf(inv_out,"missing definition [?]");
                return 0;
            }
            if (parmnum < 192) {
                sprintf(inv_out,"var discipline=%d master_table=%d parmcat=%d parm=%d", 
                      discipline, mastertab, parmcat, parmnum);
	    }
            else {
                sprintf(inv_out,"var discipline=%d center=%d local_table=%d parmcat=%d parm=%d",
                  discipline, center, localtab, parmcat, parmnum);
            }
        }
    }
    return 0;
}

/*
 * HEADER:400:pdt:inv:0:Product Definition Table (Code Table 4.0)
 */

int f_pdt(ARG0) {
    if (mode < 0) return 0;
    sprintf(inv_out,"pdt=%d", code_table_4_0(sec));
    return 0;
}
