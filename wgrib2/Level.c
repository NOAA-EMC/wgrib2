#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Levels.c
 *   2006: public domain wesley ebisuzaki
 *   1/2007: cleanup M. Schwarb
 *   1/2007: Caser Tejeda Hernandez found error in meter underground
 *   2/2007: level 11
 *   2/2007: spelling error fixed
 *   9/2008: type 241 added Nick Lott 
 */


/*
 * HEADER:200:lev:inv:0:level (code table 4.5)
 */

/* code table 4.5 */

const char *level_table[192] = {
/* 0 */ "reserved",
/* 1 */ "surface",
/* 2 */ "cloud base",
/* 3 */ "cloud top",
/* 4 */ "0C isotherm",
/* 5 */ "level of adiabatic condensation from sfc",
/* 6 */ "max wind",
/* 7 */ "tropopause",
/* 8 */ "top of atmosphere",
/* 9 */ "sea bottom",
/* 10 */ "entire atmosphere",
/* 11 */ "cumulonimbus base",
/* 12 */ "cumulonimbus top",
/* 13 */ "lowest level %g%% integrated cloud cover",
/* 14 */ "level of free convection",
/* 15 */ "convection condensation level",
/* 16 */ "level of neutral buoyancy",
/* 17 */ "reserved",
/* 18 */ "reserved",
/* 19 */ "reserved",
/* 20 */ "%g K level",
/* 21 */ "lowest level > %g kg/m^3",
/* 22 */ "highest level > %g kg/m^3",
/* 23 */ "lowest level > %g Bq/m^3",
/* 24 */ "highest level > %g Bg/m^3",
/* 25 */ "reserved",
/* 26 */ "reserved",
/* 27 */ "reserved",
/* 28 */ "reserved",
/* 29 */ "reserved",
/* 30 */ "reserved",
/* 31 */ "reserved",
/* 32 */ "reserved",
/* 33 */ "reserved",
/* 34 */ "reserved",
/* 35 */ "reserved",
/* 36 */ "reserved",
/* 37 */ "reserved",
/* 38 */ "reserved",
/* 39 */ "reserved",
/* 40 */ "reserved",
/* 41 */ "reserved",
/* 42 */ "reserved",
/* 43 */ "reserved",
/* 44 */ "reserved",
/* 45 */ "reserved",
/* 46 */ "reserved",
/* 47 */ "reserved",
/* 48 */ "reserved",
/* 49 */ "reserved",
/* 50 */ "reserved",
/* 51 */ "reserved",
/* 52 */ "reserved",
/* 53 */ "reserved",
/* 54 */ "reserved",
/* 55 */ "reserved",
/* 56 */ "reserved",
/* 57 */ "reserved",
/* 58 */ "reserved",
/* 59 */ "reserved",
/* 60 */ "reserved",
/* 61 */ "reserved",
/* 62 */ "reserved",
/* 63 */ "reserved",
/* 64 */ "reserved",
/* 65 */ "reserved",
/* 66 */ "reserved",
/* 67 */ "reserved",
/* 68 */ "reserved",
/* 69 */ "reserved",
/* 70 */ "reserved",
/* 71 */ "reserved",
/* 72 */ "reserved",
/* 73 */ "reserved",
/* 74 */ "reserved",
/* 75 */ "reserved",
/* 76 */ "reserved",
/* 77 */ "reserved",
/* 78 */ "reserved",
/* 79 */ "reserved",
/* 80 */ "reserved",
/* 81 */ "reserved",
/* 82 */ "reserved",
/* 83 */ "reserved",
/* 84 */ "reserved",
/* 85 */ "reserved",
/* 86 */ "reserved",
/* 87 */ "reserved",
/* 88 */ "reserved",
/* 89 */ "reserved",
/* 90 */ "reserved",
/* 91 */ "reserved",
/* 92 */ "reserved",
/* 93 */ "reserved",
/* 94 */ "reserved",
/* 95 */ "reserved",
/* 96 */ "reserved",
/* 97 */ "reserved",
/* 98 */ "reserved",
/* 99 */ "reserved",
/* 100 */ "%g mb",
/* 101 */ "mean sea level",
/* 102 */ "%g m above mean sea level",
/* 103 */ "%g m above ground",
/* 104 */ "%g sigma level",
/* 105 */ "%g hybrid level",
/* 106 */ "%g m underground",
/* 107 */ "%g K isentropic level",
/* 108 */ "%g mb above ground",
/* 109 */ "PV=%g (Km^2/kg/s) surface",
/* 110 */ "reserved",
/* 111 */ "%g Eta level",
/* 112 */ "reserved",
/* 113 */ "%g logarithmic hybrid level",
/* 114 */ "snow level",
/* 115 */ "%g sigma height level",
/* 116 */ "reserved",
/* 117 */ "mixed layer depth",
/* 118 */ "%g hybrid height level",
/* 119 */ "%g hybrid pressure level",
/* 120 */ "reserved",
/* 121 */ "reserved",
/* 122 */ "reserved",
/* 123 */ "reserved",
/* 124 */ "reserved",
/* 125 */ "reserved",
/* 126 */ "reserved",
/* 127 */ "reserved",
/* 128 */ "reserved",
/* 129 */ "reserved",
/* 130 */ "reserved",
/* 131 */ "reserved",
/* 132 */ "reserved",
/* 133 */ "reserved",
/* 134 */ "reserved",
/* 135 */ "reserved",
/* 136 */ "reserved",
/* 137 */ "reserved",
/* 138 */ "reserved",
/* 139 */ "reserved",
/* 140 */ "reserved",
/* 141 */ "reserved",
/* 142 */ "reserved",
/* 143 */ "reserved",
/* 144 */ "reserved",
/* 145 */ "reserved",
/* 146 */ "reserved",
/* 147 */ "reserved",
/* 148 */ "reserved",
/* 149 */ "reserved",
/* 150 */ "%g generalized vertical height coordinate",
/* 151 */ "soil level %g",
/* 152 */ "reserved",
/* 153 */ "reserved",
/* 154 */ "reserved",
/* 155 */ "reserved",
/* 156 */ "reserved",
/* 157 */ "reserved",
/* 158 */ "reserved",
/* 159 */ "reserved",
/* 160 */ "%g m below sea level",
/* 161 */ "%g m below water surface",
/* 162 */ "lake or river bottom",
/* 163 */ "bottom of sediment layer",
/* 164 */ "bottom of thermally active sediment layer",
/* 165 */ "bottom of sediment layer penetrated by thermal wave",
/* 166 */ "maxing layer",
/* 167 */ "bottom of root zone",
/* 168 */ "reserved",
/* 169 */ "reserved",
/* 170 */ "reserved",
/* 171 */ "reserved",
/* 172 */ "reserved",
/* 173 */ "reserved",
/* 174 */ "top surface of ice on sea, lake or river",
/* 175 */ "top surface of ice, und snow on sea, lake or river",
/* 176 */ "bottom surface ice on sea, lake or river",
/* 177 */ "deep soil",
/* 178 */ "reserved",
/* 179 */ "top surface of glacier ice and inland ice",
/* 180 */ "deep inland or glacier ice",
/* 181 */ "grid tile land fraction as a model surface",
/* 182 */ "grid tile water fraction as a model surface",
/* 183 */ "grid tile ice fraction on sea, lake or river as a model surface",
/* 184 */ "grid tile glacier ice and inland ice fraction as a model surface",
/* 185 */ "reserved",
/* 186 */ "reserved",
/* 187 */ "reserved",
/* 188 */ "reserved",
/* 189 */ "reserved",
/* 190 */ "reserved",
/* 191 */ "reserved"
};

// 192..255
const char *ncep_level_table[64] = {
/* 192 */ "reserved",
/* 193 */ "reserved",
/* 194 */ "reserved",
/* 195 */ "reserved",
/* 196 */ "reserved",
/* 197 */ "reserved",
/* 198 */ "reserved",
/* 199 */ "reserved",
/* 200 */ "entire atmosphere (considered as a single layer)",
/* 201 */ "entire ocean (considered as a single layer)",
/* 202 */ "reserved",
/* 203 */ "reserved",
/* 204 */ "highest tropospheric freezing level",
/* 205 */ "reserved",
/* 206 */ "grid scale cloud bottom level",
/* 207 */ "grid scale cloud top level",
/* 208 */ "reserved",
/* 209 */ "boundary layer cloud bottom level",
/* 210 */ "boundary layer cloud top level",
/* 211 */ "boundary layer cloud layer",
/* 212 */ "low cloud bottom level",
/* 213 */ "low cloud top level",
/* 214 */ "low cloud layer",
/* 215 */ "cloud ceiling",
/* 216 */ "reserved",
/* 217 */ "reserved",
/* 218 */ "reserved",
/* 219 */ "reserved",
/* 220 */ "planetary boundary layer",
/* 221 */ "layer between two hybrid levels",
/* 222 */ "middle cloud bottom level",
/* 223 */ "middle cloud top level",
/* 224 */ "middle cloud layer",
/* 225 */ "reserved",
/* 226 */ "reserved",
/* 227 */ "reserved",
/* 228 */ "reserved",
/* 229 */ "reserved",
/* 230 */ "reserved",
/* 231 */ "reserved",
/* 232 */ "high cloud bottom level",
/* 233 */ "high cloud top level",
/* 234 */ "high cloud layer",
/* 235 */ "%gC ocean isotherm",
/* 236 */ "layer between two depths below ocean surface",
/* 237 */ "bottom of ocean mixed layer",
/* 238 */ "bottom of ocean isothermal layer",
/* 239 */ "layer ocean surface and 26C ocean isothermal level",
/* 240 */ "ocean mixed layer",
/* 241 */ "%g in sequence",
/* 242 */ "convective cloud bottom level",
/* 243 */ "convective cloud top level",
/* 244 */ "convective cloud layer",
/* 245 */ "lowest level of the wet bulb zero",
/* 246 */ "maximum equivalent potential temperature level",
/* 247 */ "equilibrium level",
/* 248 */ "shallow convective cloud bottom level",
/* 249 */ "shallow convective cloud top level",
/* 250 */ "reserved",
/* 251 */ "deep convective cloud bottom level",
/* 252 */ "deep convective cloud top level",
/* 253 */ "lowest bottom level of supercooled liquid water layer",
/* 254 */ "highest top level of supercooled liquid water layer",
/* 255 */ "missing"
};


int level1(int mode, int type, int undef_val, float value, int center, int subcenter, char *inv_out);
int level2(int mode, int type1, int undef_val1, float value1, int type2, int undef_val2, 
   float value2, int center, int subcenter, char *inv_out);

int f_lev(ARG0) {

    int level_type1, level_type2;
    float val1, val2;
    int undef_val1, undef_val2;
    int center, subcenter;

    if (mode < 0) return 0;

    center = GB2_Center(sec);
    subcenter = GB2_Subcenter(sec);

    fixed_surfaces(sec, &level_type1, &val1, &undef_val1, &level_type2, &val2, &undef_val2);

    if (mode > 1) {
	if (undef_val1 == 0) sprintf(inv_out,"lvl1=(%d,%lg) ",level_type1,val1);
	else sprintf(inv_out,"lvl1=(%d,missing) ",level_type1);
	inv_out += strlen(inv_out);

	if (undef_val2 == 0) sprintf(inv_out,"lvl2=(%d,%lg):",level_type2,val2);
	else sprintf(inv_out,"lvl2=(%d,missing):",level_type2);
	inv_out += strlen(inv_out);
    }

    level2(mode, level_type1, undef_val1, val1, level_type2, undef_val2, val2, center, subcenter, inv_out);
    return 0;
}

/*
 * level2 is for layers
 */

int level2(int mode, int type1, int undef_val1, float value1, int type2, int undef_val2, float value2, int center, int subcenter,
	char *inv_out) {

    if (type1 == 100 && type2 == 100) {
	sprintf(inv_out,"%g-%g mb",value1/100,value2/100);
    }
    else if (type1 == 102 && type2 == 102) {
	sprintf(inv_out,"%g-%g m above mean sea level",value1,value2);
    }
    else if (type1 == 103 && type2 == 103) {
	sprintf(inv_out,"%g-%g m above ground",value1,value2);
    }
    else if (type1 == 104 && type2 == 104) {
	sprintf(inv_out,"%g-%g sigma layer",value1,value2);
    }
    else if (type1 == 105 && type2 == 105) {
	sprintf(inv_out,"%g-%g hybrid layer",value1,value2);
    }
    else if (type1 == 106 && type2 == 106) {
	/* sprintf(inv_out,"%g-%g m below ground",value1/100,value2/100); removed 1/2007 */
	sprintf(inv_out,"%g-%g m below ground",value1,value2);
    }
    else if (type1 == 107 && type2 == 107) {
	sprintf(inv_out,"%g-%g K isentropic layer",value1,value2);
    }
    else if (type1 == 108 && type2 == 108) {
	sprintf(inv_out,"%g-%g mb above ground",value1/100,value2/100);
    }
    else if (type1 == 111 && type2 == 111) {
	sprintf(inv_out,"%g-%g Eta layer",value1,value2);
    }
    else if (type1 == 115 && type2 == 115) {
	sprintf(inv_out,"%g-%g sigma height layer",value1,value2);
    }
    else if (type1 == 118 && type2 == 118) {
	sprintf(inv_out,"%g-%g hybrid height layer",value1,value2);
    }
    else if (type1 == 119 && type2 == 119) {
	sprintf(inv_out,"%g-%g hybrid pressure layer",value1,value2);
    }
    else if (type1 == 160 && type2 == 160) {
	sprintf(inv_out,"%g-%g m below sea level",value1,value2);
    }
    else if (type1 == 161 && type2 == 161) {
	sprintf(inv_out,"%g-%g m ocean layer",value1,value2);
    }
    else if (type1 == 1 && type2 == 8) {
	sprintf(inv_out,"atmos col");		// compatible with wgrib
    }
    else if (type1 == 9 && type2 == 1) {
	sprintf(inv_out,"ocean column");
    }
    else if (center == NCEP && type1 == 235 && type2 == 235) {
	    sprintf(inv_out,"%g-%gC ocean isotherm layer", value1/10,value2/10);
    }
    else if (center == NCEP && type1 == 236 && type2 == 236) {	// obsolete
	    sprintf(inv_out,"%g-%g m ocean layer", value1*10,value2*10);
    }
    else if (type1 == 255 && type2 == 255) {
	    sprintf(inv_out,"no_level");
    }
    else {
        level1(mode, type1, undef_val1, value1, center, subcenter,inv_out);
	inv_out += strlen(inv_out);
        if (type2 != 255) {
	    sprintf(inv_out," - ");
	    inv_out += strlen(inv_out);
	    level1(mode, type2, undef_val2, value2, center, subcenter,inv_out);
        }
    }
    return 0;
}

/*
 * level1 is for a single level (not a layer)
 */

int level1(int mode, int type, int undef_val, float val, int center, int subcenter,char *inv_out) {

    /* WMO defined levels */

    if (type < 192) {
        if (type == 100 || type == 108) val = val * 0.01;  // Pa -> mb
	sprintf(inv_out,level_table[type], val);
	return 0;
    }

    // no numeric information
    if (type == 255) return 8;

    /* local table for NCEP */
    if (center == NCEP && type >= 192 && type <= 254) {
        if (type == 235) val *= 0.01;  // C -> 0.1C
	sprintf(inv_out,ncep_level_table[type-192], val);
    }

    else {
        if (undef_val == 0) sprintf(inv_out,"local level type %d %g", type, val);
        else sprintf(inv_out,"local level type %d", type);
    }

    return 0;
}
