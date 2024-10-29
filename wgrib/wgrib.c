#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
/* 
 * version 1.2.1 of grib headers  w. ebisuzaki 
 *         1.2.2 added access to spectral reference value l. kornblueh
 */

#ifndef INT2
#define INT2(a,b)   ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 0x7f) << 8) + b))
#endif

#define BDS_LEN(bds)		(ec_large_grib ? len_ec_bds : ((int) ((bds[0]<<16)+(bds[1]<<8)+bds[2])) )

#define BDS_Flag(bds)		(bds[3])

#define BDS_Grid(bds)		((bds[3] & 128) == 0)
#define BDS_Harmonic(bds)	(bds[3] & 128)

#define BDS_Packing(bds)	((bds[3] & 64) != 0)
#define BDS_SimplePacking(bds)	((bds[3] & 64) == 0)
#define BDS_ComplexPacking(bds)	((bds[3] & 64) != 0)

#define BDS_OriginalType(bds)	((bds[3] & 32) != 0)
#define BDS_OriginalFloat(bds)	((bds[3] & 32) == 0)
#define BDS_OriginalInt(bds)	((bds[3] & 32) != 0)

#define BDS_MoreFlags(bds)      ((bds[3] & 16) != 0)
#define BDS_UnusedBits(bds)	((int) (bds[3] & 15))

#define BDS_BinScale(bds)	INT2(bds[4],bds[5])

#define BDS_RefValue(bds)	(ibm2flt(bds+6))
#define BDS_NumBits(bds)	((int) bds[10])

#define BDS_Harmonic_RefValue(bds) (ibm2flt(bds+11))

#define BDS_DataStart(bds)      ((int) (11 + BDS_MoreFlags(bds)*3))
#define BDS_P1(bds)		(bds[16] * 256 + bds[17])
#define BDS_P2(bds)		(bds[18] * 256 + bds[19])

/* undefined value -- if bitmap */
#define UNDEFINED		9.999e20

/* version 1.2 of grib headers  w. ebisuzaki */

#define BMS_LEN(bms)		((bms) == NULL ? 0 : (bms[0]<<16)+(bms[1]<<8)+bms[2])
#define BMS_UnusedBits(bms)	((bms) == NULL ? 0 : bms[3])
#define BMS_StdMap(bms)		((bms) == NULL ? 0 : ((bms[4]<<8) + bms[5]))
#define BMS_bitmap(bms)		((bms) == NULL ? NULL : (bms)+6)
#define BMS_nxny(bms)		((((bms) == NULL) || BMS_StdMap(bms)) \
	? 0 : (BMS_LEN(bms)*8 - 48 - BMS_UnusedBits(bms)))
/* cnames_file.c */

/* search order for parameter names
 *
 * #define P_TABLE_FIRST
 * look at external parameter table first
 *
 * otherwise use builtin NCEP-2 or ECMWF-160 first
 */
/* #define P_TABLE_FIRST */

/* search order for external parameter table
 * 1) environment variable GRIBTAB
 * 2) environment variable gribtab
 * 3) the file 'gribtab' in current directory
 */


/* cnames.c */
/* then default values */
char *k5toa(unsigned char *pds);
char *k5_comments(unsigned char *pds);
int setup_user_table(int center, int subcenter, int ptable);


struct ParmTable {
	/* char *name, *comment; */
	char *name, *comment;
};
/* version 1.4.5 of grib headers  w. ebisuzaki */
/* this version is incomplete */
/* 5/00 - dx/dy or di/dj controlled by bit 1 of resolution byte */
/* 8/00 - dx/dy or di/dj for polar and lambert not controlled by res. byte */
/* Added headers for the triangular grid of the gme model of DWD
         Helmut P. Frank, 13.09.2001 */
/* Clean up of triangular grid properties access and added spectral information
         Luis Kornblueh, 27.03.2002 */
/* 8/08 - dx/dy (polar,lambert) controlled by bit 1 of resolution byte */
/* 5/11 Paul Schou: fixed GDS_Lambert_LonSP(gds) */
/* 6/11 Jeffery S. Smith Albers equal area projection */

#ifndef INT3
#define INT3(a,b,c) ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 127) << 16)+(b<<8)+c))
#endif
#ifndef INT2
#define INT2(a,b)   ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 127) << 8) + b))
#endif

#ifndef UINT4
#define UINT4(a,b,c,d) ((int) ((a << 24) + (b << 16) + (c << 8) + (d)))
#endif

#ifndef UINT3
#define UINT3(a,b,c) ((int) ((a << 16) + (b << 8) + (c)))
#endif

#ifndef UINT2
#define UINT2(a,b) ((int) ((a << 8) + (b)))
#endif


#define GDS_Len1(gds)		(gds[0])
#define GDS_Len2(gds)		(gds[1])
#define GDS_Len3(gds)		(gds[2])
#define GDS_LEN(gds)		((int) ((gds[0]<<16)+(gds[1]<<8)+gds[2]))

#define GDS_NV(gds)		(gds[3])
#define GDS_DataType(gds)	(gds[5])

#define GDS_LatLon(gds)		(gds[5] == 0)
#define GDS_Mercator(gds)	(gds[5] == 1)
#define GDS_Gnomonic(gds)	(gds[5] == 2)
#define GDS_Lambert(gds)	(gds[5] == 3)
#define GDS_Gaussian(gds)	(gds[5] == 4)
#define GDS_Polar(gds)		(gds[5] == 5)
#define GDS_Albers(gds)		(gds[5] == 8)
#define GDS_RotLL(gds)		(gds[5] == 10)
#define GDS_Harmonic(gds)	(gds[5] == 50)
#define GDS_Triangular(gds)	(gds[5] == 192)
#define GDS_ssEgrid(gds)	(gds[5] == 201)	/* semi-staggered E grid */
#define GDS_fEgrid(gds)		(gds[5] == 202) /* filled E grid */
#define GDS_ss2dEgrid(gds)	(gds[5] == 203) /* semi-staggered E grid 2 d*/
#define GDS_ss2dBgrid(gds)      (gds[5] == 205) /* semi-staggered B grid 2 d*/

#define GDS_has_dy(mode)	((mode) & 128)
#define GDS_LatLon_nx(gds)	((int) ((gds[6] << 8) + gds[7]))
#define GDS_LatLon_ny(gds)	((int) ((gds[8] << 8) + gds[9]))
#define GDS_LatLon_La1(gds)	INT3(gds[10],gds[11],gds[12])
#define GDS_LatLon_Lo1(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_LatLon_mode(gds)	(gds[16])
#define GDS_LatLon_La2(gds)	INT3(gds[17],gds[18],gds[19])
#define GDS_LatLon_Lo2(gds)	INT3(gds[20],gds[21],gds[22])

#define GDS_LatLon_dx(gds)      (gds[16] & 128 ? INT2(gds[23],gds[24]) : 0)
#define GDS_LatLon_dy(gds)      (gds[16] & 128 ? INT2(gds[25],gds[26]) : 0)
#define GDS_Gaussian_nlat(gds)  ((gds[25]<<8)+gds[26])

#define GDS_LatLon_scan(gds)	(gds[27])

#define GDS_Polar_nx(gds)	(gds[16] & 128 ? ((gds[6] << 8) + gds[7]) : 0)
#define GDS_Polar_ny(gds)	(gds[16] & 128 ? ((gds[8] << 8) + gds[9]) : 0)
#define GDS_Polar_La1(gds)	INT3(gds[10],gds[11],gds[12])
#define GDS_Polar_Lo1(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_Polar_mode(gds)	(gds[16])
#define GDS_Polar_Lov(gds)	INT3(gds[17],gds[18],gds[19])
#define GDS_Polar_scan(gds)	(gds[27])
#define GDS_Polar_Dx(gds)	INT3(gds[20], gds[21], gds[22])
#define GDS_Polar_Dy(gds)	INT3(gds[23], gds[24], gds[25])
#define GDS_Polar_pole(gds)	((gds[26] & 128) == 128)

#define GDS_Lambert_nx(gds)	((gds[6] << 8) + gds[7])
#define GDS_Lambert_ny(gds)	((gds[8] << 8) + gds[9])
#define GDS_Lambert_La1(gds)	INT3(gds[10],gds[11],gds[12])
#define GDS_Lambert_Lo1(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_Lambert_mode(gds)	(gds[16])
#define GDS_Lambert_Lov(gds)	INT3(gds[17],gds[18],gds[19])
#define GDS_Lambert_dx(gds)	INT3(gds[20],gds[21],gds[22])
#define GDS_Lambert_dy(gds)	INT3(gds[23],gds[24],gds[25])
#define GDS_Lambert_NP(gds)	((gds[26] & 128) == 0)
#define GDS_Lambert_scan(gds)   (gds[27])
#define GDS_Lambert_Latin1(gds)	INT3(gds[28],gds[29],gds[30])
#define GDS_Lambert_Latin2(gds)	INT3(gds[31],gds[32],gds[33])
#define GDS_Lambert_LatSP(gds)	INT3(gds[34],gds[35],gds[36])

/* bug found by Paul Schou 5/3/2011
   #define GDS_Lambert_LonSP(gds)	INT3(gds[37],gds[37],gds[37])
*/
#define GDS_Lambert_LonSP(gds)	INT3(gds[37],gds[38],gds[39])

#define GDS_ssEgrid_n(gds)	UINT2(gds[6],gds[7])
#define GDS_ssEgrid_n_dum(gds)  UINT2(gds[8],gds[9])
#define GDS_ssEgrid_La1(gds)	INT3(gds[10],gds[11],gds[12])
#define GDS_ssEgrid_Lo1(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_ssEgrid_mode(gds)	(gds[16])
#define GDS_ssEgrid_La2(gds)	UINT3(gds[17],gds[18],gds[19])
#define GDS_ssEgrid_Lo2(gds)	UINT3(gds[20],gds[21],gds[22])
#define GDS_ssEgrid_di(gds)	(gds[16] & 128 ? INT2(gds[23],gds[24]) : 0)
#define GDS_ssEgrid_dj(gds)	(gds[16] & 128 ? INT2(gds[25],gds[26]) : 0)
#define GDS_ssEgrid_scan(gds)	(gds[27])

#define GDS_fEgrid_n(gds)	UINT2(gds[6],gds[7])
#define GDS_fEgrid_n_dum(gds)   UINT2(gds[8],gds[9])
#define GDS_fEgrid_La1(gds)	INT3(gds[10],gds[11],gds[12])
#define GDS_fEgrid_Lo1(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_fEgrid_mode(gds)	(gds[16])
#define GDS_fEgrid_La2(gds)	UINT3(gds[17],gds[18],gds[19])
#define GDS_fEgrid_Lo2(gds)	UINT3(gds[20],gds[21],gds[22])
#define GDS_fEgrid_di(gds)	(gds[16] & 128 ? INT2(gds[23],gds[24]) : 0)
#define GDS_fEgrid_dj(gds)	(gds[16] & 128 ? INT2(gds[25],gds[26]) : 0)
#define GDS_fEgrid_scan(gds)	(gds[27])

#define GDS_ss2dEgrid_nx(gds)     UINT2(gds[6],gds[7])
#define GDS_ss2dEgrid_ny(gds)     UINT2(gds[8],gds[9])
#define GDS_ss2dEgrid_La1(gds)    INT3(gds[10],gds[11],gds[12])
#define GDS_ss2dEgrid_Lo1(gds)    INT3(gds[13],gds[14],gds[15])
#define GDS_ss2dEgrid_mode(gds)   (gds[16])
#define GDS_ss2dEgrid_La2(gds)    INT3(gds[17],gds[18],gds[19])
#define GDS_ss2dEgrid_Lo2(gds)    INT3(gds[20],gds[21],gds[22])
#define GDS_ss2dEgrid_di(gds)     (gds[16] & 128 ? INT2(gds[23],gds[24]) : 0)
#define GDS_ss2dEgrid_dj(gds)     (gds[16] & 128 ? INT2(gds[25],gds[26]) : 0)
#define GDS_ss2dEgrid_scan(gds)   (gds[27])

#define GDS_ss2dBgrid_nx(gds)     UINT2(gds[6],gds[7])
#define GDS_ss2dBgrid_ny(gds)     UINT2(gds[8],gds[9])
#define GDS_ss2dBgrid_La1(gds)    INT3(gds[10],gds[11],gds[12])
#define GDS_ss2dBgrid_Lo1(gds)    INT3(gds[13],gds[14],gds[15])
#define GDS_ss2dBgrid_mode(gds)   (gds[16])
#define GDS_ss2dBgrid_La2(gds)    INT3(gds[17],gds[18],gds[19])
#define GDS_ss2dBgrid_Lo2(gds)    INT3(gds[20],gds[21],gds[22])
#define GDS_ss2dBgrid_di(gds)     (gds[16] & 128 ? INT2(gds[23],gds[24]) : 0)
#define GDS_ss2dBgrid_dj(gds)     (gds[16] & 128 ? INT2(gds[25],gds[26]) : 0)
#define GDS_ss2dBgrid_scan(gds)   (gds[27]) 


#define GDS_Merc_nx(gds)	UINT2(gds[6],gds[7])
#define GDS_Merc_ny(gds)	UINT2(gds[8],gds[9])
#define GDS_Merc_La1(gds)	INT3(gds[10],gds[11],gds[12])
#define GDS_Merc_Lo1(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_Merc_mode(gds)	(gds[16])
#define GDS_Merc_La2(gds)	INT3(gds[17],gds[18],gds[19])
#define GDS_Merc_Lo2(gds)	INT3(gds[20],gds[21],gds[22])
#define GDS_Merc_Latin(gds)	INT3(gds[23],gds[24],gds[25])
#define GDS_Merc_scan(gds)	(gds[27])
#define GDS_Merc_dx(gds)        (gds[16] & 128 ? INT3(gds[28],gds[29],gds[30]) : 0)
#define GDS_Merc_dy(gds)        (gds[16] & 128 ? INT3(gds[31],gds[32],gds[33]) : 0)

/* rotated Lat-lon grid */

#define GDS_RotLL_nx(gds)	UINT2(gds[6],gds[7])
#define GDS_RotLL_ny(gds)	UINT2(gds[8],gds[9])
#define GDS_RotLL_La1(gds)	INT3(gds[10],gds[11],gds[12])
#define GDS_RotLL_Lo1(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_RotLL_mode(gds)	(gds[16])
#define GDS_RotLL_La2(gds)	INT3(gds[17],gds[18],gds[19])
#define GDS_RotLL_Lo2(gds)	INT3(gds[20],gds[21],gds[22])
#define GDS_RotLL_dx(gds)       (gds[16] & 128 ? INT2(gds[23],gds[24]) : 0)
#define GDS_RotLL_dy(gds)       (gds[16] & 128 ? INT2(gds[25],gds[26]) : 0)
#define GDS_RotLL_scan(gds)	(gds[27])
#define GDS_RotLL_LaSP(gds)	INT3(gds[32],gds[33],gds[34])
#define GDS_RotLL_LoSP(gds)	INT3(gds[35],gds[36],gds[37])
#define GDS_RotLL_RotAng(gds)	ibm2flt(&(gds[38]))

/* Triangular grid of DWD */
#define GDS_Triangular_ni2(gds)	INT2(gds[6],gds[7])
#define GDS_Triangular_ni3(gds)	INT2(gds[8],gds[9])
#define GDS_Triangular_ni(gds)	INT3(gds[13],gds[14],gds[15])
#define GDS_Triangular_nd(gds)  INT3(gds[10],gds[11],gds[12])

/* Harmonics data */
#define GDS_Harmonic_nj(gds)     ((int) ((gds[6] << 8) + gds[7])) 
#define GDS_Harmonic_nk(gds)     ((int) ((gds[8] << 8) + gds[9])) 
#define GDS_Harmonic_nm(gds)     ((int) ((gds[10] << 8) + gds[11])) 
#define GDS_Harmonic_type(gds)   (gds[12])
#define GDS_Harmonic_mode(gds)   (gds[13])

/* index of NV and PV */
#define GDS_PV(gds)		((gds[3] == 0) ? -1 : (int) gds[4] - 1)
#define GDS_PL(gds)		((gds[4] == 255) ? -1 : (int) gds[3] * 4 + (int) gds[4] - 1)

enum Def_NCEP_Table {rean, opn, rean_nowarn, opn_nowarn};

unsigned char *seek_grib(FILE *file, unsigned long *pos, long *len_grib, 
        unsigned char *buffer, unsigned int buf_len);

int read_grib(FILE *file, long pos, long len_grib, unsigned char *buffer);

long echack(FILE *file, long pos, long len_grib);

double ibm2flt(unsigned char *ibm);
 
void BDS_unpack(float *flt, unsigned char *bds, unsigned char *bitmap,
        int n_bits, int n, double ref, double scale);

int BDS_NValues(unsigned char *bds);

double int_power(double x, int y);

int flt2ieee(float x, unsigned char *ieee);

int wrtieee(float *array, int n, int header, FILE *output);
int wrtieee_header(unsigned int n, FILE *output);

void levels(int, int, int, int verbose);
 
void PDStimes(int time_range, int p1, int p2, int time_unit);

int missing_points(unsigned char *bitmap, int n);

void EC_ext(unsigned char *pds, char *prefix, char *suffix, int verbose);

int GDS_grid(unsigned char *gds, unsigned char *bds, int *nx, int *ny, 
             long int *nxny);

void GDS_prt_thin_lon(unsigned char *gds);

void GDS_winds(unsigned char *gds, int verbose);

int PDS_date(unsigned char *pds, int option, int verf_time);

int add_time(int *year, int *month, int *day, int *hour, int dtime, int unit);

int verf_time(unsigned char *pds, int *year, int *month, int *day, int *hour);

void print_pds(unsigned char *pds, int print_PDS, int print_PDS10, int verbose);
void print_gds(unsigned char *gds, int print_GDS, int print_GDS10, int verbose);

void ensemble(unsigned char *pds, int mode);
/* version 3.4 of grib headers  w. ebisuzaki */
/* this version is incomplete */
/* add center DWD    Helmut P. Frank */
/* 10/02 add center CPTEC */
/* 29/04/2005 add center CHM   Luiz Claudio M. Fonseca*/
/* 11/2008 add center LAMI   Davide Sacchetti */

#ifndef INT2
#define INT2(a,b)   ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 0x7f) << 8) + b))
#endif
#ifndef UINT4
#define UINT4(a,b,c,d) ((int) ((a << 24) + (b << 16) + (c << 8) + (d)))
#endif
#ifndef UINT2
#define UINT2(a,b) ((int) ((a << 8) + (b)))
#endif
#define __LEN24(pds)    ((pds) == NULL ? 0 : (int) ((pds[0]<<16)+(pds[1]<<8)+pds[2]))

#define PDS_Len1(pds)		(pds[0])
#define PDS_Len2(pds)		(pds[1])
#define PDS_Len3(pds)		(pds[2])
#define PDS_LEN(pds)		((int) ((pds[0]<<16)+(pds[1]<<8)+pds[2]))
#define PDS_Vsn(pds)		(pds[3])
#define PDS_Center(pds)		(pds[4])
#define PDS_Model(pds)		(pds[5])
#define PDS_Grid(pds)		(pds[6])
#define PDS_HAS_GDS(pds)	((pds[7] & 128) != 0)
#define PDS_HAS_BMS(pds)	((pds[7] & 64) != 0)
#define PDS_PARAM(pds)		(pds[8])
#define PDS_L_TYPE(pds)		(pds[9])
#define PDS_LEVEL1(pds)		(pds[10])
#define PDS_LEVEL2(pds)		(pds[11])

#define PDS_KPDS5(pds)		(pds[8])
#define PDS_KPDS6(pds)		(pds[9])
#define PDS_KPDS7(pds)		((int) ((pds[10]<<8) + pds[11]))

/* this requires a 32-bit default integer machine */
#define PDS_Field(pds)		((pds[8]<<24)+(pds[9]<<16)+(pds[10]<<8)+pds[11])

#define PDS_Year(pds)		(pds[12])
#define PDS_Month(pds)		(pds[13])
#define PDS_Day(pds)		(pds[14])
#define PDS_Hour(pds)		(pds[15])
#define PDS_Minute(pds)		(pds[16])
#define PDS_ForecastTimeUnit(pds)	(pds[17])
#define PDS_P1(pds)		(pds[18])
#define PDS_P2(pds)		(pds[19])
#define PDS_TimeRange(pds)	(pds[20])
#define PDS_NumAve(pds)		((int) ((pds[21]<<8)+pds[22]))
#define PDS_NumMissing(pds)	(pds[23])
#define PDS_Century(pds)	(pds[24])
#define PDS_Subcenter(pds)	(pds[25])
#define PDS_DecimalScale(pds)	INT2(pds[26],pds[27])
/* old #define PDS_Year4(pds)   (pds[12] + 100*(pds[24] - (pds[12] != 0))) */
#define PDS_Year4(pds)          (pds[12] + 100*(pds[24] - 1))

/* various centers */
#define NMC			7
#define JMA			34
#define ECMWF			98
#define DWD			78
#define CMC			54
#define CPTEC			46
#define CHM			146
#define LAMI			200

/* ECMWF Extensions */

#define PDS_EcLocalId(pds)	(PDS_LEN(pds) >= 41 ? (pds[40]) : 0)
#define PDS_EcClass(pds)	(PDS_LEN(pds) >= 42 ? (pds[41]) : 0)
#define PDS_EcType(pds)		(PDS_LEN(pds) >= 43 ? (pds[42]) : 0)
#define PDS_EcStream(pds)	(PDS_LEN(pds) >= 45 ? (INT2(pds[43], pds[44])) : 0)

#define PDS_EcENS(pds)		(PDS_LEN(pds) >= 52 && pds[40] == 1 && \
				pds[43] * 256 + pds[44] == 1035 && pds[50] != 0)
#define PDS_EcFcstNo(pds)	(pds[49])

#define PDS_Ec16Version(pds)	(pds + 45)
#define PDS_Ec16Number(pds)	(PDS_EcLocalId(pds) == 16 ? UINT2(pds[49],pds[50]) : 0)
#define PDS_Ec16SysNum(pds)	(PDS_EcLocalId(pds) == 16 ? UINT2(pds[51],pds[52]) : 0)
#define PDS_Ec16MethodNum(pds)	(PDS_EcLocalId(pds) == 16 ? UINT2(pds[53],pds[54]) : 0)
#define PDS_Ec16VerfMon(pds)	(PDS_EcLocalId(pds) == 16 ? UINT4(pds[55],pds[56],pds[57],pds[58]) : 0)
#define PDS_Ec16AvePeriod(pds)	(PDS_EcLocalId(pds) == 16 ? pds[59] : 0)
#define PDS_Ec16FcstMon(pds)	(PDS_EcLocalId(pds) == 16 ? UINT2(pds[60],pds[61]) : 0)

/* NCEP Extensions */

#define PDS_NcepENS(pds)	(PDS_LEN(pds) >= 44 && pds[25] == 2 && pds[40] == 1)
#define PDS_NcepFcstType(pds)	(pds[41])
#define PDS_NcepFcstNo(pds)	(pds[42])
#define PDS_NcepFcstProd(pds)	(pds[43])

/* time units */

#define MINUTE  0
#define HOUR    1
#define DAY     2
#define MONTH   3
#define YEAR    4
#define DECADE  5
#define NORMAL  6
#define CENTURY 7
#define HOURS3  10
#define HOURS6  11
#define HOURS12  12
#define MINUTES15 13
#define MINUTES30 14
#define SECOND  254



#define VERSION "v1.8.5 (9-2023) Wesley Ebisuzaki\n\t\tDWD-tables 2,201-205 (11-28-2005) Helmut P. Frank\n\t\tspectral: Luis Kornblueh (MPI)"

#define CHECK_GRIB
/* #define DEBUG */

/*
 * wgrib.c is placed into the public domain.  While you could
 * legally do anything you want with the code, telling the world
 * that you wrote it would be uncool.  Selling it would be really
 * uncool.  The code was originally written for NMC/NCAR Reanalysis 
 * and handles most GRIB files except for the ECMWF spectral files.
 * (ECMWF's spectral->grid code are copyrighted and in FORTRAN.)
 * The code, as usual, is not waranteed to be fit for any purpose 
 * what so ever.  However, wgrib is operational NCEP code, so it
 * better work for our files.
 */

/*
 * wgrib.c extract/inventory grib records
 *
 *                              Wesley Ebisuzaki
 *
 * See Changes for update information
 *
 */

/*
 * MSEEK = I/O buffer size for seek_grib
 */

#define MSEEK 1024
#define BUFF_ALLOC0	40000


#ifndef min
#define min(a,b)  ((a) < (b) ? (a) : (b))
#define max(a,b)  ((a) < (b) ? (b) : (a))
#endif

#ifndef DEF_T62_NCEP_TABLE
#define DEF_T62_NCEP_TABLE	rean
#endif
enum Def_NCEP_Table def_ncep_table = DEF_T62_NCEP_TABLE;
int minute = 0;
int ncep_ens = 0;
int cmc_eq_ncep = 0;
extern int ec_large_grib, len_ec_bds;

int main(int argc, char **argv) {

    unsigned char *buffer;
    float *array;
    double temp, rmin, rmax;
    int i, nx, ny, file_arg;
    long int len_grib, nxny, buffer_size, n_dump, count = 1;
    long unsigned pos = 0;
    unsigned char *msg, *pds, *gds, *bms, *bds, *pointer, *end_msg;
    FILE *input, *dump_file = NULL;
    char line[2000];
    enum {BINARY, TEXT, IEEE, GRIB, NONE} output_type = NONE;
    enum {DUMP_ALL, DUMP_RECORD, DUMP_POSITION, DUMP_LIST, INVENTORY} 
	mode = INVENTORY;
    enum {none, dwd, simple} header = simple;

    long int dump = -1;
    int verbose = 0, append = 0, v_time = 0, year_4 = 0, output_PDS_GDS = 0;
    int print_GDS = 0, print_GDS10 = 0, print_PDS = 0, print_PDS10 = 0;
    char *dump_file_name = "dump", open_parm[3];
    int return_code = 0;

    if (argc == 1) {
	fprintf(stderr, "\nPortable Grib decoder for %s etc.\n",
	    (def_ncep_table == opn_nowarn || def_ncep_table == opn) ?
	    "NCEP Operations" : "NCEP/NCAR Reanalysis");
	fprintf(stderr, "   it slices, dices    %s\n", VERSION);
	fprintf(stderr, "   usage: %s [grib file] [options]\n\n", argv[0]);

	fprintf(stderr, "Inventory/diagnostic-output selections\n");
	fprintf(stderr, "   -s/-v                   short/verbose inventory\n");
	fprintf(stderr, "   -V                      diagnostic output (not inventory)\n");
	fprintf(stderr, "   (none)                  regular inventory\n");

	fprintf(stderr, " Options\n");
	fprintf(stderr, "   -PDS/-PDS10             print PDS in hex/decimal\n");
	fprintf(stderr, "   -GDS/-GDS10             print GDS in hex/decimal\n");
	fprintf(stderr, "   -verf                   print forecast verification time\n");
	fprintf(stderr, "   -ncep_opn/-ncep_rean    default T62 NCEP grib table\n");
	fprintf(stderr, "   -4yr                    print year using 4 digits\n");
	fprintf(stderr, "   -min                    print minutes\n");
	fprintf(stderr, "   -ncep_ens               ensemble info encoded in ncep format\n");

	fprintf(stderr, "Decoding GRIB selection\n");
	fprintf(stderr, "   -d [record number|all]  decode record number\n");
	fprintf(stderr, "   -p [byte position]      decode record at byte position\n");
	fprintf(stderr, "   -i                      decode controlled by stdin (inventory list)\n");
	fprintf(stderr, "   (none)                  no decoding\n");

	fprintf(stderr, " Options\n");
	fprintf(stderr, "   -text/-ieee/-grib/-bin  convert to text/ieee/grib/bin (default)\n");
	fprintf(stderr, "   -nh/-h                  output will have no headers/headers (default)\n");
	fprintf(stderr, "   -dwdgrib                output dwd headers, grib (do not append)\n");
	fprintf(stderr, "   -H                      output will include PDS and GDS (-bin/-ieee only)\n");
	fprintf(stderr, "   -append                 append to output file\n");
	fprintf(stderr, "   -o [file]               output file name, 'dump' is default\n");
	fprintf(stderr, " Misc\n");
	fprintf(stderr, "   -cmc [file]             use NCEP tables for CMC (dangerous)\n");
	exit(8);
    }
    file_arg = 0;
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i],"-PDS") == 0) {
	    print_PDS = 1;
	    continue;
	}
	if (strcmp(argv[i],"-PDS10") == 0) {
	    print_PDS10 = 1;
	    continue;
	}
	if (strcmp(argv[i],"-GDS") == 0) {
	    print_GDS = 1;
	    continue;
	}
	if (strcmp(argv[i],"-GDS10") == 0) {
	    print_GDS10 = 1;
	    continue;
	}
	if (strcmp(argv[i],"-v") == 0) {
	    verbose = 1;
	    continue;
	}
	if (strcmp(argv[i],"-V") == 0) {
	    verbose = 2;
	    continue;
	}
	if (strcmp(argv[i],"-s") == 0) {
	    verbose = -1;
	    continue;
	}
	if (strcmp(argv[i],"-text") == 0) {
	    output_type = TEXT;
	    continue;
	}
	if (strcmp(argv[i],"-bin") == 0) {
	    output_type = BINARY;
	    continue;
	}
	if (strcmp(argv[i],"-ieee") == 0) {
	    output_type = IEEE;
	    continue;
	}
	if (strcmp(argv[i],"-grib") == 0) {
	    output_type = GRIB;
	    continue;
	}
	if (strcmp(argv[i],"-nh") == 0) {
	    header = none;
	    continue;
	}
	if (strcmp(argv[i],"-h") == 0) {
	    header = simple;
	    continue;
	}
	if (strcmp(argv[i],"-dwdgrib") == 0) {
	    header = dwd;
	    output_type = GRIB;
	    continue;
	}
	if (strcmp(argv[i],"-append") == 0) {
	    append = 1;
	    continue;
	}
	if (strcmp(argv[i],"-verf") == 0) {
	    v_time = 1;
	    continue;
        }
	if (strcmp(argv[i],"-cmc") == 0) {
	    cmc_eq_ncep = 1;
	    continue;
        }
	if (strcmp(argv[i],"-d") == 0) {
	    if (strcmp(argv[i+1],"all") == 0) {
	        mode = DUMP_ALL;
	    }
	    else {
	        dump = atol(argv[i+1]);
	        mode = DUMP_RECORD;
	    }
	    i++;
	    if (output_type == NONE) output_type = BINARY;
	    continue;
	}
	if (strcmp(argv[i],"-p") == 0) {
	    pos = atol(argv[i+1]);
	    i++;
	    dump = 1;
	    if (output_type == NONE) output_type = BINARY;
	    mode = DUMP_POSITION;
	    continue;
	}
	if (strcmp(argv[i],"-i") == 0) {
	    if (output_type == NONE) output_type = BINARY;
	    mode = DUMP_LIST;
	    continue;
	}
	if (strcmp(argv[i],"-H") == 0) {
	    output_PDS_GDS = 1;
	    continue;
	}
	if (strcmp(argv[i],"-NH") == 0) {
	    output_PDS_GDS = 0;
	    continue;
	}
	if (strcmp(argv[i],"-4yr") == 0) {
	    year_4 = 1;
	    continue;
	}
	if (strcmp(argv[i],"-ncep_opn") == 0) {
	    def_ncep_table = opn_nowarn;
	    continue;
	}
	if (strcmp(argv[i],"-ncep_rean") == 0) {
	    def_ncep_table = rean_nowarn;
	    continue;
	}
	if (strcmp(argv[i],"-o") == 0) {
	    dump_file_name = argv[i+1];
	    i++;
	    continue;
	}
	if (strcmp(argv[i],"--v") == 0) {
	    printf("wgrib: %s\n", VERSION);
	    exit(0);
	}
	if (strcmp(argv[i],"-min") == 0) {
	    minute = 1;
	    continue;
	}
	if (strcmp(argv[i],"-ncep_ens") == 0) {
	    ncep_ens = 1;
	    continue;
	}
	if (file_arg == 0) {
	    file_arg = i;
	}
	else {
	    fprintf(stderr,"argument: %s ????\n", argv[i]);
	}
    }
    if (file_arg == 0) {
	fprintf(stderr,"no GRIB file to process\n");
	exit(8);
    }
    if ((input = fopen(argv[file_arg],"rb")) == NULL) {
        fprintf(stderr,"could not open file: %s\n", argv[file_arg]);
        exit(7);
    }

    if ((buffer = (unsigned char *) malloc(BUFF_ALLOC0)) == NULL) {
	fprintf(stderr,"not enough memory\n");
    }
    buffer_size = BUFF_ALLOC0;

    /* open output file */
    if (mode != INVENTORY) {
	open_parm[0] = append ? 'a' : 'w'; open_parm[1] = 'b'; open_parm[2] = '\0';
	if (output_type == TEXT) open_parm[1] = '\0';

	if ((dump_file = fopen(dump_file_name,open_parm)) == NULL) {
	    fprintf(stderr,"could not open dump file\n");
	    exit(8);
        }
	if (header == dwd && output_type == GRIB) wrtieee_header(0, dump_file);
    }

    /* skip dump - 1 records */
    for (i = 1; i < dump; i++) {
	msg = seek_grib(input, &pos, &len_grib, buffer, MSEEK);
	if (msg == NULL) {
	    fprintf(stderr, "ran out of data or bad file\n");
	    exit(8);
	}
	pos += len_grib;
    }
    if (dump > 0) count += dump - 1;
    n_dump = 0;

    for (;;) {
	if (n_dump == 1 && (mode == DUMP_RECORD || mode == DUMP_POSITION)) break;
	if (mode == DUMP_LIST) {
	    if (fgets(line,sizeof(line), stdin) == NULL) break;
            line[sizeof(line) - 1] = 0;
            if (sscanf(line,"%ld:%lu:", &count, &pos) != 2) {
		fprintf(stderr,"bad input from stdin\n");
                fprintf(stderr,"   %s\n", line);
	        exit(8);
	    }
	}

fail:
	msg = seek_grib(input, &pos, &len_grib, buffer, MSEEK);
	if (msg == NULL) {
	    if (mode == INVENTORY || mode == DUMP_ALL) break;
	    fprintf(stderr,"missing GRIB record(s)\n");
	    exit(8);
	}

        /* read all whole grib record */
        if (len_grib + msg - buffer > buffer_size) {
            buffer_size = len_grib + msg - buffer + 1000;
            buffer = (unsigned char *) realloc((void *) buffer, buffer_size);
            if (buffer == NULL) {
                fprintf(stderr,"ran out of memory\n");
                exit(8);
            }
        }
        if (read_grib(input, pos, len_grib, buffer) == 0) {
                fprintf(stderr,"error, could not read to end of record %ld\n",count);
                exit(8);
	}

	/* parse grib message */

	msg = buffer;
	end_msg = msg + len_grib - 1;

	/* check if last 4 bytes are '7777' */

//	simple check is for last 4 bytes == '7777'
//    	better check to see if pointers don't go past end_msg
//      if (end_msg[0] != 0x37 || end_msg[-1] != 0x37 || end_msg[-2] != 0x37 || end_msg[-3] != 0x37) {
//	    fprintf(stderr,"Skipping illegal grib1 message: error expected ending 7777\n");
//	    pos++;
//	    goto fail;
//	}

	if (msg + 8 + 27 > end_msg) {
	    pos++;
	    goto fail;
	}

        pds = (msg + 8);
        pointer = pds + PDS_LEN(pds);

	if (pointer > end_msg) {
	    pos++;
	    goto fail;
	}

        if (PDS_HAS_GDS(pds)) {
            gds = pointer;
            pointer += GDS_LEN(gds);
	    if (pointer > end_msg) {
	        pos++;
	        goto fail;
	    }
        }
        else {
            gds = NULL;
        }

        if (PDS_HAS_BMS(pds)) {
            bms = pointer;
            pointer += BMS_LEN(bms);
        }
        else {
            bms = NULL;
        }

	if (pointer+10 > end_msg) {
	    pos++;
	    goto fail;
	}

        bds = pointer;
        pointer += BDS_LEN(bds);

	if (pointer-msg+4 != len_grib) {
	    fprintf(stderr,"Len of grib message is inconsistent.\n");
	}

        /* end section - "7777" in ascii */
        if (pointer[0] != 0x37 || pointer[1] != 0x37 ||
            pointer[2] != 0x37 || pointer[3] != 0x37) {
            fprintf(stderr,"\n\n    missing end section\n");
            fprintf(stderr, "%2x %2x %2x %2x\n", pointer[0], pointer[1], 
		pointer[2], pointer[3]);
#ifdef DEBUG
	    printf("ignoring missing end section\n");
#else
	    exit(8);
#endif
        }

	/* figure out size of array */
	if (gds != NULL) {
	    GDS_grid(gds, bds, &nx, &ny, &nxny);
	}
	else if (bms != NULL) {
	    nxny = nx = BMS_nxny(bms);
	    ny = 1;
	}
	else {
	    if (BDS_NumBits(bds) == 0) {
                nxny = nx = 1;
                fprintf(stderr,"Missing GDS, constant record .. cannot "
                    "determine number of data points\n");
	    }
	    else {
	        nxny = nx = BDS_NValues(bds);
	    }
	    ny = 1;
	}
#ifdef CHECK_GRIB
	if (gds && ! GDS_Harmonic(gds)) {
	/* this grib check only works for simple packing */
	/* turn off if harmonic */
	    if (BDS_NumBits(bds) != 0) {
	        i = BDS_NValues(bds);
	        if (bms != NULL) {
	            i += missing_points(BMS_bitmap(bms),nxny);
	        }
	        if (i != nxny) {
	            fprintf(stderr,"grib header at record %ld: two values of nxny %ld %d\n",
			count,nxny,i);
		    fprintf(stderr,"   LEN %d DataStart %d UnusedBits %d #Bits %d nxny %ld\n",
			BDS_LEN(bds), BDS_DataStart(bds),BDS_UnusedBits(bds),
			BDS_NumBits(bds), nxny);
		    return_code = 15;
		    nxny = nx = i;
		    ny = 1;
	        }
	    }
 
        }
#endif
 
        if (verbose <= 0) {
	    printf("%ld:%lu:d=", count, pos);
	    PDS_date(pds,year_4,v_time);
	    printf(":%s:", k5toa(pds));

            if (verbose == 0) printf("kpds5=%d:kpds6=%d:kpds7=%d:TR=%d:P1=%d:P2=%d:TimeU=%d:",
	        PDS_PARAM(pds),PDS_KPDS6(pds),PDS_KPDS7(pds),
	        PDS_TimeRange(pds),PDS_P1(pds),PDS_P2(pds),
                PDS_ForecastTimeUnit(pds));
	    levels(PDS_KPDS6(pds), PDS_KPDS7(pds),PDS_Center(pds),verbose); printf(":");
	    PDStimes(PDS_TimeRange(pds),PDS_P1(pds),PDS_P2(pds),
                PDS_ForecastTimeUnit(pds));
	    if (PDS_Center(pds) == ECMWF) EC_ext(pds,"",":",verbose);
	    ensemble(pds, verbose);
	    printf("NAve=%d",PDS_NumAve(pds));
	    if (print_PDS || print_PDS10) print_pds(pds, print_PDS, print_PDS10, verbose);
	    if (gds && (print_GDS || print_GDS10)) print_gds(gds, print_GDS, print_GDS10, verbose);
            printf("\n");
       }
       else if (verbose == 1) {
	    printf("%ld:%lu:D=", count, pos);
            PDS_date(pds, 1, v_time);
	    printf(":%s:", k5toa(pds));
	    levels(PDS_KPDS6(pds), PDS_KPDS7(pds), PDS_Center(pds),verbose); printf(":");
            printf("kpds=%d,%d,%d:",
	        PDS_PARAM(pds),PDS_KPDS6(pds),PDS_KPDS7(pds));
	    PDStimes(PDS_TimeRange(pds),PDS_P1(pds),PDS_P2(pds),
                PDS_ForecastTimeUnit(pds));
	    if (PDS_Center(pds) == ECMWF) EC_ext(pds,"",":",verbose);
	    ensemble(pds, verbose);
	    GDS_winds(gds, verbose);
            printf("\"%s", k5_comments(pds));
	    if (print_PDS || print_PDS10) print_pds(pds, print_PDS, print_PDS10, verbose);
	    if (gds && (print_GDS || print_GDS10)) print_gds(gds, print_GDS, print_GDS10, verbose);
            printf("\n");
	}
        else if (verbose == 2) {
	    printf("rec %ld:%lu:date ", count, pos);
	    PDS_date(pds, 1, v_time);
	    printf(" %s kpds5=%d kpds6=%d kpds7=%d levels=(%d,%d) grid=%d ", 
	        k5toa(pds), PDS_PARAM(pds), PDS_KPDS6(pds), PDS_KPDS7(pds), 
                PDS_LEVEL1(pds), PDS_LEVEL2(pds), PDS_Grid(pds));
	        levels(PDS_KPDS6(pds),PDS_KPDS7(pds),PDS_Center(pds),verbose);

	    printf(" ");
	    if (PDS_Center(pds) == ECMWF) EC_ext(pds,""," ",verbose);
	    ensemble(pds, verbose);
	    PDStimes(PDS_TimeRange(pds),PDS_P1(pds),PDS_P2(pds),
                 PDS_ForecastTimeUnit(pds));
	    if (bms != NULL) 
		printf(" bitmap: %d undef", missing_points(BMS_bitmap(bms),nxny));
            printf("\n  %s=%s\n", k5toa(pds), k5_comments(pds));
	
            printf("  timerange %d P1 %d P2 %d TimeU %d  nx %d ny %d GDS grid %d "
		"num_in_ave %d missing %d\n", 
	        PDS_TimeRange(pds),PDS_P1(pds),PDS_P2(pds), 
                PDS_ForecastTimeUnit(pds), nx, ny, 
                gds == NULL ? -1 : GDS_DataType(gds), 
                PDS_NumAve(pds), PDS_NumMissing(pds));

	    printf("  center %d subcenter %d process %d Table %d", 
		PDS_Center(pds),PDS_Subcenter(pds),PDS_Model(pds),
                PDS_Vsn(pds));
	    GDS_winds(gds, verbose);
	    printf("\n");

	    if (gds && GDS_LatLon(gds) && nx != -1) 
		printf("  latlon: lat  %f to %f by %f  nxny %ld\n"
                       "          long %f to %f by %f, (%d x %d) scan %d "
                       "mode %d bdsgrid %d\n",
		  0.001*GDS_LatLon_La1(gds), 0.001*GDS_LatLon_La2(gds),
		  0.001*GDS_LatLon_dy(gds), nxny, 0.001*GDS_LatLon_Lo1(gds),
		  0.001*GDS_LatLon_Lo2(gds), 0.001*GDS_LatLon_dx(gds),
	    	  nx, ny, GDS_LatLon_scan(gds), GDS_LatLon_mode(gds),
		  BDS_Grid(bds));
	    else if (gds && GDS_LatLon(gds) && nx == -1) {
		printf("  thinned latlon: lat  %f to %f by %f  nxny %ld\n"
                       "          long %f to %f, %ld grid pts   (%d x %d) scan %d"
			" mode %d bdsgrid %d\n",
		  0.001*GDS_LatLon_La1(gds), 0.001*GDS_LatLon_La2(gds),
		  0.001*GDS_LatLon_dy(gds), nxny, 0.001*GDS_LatLon_Lo1(gds),
		  0.001*GDS_LatLon_Lo2(gds),
	    	  nxny, nx, ny, GDS_LatLon_scan(gds), GDS_LatLon_mode(gds),
		  BDS_Grid(bds));
		  GDS_prt_thin_lon(gds);
	    }
	    else if (gds && GDS_Gaussian(gds) && nx != -1)
		printf("  gaussian: lat  %f to %f\n"
                       "            long %f to %f by %f, (%d x %d) scan %d"
			" mode %d bdsgrid %d\n",
		  0.001*GDS_LatLon_La1(gds), 0.001*GDS_LatLon_La2(gds),
		  0.001*GDS_LatLon_Lo1(gds), 0.001*GDS_LatLon_Lo2(gds), 
		  0.001*GDS_LatLon_dx(gds),
	    	  nx, ny, GDS_LatLon_scan(gds), GDS_LatLon_mode(gds),
		  BDS_Grid(bds));
	    else if (gds && GDS_Gaussian(gds) && nx == -1) {
		printf("  thinned gaussian: lat  %f to %f\n"
                       "          long %f to %f, %ld grid pts   (%d x %d) scan %d"
			" mode %d bdsgrid %d\n",
		  0.001*GDS_LatLon_La1(gds), 0.001*GDS_LatLon_La2(gds),
		  0.001*GDS_LatLon_Lo1(gds), 0.001*GDS_LatLon_Lo2(gds),
	    	  nxny, nx, ny, GDS_LatLon_scan(gds), GDS_LatLon_mode(gds),
		  BDS_Grid(bds));
		  GDS_prt_thin_lon(gds);
	    }
	    else if (gds && GDS_Polar(gds))
		printf("  polar stereo: Lat1 %f Long1 %f Orient %f\n"
			"     %s pole (%d x %d) Dx %d Dy %d scan %d mode %d\n",
		    0.001*GDS_Polar_La1(gds),0.001*GDS_Polar_Lo1(gds),
		    0.001*GDS_Polar_Lov(gds),
		    GDS_Polar_pole(gds) == 0 ? "north" : "south", nx,ny,
		    GDS_Polar_Dx(gds),GDS_Polar_Dy(gds),
		    GDS_Polar_scan(gds), GDS_Polar_mode(gds));
	    else if (gds && GDS_Lambert(gds))
		printf("  Lambert Conf: Lat1 %f Lon1 %f Lov %f\n"
                       "      Latin1 %f Latin2 %f LatSP %f LonSP %f\n"
                       "      %s (%d x %d) Dx %f Dy %f scan %d mode %d\n",
                     0.001*GDS_Lambert_La1(gds),0.001*GDS_Lambert_Lo1(gds),
                     0.001*GDS_Lambert_Lov(gds),
                     0.001*GDS_Lambert_Latin1(gds), 0.001*GDS_Lambert_Latin2(gds),
                     0.001*GDS_Lambert_LatSP(gds), 0.001*GDS_Lambert_LonSP(gds),
                      GDS_Lambert_NP(gds) ? "North Pole": "South Pole",
                     GDS_Lambert_nx(gds), GDS_Lambert_ny(gds),
                     0.001*GDS_Lambert_dx(gds), 0.001*GDS_Lambert_dy(gds),
                     GDS_Lambert_scan(gds), GDS_Lambert_mode(gds));
	    else if (gds && GDS_Albers(gds))
		/* Albers equal area has same parameters as Lambert conformal */
		printf("  Albers Equal-Area: Lat1 %f Lon1 %f Lov %f\n"
                       "      Latin1 %f Latin2 %f LatSP %f LonSP %f\n"
                       "      %s (%d x %d) Dx %f Dy %f scan %d mode %d\n",
                     0.001*GDS_Lambert_La1(gds),0.001*GDS_Lambert_Lo1(gds),
                     0.001*GDS_Lambert_Lov(gds),
                     0.001*GDS_Lambert_Latin1(gds), 0.001*GDS_Lambert_Latin2(gds),
                     0.001*GDS_Lambert_LatSP(gds), 0.001*GDS_Lambert_LonSP(gds),
                      GDS_Lambert_NP(gds) ? "North Pole": "South Pole",
                     GDS_Lambert_nx(gds), GDS_Lambert_ny(gds),
                     0.001*GDS_Lambert_dx(gds), 0.001*GDS_Lambert_dy(gds),
                     GDS_Lambert_scan(gds), GDS_Lambert_mode(gds));
	    else if (gds && GDS_Mercator(gds))
		printf("  Mercator: lat  %f to %f by %f km  nxny %ld\n"
                       "          long %f to %f by %f km, (%d x %d) scan %d"
			" mode %d Latin %f bdsgrid %d\n",
		  0.001*GDS_Merc_La1(gds), 0.001*GDS_Merc_La2(gds),
		  0.001*GDS_Merc_dy(gds), nxny, 0.001*GDS_Merc_Lo1(gds),
		  0.001*GDS_Merc_Lo2(gds), 0.001*GDS_Merc_dx(gds),
	    	  nx, ny, GDS_Merc_scan(gds), GDS_Merc_mode(gds), 
		  0.001*GDS_Merc_Latin(gds), BDS_Grid(bds));
	    else if (gds && GDS_ssEgrid(gds))
		printf("  Semi-staggered Arakawa E-Grid: lat0 %f lon0 %f nxny %d\n"
                       "    dLat %f dLon %f (%d x %d) scan %d mode %d\n",
		  0.001*GDS_ssEgrid_La1(gds), 0.001*GDS_ssEgrid_Lo1(gds), 
                  GDS_ssEgrid_n(gds)*GDS_ssEgrid_n_dum(gds), 
                  0.001*GDS_ssEgrid_dj(gds), 0.001*GDS_ssEgrid_di(gds), 
                  GDS_ssEgrid_Lo2(gds), GDS_ssEgrid_La2(gds),
                  GDS_ssEgrid_scan(gds), GDS_ssEgrid_mode(gds));
            else if (gds && GDS_ss2dEgrid(gds))
                printf("  Semi-staggered Arakawa E-Grid (2D): lat0 %f lon0 %f nxny %d\n"
                       "    dLat %f dLon %f (tlm0d %f tph0d %f) scan %d mode %d\n",
                   0.001*GDS_ss2dEgrid_La1(gds), 0.001*GDS_ss2dEgrid_Lo1(gds),
                   GDS_ss2dEgrid_nx(gds)*GDS_ss2dEgrid_ny(gds),
                   0.001*GDS_ss2dEgrid_dj(gds), 0.001*GDS_ss2dEgrid_di(gds),
                   0.001*GDS_ss2dEgrid_Lo2(gds), 0.001*GDS_ss2dEgrid_La2(gds),
                   GDS_ss2dEgrid_scan(gds), GDS_ss2dEgrid_mode(gds));
            else if (gds && GDS_ss2dBgrid(gds))
                printf("  Semi-staggered Arakawa B-Grid (2D): lat0 %f lon0 %f nxny %d\n"
                       "    dLat %f dLon %f (tlm0d %f tph0d %f) scan %d mode %d\n",
                   0.001*GDS_ss2dBgrid_La1(gds), 0.001*GDS_ss2dBgrid_Lo1(gds),
                   GDS_ss2dBgrid_nx(gds)*GDS_ss2dBgrid_ny(gds),
                   0.001*GDS_ss2dBgrid_dj(gds), 0.001*GDS_ss2dBgrid_di(gds),
                   0.001*GDS_ss2dBgrid_Lo2(gds), 0.001*GDS_ss2dBgrid_La2(gds),
                   GDS_ss2dBgrid_scan(gds), GDS_ss2dBgrid_mode(gds)); 
	    else if (gds && GDS_fEgrid(gds)) 
		printf("  filled Arakawa E-Grid: lat0 %f lon0 %f nxny %d\n"
                       "    dLat %f dLon %f (%d x %d) scan %d mode %d\n",
		  0.001*GDS_fEgrid_La1(gds), 0.001*GDS_fEgrid_Lo1(gds), 
                  GDS_fEgrid_n(gds)*GDS_fEgrid_n_dum(gds), 
                  0.001*GDS_fEgrid_dj(gds), 0.001*GDS_fEgrid_di(gds), 
                  GDS_fEgrid_Lo2(gds), GDS_fEgrid_La2(gds),
                  GDS_fEgrid_scan(gds), GDS_fEgrid_mode(gds));
	    else if (gds && GDS_RotLL(gds))
		printf("  rotated LatLon grid  lat %f to %f  lon %f to %f\n"
		       "    nxny %ld  (%d x %d)  dx %d dy %d  scan %d  mode %d\n"
		       "    transform: south pole lat %f lon %f  rot angle %f\n", 
		   0.001*GDS_RotLL_La1(gds), 0.001*GDS_RotLL_La2(gds), 
		   0.001*GDS_RotLL_Lo1(gds), 0.001*GDS_RotLL_Lo2(gds),
		   nxny, GDS_RotLL_nx(gds), GDS_RotLL_ny(gds),
		   GDS_RotLL_dx(gds), GDS_RotLL_dy(gds),
		   GDS_RotLL_scan(gds), GDS_RotLL_mode(gds),
		   0.001*GDS_RotLL_LaSP(gds), 0.001*GDS_RotLL_LoSP(gds),
		   GDS_RotLL_RotAng(gds) );
	    else if (gds && GDS_Gnomonic(gds))
		printf("  Gnomonic grid\n");
	    else if (gds && GDS_Harmonic(gds))
		printf("  Harmonic (spectral):  pentagonal spectral truncation: nj %d nk %d nm %d\n",
		       GDS_Harmonic_nj(gds), GDS_Harmonic_nk(gds),
		       GDS_Harmonic_nm(gds));
		if (gds && GDS_Harmonic_type(gds) == 1)
		  printf("  Associated Legendre polynomials\n");
            else if (gds && GDS_Triangular(gds))
                printf("  Triangular grid:  nd %d ni %d (= 2^%d x 3^%d)\n",
		    GDS_Triangular_nd(gds), GDS_Triangular_ni(gds), 
                    GDS_Triangular_ni2(gds), GDS_Triangular_ni3(gds) );
	    if (print_PDS || print_PDS10) 
                print_pds(pds, print_PDS, print_PDS10, verbose);
	    if (gds && (print_GDS || print_GDS10)) 
                 print_gds(gds, print_GDS, print_GDS10, verbose);
	}

	if (mode != INVENTORY && output_type == GRIB) {
	        if (header == dwd) wrtieee_header((int) len_grib, dump_file);
	        fwrite((void *) msg, sizeof(char), len_grib, dump_file);
	        if (header == dwd) wrtieee_header((int) len_grib, dump_file);
	    n_dump++;
	}

	if ((mode != INVENTORY && output_type != GRIB) || verbose > 1) {
	    /* decode numeric data */
 
            if ((array = (float *) malloc(sizeof(float) * nxny)) == NULL) {
                fprintf(stderr,"memory problems\n");
                exit(8);
            }

	    temp = int_power(10.0, - PDS_DecimalScale(pds));

 	    BDS_unpack(array, bds, BMS_bitmap(bms), BDS_NumBits(bds), nxny,
			   temp*BDS_RefValue(bds),temp*int_power(2.0, BDS_BinScale(bds)));

	    if (verbose > 1) {
		rmin = FLT_MAX;
		rmax = -FLT_MAX;
	        for (i = 0; i < nxny; i++) {
		    if (fabs(array[i]-UNDEFINED) > 0.0001*UNDEFINED) {
	                rmin = min(rmin,array[i]);
	                rmax = max(rmax,array[i]);
		    }
	        }
	        printf("  min/max data %g %g  num bits %d "
			" BDS_Ref %g  DecScale %d BinScale %d\n", 
		    rmin, rmax, BDS_NumBits(bds), BDS_RefValue(bds),
		    PDS_DecimalScale(pds), BDS_BinScale(bds));
	    }

	    if (mode != INVENTORY && output_type != GRIB) {
		/* dump code */
		if (output_PDS_GDS == 1) {
		    /* insert code here */
	            if (output_type == BINARY || output_type == IEEE) {
			/* write PDS */
			i = PDS_LEN(pds) + 4;
	                if (header == simple && output_type == BINARY) 
				fwrite((void *) &i, sizeof(int), 1, dump_file);
	                if (header == simple && output_type == IEEE) wrtieee_header(i, dump_file);
	                fwrite((void *) "PDS ", 1, 4, dump_file);
	                fwrite((void *) pds, 1, i - 4, dump_file);
	                if (header == simple && output_type == BINARY) 
				fwrite((void *) &i, sizeof(int), 1, dump_file);
	                if (header == simple && output_type == IEEE) wrtieee_header(i, dump_file);

			/* write GDS */
			i = (gds) ?  GDS_LEN(gds) + 4 : 4;
	                if (header == simple && output_type == BINARY) 
				fwrite((void *) &i, sizeof(int), 1, dump_file);
	                if (header == simple && output_type == IEEE) wrtieee_header(i, dump_file);
	                fwrite((void *) "GDS ", 1, 4, dump_file);
	                if (gds) fwrite((void *) gds, 1, i - 4, dump_file);
	                if (header == simple && output_type == BINARY) 
				fwrite((void *) &i, sizeof(int), 1, dump_file);
	                if (header == simple && output_type == IEEE) wrtieee_header(i, dump_file);
		    }
		} 

	        if (output_type == BINARY) {
	            i = nxny * sizeof(float);
	            if (header == simple) fwrite((void *) &i, sizeof(int), 1, dump_file);
	            fwrite((void *) array, sizeof(float), nxny, dump_file);
	            if (header == simple) fwrite((void *) &i, sizeof(int), 1, dump_file);
	        }
		else if (output_type == IEEE) {
		    wrtieee(array, nxny, header, dump_file);
		}
	        else if (output_type == TEXT) {
	            /* number of points in grid */
	            if (header == simple) {
		        if (nx <= 0 || ny <= 0 || nxny != nx*ny) {
                            fprintf(dump_file, "%ld %d\n", nxny, 1);
			}
			else {
			    fprintf(dump_file, "%d %d\n", nx, ny);
			}
		    }
	            for (i = 0; i < nxny; i++) {
		        fprintf(dump_file,"%g\n", array[i]);
		    }
	        }
	        n_dump++;
	    }
	    free(array);
	    if (verbose > 0) printf("\n");
	}

	if (output_type != NONE) fflush(dump_file);
	fflush(stdout);
	    
        pos += len_grib;
        count++;
    }

    if (mode != INVENTORY) {
	if (header == dwd && output_type == GRIB) wrtieee_header(0, dump_file);
	if (ferror(dump_file)) {
		fprintf(stderr,"error writing %s\n",dump_file_name);
		exit(8);
	}
    }
    fclose(input);
    return (return_code);
}

void print_pds(unsigned char *pds, int print_PDS, int print_PDS10, int verbose) {
    int i, j;

    j = PDS_LEN(pds);
    if (verbose < 2) {
        if (print_PDS && verbose < 2) {
            printf(":PDS=");
            for (i = 0; i < j; i++) {
                printf("%2.2x", (int) pds[i]);
            }
        }
        if (print_PDS10 && verbose < 2) {
            printf(":PDS10=");
            for (i = 0; i < j; i++) {
                printf(" %d", (int) pds[i]);
            }
        }
    }
    else {
        if (print_PDS) {
            printf("  PDS(1..%d)=",j);
            for (i = 0; i < j; i++) {
                if (i % 20 == 0) printf("\n    %4d:",i+1);
                printf(" %3.2x", (int) pds[i]);
            }
            printf("\n");
        }
        if (print_PDS10) {
            printf("  PDS10(1..%d)=",j);
            for (i = 0; i < j; i++) {
                if (i % 20 == 0) printf("\n    %4d:",i+1);
                printf(" %3d", (int) pds[i]);
            }
            printf("\n");
        }
    }
}

void print_gds(unsigned char *gds, int print_GDS, int print_GDS10, int verbose) {
    int i, j;

    j = GDS_LEN(gds);
    if (verbose < 2) {
        if (print_GDS && verbose < 2) {
            printf(":GDS=");
            for (i = 0; i < j; i++) {
                printf("%2.2x", (int) gds[i]);
            }
        }
        if (print_GDS10 && verbose < 2) {
            printf(":GDS10=");
            for (i = 0; i < j; i++) {
                printf(" %d", (int) gds[i]);
            }
        }
    }
    else {
        if (print_GDS) {
            printf("  GDS(1..%d)=",j);
            for (i = 0; i < j; i++) {
                if (i % 20 == 0) printf("\n    %4d:",i+1);
                printf(" %3.2x", (int) gds[i]);
            }
            printf("\n");
        }
        if (print_GDS10) {
            printf("  GDS10(1..%d)=",j);
            for (i = 0; i < j; i++) {
                if (i % 20 == 0) printf("\n    %4d:",i+1);
                printf(" %3d", (int) gds[i]);
            }
            printf("\n");
        }
    }
}
/*
 * find next grib header
 *
 * file = what do you think?
 * pos = initial position to start looking at  ( = 0 for 1st call)
 *       returns with position of next grib header (units=bytes)
 * len_grib = length of the grib record (bytes)
 * buffer[buf_len] = buffer for reading/writing
 *
 * returns (char *) to start of GRIB header+PDS
 *         NULL if not found
 *
 * adapted from SKGB (Mark Iredell)
 *
 * v1.1 9/94 Wesley Ebisuzaki
 * v1.2 3/96 Wesley Ebisuzaki handles short records at end of file
 * v1.3 8/96 Wesley Ebisuzaki increase NTRY from 3 to 100 for the folks
 *      at Automation decided a 21 byte WMO bulletin header wasn't long 
 *      enough and decided to go to an 8K header.  
 * v1.4 11/10/2001 D. Haalman, looks at entire file, does not try
 *      to read past EOF
 *      3/8/2010 echack added by Brian Doty
 * v1.5 5/2011 changes for ECMWF who have grib1+grib2 files, scan entire file
 */

#ifndef min
   #define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

/* #define LEN_HEADER_PDS (28+42+100) */
#define LEN_HEADER_PDS (28+8)

int ec_large_grib = 0,  len_ec_bds;

unsigned char *seek_grib(FILE *file, unsigned long *pos, long *len_grib, 
        unsigned char *buffer, unsigned int buf_len) {

    int i, len;
    long length_grib;
    static int warn_grib2 = 0;
    clearerr(file);
    while ( !feof(file) ) {

        if (fseek(file, *pos, SEEK_SET) == -1) break;
	i = fread(buffer, sizeof (unsigned char), buf_len, file);     
        if (ferror(file)) break;
        len = i - LEN_HEADER_PDS;
     
        for (i = 0; i < len; i++) {
            if (buffer[i] == 'G' && buffer[i+1] == 'R' && buffer[i+2] == 'I'
                && buffer[i+3] == 'B') {
		/* grib edition 1 */
		if (buffer[i+7] == 1) {
                    *pos = i + *pos;
                    *len_grib = length_grib = (buffer[i+4] << 16) + (buffer[i+5] << 8) +
                            buffer[i+6];

		    /* small records don't have ECMWF hack */
		    if ((length_grib & 0x800000) == 0) { ec_large_grib = 0; return (buffer + i); }

		    /* potential for ECMWF hack */
		    ec_large_grib = 1;
		    *len_grib = echack(file, *pos, length_grib);
                    return (buffer+i);
		}

		/* grib edition 2 */
		else if (buffer[i+7] == 2) {
		    if (warn_grib2++ == 0) fprintf(stderr,"grib2 message ignored (use wgrib2)\n");
		}

            }
        }
	*pos = *pos + (buf_len - LEN_HEADER_PDS);
    }

    *len_grib = 0;
    return (unsigned char *) NULL;
}


/* If the encoded grib record length is long enough, we may have an encoding
   of an even longer record length using the ecmwf hack.  To check for this
   requires getting the length of the binary data section.  To get this requires
   getting the lengths of the various sections before the bds.  To see if those
   sections are there requires checking the flags in the pds.  */

long echack(FILE *file, long pos, long len_grib) {

    int gdsflg, bmsflg, center;
    unsigned int pdslen, gdslen, bmslen, bdslen;
    unsigned char buf[8];
    long len;

    len = len_grib;

    /* Get pdslen */

    if (fseek(file, pos+8, SEEK_SET) == -1) return 0;
    if (fread(buf, sizeof (unsigned char), 8, file) != 8) return 0;
    pdslen = __LEN24(buf);

    center = buf[4];

    /* know that NCEP and CMC do not use echack */
    if (center == NMC || center == CMC) {
	ec_large_grib = 0;
        return len_grib;
    }


    gdsflg = buf[7] & 128;
    bmsflg = buf[7] & 64;

    gdslen=0;
    if (gdsflg) {
        if (fseek(file, pos+8+pdslen, SEEK_SET) == -1) return 0;
        if (fread(buf, sizeof (unsigned char), 3, file) != 3) return 0;
        gdslen = __LEN24(buf);
    }

    /* if there, get length of bms */

    bmslen = 0;
    if (bmsflg) {
       if (fseek(file, pos+8+pdslen+gdslen, SEEK_SET) == -1) return 0;
       if (fread(buf, sizeof (unsigned char), 3, file) != 3) return 0;
       bmslen = __LEN24(buf);
    }

    /* get bds length */

    if (fseek(file, pos+8+pdslen+gdslen+bmslen, SEEK_SET) == -1) return 0;
    if (fread(buf, sizeof (unsigned char), 3, file) != 3) return 0;
    bdslen = __LEN24(buf);

    /* Now we can check if this record is hacked */

    if (bdslen >= 120) {
	/* normal record */
	ec_large_grib = 0;
    }
    else {
        /* ECMWF hack */
        len_grib = (len & 0x7fffff) * 120 - bdslen + 4;
        len_ec_bds = len_grib - (12 + pdslen + gdslen + bmslen);
	ec_large_grib = 1;
    }
    return len_grib;
}

/* ibm2flt       wesley ebisuzaki
 *
 * v1.1 .. faster
 * v1.1 .. if mant == 0 -> quick return
 *
 */


double ibm2flt(unsigned char *ibm) {

	int positive, power;
	unsigned int abspower;
	long int mant;
	double value, exp;

	mant = (ibm[1] << 16) + (ibm[2] << 8) + ibm[3];
        if (mant == 0) return 0.0;

	positive = (ibm[0] & 0x80) == 0;
	power = (int) (ibm[0] & 0x7f) - 64;
	abspower = power > 0 ? power : -power;


	/* calc exp */
	exp = 16.0;
	value = 1.0;
	while (abspower) {
		if (abspower & 1) {
			value *= exp;
		}
		exp = exp * exp;
		abspower >>= 1;
	}

	if (power < 0) value = 1.0 / value;
	value = value * mant / 16777216.0;
	if (positive == 0) value = -value;
	return value;
}
	
/*
 * read_grib.c
 *
 * reads grib message
 *
 * input: pos, byte position of grib message
 *        len_grib, length of grib message
 * output: *buffer, grib message
 *
 * note: call seek_grib first
 *
 * v1.0 9/94 Wesley Ebisuzaki
 *
 */

int read_grib(FILE *file, long pos, long len_grib, unsigned char *buffer) {

    int i;


    if (fseek(file, pos, SEEK_SET) == -1) {
	    return 0;
    }

    i = fread(buffer, sizeof (unsigned char), len_grib, file);
    return (i == len_grib);
}

/*
 * w. ebisuzaki
 *
 *  return x**y
 *
 *
 *  input: double x
 *	   int y
 */
double int_power(double x, int y) {

	double value;

	if (y < 0) {
		y = -y;
		x = 1.0 / x;
	}
	value = 1.0;

	while (y) {
		if (y & 1) {
			value *= x;
		}
		x = x * x;
		y >>= 1;
	}
	return value;
}

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

/* 1996				wesley ebisuzaki
 *
 * Unpack BDS section
 *
 * input: *bits, pointer to packed integer data
 *        *bitmap, pointer to bitmap (undefined data), NULL if none
 *        n_bits, number of bits per packed integer
 *        n, number of data points (includes undefined data)
 *        ref, scale: flt[] = ref + scale*packed_int
 * output: *flt, pointer to output array
 *        undefined values filled with UNDEFINED
 *
 * note: code assumes an integer > 32 bits
 *
 * 7/98 v1.2.1 fix bug for bitmaps and nbit >= 25 found by Larry Brasfield
 * 2/01 v1.2.2 changed jj from long int to double
 * 3/02 v1.2.3 added unpacking extensions for spectral data 
 *             Luis Kornblueh, MPIfM 
 * 7/06 v.1.2.4 fixed some bug complex packed data was not set to undefined
 */

static unsigned int mask[] = {0,1,3,7,15,31,63,127,255};
static unsigned int map_masks[8] = {128, 64, 32, 16, 8, 4, 2, 1};
static double shift[9] = {1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0};

void BDS_unpack(float *flt, unsigned char *bds, unsigned char *bitmap,
	int n_bits, int n, double ref, double scale) {

    unsigned char *bits;

    int i, mask_idx, t_bits, c_bits, j_bits;
    unsigned int j, map_mask, tbits, jmask, bbits;
    double jj;


    if (BDS_ComplexPacking(bds)) {
	fprintf(stderr,"*** Cannot decode complex packed fields n=%d***\n", n);
	exit(8);
	for (i = 0; i < n; i++) {
	    *flt++ = UNDEFINED;
	}
	return;
    }

    if (BDS_Harmonic(bds)) {
        bits = bds + 15;
        /* fill in global mean */
        *flt++ = BDS_Harmonic_RefValue(bds);
        n -= 1; 
    }
    else {
        bits = bds + 11;  
    }

    tbits = bbits = 0;

    /* assume integer has 32+ bits */
    if (n_bits <= 25) {
        jmask = (1 << n_bits) - 1;
        t_bits = 0;

        if (bitmap) {
	    for (i = 0; i < n; i++) {
		/* check bitmap */
		mask_idx = i & 7;
		if (mask_idx == 0) bbits = *bitmap++;
	        if ((bbits & map_masks[mask_idx]) == 0) {
		    *flt++ = UNDEFINED;
		    continue;
	        }

	        while (t_bits < n_bits) {
	            tbits = (tbits * 256) + *bits++;
	            t_bits += 8;
	        }
	        t_bits -= n_bits;
	        j = (tbits >> t_bits) & jmask;
	        *flt++ = ref + scale*j;
            }
        }
        else {
	    for (i = 0; i < n; i++) {
		if (n_bits - t_bits > 8) {
                    tbits = (tbits << 16) | (bits[0] << 8) | (bits[1]);
		    bits += 2;
                    t_bits += 16;
		}
                while (t_bits < n_bits) {
                    tbits = (tbits * 256) + *bits++;
                    t_bits += 8;
                }
                t_bits -= n_bits;
                flt[i] = (tbits >> t_bits) & jmask;
            }
	    /* at least this vectorizes :) */
	    for (i = 0; i < n; i++) {
		flt[i] = ref + scale*flt[i];
	    }
        }
    }
    else {
	/* older unoptimized code, not often used */
        c_bits = 8;
        map_mask = 128;
        while (n-- > 0) {
	    if (bitmap) {
	        j = (*bitmap & map_mask);
	        if ((map_mask >>= 1) == 0) {
		    map_mask = 128;
		    bitmap++;
	        }
	        if (j == 0) {
		    *flt++ = UNDEFINED;
		    continue;
	        }
	    }

	    jj = 0.0;
	    j_bits = n_bits;
	    while (c_bits <= j_bits) {
	        if (c_bits == 8) {
		    jj = jj * 256.0  + (double) (*bits++);
		    j_bits -= 8;
	        }
	        else {
		    jj = (jj * shift[c_bits]) + (double) (*bits & mask[c_bits]);
		    bits++;
		    j_bits -= c_bits;
		    c_bits = 8;
	        }
	    }
	    if (j_bits) {
	        c_bits -= j_bits;
	        jj = (jj * shift[j_bits]) + (double) ((*bits >> c_bits) & mask[j_bits]);
	    }
	    *flt++ = ref + scale*jj;
        }
    }
    return;
}

/*
 * convert a float to an ieee single precision number v1.1
 * (big endian)
 *                      Wesley Ebisuzaki
 *
 * bugs: doesn't handle subnormal numbers
 * bugs: assumes length of integer >= 25 bits
 */

int flt2ieee(float x, unsigned char *ieee) {

	int sign, exp;
        unsigned int umant;
	double mant;

	if (x == 0.0) {
		ieee[0] = ieee[1] = ieee[2] = ieee[3] = 0;
		return 0;
	}

	/* sign bit */
	if (x < 0.0) {
		sign = 128;
		x = -x;
	}
	else sign = 0;
	mant = frexp((double) x, &exp);

        /* 2^24 = 16777216 */

	umant = mant * 16777216 + 0.5;
	if (umant >= 16777216) {
            umant = umant / 2;
            exp++;
        }
        /* bit 24 should be a 1 .. not used in ieee format */

	exp = exp - 1 + 127;

	if (exp < 0) {
		/* signed zero */
		ieee[0] = sign;
		ieee[1] = ieee[2] = ieee[3] = 0;
		return 0;
	}
	if (exp > 255) {
		/* signed infinity */
		ieee[0] = sign + 127;
		ieee[1] = 128;
                ieee[2] = ieee[3] = 0;
                return 0;
	}
	/* normal number */

	ieee[0] = sign + (exp >> 1);

        ieee[3] = umant & 255;
        ieee[2] = (umant >> 8) & 255;
        ieee[1] = ((exp & 1) << 7) + ((umant >> 16) & 127);
	return 0;
}


/* wesley ebisuzaki v1.3
 *
 * write ieee file -- big endian format
 *
 * input float *array		data to be written
 *	 int n			size of array
 *	 int header		1 for f77 style header 0 for none
 *				(header is 4 byte header
 *	 FILE *output		output file
 *
 * v1.2 7/97 buffered, faster
 * v1.3 2/99 fixed (typo) error in wrtieee_header found by
 *     Bob Farquhar
 */

#define BSIZ 1024*4

int wrtieee(float *array, int n, int header, FILE *output) {

	unsigned long int l;
	int i, nbuf;
	unsigned char buff[BSIZ];
	unsigned char h4[4];

	nbuf = 0;
	h4[0] = h4[1] =  h4[2] = h4[3] = 0;
	if (header) {
		l = n * 4;
		for (i = 0; i < 4; i++) {
			h4[i] = l & 255;
			l >>= 8;
		}
		buff[nbuf++] = h4[3];
		buff[nbuf++] = h4[2];
		buff[nbuf++] = h4[1];
		buff[nbuf++] = h4[0];
	}
	for (i = 0; i < n; i++) {
		if (nbuf >= BSIZ) {
		    fwrite(buff, 1, BSIZ, output);
		    nbuf = 0;
		}
		flt2ieee(array[i], buff + nbuf);
		nbuf += 4;
	}
	if (header) {
		if (nbuf == BSIZ) {
		    fwrite(buff, 1, BSIZ, output);
		    nbuf = 0;
		}
		buff[nbuf++] = h4[3];
		buff[nbuf++] = h4[2];
		buff[nbuf++] = h4[1];
		buff[nbuf++] = h4[0];
	}
	if (nbuf) fwrite(buff, 1, nbuf, output);
	return 0;
}

/* write a big-endian 4 byte integer .. f77 IEEE  header */

int wrtieee_header(unsigned int n, FILE *output) {
	unsigned h4[4];

	h4[0] = n & 255;
	h4[1] = (n >> 8) & 255;
	h4[2] = (n >> 16) & 255;
	h4[3] = (n >> 24) & 255;

	putc(h4[3],output);
	putc(h4[2],output);
	putc(h4[1],output);
	putc(h4[0],output);

	return 0;
}


/* wesley ebisuzaki v1.0
 *
 * levels.c
 *
 * prints out a simple description of kpds6, kpds7
 *    (level/layer data)
 *  kpds6 = octet 10 of the PDS
 *  kpds7 = octet 11 and 12 of the PDS
 *    (kpds values are from NMC's grib routines)
 *  center = PDS_Center(pds) .. NMC, ECMWF, etc
 *
 * the description of the levels is 
 *   (1) incomplete
 *   (2) include some NMC-only values (>= 200?)
 *
 * v1.1 wgrib v1.7.3.1 updated with new levels
 * v1.2 added new level and new parameter
 * v1.2.1 modified level 117 pv units
 * v1.2.2 corrected level 141
 * v1.2.3 fixed layer 206 (was 205)
 * v1.2.4 layer 210: new wmo defn > NCEP version
 * v1.2.5 updated table 3/2007 to on388
 */

void levels(int kpds6, int kpds7, int center, int verbose) {

	int o11, o12;
    
	/* octets 11 and 12 */
	o11 = kpds7 / 256;
	o12 = kpds7 % 256;


	switch (kpds6) {

	case 1: printf("sfc");
		break;
	case 2: printf("cld base");
		break;
	case 3: printf("cld top");
		break;
	case 4: printf("0C isotherm");
		break;
	case 5: printf("cond lev");
		break;
	case 6: printf("max wind lev");
		break;
	case 7: printf("tropopause");
		break;
	case 8: printf("nom. top");
		break;
	case 9: printf("sea bottom");
		break;
	case 200:
	case 10: printf("atmos col");
		break;

	case 12: printf("low cld bot");
		break;

	case 13: printf("low cld top");
		break;
	case 14: printf("low cld lay");
		break;
	case 20: 
		if (verbose == 2) printf("temp=%fK", kpds7/100.0);
		else printf("T=%fK", kpds7/100.0);
		break;
	case 22:
	case 222: printf("mid cld bot");
		break;
	case 23:
	case 223: printf("mid cld top");
		break;
	case 24:
	case 224: printf("mid cld lay");
		break;
	case 32:
	case 232: printf("high cld bot");
		break;
	case 33:
	case 233: printf("high cld top");
		break;
	case 34:
	case 234: printf("high cld lay");
		break;

	case 201: printf("ocean column");
		break;
	case 204: printf("high trop freezing lvl");
		break;
	case 206: printf("grid-scale cld bot");
		break;
	case 207: printf("grid-scale cld top");
		break;
	case 209: printf("bndary-layer cld bot");
		break;
	case 210: 
                if (center == NMC) printf("bndary-layer cld top");
		else printf("%.2f mb",kpds7*0.01);
		break;
	case 211: 
		if (center == JMA) printf("soil column");
		else printf("bndary-layer cld layer");
		break;
        case 212:
                if (center == JMA) printf("bot land sfc model");
                else printf("low cld bot");
                break;
	case 213:
                if (center == JMA) printf("soil layer %d",kpds7);
		else printf("low cld top");
		break;
	case 214: printf("low cld lay");
		break;
	case 215: printf("cloud ceiling");
		break;
	case 216: printf("Cb base");
		break;
	case 217: printf("Cb top");
		break;
	case 220: printf("planetary boundary layer (from Richardson no.)");
		break;
	case 235: if (kpds7 % 10 == 0)
		printf("%dC ocean isotherm level",kpds7/10);
		else printf("%.1fC ocean isotherm level",kpds7/10.0);
		break;
	case 236: printf("%d-%dm ocean layer",o11*10,o12*10);
		break;
	case 237: printf("ocean mixed layer bot");
		break;
	case 238: printf("ocean isothermal layer bot");
		break;
	case 239: printf("sfc-26C ocean layer");
		break;
	case 240: printf("ocean mixed layer");
		break;
	case 241: printf("ordered sequence of data");
		break;
	case 242: printf("convect-cld bot");
		break;
	case 243: printf("convect-cld top");
		break;
	case 244: printf("convect-cld layer");
		break;
	case 245: printf("lowest level of wet bulb zero");
		break;
	case 246: printf("max e-pot-temp lvl");
		break;
	case 247: printf("equilibrium lvl");
		break;
	case 248: printf("shallow convect-cld bot");
		break;
	case 249: printf("shallow convect-cld top");
		break;
	case 251: printf("deep convect-cld bot");
		break;
	case 252: printf("deep convect-cld top");
		break;
	case 253: printf("lowest bottom level of supercooled liequid water layer");
		break;
	case 254: printf("highest top level of supercooled liquid water layer");
		break;
	case 100: printf("%d mb",kpds7);
	 	break;
	case 101: printf("%d-%d mb",o11*10,o12*10);
	 	break;
	case 102: printf("MSL");
	 	break;
	case 103: printf("%d m above MSL",kpds7);
	 	break;
	case 104: printf("%d-%d m above msl",o11*100,o12*100);
	 	break;
	case 105: printf("%d m above gnd",kpds7);
	 	break;
	case 106: printf("%d-%d m above gnd",o11*100,o12*100);
	 	break;
	case 107: printf("sigma=%.4f",kpds7/10000.0);
	 	break;
	case 108: printf("sigma %.2f-%.2f",o11/100.0,o12/100.0);
	 	break;
	case 109: printf("hybrid lev %d",kpds7);
	 	break;
	case 110: printf("hybrid %d-%d",o11,o12);
	 	break;
	case 111: printf("%d cm down",kpds7);
	 	break;
	case 112: printf("%d-%d cm down",o11,o12);
	 	break;
	case 113: 
		if (verbose == 2) printf("pot-temp=%dK",kpds7);
		else printf("%dK",kpds7);
	 	break;
	case 114: printf("%d-%dK",475-o11,475-o12);
	 	break;
	case 115: printf("%d mb above gnd",kpds7);
	 	break;
	case 116: printf("%d-%d mb above gnd",o11,o12);
	 	break;
	case 117: printf("%d pv units",INT2(o11,o12)); /* units are suspect */
	 	break;
	case 119: printf("%.5f (ETA level)",kpds7/10000.0);
	 	break;
	case 120: printf("%.2f-%.2f (ETA levels)",o11/100.0,o12/100.0);
	 	break;
	case 121: printf("%d-%d mb",1100-o11,1100-o12);
	 	break;
	case 125: printf("%d cm above gnd",kpds7);
	 	break;
	case 126: 
		if (center == NMC) printf("%.2f mb",kpds7*0.01);
	 	break;
	case 128: printf("%.3f-%.3f (sigma)",1.1-o11/1000.0, 1.1-o12/1000.0);
	 	break;
	case 141: printf("%d-%d mb",o11*10,1100-o12);
	 	break;
	case 160: printf("%d m below sea level",kpds7);
	 	break;
	default:
	 	break;
	}
}

/*
 * PDStimes.c   v1.2 wesley ebisuzaki
 *
 * prints something readable for time code in grib file
 *
 * not all cases decoded
 * for NCEP/NCAR Reanalysis
 *
 * v1.2.1 1/99 fixed forecast time unit table
 * v1.2.2 10/01 add time_range = 11 (at DWD)  Helmut P. Frank
 * v1.2.3 10/05 add time units 13 = 15 min, 14 = 30 min, and
 *              time range 13 = nudging analysis, 14 = relabeled forecast
 *              (at DWD), Helmut P. Frank
 */

static char *units[] = {
	"min", "hr", "d", "mon", "yr",
	"decade", "normal", "century", "??", "??", " x3 hours", " x6 hours",
        " x12 hours",
        "x15 min", "x30 min", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", "??", "??", "??", "??", "??", "??", "??", "??", "??",
        "??", " sec"}; 

void PDStimes(int time_range, int p1, int p2, int time_unit) {

	char *unit;
	enum {anal, fcst, unknown} type;
	int fcst_len = 0;

	if (time_unit >= 0 && time_unit <= sizeof(units)/sizeof(char *))
             unit = units[time_unit];
	else unit = "";

        /* change x3/x6/x12 to hours */

        if (time_unit == HOURS3) {
	    p1 *= 3; p2 *= 3;
	    time_unit = HOUR;
        }
        else if (time_unit == HOURS6) {
	    p1 *= 6; p2 *= 6;
	    time_unit = HOUR;
        }
        else if (time_unit == HOURS12) {
	    p1 *= 12; p2 *= 12;
	    time_unit = HOUR;
        }
	else if (time_unit == MINUTES30) {
	    p1 *= 30; p2 *= 30;
	    time_unit = MINUTE;
        }
	else if (time_unit == MINUTES15) {
	    p1 *= 15; p2 *= 15;
	    time_unit = MINUTE;
        }
	/* turn off 5/13/2010 
	if (time_unit == MINUTE && p1 % 60 == 0 && p2 % 60 == 0) {
	    p1 /= 60; p2 /= 60;
	    time_unit = HOUR;
        }
	*/

	if (time_unit >= 0 && time_unit <= sizeof(units)/sizeof(char *))
             unit = units[time_unit];
	else unit = "";

	/* figure out if analysis or forecast */
	/* in GRIB, there is a difference between init and uninit analyses */
	/* not case at NMC .. no longer run initialization */
	/* ignore diff between init an uninit analyses */

	switch (time_range) {

	case 0:
	case 1:
	case 113:
	case 114:
	case 118:
		if (p1 == 0) type = anal;
		else {
			type = fcst;
			fcst_len = p1;
		}
		break;
	case 10: /* way NMC uses it, should be unknown? */
		type = fcst;
		fcst_len = p1*256 + p2;
		if (fcst_len == 0) type = anal;
		break;

	case 51:
		type = unknown;
		break;
	case 123:
	case 124:
		type = anal;
		break;

	case 135:
		type = anal;
		break;

	default: type = unknown;
		break;
	}

	/* ----------------------------------------------- */

	if (type == anal) printf("anl:");
	else if (type == fcst) printf("%d%s fcst:",fcst_len,unit);


	if (time_range == 123 || time_range == 124) {
		if (p1 != 0) printf("start@%d%s:",p1,unit);
	}


	/* print time range */


	switch (time_range) {

	case 0:
	case 1:
	case 10:
		break;
	case 2: printf("valid %d-%d%s:",p1,p2,unit);
		break;
	case 3: printf("%d-%d%s ave:",p1,p2,unit);
		break;
	case 4: printf("%d-%d%s acc:",p1,p2,unit);
		break;
	case 5: printf("%d-%d%s diff:",p1,p2,unit);
		break;
        case 6: printf("-%d to -%d %s ave:", p1,p2,unit);
                break;
        case 7: printf("-%d to %d %s ave:", p1,p2,unit);
                break;
	case 11: if (p1 > 0) {
		    printf("init fcst %d%s:",p1,unit);
		}
		else {
	            printf("time?:");
		}
		break;
	case 13: printf("nudge ana %d%s:",p1,unit);
		break;
	case 14: printf("rel. fcst %d%s:",p1,unit);
		break;
	case 51: if (p1 == 0) {
		    /* printf("clim %d%s:",p2,unit); */
		    printf("0-%d%s product:ave@1yr:",p2,unit);
		}
		else if (p1 == 1) {
		    /* printf("clim (diurnal) %d%s:",p2,unit); */
		    printf("0-%d%s product:same-hour,ave@1yr:",p2,unit);
		}
		else {
		    printf("clim? p1=%d? %d%s?:",p1,p2,unit);
		}
		break;
	case 113:
	case 123:
		printf("ave@%d%s:",p2,unit);
		break;
	case 114:
	case 124:
		printf("acc@%d%s:",p2,unit);
		break;
	case 115:
		printf("ave of fcst:%d to %d%s:",p1,p2,unit);
		break;
	case 116:
		printf("acc of fcst:%d to %d%s:",p1,p2,unit);
		break;
	case 118: 
		printf("var@%d%s:",p2,unit);
		break;
	case 128:
		printf("%d-%d%s fcst acc:ave@24hr:", p1, p2, unit);
		break;
	case 129:
		printf("%d-%d%s fcst acc:ave@%d%s:", p1, p2, unit, p2-p1,unit);
		break;
	case 130:
		printf("%d-%d%s fcst ave:ave@24hr:", p1, p2, unit);
		break;
	case 131:
		printf("%d-%d%s fcst ave:ave@%d%s:", p1, p2, unit,p2-p1,unit);
		break;
		/* for CFS */
	case 132:
		printf("%d-%d%s anl:ave@1yr:", p1, p2, unit);
		break;
	case 133:
		printf("%d-%d%s fcst:ave@1yr:", p1, p2, unit);
		break;
	case 134:
		printf("%d-%d%s fcst-anl:rms@1yr:", p1, p2, unit);
		break;
	case 135:
		printf("%d-%d%s fcst-fcst_mean:rms@1yr:", p1, p2, unit);
		break;
	case 136:
		printf("%d-%d%s anl-anl_mean:rms@1yr:", p1, p2, unit);
		break;
	case 137:
		printf("%d-%d%s fcst acc:ave@6hr:", p1, p2, unit);
		break;
	case 138:
		printf("%d-%d%s fcst ave:ave@6hr:", p1, p2, unit);
		break;
	case 139:
		printf("%d-%d%s fcst acc:ave@12hr:", p1, p2, unit);
		break;
	case 140:
		printf("%d-%d%s fcst ave:ave@12hr:", p1, p2, unit);
		break;
		
	default: printf("time?:");
	}
}

/*
 *  number of missing data points w. ebisuzaki
 *
 *  v1.1: just faster my dear
 *  v1.2: just faster my dear
 *
 */

static int bitsum[256] = {
    8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1, 
    4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0};


int missing_points(unsigned char *bitmap, int n) {

    int count;
    unsigned int tmp;
    if (bitmap == NULL) return 0;

    count = 0;
    while (n >= 8) {
	tmp = *bitmap++;
	n -= 8;
        count += bitsum[tmp];
    }
    tmp = *bitmap | ((1 << (8 - n)) - 1);
    count += bitsum[tmp];

    return count;
}

/*
 * parameter table for NCEP (operations)
 * center = 7, subcenter != 2 parameter table = 1, 2, 3 etc
 * note: see reanalysis parameter table for problems
 * updated 3/2003
 */

const struct ParmTable parm_table_ncep_opn[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land cover (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"MSLSA", "Mean sea level pressure (Std Atm) [Pa]"},
      /* 129 */ {"MSLMA", "Mean sea level pressure (MAPS) [Pa]"},
      /* 130 */ {"MSLET", "Mean sea level pressure (ETA model) [Pa]"},
      /* 131 */ {"LFTX", "Surface lifted index [K]"},
      /* 132 */ {"4LFTX", "Best (4-layer) lifted index [K]"},
      /* 133 */ {"KX", "K index [K]"},
      /* 134 */ {"SX", "Sweat index [K]"},
      /* 135 */ {"MCONV", "Horizontal moisture divergence [kg/kg/s]"},
      /* 136 */ {"VWSH", "Vertical speed shear [1/s]"},
      /* 137 */ {"TSLSA", "3-hr pressure tendency (Std Atmos Red) [Pa/s]"},
      /* 138 */ {"BVF2", "Brunt-Vaisala frequency^2 [1/s^2]"},
      /* 139 */ {"PVMW", "Potential vorticity (mass-weighted) [1/s/m]"},
      /* 140 */ {"CRAIN", "Categorical rain [yes=1;no=0]"},
      /* 141 */ {"CFRZR", "Categorical freezing rain [yes=1;no=0]"},
      /* 142 */ {"CICEP", "Categorical ice pellets [yes=1;no=0]"},
      /* 143 */ {"CSNOW", "Categorical snow [yes=1;no=0]"},
      /* 144 */ {"SOILW", "Volumetric soil moisture [fraction]"},
      /* 145 */ {"PEVPR", "Potential evaporation rate [W/m^2]"},
      /* 146 */ {"CWORK", "Cloud work function [J/kg]"},
      /* 147 */ {"U-GWD", "Zonal gravity wave stress [N/m^2]"},
      /* 148 */ {"V-GWD", "Meridional gravity wave stress [N/m^2]"},
      /* 149 */ {"PV", "Potential vorticity [m^2/s/kg]"},
      /* 150 */ {"COVMZ", "Covariance between u and v [m^2/s^2]"},
      /* 151 */ {"COVTZ", "Covariance between u and T [K*m/s]"},
      /* 152 */ {"COVTM", "Covariance between v and T [K*m/s]"},
      /* 153 */ {"CLWMR", "Cloud water [kg/kg]"},
      /* 154 */ {"O3MR", "Ozone mixing ratio [kg/kg]"},
      /* 155 */ {"GFLUX", "Ground heat flux [W/m^2]"},
      /* 156 */ {"CIN", "Convective inhibition [J/kg]"},
      /* 157 */ {"CAPE", "Convective Avail. Pot. Energy [J/kg]"},
      /* 158 */ {"TKE", "Turbulent kinetic energy [J/kg]"},
      /* 159 */ {"CONDP", "Lifted parcel condensation pressure [Pa]"},
      /* 160 */ {"CSUSF", "Clear sky upward solar flux [W/m^2]"},
      /* 161 */ {"CSDSF", "Clear sky downward solar flux [W/m^2]"},
      /* 162 */ {"CSULF", "Clear sky upward long wave flux [W/m^2]"},
      /* 163 */ {"CSDLF", "Clear sky downward long wave flux [W/m^2]"},
      /* 164 */ {"CFNSF", "Cloud forcing net solar flux [W/m^2]"},
      /* 165 */ {"CFNLF", "Cloud forcing net long wave flux [W/m^2]"},
      /* 166 */ {"VBDSF", "Visible beam downward solar flux [W/m^2]"},
      /* 167 */ {"VDDSF", "Visible diffuse downward solar flux [W/m^2]"},
      /* 168 */ {"NBDSF", "Near IR beam downward solar flux [W/m^2]"},
      /* 169 */ {"NDDSF", "Near IR diffuse downward solar flux [W/m^2]"},
      /* 170 */ {"RWMR", "Rain water mixing ratio [kg/kg]"},
      /* 171 */ {"SNMR", "Snow mixing ratio [kg/kg]"},
      /* 172 */ {"MFLX", "Momentum flux [N/m^2]"},
      /* 173 */ {"LMH", "Mass point model surface [non-dim]"},
      /* 174 */ {"LMV", "Velocity point model surface [non-dim]"},
      /* 175 */ {"MLYNO", "Model layer number (from bottom up) [non-dim]"},
      /* 176 */ {"NLAT", "Latitude (-90 to +90) [deg]"},
      /* 177 */ {"ELON", "East longitude (0-360) [deg]"},
      /* 178 */ {"ICMR", "Ice mixing ratio [kg/kg]"},
      /* 179 */ {"GRMR", "Graupel mixing ratio [kg/kg]"},
      /* 180 */ {"GUST", "Surface wind gust [m/s]"},
      /* 181 */ {"LPSX", "x-gradient of log pressure [1/m]"},
      /* 182 */ {"LPSY", "y-gradient of log pressure [1/m]"},
      /* 183 */ {"HGTX", "x-gradient of height [m/m]"},
      /* 184 */ {"HGTY", "y-gradient of height [m/m]"},
      /* 185 */ {"TURB", "Turbulence SIGMET/AIRMET [non-dim]"},
      /* 186 */ {"ICNG", "Icing SIGMET/AIRMET [non-dim]"},
      /* 187 */ {"LTNG", "Lightning [non-dim]"},
      /* 188 */ {"DRIP", "Rate of water dropping from canopy to gnd [kg/m^2]"},
      /* 189 */ {"VPTMP", "Virtual pot. temp. [K]"},
      /* 190 */ {"HLCY", "Storm relative helicity [m^2/s^2]"},
      /* 191 */ {"PROB", "Prob. from ensemble [non-dim]"},
      /* 192 */ {"PROBN", "Prob. from ensemble norm. to clim. expect. [non-dim]"},
      /* 193 */ {"POP", "Prob. of precipitation [%]"},
      /* 194 */ {"CPOFP", "Prob. of frozen precipitation [%]"},
      /* 195 */ {"CPOZP", "Prob. of freezing precipitation [%]"},
      /* 196 */ {"USTM", "u-component of storm motion [m/s]"},
      /* 197 */ {"VSTM", "v-component of storm motion [m/s]"},
      /* 198 */ {"NCIP", "No. concen. ice particles []"},
      /* 199 */ {"EVBS", "Direct evaporation from bare soil [W/m^2]"},
      /* 200 */ {"EVCW", "Canopy water evaporation [W/m^2]"},
      /* 201 */ {"ICWAT", "Ice-free water surface [%]"},
      /* 202 */ {"CWDI", "Convective weather detection index []"},
      /* 203 */ {"VAFTAD", "VAFTAD?? [??]"},
      /* 204 */ {"DSWRF", "Downward short wave flux [W/m^2]"},
      /* 205 */ {"DLWRF", "Downward long wave flux [W/m^2]"},
      /* 206 */ {"UVI", "Ultraviolet index [W/m^2]"},
      /* 207 */ {"MSTAV", "Moisture availability [%]"},
      /* 208 */ {"SFEXC", "Exchange coefficient [(kg/m^3)(m/s)]"},
      /* 209 */ {"MIXLY", "No. of mixed layers next to surface [integer]"},
      /* 210 */ {"TRANS", "Transpiration [W/m^2]"},
      /* 211 */ {"USWRF", "Upward short wave flux [W/m^2]"},
      /* 212 */ {"ULWRF", "Upward long wave flux [W/m^2]"},
      /* 213 */ {"CDLYR", "Non-convective cloud [%]"},
      /* 214 */ {"CPRAT", "Convective precip. rate [kg/m^2/s]"},
      /* 215 */ {"TTDIA", "Temp. tendency by all physics [K/s]"},
      /* 216 */ {"TTRAD", "Temp. tendency by all radiation [K/s]"},
      /* 217 */ {"TTPHY", "Temp. tendency by non-radiation physics [K/s]"},
      /* 218 */ {"PREIX", "Precip index (0.0-1.00) [fraction]"},
      /* 219 */ {"TSD1D", "Std. dev. of IR T over 1x1 deg area [K]"},
      /* 220 */ {"NLGSP", "Natural log of surface pressure [ln(kPa)]"},
      /* 221 */ {"HPBL", "Planetary boundary layer height [m]"},
      /* 222 */ {"5WAVH", "5-wave geopotential height [gpm]"},
      /* 223 */ {"CNWAT", "Plant canopy surface water [kg/m^2]"},
      /* 224 */ {"SOTYP", "Soil type (Zobler) [0..9]"},
      /* 225 */ {"VGTYP", "Vegetation type (as in SiB) [0..13]"},
      /* 226 */ {"BMIXL", "Blackadar's mixing length scale [m]"},
      /* 227 */ {"AMIXL", "Asymptotic mixing length scale [m]"},
      /* 228 */ {"PEVAP", "Pot. evaporation [kg/m^2]"},
      /* 229 */ {"SNOHF", "Snow phase-change heat flux [W/m^2]"},
      /* 230 */ {"5WAVA", "5-wave geopot. height anomaly [gpm]"},
      /* 231 */ {"MFLUX", "Convective cloud mass flux [Pa/s]"},
      /* 232 */ {"DTRF", "Downward total radiation flux [W/m^2]"},
      /* 233 */ {"UTRF", "Upward total radiation flux [W/m^2]"},
      /* 234 */ {"BGRUN", "Baseflow-groundwater runoff [kg/m^2]"},
      /* 235 */ {"SSRUN", "Storm surface runoff [kg/m^2]"},
      /* 236 */ {"SIPD", "Supercooled large droplet (SLD) icing pot. diagn. []"},
      /* 237 */ {"O3TOT", "Total ozone [kg/m^2]"},
      /* 238 */ {"SNOWC", "Snow cover [%]"},
      /* 239 */ {"SNOT", "Snow temp. [K]"},
      /* 240 */ {"COVTW", "Covariance T and w [K*m/s]"},
      /* 241 */ {"LRGHR", "Large scale condensation heating [K/s]"},
      /* 242 */ {"CNVHR", "Deep convective heating [K/s]"},
      /* 243 */ {"CNVMR", "Deep convective moistening [kg/kg/s]"},
      /* 244 */ {"SHAHR", "Shallow convective heating [K/s]"},
      /* 245 */ {"SHAMR", "Shallow convective moistening [kg/kg/s]"},
      /* 246 */ {"VDFHR", "Vertical diffusion heating [K/s]"},
      /* 247 */ {"VDFUA", "Vertical diffusion zonal accel [m/s^2]"},
      /* 248 */ {"VDFVA", "Vertical diffusion meridional accel [m/s^2]"},
      /* 249 */ {"VDFMR", "Vertical diffusion moistening [kg/kg/s]"},
      /* 250 */ {"SWHR", "Solar radiative heating [K/s]"},
      /* 251 */ {"LWHR", "Longwave radiative heating [K/s]"},
      /* 252 */ {"CD", "Drag coefficient [non-dim]"},
      /* 253 */ {"FRICV", "Friction velocity [m/s]"},
      /* 254 */ {"RI", "Richardson number [non-dim]"},
      /* 255 */ {"var255", "undefined"},
};

/*
 * parameter table for the NCEP/NCAR Reanalysis Project
 * center = 7, subcenter = 0/2, parameter table = 1/2
 * in a SNAFU the operational and reanalysis tables diverged
 * and both retained the same parameter table numbers (1,2)
 *
 * some of the Reanalysis files have subcenter=2 while others
 * use subcenter=0  (subcenter field is not standard (7/97))
 *
 * Some ways to tell Reanalysis files from OPN files
 *  Reanalysis: always generated by process 80 - T62 28 level model
 * Original subcenter=0 Reanalysis files had 
 *  2.5x2.5 (144x73) lat-long grid or 192x94 Gaussian grid (PDS grid=255?)
 */

const struct ParmTable parm_table_ncep_reanal[256] = {
   /* 0 */ {"var0", "undefined"},
   /* 1 */ {"PRES", "Pressure [Pa]"},
   /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
   /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
   /* 4 */ {"var4", "undefined"},
   /* 5 */ {"var5", "undefined"},
   /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
   /* 7 */ {"HGT", "Geopotential height [gpm]"},
   /* 8 */ {"DIST", "Geometric height [m]"},
   /* 9 */ {"HSTDV", "Std dev of height [m]"},
   /* 10 */ {"HVAR", "Variance of height [m^2]"},
   /* 11 */ {"TMP", "Temp. [K]"},
   /* 12 */ {"VTMP", "Virtual temp. [K]"},
   /* 13 */ {"POT", "Potential temp. [K]"},
   /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
   /* 15 */ {"TMAX", "Max. temp. [K]"},
   /* 16 */ {"TMIN", "Min. temp. [K]"},
   /* 17 */ {"DPT", "Dew point temp. [K]"},
   /* 18 */ {"DEPR", "Dew point depression [K]"},
   /* 19 */ {"LAPR", "Lapse rate [K/m]"},
   /* 20 */ {"VISIB", "Visibility [m]"},
   /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
   /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
   /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
   /* 24 */ {"var24", "undefined"},
   /* 25 */ {"TMPA", "Temp. anomaly [K]"},
   /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
   /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
   /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
   /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
   /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
   /* 31 */ {"WDIR", "Wind direction [deg]"},
   /* 32 */ {"WIND", "Wind speed [m/s]"},
   /* 33 */ {"UGRD", "u wind [m/s]"},
   /* 34 */ {"VGRD", "v wind [m/s]"},
   /* 35 */ {"STRM", "Stream function [m^2/s]"},
   /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
   /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
   /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
   /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
   /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
   /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
   /* 42 */ {"ABSD", "Absolute divergence [/s]"},
   /* 43 */ {"RELV", "Relative vorticity [/s]"},
   /* 44 */ {"RELD", "Relative divergence [/s]"},
   /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
   /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
   /* 47 */ {"DIRC", "Direction of current [deg]"},
   /* 48 */ {"SPC", "Speed of current [m/s]"},
   /* 49 */ {"UOGRD", "u of current [m/s]"},
   /* 50 */ {"VOGRD", "v of current [m/s]"},
   /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
   /* 52 */ {"RH", "Relative humidity [%]"},
   /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
   /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
   /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
   /* 56 */ {"SATD", "Saturation deficit [Pa]"},
   /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
   /* 58 */ {"CICE", "Cloud Ice [kg/kg]"},
   /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
   /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
   /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
   /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
   /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
   /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
   /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
   /* 66 */ {"SNOD", "Snow depth [m]"},
   /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
   /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
   /* 69 */ {"MTHD", "Main thermocline depth [m]"},
   /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
   /* 71 */ {"TCDC", "Total cloud cover [%]"},
   /* 72 */ {"CDCON", "Convective cloud cover [%]"},
   /* 73 */ {"LCDC", "Low level cloud cover [%]"},
   /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
   /* 75 */ {"HCDC", "High level cloud cover [%]"},
   /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
   /* 77 */ {"var77", "undefined"},
   /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
   /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
   /* 80 */ {"WTMP", "Water temp. [K]"},
   /* 81 */ {"LAND", "Land-sea mask [1=land; 0=sea]"},
   /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
   /* 83 */ {"SFCR", "Surface roughness [m]"},
   /* 84 */ {"ALBDO", "Albedo [%]"},
   /* 85 */ {"TSOIL", "Soil temp. [K]"},
   /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
   /* 87 */ {"VEG", "Vegetation [%]"},
   /* 88 */ {"SALTY", "Salinity [kg/kg]"},
   /* 89 */ {"DEN", "Density [kg/m^3]"},
   /* 90 */ {"RUNOF", "Runoff [kg/m^2]"},
   /* 91 */ {"ICEC", "Ice concentration [ice=1;no ice=0]"},
   /* 92 */ {"ICETK", "Ice thickness [m]"},
   /* 93 */ {"DICED", "Direction of ice drift [deg]"},
   /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
   /* 95 */ {"UICE", "u of ice drift [m/s]"},
   /* 96 */ {"VICE", "v of ice drift [m/s]"},
   /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
   /* 98 */ {"ICED", "Ice divergence [/s]"},
   /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
   /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
   /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
   /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
   /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
   /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
   /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
   /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
   /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
   /* 108 */ {"PERPW", "Primary wave mean period [s]"},
   /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
   /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
   /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
   /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
   /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
   /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
   /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
   /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
   /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
   /* 118 */ {"var118", "undefined"},
   /* 119 */ {"var119", "undefined"},
   /* 120 */ {"var120", "undefined"},
   /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
   /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
   /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
   /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
   /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
   /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
   /* 127 */ {"IMGD", "Image data [integer]"},
   /* 128 */ {"MSLSA", "Mean sea level pressure (Std Atm) [Pa]"},
   /* 129 */ {"MSLMA", "Mean sea level pressure (MAPS) [Pa]"},
   /* 130 */ {"MSLET", "Mean sea level pressure (ETA model) [Pa]"},
   /* 131 */ {"LFTX", "Surface lifted index [K]"},
   /* 132 */ {"4LFTX", "Best (4-layer) lifted index [K]"},
   /* 133 */ {"KX", "K index [K]"},
   /* 134 */ {"SX", "Sweat index [K]"},
   /* 135 */ {"MCONV", "Horizontal moisture divergence [kg/kg/s]"},
   /* 136 */ {"VSSH", "Vertical speed shear [1/s]"},
   /* 137 */ {"TSLSA", "3-hr pressure tendency [Pa/s]"},
   /* 138 */ {"BVF2", "Brunt-Vaisala frequency^2 [1/s^2]"},
   /* 139 */ {"PVMW", "Potential vorticity (mass-weighted) [1/s/m]"},
   /* 140 */ {"CRAIN", "Categorical rain [yes=1;no=0]"},
   /* 141 */ {"CFRZR", "Categorical freezing rain [yes=1;no=0]"},
   /* 142 */ {"CICEP", "Categorical ice pellets [yes=1;no=0]"},
   /* 143 */ {"CSNOW", "Categorical snow [yes=1;no=0]"},
   /* 144 */ {"SOILW", "Volumetric soil moisture [fraction]"},
   /* 145 */ {"PEVPR", "Potential evaporation rate [W/m^2]"},
   /* 146 */ {"CWORK", "Cloud work function [J/kg]"},
   /* 147 */ {"U-GWD", "Zonal gravity wave stress [N/m^2]"},
   /* 148 */ {"V-GWD", "Meridional gravity wave stress [N/m^2]"},
   /* 149 */ {"PV___", "Potential vorticity [m^2/s/kg]"},
   /* 150 */ {"var150", "undefined"},
   /* 151 */ {"var151", "undefined"},
   /* 152 */ {"var152", "undefined"},
   /* 153 */ {"MFXDV", "Moisture flux divergence [gr/gr*m/s/m]"},
   /* 154 */ {"var154", "undefined"},
   /* 155 */ {"GFLUX", "Ground heat flux [W/m^2]"},
   /* 156 */ {"CIN", "Convective inhibition [J/kg]"},
   /* 157 */ {"CAPE", "Convective Avail. Pot. Energy [J/kg]"},
   /* 158 */ {"TKE", "Turbulent kinetic energy [J/kg]"},
   /* 159 */ {"CONDP", "Lifted parcel condensation pressure [Pa]"},
   /* 160 */ {"CSUSF", "Clear sky upward solar flux [W/m^2]"},
   /* 161 */ {"CSDSF", "Clear sky downward solar flux [W/m^2]"},
   /* 162 */ {"CSULF", "Clear sky upward long wave flux [W/m^2]"},
   /* 163 */ {"CSDLF", "Clear sky downward long wave flux [W/m^2]"},
   /* 164 */ {"CFNSF", "Cloud forcing net solar flux [W/m^2]"},
   /* 165 */ {"CFNLF", "Cloud forcing net long wave flux [W/m^2]"},
   /* 166 */ {"VBDSF", "Visible beam downward solar flux [W/m^2]"},
   /* 167 */ {"VDDSF", "Visible diffuse downward solar flux [W/m^2]"},
   /* 168 */ {"NBDSF", "Near IR beam downward solar flux [W/m^2]"},
   /* 169 */ {"NDDSF", "Near IR diffuse downward solar flux [W/m^2]"},
   /* 170 */ {"USTR", "U wind stress [N/m^2]"},
   /* 171 */ {"VSTR", "V wind stress [N/m^2]"},
   /* 172 */ {"MFLX", "Momentum flux [N/m^2]"},
   /* 173 */ {"LMH", "Mass point model surface [integer]"},
   /* 174 */ {"LMV", "Velocity point model surface [integer]"},
   /* 175 */ {"SGLYR", "Nearby model level [integer]"},
   /* 176 */ {"NLAT", "Latitude [deg]"},
   /* 177 */ {"ELON", "Longitude [deg]"},
   /* 178 */ {"UMAS", "Mass weighted u [gm/m*K*s]"},
   /* 179 */ {"VMAS", "Mass weighted v [gm/m*K*s]"},
   /* 180 */ {"XPRATE", "corrected precip [kg/m^2/s]"},
   /* 181 */ {"LPSX", "x-gradient of log pressure [1/m]"},
   /* 182 */ {"LPSY", "y-gradient of log pressure [1/m]"},
   /* 183 */ {"HGTX", "x-gradient of height [m/m]"},
   /* 184 */ {"HGTY", "y-gradient of height [m/m]"},
   /* 185 */ {"STDZ", "Std dev of Geop. hgt. [m]"},
   /* 186 */ {"STDU", "Std dev of zonal wind [m/s]"},
   /* 187 */ {"STDV", "Std dev of meridional wind [m/s]"},
   /* 188 */ {"STDQ", "Std dev of spec. hum. [gm/gm]"},
   /* 189 */ {"STDT", "Std dev of temp. [K]"},
   /* 190 */ {"CBUW", "Covar. u and omega [m/s*Pa/s]"},
   /* 191 */ {"CBVW", "Covar. v and omega [m/s*Pa/s]"},
   /* 192 */ {"CBUQ", "Covar. u and specific hum [m/s*gm/gm]"},
   /* 193 */ {"CBVQ", "Covar. v and specific hum [m/s*gm/gm]"},
   /* 194 */ {"CBTW", "Covar. T and omega [K*Pa/s]"},
   /* 195 */ {"CBQW", "Covar. spec. hum and omega [gm/gm*Pa/s]"},
   /* 196 */ {"CBMZW", "Covar. v and u [m^2/s^2]"},
   /* 197 */ {"CBTZW", "Covar. u and T [K*m/s]"},
   /* 198 */ {"CBTMW", "Covar. v and T [K*m/s]"},
   /* 199 */ {"STDRH", "Std dev of Rel. Hum. [%]"},
   /* 200 */ {"SDTZ", "Std dev of time tend of geop. hgt [m]"},
   /* 201 */ {"ICWAT", "Ice-free water surface [%]"},
   /* 202 */ {"SDTU", "Std dev of time tend of zonal wind [m/s]"},
   /* 203 */ {"SDTV", "Std dev of time tend of merid wind [m/s]"},
   /* 204 */ {"DSWRF", "Downward solar radiation flux [W/m^2]"},
   /* 205 */ {"DLWRF", "Downward long wave flux [W/m^2]"},
   /* 206 */ {"SDTQ", "Std dev of time tend of spec. hum [gm/gm]"},
   /* 207 */ {"MSTAV", "Moisture availability [%]"},
   /* 208 */ {"SFEXC", "Exchange coefficient [kg*m/m^3/s]"},
   /* 209 */ {"MIXLY", "No. of mixed layers next to sfc [integer]"},
   /* 210 */ {"SDTT", "Std dev of time tend of temp. [K]"},
   /* 211 */ {"USWRF", "Upward solar radiation flux [W/m^2]"},
   /* 212 */ {"ULWRF", "Upward long wave flux [W/m^2]"},
   /* 213 */ {"CDLYR", "Non-convective cloud [%]"},
   /* 214 */ {"CPRAT", "Convective precip. rate [kg/m^2/s]"},
   /* 215 */ {"TTDIA", "Temp. tendency by all physics [K/s]"},
   /* 216 */ {"TTRAD", "Temp. tendency by all radiation [K/s]"},
   /* 217 */ {"TTPHY", "Temp. tendency by nonrad physics [K/s]"},
   /* 218 */ {"PREIX", "Precipitation index [fraction]"},
   /* 219 */ {"TSD1D", "Std dev of IR T over 1x1 deg area [K]"},
   /* 220 */ {"NLSGP", "Natural log of surface pressure [ln(kPa)]"},
   /* 221 */ {"SDTRH", "Std dev of time tend of rel hum [%]"},
   /* 222 */ {"5WAVH", "5-wave geopotential height [gpm]"},
   /* 223 */ {"CNWAT", "Plant canopy surface water [kg/m^2]"},
   /* 224 */ {"PLTRS", "Max. stomato plant resistance [s/m]"},
   /* 225 */ {"RHCLD", "RH-type cloud cover [%]"},
   /* 226 */ {"BMIXL", "Blackadar's mixing length scale [m]"},
   /* 227 */ {"AMIXL", "Asymptotic mixing length scale [m]"},
   /* 228 */ {"PEVAP", "Pot. evaporation [kg/m^2]"},
   /* 229 */ {"SNOHF", "Snow melt heat flux [W/m^2]"},
   /* 230 */ {"SNOEV", "Snow sublimation heat flux [W/m^2]"},
   /* 231 */ {"MFLUX", "Convective cloud mass flux [Pa/s]"},
   /* 232 */ {"DTRF", "Downward total radiation flux [W/m^2]"},
   /* 233 */ {"UTRF", "Upward total radiation flux [W/m^2]"},
   /* 234 */ {"BGRUN", "Baseflow-groundwater runoff [kg/m^2]"},
   /* 235 */ {"SSRUN", "Storm surface runoff [kg/m^2]"},
   /* 236 */ {"var236", "undefined"},
   /* 237 */ {"OZONE", "Total column ozone [Dobson]"},
   /* 238 */ {"SNOWC", "Snow cover [%]"},
   /* 239 */ {"SNOT", "Snow temp. [K]"},
   /* 240 */ {"GLCR", "Permanent snow points [mask]"},
   /* 241 */ {"LRGHR", "Large scale condensation heating [K/s]"},
   /* 242 */ {"CNVHR", "Deep convective heating [K/s]"},
   /* 243 */ {"CNVMR", "Deep convective moistening [kg/kg/s]"},
   /* 244 */ {"SHAHR", "Shallow convective heating [K/s]"},
   /* 245 */ {"SHAMR", "Shallow convective moistening [kg/kg/s]"},
   /* 246 */ {"VDFHR", "Vertical diffusion heating [K/s]"},
   /* 247 */ {"VDFUA", "Vertical diffusion zonal accel [m/s^2]"},
   /* 248 */ {"VDFVA", "Vertical diffusion meridional accel [m/s^2]"},
   /* 249 */ {"VDFMR", "Vertical diffusion moistening [kg/kg/s]"},
   /* 250 */ {"SWHR", "Solar radiative heating [K/s]"},
   /* 251 */ {"LWHR", "Longwave radiative heating [K/s]"},
   /* 252 */ {"CD", "Drag coefficient [non-dim]"},
   /* 253 */ {"FRICV", "Friction velocity [m/s]"},
   /* 254 */ {"RI", "Richardson number [non-dim]"},
   /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_nceptab_131[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land-sea coverage (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"MSLSA", "Mean sea level pressure (Std Atm) [Pa]"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"MSLET", "Mean sea level pressure (ETA model) [Pa]"},
      /* 131 */ {"LFTX", "Surface lifted index [K]"},
      /* 132 */ {"4LFTX", "Best (4-layer) lifted index [K]"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"PRESN", "Pressure (nearest grid point) [Pa]"},
      /* 135 */ {"MCONV", "Horizontal moisture divergence [kg/kg/s]"},
      /* 136 */ {"VWSH", "Vertical speed shear [1/s]"},
      /* 137 */ {"LTNGSD", "Lightning strike density [1/m^2/s]"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"PVMW", "Potential vorticity (mass-weighted) [1/s/m]"},
      /* 140 */ {"CRAIN", "Categorical rain [yes=1;no=0]"},
      /* 141 */ {"CFRZR", "Categorical freezing rain [yes=1;no=0]"},
      /* 142 */ {"CICEP", "Categorical ice pellets [yes=1;no=0]"},
      /* 143 */ {"CSNOW", "Categorical snow [yes=1;no=0]"},
      /* 144 */ {"SOILW", "Volumetric soil moisture (frozen + liquid) [fraction]"},
      /* 145 */ {"PEVPR", "Potential evaporation rate [W/m^2]"},
      /* 146 */ {"VEGT", "Vegetation canopy temperature [K]"},
      /* 147 */ {"BARET", "Bare soil surface skin temperature [K]"},
      /* 148 */ {"AVSFT", "Average surface skin temperature [K]"},
      /* 149 */ {"RADT", "Effective radiative skin temperature [K]"},
      /* 150 */ {"SSTOR", "Surface water storage [kg/m^2]"},
      /* 151 */ {"LSOIL", "Liquid soil moisture content (non-frozen) [kg/m^2]"},
      /* 152 */ {"EWATR", "Open water evaporation (standing water) [W/m^2]"},
      /* 153 */ {"CLWMR", "Cloud water [kg/kg]"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"GFLUX", "Ground Heat Flux [W/m^2]"},
      /* 156 */ {"CIN", "Convective inhibition [J/kg]"},
      /* 157 */ {"CAPE", "Convective available potential energy [J/kg]"},
      /* 158 */ {"TKE", "Turbulent Kinetic Energy [J/kg]"},
      /* 159 */ {"MXSALB", "Maximum snow albedo [%]"},
      /* 160 */ {"SOILL", "Liquid volumetric soil moisture (non-frozen) [fraction]"},
      /* 161 */ {"ASNOW", "Frozen precipitation (e.g. snowfall) [kg/m^2]"},
      /* 162 */ {"ARAIN", "Liquid precipitation (rainfall) [kg/m^2]"},
      /* 163 */ {"GWREC", "Groundwater recharge [kg/m^2]"},
      /* 164 */ {"QREC", "Flood plain recharge [kg/m^2]"},
      /* 165 */ {"SNOWT", "Snow temperature, depth-avg [K]"},
      /* 166 */ {"VBDSF", "Visible beam downward solar flux [W/m^2]"},
      /* 167 */ {"VDDSF", "Visible diffuse downward solar flux [W/m^2]"},
      /* 168 */ {"NBDSF", "Near IR beam downward solar flux [W/m^2]"},
      /* 169 */ {"NDDSF", "Near IR diffuse downward solar flux [W/m^2]"},
      /* 170 */ {"SNFALB", "Snow-free albedo [%]"},
      /* 171 */ {"RLYRS", "Number of soil layers in root zone [non-dim]"},
      /* 172 */ {"FLX", "Momentum flux N/m2 [M]"},
      /* 173 */ {"LMH", "Mass point model surface [non-dim]"},
      /* 174 */ {"LMV", "Velocity point model surface [non-dim]"},
      /* 175 */ {"MLYNO", "Model layer number (from bottom up) [non-dim]"},
      /* 176 */ {"NLAT", "Latitude (-90 to +90) [deg]"},
      /* 177 */ {"ELON", "East longitude (0-360) [deg]"},
      /* 178 */ {"ICMR", "Ice mixing ratio [kg/kg]"},
      /* 179 */ {"ACOND", "Aerodynamic conductance [m/s]"},
      /* 180 */ {"SNOAG", "Snow age [s]"},
      /* 181 */ {"CCOND", "Canopy conductance [m/s]"},
      /* 182 */ {"LAI", "Leaf area index (0-9) [non-dim]"},
      /* 183 */ {"SFCRH", "Roughness length for heat [m]"},
      /* 184 */ {"SALBD", "Snow albedo (over snow cover area only) [%]"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"NDVI", "Normalized Difference Vegetation Index []"},
      /* 188 */ {"DRIP", "Rate of water dropping from canopy to gnd [kg/m^2]"},
      /* 189 */ {"LANDN", "Land-sea coverage (nearest neighbor) [land=1,sea=0]"},
      /* 190 */ {"HLCY", "Storm relative helicity [m^2/s^2]"},
      /* 191 */ {"NLATN", "Latitude (nearest neigbhbor) (-90 to +90) [deg]"},
      /* 192 */ {"ELONN", "East longitude (nearest neigbhbor) (0-360) [deg]"},
      /* 193 */ {"APTMP", "Apparent temperature [K]"},
      /* 194 */ {"CPOFP", "Prob. of frozen precipitation [%]"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"USTM", "u-component of storm motion [m/s]"},
      /* 197 */ {"VSTM", "v-component of storm motion [m/s]"},
      /* 198 */ {"SBSNO", "Sublimation (evaporation from snow) [W/m^2]"},
      /* 199 */ {"EVBS", "Direct evaporation from bare soil [W/m^2]"},
      /* 200 */ {"EVCW", "Canopy water evaporation [W/m^2]"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"APCPN", "Total precipitation (nearest grid point) [kg/m^2]"},
      /* 203 */ {"RSMIN", "Minimal stomatal resistance [s/m]"},
      /* 204 */ {"DSWRF", "Downward shortwave radiation flux [W/m^2]"},
      /* 205 */ {"DLWRF", "Downward longwave radiation flux [W/m^2]"},
      /* 206 */ {"ACPCPN", "Convective precipitation (nearest grid point) [kg/m^2]"},
      /* 207 */ {"MSTAV", "Moisture availability [%]"},
      /* 208 */ {"SFEXC", "Exchange coefficient [(kg/m^3)(m/s)]"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"TRANS", "Transpiration [W/m^2]"},
      /* 211 */ {"USWRF", "Upward short wave radiation flux [W/m^2]"},
      /* 212 */ {"ULWRF", "Upward long wave radiation flux [W/m^2]"},
      /* 213 */ {"CDLYR", "Non-convective cloud [%]"},
      /* 214 */ {"CPRAT", "Convective precip. rate [kg/m^2/s]"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"TTRAD", "Temp. tendency by all radiation [K/s]"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"HGTN", "Geopotential Height (nearest grid point) [gpm]"},
      /* 219 */ {"WILT", "Wilting point [fraction]"},
      /* 220 */ {"FLDCP", "Field Capacity [fraction]"},
      /* 221 */ {"HPBL", "Planetary boundary layer height [m]"},
      /* 222 */ {"SLTYP", "Surface slope type [Index]"},
      /* 223 */ {"CNWAT", "Plant canopy surface water [kg/m^2]"},
      /* 224 */ {"SOTYP", "Soil type [Index]"},
      /* 225 */ {"VGTYP", "Vegetation type [Index]"},
      /* 226 */ {"BMIXL", "Blackadars mixing length scale [m]"},
      /* 227 */ {"AMIXL", "Asymptotic mixing length scale [m]"},
      /* 228 */ {"PEVAP", "Potential evaporation [kg/m^2]"},
      /* 229 */ {"SNOHF", "Snow phase-change heat flux [W/m^2]"},
      /* 230 */ {"SMREF", "Transpiration stress-onset (soil moisture) [fraction]"},
      /* 231 */ {"SMDRY", "Direct evaporation cease (soil moisture) [fraction]"},
      /* 232 */ {"WVINC", "water vapor added by precip assimilation [kg/m^2]"},
      /* 233 */ {"WCINC", "water condensate added by precip assimilaition [kg/m^2]"},
      /* 234 */ {"BGRUN", "Subsurface runoff (baseflow) [kg/m^2]"},
      /* 235 */ {"SSRUN", "Surface runoff (non-infiltrating) [kg/m^2]"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"WVCONV", "Water vapor flux convergence (vertical int) [kg/m^2]"},
      /* 238 */ {"SNOWC", "Snow cover [%]"},
      /* 239 */ {"SNOT", "Snow temperature [K]"},
      /* 240 */ {"POROS", "Soil porosity [fraction]"},
      /* 241 */ {"WCCONV", "Water condensate flux convergence (vertical int) [kg/m^2]"},
      /* 242 */ {"WVUFLX", "Water vapor zonal flux (vertical int)[kg/m]"},
      /* 243 */ {"WVVFLX", "Water vapor meridional flux (vertical int) [kg/m]"},
      /* 244 */ {"WCUFLX", "Water condensate zonal flux (vertical int) [kg/m]"},
      /* 245 */ {"WCVFLX", "Water condensate meridional flux (vertical int) [kg/m]"},
      /* 246 */ {"RCS", "Solar parameter in canopy conductance [fraction]"},
      /* 247 */ {"RCT", "Temperature parameter in canopy conductance [fraction]"},
      /* 248 */ {"RCQ", "Humidity parameter in canopy conductance [fraction]"},
      /* 249 */ {"RCSOL", "Soil moisture parameter in canopy conductance [fraction]"},
      /* 250 */ {"SWHR", "Solar radiative heating [K/s]"},
      /* 251 */ {"LWHR", "Longwave radiative heating [K/s]"},
      /* 252 */ {"CD", "Surface drag coefficient [non-dim]"},
      /* 253 */ {"FRICV", "Surface friction velocity [m/s]"},
      /* 254 */ {"RI", "Richardson number [non-dim]"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_nceptab_130[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land cover (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"SOILW", "Volumetric soil moisture (frozen + liquid) [fraction]"},
      /* 145 */ {"PEVPR", "Potential evaporation rate [W/m^2]"},
      /* 146 */ {"VEGT", "Vegetation canopy temperature [K]"},
      /* 147 */ {"BARET", "Bare soil surface skin temperature [K]"},
      /* 148 */ {"AVSFT", "Average surface skin temperature [K]"},
      /* 149 */ {"RADT", "Effective radiative skin temperature [K]"},
      /* 150 */ {"SSTOR", "Surface water storage [Kg/m^2]"},
      /* 151 */ {"LSOIL", "Liquid soil moisture content (non-frozen) [Kg/m^2]"},
      /* 152 */ {"EWATR", "Open water evaporation (standing water) [W/m^2]"},
      /* 153 */ {"NCRAIN", "Number concentration for rain particles [-]"},
      /* 154 */ {"LSPA", "Land Surface Precipitation Accumulation [kg/m^2]"},
      /* 155 */ {"GFLUX", "Ground Heat Flux [W/m^2]"},
      /* 156 */ {"CIN", "Convective inhibition [J/Kg]"},
      /* 157 */ {"CAPE", "Convective available potential energy [J/Kg]"},
      /* 158 */ {"TKE", "Turbulent Kinetic Energy [J/Kg]"},
      /* 159 */ {"MXSALB", "Maximum snow albedo [%]"},
      /* 160 */ {"SOILL", "Liquid volumetric soil moisture (non-frozen) [fraction]"},
      /* 161 */ {"ASNOW", "Frozen precipitation (e.g. snowfall) [Kg/m^2]"},
      /* 162 */ {"ARAIN", "Liquid precipitation (rainfall) [Kg/m^2]"},
      /* 163 */ {"GWREC", "Groundwater recharge [Kg/m^2]"},
      /* 164 */ {"QREC", "Flood plain recharge [Kg/m^2]"},
      /* 165 */ {"SNOWT", "Snow temperature, depth-avg [K]"},
      /* 166 */ {"VBDSF", "Visible beam downward solar flux [W/m^2]"},
      /* 167 */ {"VDDSF", "Visible diffuse downward solar flux [W/m^2]"},
      /* 168 */ {"NBDSF", "Near IR beam downward solar flux [W/m^2]"},
      /* 169 */ {"NDDSF", "Near IR diffuse downward solar flux [W/m^2]"},
      /* 170 */ {"SNFALB", "Snow-free albedo [%]"},
      /* 171 */ {"RLYRS", "Number of soil layers in root zone [non-dim]"},
      /* 172 */ {"MFLX", "Momentum flux [N/m^2]"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"NLAT", "Latitude (-90 to +90) [deg]"},
      /* 177 */ {"ELON", "East longitude (0-360) [deg]"},
      /* 178 */ {"FLDCAP", "Field capacity [fraction]"},
      /* 179 */ {"ACOND", "Aerodynamic conductance [m/s]"},
      /* 180 */ {"SNOAG", "Snow age [s]"},
      /* 181 */ {"CCOND", "Canopy conductance [m/s]"},
      /* 182 */ {"LAI", "Leaf area index (0-9) [non-dim]"},
      /* 183 */ {"SFCRH", "Roughness length for heat [m]"},
      /* 184 */ {"SALBD", "Snow albedo (over snow cover area only) [%]"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"NDVI", "Normalized Difference Vegetation Index []"},
      /* 188 */ {"DRIP", "Canopy drip [Kg/m^2]"},
      /* 189 */ {"VBSALB", "Visible, black sky albedo [%]"},
      /* 190 */ {"VWSALB", "Visible, white sky albedo [%]"},
      /* 191 */ {"NBSALB", "Near IR, black sky albedo [%]"},
      /* 192 */ {"NWSALB", "Near IR, white sky albedo [%]"},
      /* 193 */ {"FRZR", "Freezing rain [kg/m^2]"},
      /* 194 */ {"FROZR", "Frozen rain [kg/m^2]"},
      /* 195 */ {"TSNOW", "Total snow [kg/m^2]"},
      /* 196 */ {"MTERH", "Model terrain height [m]"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"SBSNO", "Sublimation (evaporation from snow) [W/m^2]"},
      /* 199 */ {"EVBS", "Direct evaporation from bare soil [W/m^2]"},
      /* 200 */ {"EVCW", "Canopy water evaporation [W/m^2]"},
      /* 201 */ {"VTCIN", "Virtual temperature based convective inhibition [J/kg]"},
      /* 202 */ {"VTCAPE", "Virtual temperature based convective available pot. energy [J/Kg]"},
      /* 203 */ {"RSMIN", "Minimal stomatal resistance [s/m]"},
      /* 204 */ {"DSWRF", "Downward shortwave radiation flux [W/m^2]"},
      /* 205 */ {"DLWRF", "Downward longwave radiation flux [W/m^2]"},
      /* 206 */ {"VIL", "Vertically integrated liquid [Kg/m^2]"},
      /* 207 */ {"MSTAV", "Moisture availability [%]"},
      /* 208 */ {"SFEXC", "Exchange coefficient [(Kg/m^3)(m/s)]"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"TRANS", "Transpiration [W/m^2]"},
      /* 211 */ {"USWRF", "Upward short wave radiation flux [W/m^2]"},
      /* 212 */ {"ULWRF", "Upward long wave radiation flux [W/m^2]"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"WILT", "Wilting point [fraction]"},
      /* 220 */ {"FLDCP", "Field Capacity [fraction]"},
      /* 221 */ {"HPBL", "Planetary boundary layer height [m]"},
      /* 222 */ {"SLTYP", "Surface slope type [Index]"},
      /* 223 */ {"CNWAT", "Plant canopy surface water [Kg/m^2]"},
      /* 224 */ {"SOTYP", "Soil type [Index]"},
      /* 225 */ {"VGTYP", "Vegetation type [Index]"},
      /* 226 */ {"BMIXL", "Blackadars mixing length scale [m]"},
      /* 227 */ {"AMIXL", "Asymptotic mixing length scale [m]"},
      /* 228 */ {"PEVAP", "Potential evaporation [Kg/m^2]"},
      /* 229 */ {"SNOHF", "Snow phase-change heat flux [W/m^2]"},
      /* 230 */ {"SMREF", "Transpiration stress-onset (soil moisture) [fraction]"},
      /* 231 */ {"SMDRY", "Direct evaporation cease (soil moisture) [fraction]"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"BGRUN", "Subsurface runoff (baseflow) [Kg/m^2]"},
      /* 235 */ {"SSRUN", "Surface runoff (non-infiltrating) [Kg/m^2]"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"SNOWC", "Snow cover [%]"},
      /* 239 */ {"SNOT", "Snow temperature [K]"},
      /* 240 */ {"POROS", "Soil porosity [fraction]"},
      /* 241 */ {"SBT112", "Simulated brightness temp for GOES11, channel 2 [K]"},
      /* 242 */ {"SBT113", "Simulated brightness temp for GOES11, channel 3 [K]"},
      /* 243 */ {"SBT114", "Simulated brightness temp for GOES11, channel 4 [K]"},
      /* 244 */ {"SBT115", "Simulated brightness temp for GOES11, channel 5 [K]"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"RCS", "Solar parameter in canopy conductance [fraction]"},
      /* 247 */ {"RCT", "Temperature parameter in canopy conductance [fraction]"},
      /* 248 */ {"RCQ", "Humidity parameter in canopy conductance [fraction]"},
      /* 249 */ {"RCSOL", "Soil moisture parameter in canopy conductance [fraction]"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"CD", "Surface drag coefficient [non-dim]"},
      /* 253 */ {"FRICV", "Surface friction velocity [m/s]"},
      /* 254 */ {"RI", "Richardson number [non-dim]"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_nceptab_133[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land-sea coverage (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"POZT", "Ozone production from T term"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"OMGALF", "omega divided by density"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"COVZZ", "Covariance between u and u"},
      /* 165 */ {"COVMM", "Covariance between v and v"},
      /* 166 */ {"COVQZ", "Covariance between q and u"},
      /* 167 */ {"COVQM", "Covariance between q and v"},
      /* 168 */ {"COVTVV", "Covariance between T and omega"},
      /* 169 */ {"COVQVV", "Covariance between q and omega"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"LRGMR", "Large scale moistening rate"},
      /* 174 */ {"VDFOZ", "Ozone vertical diffusion"},
      /* 175 */ {"POZ", "Ozone production"},
      /* 176 */ {"AMSRE9", "Sim brightness tmp for AMSRE on Aqua channel 9 [K]"},
      /* 177 */ {"AMSRE10", "Sim brightness tmp for AMSRE on Aqua channel 10 [K]"},
      /* 178 */ {"AMSRE11", "Sim brightness tmp for AMSRE on Aqua channel 11 [K]"},
      /* 179 */ {"AMSRE12", "Sim brightness tmp for AMSRE on Aqua channel 12 [K]"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"GWDU", "Gravity wave drag u acceleration"},
      /* 182 */ {"GWDV", "Gravity wave drag v acceleration"},
      /* 183 */ {"CNVU", "Convective u momentum mixing acceleration"},
      /* 184 */ {"CNVV", "Convective v momentum mixing acceleration"},
      /* 185 */ {"AKHS", "Sfc exchange coeff for T and Q divided by delta z"},
      /* 186 */ {"AKMS", "Sfc exchange coeff for U and V divided by delta z"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"TOZ", "Ozone tendency"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"SUNSD", "Sunshine duration [s]"},
      /* 192 */ {"MOSF", "Meridional overturning stream function [10^6m^3/s]"},
      /* 193 */ {"EPSR", "Radiative emiissivity"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"QZ0", "q at top of viscous sublayer"},
      /* 196 */ {"CNGWDU", "Convective gravity wave drag zonal acceleration"},
      /* 197 */ {"CNGWDV", "Convective gravity wave drag meridional acceleration"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"THZ0", "Theta at top of viscous sublayer"},
      /* 202 */ {"CNVUMF", "Convective updraft mass flux"},
      /* 203 */ {"COVPSPS", "Covariance between psfc and psfc"},
      /* 204 */ {"QMAX", "Maximum specific humidity at 2m"},
      /* 205 */ {"QMIN", "Minimum specific humidity at 2m"},
      /* 206 */ {"COVQQ", "Covariance between q and q"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"CNVDMF", "Convective downdraft mass flux"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"CNVDEMF", "Convective detrainment mass flux"},
      /* 220 */ {"COVVVVV", "Covariance between omega and omega"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"COVTT", "Covariance between T and T"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"WTEND", "Tendency of vertical velocity"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"POZO", "Ozone production from col ozone term"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_nceptab_128[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land cover (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"AVDEPTH", "Ocean depth - mean [m]"},
      /* 129 */ {"DEPTH", "Ocean depth - instantaneous [m]"},
      /* 130 */ {"ELEV", "Ocean surface elevation relative to geoid [m]"},
      /* 131 */ {"MXEL24", "Max ocean surface elevation in last 24 hours [m]"},
      /* 132 */ {"MNEL24", "Min ocean surface elevation in last 24 hours [m]"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"O2", "Oxygen (O2 (aq)) []"},
      /* 136 */ {"PO4", "PO4 [Mol/kg]"},
      /* 137 */ {"NO3", "NO3 [Mol/kg]"},
      /* 138 */ {"SiO4", "SiO4 [Mol/kg]"},
      /* 139 */ {"CO2aq", "CO2 (aq) [Mol/kg]"},
      /* 140 */ {"HCO3", "HCO3 - [Mol/kg]"},
      /* 141 */ {"CO3", "CO3 -- [Mol/kg]"},
      /* 142 */ {"TCO2", "TCO2 [Mol/kg]"},
      /* 143 */ {"TALK", "TALK [Mol/kg]"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"S11", "S11 - 1,1 component of ice stress tensor []"},
      /* 147 */ {"S12", "S12 - 1,2 component of ice stress tensor []"},
      /* 148 */ {"S22", "S22 - 2,2 component of ice stress tensor []"},
      /* 149 */ {"INV1", "T1 - First invariant of stress tensor []"},
      /* 150 */ {"INV2", "T2 - Second invariant of stress tensor []"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"WVRGH", "Wave Roughness[ ]"},
      /* 156 */ {"WVSTRS", "Wave Stresses []"},
      /* 157 */ {"WHITE", "Whitecap coverage []"},
      /* 158 */ {"SWDIRWID", "Swell direction width []"},
      /* 159 */ {"SWFREWID", "Swell frequency width []"},
      /* 160 */ {"WVAGE", "Wave age []"},
      /* 161 */ {"PWVAGE", "Physical Wave age []"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"LTURB", "Master length scale (turbulence) [m]"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"AIHFLX", "Net Air-Ice heat flux [W/m^2]"},
      /* 171 */ {"AOHFLX", "Net Air-Ocean heat flux [W/m^2]"},
      /* 172 */ {"IOHFLX", "Net Ice-Ocean heat flux [W/m^2]"},
      /* 173 */ {"IOSFLX", "Net Ice-Ocean salt flux kg/s]"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"OMLT", "Ocean Mixed Layer Temperature [K]"},
      /* 176 */ {"OMLS", "Ocean Mixed Layer Salinity [kg/kg]"},
      /* 177 */ {"OMLPOTDEN", "Ocean Mixed Layer Potential density (Referenced to 2000m) [kg/m^3]"},
      /* 178 */ {"OMLU", "U Velocity in mixed layer [m/s]"},
      /* 179 */ {"OMLV", "V Velocity in mixed layer [m/s]"},
      /* 180 */ {"ASHFL", "Assimilative Heat Flux [W/m^2]"},
      /* 181 */ {"ASSFL", "Assimilative Salt Flux [mm/day]"},
      /* 182 */ {"BOTLD", "Bottom Layer Depth [m]"},
      /* 183 */ {"UBARO", "Barotropic U Velocity [m/s]"},
      /* 184 */ {"VBARO", "Barotropic V Velocity [m/s]"},
      /* 185 */ {"INTFD", "Interface Depth [m]"},
      /* 186 */ {"WTMPC", "Temperature [C]"},
      /* 187 */ {"SALIN", "Salinity [psu]"},
      /* 188 */ {"EMNP", "Evaporation - Precipitation [cm/day]"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"KENG", "Kinetic Energy [J/kg]"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"LAYTH", "Layer Thickness[m]"},
      /* 193 */ {"SSTT", "Surface Temperature Trend [K/day]"},
      /* 194 */ {"SSST", "Surface Salinity Trend [psu/day]"},
      /* 195 */ {"OVHD", "Ocean vertical heat diffusivity [m^2/s]"},
      /* 196 */ {"OVSD", "Ocean vertical salt diffusivity [m^2/s]"},
      /* 197 */ {"OVMD", "Ocean vertical momementum diffusivity [m^2/s]"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"RERRVAR", "Relative Error Variance [pure number]"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_ecmwf_128[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"STRF", "Stream function [m**2 s**-1]"},
      /* 2 */ {"VPOT", "Velocity potential [m**2 s**-1]"},
      /* 3 */ {"PT", "Potential temperature [K]"},
      /* 4 */ {"EQPT", "Equivalent potential temperature [K]"},
      /* 5 */ {"SEPT", "Saturated equivalent potential temperature [K]"},
      /* 6 */ {"SSFR", "Soil sand fraction [(0 - 1)]"},
      /* 7 */ {"SCFR", "Soil clay fraction [(0 - 1)]"},
      /* 8 */ {"SRO", "Surface runoff [m]"},
      /* 9 */ {"SSRO", "Sub-surface runoff [m]"},
      /* 10 */ {"WIND", "Wind speed [m s**-1]"},
      /* 11 */ {"UDVW", "U component of divergent wind [m s**-1]"},
      /* 12 */ {"VDVW", "V component of divergent wind [m s**-1]"},
      /* 13 */ {"URTW", "U component of rotational wind [m s**-1]"},
      /* 14 */ {"VRTW", "V component of rotational wind [m s**-1]"},
      /* 15 */ {"ALUVP", "UV visible albedo for direct radiation [(0 - 1)]"},
      /* 16 */ {"ALUVD", "UV visible albedo for diffuse radiation [(0 - 1)]"},
      /* 17 */ {"ALNIP", "Near IR albedo for direct radiation [(0 - 1)]"},
      /* 18 */ {"ALNID", "Near IR albedo for diffuse radiation [(0 - 1)]"},
      /* 19 */ {"UVCS", "Clear sky surface UV [W m**-2 s]"},
      /* 20 */ {"PARCS", "Clear sky surface PAR [W m**-2 s]"},
      /* 21 */ {"UCTP", "Unbalanced component of temperature [K]"},
      /* 22 */ {"UCLN", "Unbalanced component of logarithm of surface pressure []"},
      /* 23 */ {"UCDV", "Unbalanced component of divergence [s**-1]"},
      /* 24 */ {"var24", "Reserved for future unbalanced components []"},
      /* 25 */ {"var25", "Reserved for future unbalanced components []"},
      /* 26 */ {"CL", "Lake cover [(0 - 1)]"},
      /* 27 */ {"CVL", "Low vegetation cover [(0 - 1)]"},
      /* 28 */ {"CVH", "High vegetation cover [(0 - 1)]"},
      /* 29 */ {"TVL", "Type of low vegetation []"},
      /* 30 */ {"TVH", "Type of high vegetation []"},
      /* 31 */ {"CI", "Sea-ice cover [(0 - 1)]"},
      /* 32 */ {"ASN", "Snow albedo [(0 - 1)]"},
      /* 33 */ {"RSN", "Snow density [kg m**-3]"},
      /* 34 */ {"SSTK", "Sea surface temperature [K]"},
      /* 35 */ {"ISTL1", "Ice surface temperature layer 1 [K]"},
      /* 36 */ {"ISTL2", "Ice surface temperature layer 2 [K]"},
      /* 37 */ {"ISTL3", "Ice surface temperature layer 3 [K]"},
      /* 38 */ {"ISTL4", "Ice surface temperature layer 4 [K]"},
      /* 39 */ {"SWVL1", "Volumetric soil water layer 1 [m**3 m**-3]"},
      /* 40 */ {"SWVL2", "Volumetric soil water layer 2 [m**3 m**-3]"},
      /* 41 */ {"SWVL3", "Volumetric soil water layer 3 [m**3 m**-3]"},
      /* 42 */ {"SWVL4", "Volumetric soil water layer 4 [m**3 m**-3]"},
      /* 43 */ {"SLT", "Soil type []"},
      /* 44 */ {"ES", "Snow evaporation [m of water]"},
      /* 45 */ {"SMLT", "Snowmelt [m of water]"},
      /* 46 */ {"SDUR", "Solar duration [s]"},
      /* 47 */ {"DSRP", "Direct solar radiation [w m**-2]"},
      /* 48 */ {"MAGSS", "Magnitude of surface stress [N m**-2 s]"},
      /* 49 */ {"10FG", "10 metre wind gust [m s**-1]"},
      /* 50 */ {"LSPF", "Large-scale precipitation fraction [s]"},
      /* 51 */ {"MX2T24", "Maximum temperature at 2 metres since last 24 hours [K]"},
      /* 52 */ {"MN2T24", "Minimum temperature at 2 metres since last 24 hours [K]"},
      /* 53 */ {"MONT", "Montgomery potential [m**2 s**-2]"},
      /* 54 */ {"PRES", "Pressure [Pa]"},
      /* 55 */ {"MEAN2T24", "Mean temperature at 2 metres since last 24 hours [K]"},
      /* 56 */ {"MN2D24", "Mean 2 metre dewpoint temperature in past 24 hours [K]"},
      /* 57 */ {"UVB", "Downward UV radiation at the surface [w m**-2 s]"},
      /* 58 */ {"PAR", "Photosynthetically active radiation at the surface [w m**-2 s]"},
      /* 59 */ {"CAPE", "Convective available potential energy [J kg**-1]"},
      /* 60 */ {"PV", "Potential vorticity [K m**2 kg**-1 s**-1]"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"OBCT", "Observation count []"},
      /* 63 */ {"var63", "Start time for skin temperature difference [s]"},
      /* 64 */ {"var64", "Finish time for skin temperature difference [s]"},
      /* 65 */ {"var65", "Skin temperature difference [K]"},
      /* 66 */ {"var66", "Leaf area index, low vegetation [m**2 / m**2]"},
      /* 67 */ {"var67", "Leaf area index, high vegetation [m**2 / m**2]"},
      /* 68 */ {"var68", "Minimum stomatal resistance, low vegetation [s m**-1]"},
      /* 69 */ {"var69", "Minimum stomatal resistance, high vegetation [s m**-1]"},
      /* 70 */ {"var70", "Biome cover, low vegetation [(0 - 1)]"},
      /* 71 */ {"var71", "Biome cover, high vegetation [(0 - 1)]"},
      /* 72 */ {"ISSRD", "Instantaneous surface solar radiation downwards [w m**-2]"},
      /* 73 */ {"ISTRD", "Instantaneous surface thermal radiation downwards [w m**-2]"},
      /* 74 */ {"SDFOR", "Standard deviation of filtered subgrid orography [m]"},
      /* 75 */ {"CRWC", "Cloud rain water content [kg kg**-1]"},
      /* 76 */ {"CSWC", "Cloud snow water content [kg kg**-1]"},
      /* 77 */ {"ETADOT", "Eta-coordinate vertical velocity [s**-1]"},
      /* 78 */ {"TCLW", "Total column liquid water [kg m**-2]"},
      /* 79 */ {"TCIW", "Total column ice water [kg m**-2]"},
      /* 80 */ {"var80", "Experimental product []"},
      /* 81 */ {"var81", "Experimental product []"},
      /* 82 */ {"var82", "Experimental product []"},
      /* 83 */ {"var83", "Experimental product []"},
      /* 84 */ {"var84", "Experimental product []"},
      /* 85 */ {"var85", "Experimental product []"},
      /* 86 */ {"var86", "Experimental product []"},
      /* 87 */ {"var87", "Experimental product []"},
      /* 88 */ {"var88", "Experimental product []"},
      /* 89 */ {"var89", "Experimental product []"},
      /* 90 */ {"var90", "Experimental product []"},
      /* 91 */ {"var91", "Experimental product []"},
      /* 92 */ {"var92", "Experimental product []"},
      /* 93 */ {"var93", "Experimental product []"},
      /* 94 */ {"var94", "Experimental product []"},
      /* 95 */ {"var95", "Experimental product []"},
      /* 96 */ {"var96", "Experimental product []"},
      /* 97 */ {"var97", "Experimental product []"},
      /* 98 */ {"var98", "Experimental product []"},
      /* 99 */ {"var99", "Experimental product []"},
      /* 100 */ {"var100", "Experimental product []"},
      /* 101 */ {"var101", "Experimental product []"},
      /* 102 */ {"var102", "Experimental product []"},
      /* 103 */ {"var103", "Experimental product []"},
      /* 104 */ {"var104", "Experimental product []"},
      /* 105 */ {"var105", "Experimental product []"},
      /* 106 */ {"var106", "Experimental product []"},
      /* 107 */ {"var107", "Experimental product []"},
      /* 108 */ {"var108", "Experimental product []"},
      /* 109 */ {"var109", "Experimental product []"},
      /* 110 */ {"var110", "Experimental product []"},
      /* 111 */ {"var111", "Experimental product []"},
      /* 112 */ {"var112", "Experimental product []"},
      /* 113 */ {"var113", "Experimental product []"},
      /* 114 */ {"var114", "Experimental product []"},
      /* 115 */ {"var115", "Experimental product []"},
      /* 116 */ {"var116", "Experimental product []"},
      /* 117 */ {"var117", "Experimental product []"},
      /* 118 */ {"var118", "Experimental product []"},
      /* 119 */ {"var119", "Experimental product []"},
      /* 120 */ {"var120", "Experimental product []"},
      /* 121 */ {"MX2T6", "Maximum temperature at 2 metres since last 6 hours [K]"},
      /* 122 */ {"MN2T6", "Minimum temperature at 2 metres since last 6 hours [K]"},
      /* 123 */ {"10FG6", "10 metre wind gust in the past 6 hours [m s**-1]"},
      /* 124 */ {"EMIS", "Surface emissivity [dimensionless]"},
      /* 125 */ {"var125", "Vertically integrated total energy [J m**-2]"},
      /* 126 */ {"var126", "Generic parameter for sensitive area prediction [Various]"},
      /* 127 */ {"AT", "Atmospheric tide []"},
      /* 128 */ {"BV", "Budget values []"},
      /* 129 */ {"Z", "Geopotential [m**2 s**-2]"},
      /* 130 */ {"T", "Temperature [K]"},
      /* 131 */ {"U", "U velocity [m s**-1]"},
      /* 132 */ {"V", "V velocity [m s**-1]"},
      /* 133 */ {"Q", "Specific humidity [kg kg**-1]"},
      /* 134 */ {"SP", "Surface pressure [Pa]"},
      /* 135 */ {"W", "Vertical velocity [Pa s**-1]"},
      /* 136 */ {"TCW", "Total column water [kg m**-2]"},
      /* 137 */ {"TCWV", "Total column water vapour [kg m**-2]"},
      /* 138 */ {"VO", "Vorticity (relative) [s**-1]"},
      /* 139 */ {"STL1", "Soil temperature level 1 [K]"},
      /* 140 */ {"SWL1", "Soil wetness level 1 [m of water]"},
      /* 141 */ {"SD", "Snow depth [m of water equivalent]"},
      /* 142 */ {"LSP", "Stratiform precipitation (Large-scale precipitation) [m]"},
      /* 143 */ {"CP", "Convective precipitation [m]"},
      /* 144 */ {"SF", "Snowfall [m of water equivalent]"},
      /* 145 */ {"BLD", "Boundary layer dissipation [W m**-2 s]"},
      /* 146 */ {"SSHF", "Surface sensible heat flux [W m**-2 s]"},
      /* 147 */ {"SLHF", "Surface latent heat flux [W m**-2 s]"},
      /* 148 */ {"CHNK", "Charnock []"},
      /* 149 */ {"SNR", "Surface net radiation [W m**-2 s]"},
      /* 150 */ {"TNR", "Top net radiation []"},
      /* 151 */ {"MSL", "Mean sea level pressure [Pa]"},
      /* 152 */ {"LNSP", "Logarithm of surface pressure []"},
      /* 153 */ {"SWHR", "Short-wave heating rate [K]"},
      /* 154 */ {"LWHR", "Long-wave heating rate [K]"},
      /* 155 */ {"D", "Divergence [s**-1]"},
      /* 156 */ {"GH", "Height [gpm]"},
      /* 157 */ {"R", "Relative humidity [%]"},
      /* 158 */ {"TSP", "Tendency of surface pressure [Pa s**-1]"},
      /* 159 */ {"BLH", "Boundary layer height [m]"},
      /* 160 */ {"SDOR", "Standard deviation of orography []"},
      /* 161 */ {"ISOR", "Anisotropy of sub-gridscale orography []"},
      /* 162 */ {"ANOR", "Angle of sub-gridscale orography [rad]"},
      /* 163 */ {"SLOR", "Slope of sub-gridscale orography []"},
      /* 164 */ {"TCC", "Total cloud cover [(0 - 1)]"},
      /* 165 */ {"10U", "10 metre U wind component [m s**-1]"},
      /* 166 */ {"10V", "10 metre V wind component [m s**-1]"},
      /* 167 */ {"2T", "2 metre temperature [K]"},
      /* 168 */ {"2D", "2 metre dewpoint temperature [K]"},
      /* 169 */ {"SSRD", "Surface solar radiation downwards [W m**-2 s]"},
      /* 170 */ {"STL2", "Soil temperature level 2 [K]"},
      /* 171 */ {"SWL2", "Soil wetness level 2 [m of water]"},
      /* 172 */ {"LSM", "Land-sea mask [(0 - 1)]"},
      /* 173 */ {"SR", "Surface roughness [m]"},
      /* 174 */ {"AL", "Albedo [(0 - 1)]"},
      /* 175 */ {"STRD", "Surface thermal radiation downwards [W m**-2 s]"},
      /* 176 */ {"SSR", "Surface solar radiation [W m**-2 s]"},
      /* 177 */ {"STR", "Surface thermal radiation [W m**-2 s]"},
      /* 178 */ {"TSR", "Top solar radiation [W m**-2 s]"},
      /* 179 */ {"TTR", "Top thermal radiation [W m**-2 s]"},
      /* 180 */ {"EWSS", "East-West surface stress [N m**-2 s]"},
      /* 181 */ {"NSSS", "North-South surface stress [N m**-2 s]"},
      /* 182 */ {"E", "Evaporation [m of water]"},
      /* 183 */ {"STL3", "Soil temperature level 3 [K]"},
      /* 184 */ {"SWL3", "Soil wetness level 3 [m of water]"},
      /* 185 */ {"CCC", "Convective cloud cover [(0 - 1)]"},
      /* 186 */ {"LCC", "Low cloud cover [(0 - 1)]"},
      /* 187 */ {"MCC", "Medium cloud cover [(0 - 1)]"},
      /* 188 */ {"HCC", "High cloud cover [(0 - 1)]"},
      /* 189 */ {"SUND", "Sunshine duration [s]"},
      /* 190 */ {"EWOV", "East-West component of sub-gridscale orographic variance [m**2]"},
      /* 191 */ {"NSOV", "North-South component of sub-gridscale orographic variance [m**2]"},
      /* 192 */ {"NWOV", "North-West/South-East component of sub-gridscale orographic variance [m**2]"},
      /* 193 */ {"NEOV", "North-East/South-West component of sub-gridscale orographic variance [m**2]"},
      /* 194 */ {"BTMP", "Brightness temperature [K]"},
      /* 195 */ {"LGWS", "Latitudinal component of gravity wave stress [N m**-2 s]"},
      /* 196 */ {"MGWS", "Meridional component of gravity wave stress [N m**-2 s]"},
      /* 197 */ {"GWD", "Gravity wave dissipation [W m**-2 s]"},
      /* 198 */ {"SRC", "Skin reservoir content [m of water]"},
      /* 199 */ {"VEG", "Vegetation fraction [(0 - 1)]"},
      /* 200 */ {"VSO", "Variance of sub-gridscale orography [m**2]"},
      /* 201 */ {"MX2T", "Maximum temperature at 2 metres since previous post-processing [K]"},
      /* 202 */ {"MN2T", "Minimum temperature at 2 metres since previous post-processing [K]"},
      /* 203 */ {"O3", "Ozone mass mixing ratio [kg kg**-1]"},
      /* 204 */ {"PAW", "Precipitation analysis weights []"},
      /* 205 */ {"RO", "Runoff [m]"},
      /* 206 */ {"TCO3", "Total column ozone [kg m**-2]"},
      /* 207 */ {"10SI", "10 metre wind speed [m s**-1]"},
      /* 208 */ {"TSRC", "Top net solar radiation, clear sky [W m**-2 s]"},
      /* 209 */ {"TTRC", "Top net thermal radiation, clear sky [W m**-2 s]"},
      /* 210 */ {"SSRC", "Surface net solar radiation, clear sky [W m**-2 s]"},
      /* 211 */ {"STRC", "Surface net thermal radiation, clear sky [W m**-2 s]"},
      /* 212 */ {"TISR", "TOA incident solar radiation [W m**-2 s]"},
      /* 213 */ {"VIMD", "Vertically integrated moisture divergence [kg m**-2]"},
      /* 214 */ {"DHR", "Diabatic heating by radiation [K]"},
      /* 215 */ {"DHVD", "Diabatic heating by vertical diffusion [K]"},
      /* 216 */ {"DHCC", "Diabatic heating by cumulus convection [K]"},
      /* 217 */ {"DHLC", "Diabatic heating large-scale condensation [K]"},
      /* 218 */ {"VDZW", "Vertical diffusion of zonal wind [m s**-1]"},
      /* 219 */ {"VDMW", "Vertical diffusion of meridional wind [m s**-1]"},
      /* 220 */ {"EWGD", "East-West gravity wave drag tendency [m s**-1]"},
      /* 221 */ {"NSGD", "North-South gravity wave drag tendency [m s**-1]"},
      /* 222 */ {"CTZW", "Convective tendency of zonal wind [m s**-1]"},
      /* 223 */ {"CTMW", "Convective tendency of meridional wind [m s**-1]"},
      /* 224 */ {"VDH", "Vertical diffusion of humidity [kg kg**-1]"},
      /* 225 */ {"HTCC", "Humidity tendency by cumulus convection [kg kg**-1]"},
      /* 226 */ {"HTLC", "Humidity tendency by large-scale condensation [kg kg**-1]"},
      /* 227 */ {"CRNH", "Change from removal of negative humidity [kg kg**-1]"},
      /* 228 */ {"TP", "Total precipitation [m]"},
      /* 229 */ {"IEWS", "Instantaneous X surface stress [N m**-2]"},
      /* 230 */ {"INSS", "Instantaneous Y surface stress [N m**-2]"},
      /* 231 */ {"ISHF", "Instantaneous surface heat flux [W m**-2]"},
      /* 232 */ {"IE", "Instantaneous moisture flux [kg m**-2 s**-1]"},
      /* 233 */ {"ASQ", "Apparent surface humidity [kg kg**-1]"},
      /* 234 */ {"LSRH", "Logarithm of surface roughness length for heat []"},
      /* 235 */ {"SKT", "Skin temperature [K]"},
      /* 236 */ {"STL4", "Soil temperature level 4 [K]"},
      /* 237 */ {"SWL4", "Soil wetness level 4 [m]"},
      /* 238 */ {"TSN", "Temperature of snow layer [K]"},
      /* 239 */ {"CSF", "Convective snowfall [m of water equivalent]"},
      /* 240 */ {"LSF", "Large-scale snowfall [m of water equivalent]"},
      /* 241 */ {"ACF", "Accumulated cloud fraction tendency [(-1 to 1)]"},
      /* 242 */ {"ALW", "Accumulated liquid water tendency [(-1 to 1)]"},
      /* 243 */ {"FAL", "Forecast albedo [(0 - 1)]"},
      /* 244 */ {"FSR", "Forecast surface roughness [m]"},
      /* 245 */ {"FLSR", "Forecast logarithm of surface roughness for heat []"},
      /* 246 */ {"CLWC", "Cloud liquid water content [kg kg**-1]"},
      /* 247 */ {"CIWC", "Cloud ice water content [kg kg**-1]"},
      /* 248 */ {"CC", "Cloud cover [(0 - 1)]"},
      /* 249 */ {"AIW", "Accumulated ice water tendency [(-1 to 1)]"},
      /* 250 */ {"ICE", "Ice age [(0 - 1)]"},
      /* 251 */ {"ATTE", "Adiabatic tendency of temperature [K]"},
      /* 252 */ {"ATHE", "Adiabatic tendency of humidity [kg kg**-1]"},
      /* 253 */ {"ATZE", "Adiabatic tendency of zonal wind [m s**-1]"},
      /* 254 */ {"ATMW", "Adiabatic tendency of meridional wind [m s**-1]"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_129[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"STRF", "Stream function [m**2 s**-1]"},
      /* 2 */ {"VPOT", "Velocity potential [m**2 s**-1]"},
      /* 3 */ {"PT", "Potential temperature [K]"},
      /* 4 */ {"EQPT", "Equivalent potential temperature [K]"},
      /* 5 */ {"SEPT", "Saturated equivalent potential temperature [K]"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"UDVW", "U component of divergent wind [m s**-1]"},
      /* 12 */ {"VDVW", "V component of divergent wind [m s**-1]"},
      /* 13 */ {"URTW", "U component of rotational wind [m s**-1]"},
      /* 14 */ {"VRTW", "V component of rotational wind [m s**-1]"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"UCTP", "Unbalanced component of temperature [K]"},
      /* 22 */ {"UCLN", "Unbalanced component of logarithm of surface pressure []"},
      /* 23 */ {"UCDV", "Unbalanced component of divergence [s**-1]"},
      /* 24 */ {"var24", "Reserved for future unbalanced components []"},
      /* 25 */ {"var25", "Reserved for future unbalanced components []"},
      /* 26 */ {"CL", "Lake cover [(0 - 1)]"},
      /* 27 */ {"CVL", "Low vegetation cover [(0 - 1)]"},
      /* 28 */ {"CVH", "High vegetation cover [(0 - 1)]"},
      /* 29 */ {"TVL", "Type of low vegetation []"},
      /* 30 */ {"TVH", "Type of high vegetation []"},
      /* 31 */ {"CI", "Sea-ice cover [(0 - 1)]"},
      /* 32 */ {"ASN", "Snow albedo [(0 - 1)]"},
      /* 33 */ {"RSN", "Snow density [kg m**-3]"},
      /* 34 */ {"SSTK", "Sea surface temperature [K]"},
      /* 35 */ {"ISTL1", "Ice surface temperature layer 1 [K]"},
      /* 36 */ {"ISTL2", "Ice surface temperature layer 2 [K]"},
      /* 37 */ {"ISTL3", "Ice surface temperature layer 3 [K]"},
      /* 38 */ {"ISTL4", "Ice surface temperature layer 4 [K]"},
      /* 39 */ {"SWVL1", "Volumetric soil water layer 1 [m**3 m**-3]"},
      /* 40 */ {"SWVL2", "Volumetric soil water layer 2 [m**3 m**-3]"},
      /* 41 */ {"SWVL3", "Volumetric soil water layer 3 [m**3 m**-3]"},
      /* 42 */ {"SWVL4", "Volumetric soil water layer 4 [m**3 m**-3]"},
      /* 43 */ {"SLT", "Soil type []"},
      /* 44 */ {"ES", "Snow evaporation [m of water]"},
      /* 45 */ {"SMLT", "Snowmelt [m of water]"},
      /* 46 */ {"SDUR", "Solar duration [s]"},
      /* 47 */ {"DSRP", "Direct solar radiation [w m**-2]"},
      /* 48 */ {"MAGSS", "Magnitude of surface stress [N m**-2 s]"},
      /* 49 */ {"10FG", "10 metre wind gust [m s**-1]"},
      /* 50 */ {"LSPF", "Large-scale precipitation fraction [s]"},
      /* 51 */ {"MX2T24", "Maximum 2 metre temperature [K]"},
      /* 52 */ {"MN2T24", "Minimum 2 metre temperature [K]"},
      /* 53 */ {"MONT", "Montgomery potential [m**2 s**-2]"},
      /* 54 */ {"PRES", "Pressure [Pa]"},
      /* 55 */ {"MEAN2T24", "Mean 2 metre temperature in past 24 hours [K]"},
      /* 56 */ {"MN2D24", "Mean 2 metre dewpoint temperature in past 24 hours [K]"},
      /* 57 */ {"UVB", "Downward UV radiation at the surface [w m**-2 s]"},
      /* 58 */ {"PAR", "Photosynthetically active radiation at the surface [w m**-2 s]"},
      /* 59 */ {"CAPE", "Convective available potential energy [J kg**-1]"},
      /* 60 */ {"PV", "Potential vorticity [K m**2 kg**-1 s**-1]"},
      /* 61 */ {"TPO", "Total precipitation from observations [Millimetres*100 + number of stations]"},
      /* 62 */ {"OBCT", "Observation count []"},
      /* 63 */ {"var63", "Start time for skin temperature difference [s]"},
      /* 64 */ {"var64", "Finish time for skin temperature difference [s]"},
      /* 65 */ {"var65", "Skin temperature difference [K]"},
      /* 66 */ {"var66", "Leaf area index, low vegetation [m**2 / m**2]"},
      /* 67 */ {"var67", "Leaf area index, high vegetation [m**2 / m**2]"},
      /* 68 */ {"var68", "Minimum stomatal resistance, low vegetation [s m**-1]"},
      /* 69 */ {"var69", "Minimum stomatal resistance, high vegetation [s m**-1]"},
      /* 70 */ {"var70", "Biome cover, low vegetation [(0 - 1)]"},
      /* 71 */ {"var71", "Biome cover, high vegetation [(0 - 1)]"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "Total column liquid water [kg m**-2]"},
      /* 79 */ {"var79", "Total column ice water [kg m**-2]"},
      /* 80 */ {"var80", "Experimental product []"},
      /* 81 */ {"var81", "Experimental product []"},
      /* 82 */ {"var82", "Experimental product []"},
      /* 83 */ {"var83", "Experimental product []"},
      /* 84 */ {"var84", "Experimental product []"},
      /* 85 */ {"var85", "Experimental product []"},
      /* 86 */ {"var86", "Experimental product []"},
      /* 87 */ {"var87", "Experimental product []"},
      /* 88 */ {"var88", "Experimental product []"},
      /* 89 */ {"var89", "Experimental product []"},
      /* 90 */ {"var90", "Experimental product []"},
      /* 91 */ {"var91", "Experimental product []"},
      /* 92 */ {"var92", "Experimental product []"},
      /* 93 */ {"var93", "Experimental product []"},
      /* 94 */ {"var94", "Experimental product []"},
      /* 95 */ {"var95", "Experimental product []"},
      /* 96 */ {"var96", "Experimental product []"},
      /* 97 */ {"var97", "Experimental product []"},
      /* 98 */ {"var98", "Experimental product []"},
      /* 99 */ {"var99", "Experimental product []"},
      /* 100 */ {"var100", "Experimental product []"},
      /* 101 */ {"var101", "Experimental product []"},
      /* 102 */ {"var102", "Experimental product []"},
      /* 103 */ {"var103", "Experimental product []"},
      /* 104 */ {"var104", "Experimental product []"},
      /* 105 */ {"var105", "Experimental product []"},
      /* 106 */ {"var106", "Experimental product []"},
      /* 107 */ {"var107", "Experimental product []"},
      /* 108 */ {"var108", "Experimental product []"},
      /* 109 */ {"var109", "Experimental product []"},
      /* 110 */ {"var110", "Experimental product []"},
      /* 111 */ {"var111", "Experimental product []"},
      /* 112 */ {"var112", "Experimental product []"},
      /* 113 */ {"var113", "Experimental product []"},
      /* 114 */ {"var114", "Experimental product []"},
      /* 115 */ {"var115", "Experimental product []"},
      /* 116 */ {"var116", "Experimental product []"},
      /* 117 */ {"var117", "Experimental product []"},
      /* 118 */ {"var118", "Experimental product []"},
      /* 119 */ {"var119", "Experimental product []"},
      /* 120 */ {"var120", "Experimental product []"},
      /* 121 */ {"MX2T6", "Maximum temperature at 2 metres [K]"},
      /* 122 */ {"MN2T6", "Minimum temperature at 2 metres [K]"},
      /* 123 */ {"10FG6", "10 metre wind gust in the past 6 hours [m s**-1]"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "Vertically integrated total energy [J m**-2]"},
      /* 126 */ {"var126", "Generic parameter for sensitive area prediction [Various]"},
      /* 127 */ {"AT", "Atmospheric tide []"},
      /* 128 */ {"BV", "Budget values []"},
      /* 129 */ {"Z", "Geopotential [m**2 s**-2]"},
      /* 130 */ {"T", "Temperature [K]"},
      /* 131 */ {"U", "U velocity [m s**-1]"},
      /* 132 */ {"V", "V velocity [m s**-1]"},
      /* 133 */ {"Q", "Specific humidity [kg kg**-1]"},
      /* 134 */ {"SP", "Surface pressure [Pa]"},
      /* 135 */ {"W", "Vertical velocity [Pa s**-1]"},
      /* 136 */ {"TCW", "Total column water [kg m**-2]"},
      /* 137 */ {"TCWV", "Total column water vapour [kg m**-2]"},
      /* 138 */ {"VO", "Vorticity (relative) [s**-1]"},
      /* 139 */ {"STL1", "Soil temperature level 1 [K]"},
      /* 140 */ {"SWL1", "Soil wetness level 1 [m of water]"},
      /* 141 */ {"SD", "Snow depth [m of water equivalent]"},
      /* 142 */ {"LSP", "Stratiform precipitation (Large-scale precipitation) [m]"},
      /* 143 */ {"CP", "Convective precipitation [m]"},
      /* 144 */ {"SF", "Snowfall (convective + stratiform) [m of water equivalent]"},
      /* 145 */ {"BLD", "Boundary layer dissipation [W m**-2 s]"},
      /* 146 */ {"SSHF", "Surface sensible heat flux [W m**-2 s]"},
      /* 147 */ {"SLHF", "Surface latent heat flux [W m**-2 s]"},
      /* 148 */ {"CHNK", "Charnock []"},
      /* 149 */ {"SNR", "Surface net radiation [W m**-2 s]"},
      /* 150 */ {"TNR", "Top net radiation []"},
      /* 151 */ {"MSL", "Mean sea level pressure [Pa]"},
      /* 152 */ {"LNSP", "Logarithm of surface pressure []"},
      /* 153 */ {"SWHR", "Short-wave heating rate [K]"},
      /* 154 */ {"LWHR", "Long-wave heating rate [K]"},
      /* 155 */ {"D", "Divergence [s**-1]"},
      /* 156 */ {"GH", "Height [m]"},
      /* 157 */ {"R", "Relative humidity [%]"},
      /* 158 */ {"TSP", "Tendency of surface pressure [Pa s**-1]"},
      /* 159 */ {"BLH", "Boundary layer height [m]"},
      /* 160 */ {"SDOR", "Standard deviation of orography []"},
      /* 161 */ {"ISOR", "Anisotropy of sub-gridscale orography []"},
      /* 162 */ {"ANOR", "Angle of sub-gridscale orography [rad]"},
      /* 163 */ {"SLOR", "Slope of sub-gridscale orography []"},
      /* 164 */ {"TCC", "Total cloud cover [(0 - 1)]"},
      /* 165 */ {"10U", "10 metre U wind component [m s**-1]"},
      /* 166 */ {"10V", "10 metre V wind component [m s**-1]"},
      /* 167 */ {"2T", "2 metre temperature [K]"},
      /* 168 */ {"2D", "2 metre dewpoint temperature [K]"},
      /* 169 */ {"SSRD", "Surface solar radiation downwards [W m**-2 s]"},
      /* 170 */ {"STL2", "Soil temperature level 2 [K]"},
      /* 171 */ {"SWL2", "Soil wetness level 2 [m of water]"},
      /* 172 */ {"LSM", "Land-sea mask [(0 - 1)]"},
      /* 173 */ {"SR", "Surface roughness [m]"},
      /* 174 */ {"AL", "Albedo [(0 - 1)]"},
      /* 175 */ {"STRD", "Surface thermal radiation downwards [W m**-2 s]"},
      /* 176 */ {"SSR", "Surface solar radiation [W m**-2 s]"},
      /* 177 */ {"STR", "Surface thermal radiation [W m**-2 s]"},
      /* 178 */ {"TSR", "Top solar radiation [W m**-2 s]"},
      /* 179 */ {"TTR", "Top thermal radiation [W m**-2 s]"},
      /* 180 */ {"EWSS", "East-West surface stress [N m**-2 s]"},
      /* 181 */ {"NSSS", "North-South surface stress [N m**-2 s]"},
      /* 182 */ {"E", "Evaporation [m of water]"},
      /* 183 */ {"STL3", "Soil temperature level 3 [K]"},
      /* 184 */ {"SWL3", "Soil wetness level 3 [m of water]"},
      /* 185 */ {"CCC", "Convective cloud cover [(0 - 1)]"},
      /* 186 */ {"LCC", "Low cloud cover [(0 - 1)]"},
      /* 187 */ {"MCC", "Medium cloud cover [(0 - 1)]"},
      /* 188 */ {"HCC", "High cloud cover [(0 - 1)]"},
      /* 189 */ {"SUND", "Sunshine duration [s]"},
      /* 190 */ {"EWOV", "East-West component of sub-gridscale orographic variance [m**2]"},
      /* 191 */ {"NSOV", "North-South component of sub-gridscale orographic variance [m**2]"},
      /* 192 */ {"NWOV", "North-West/South-East component of sub-gridscale orographic variance [m**2]"},
      /* 193 */ {"NEOV", "North-East/South-West component of sub-gridscale orographic variance [m**2]"},
      /* 194 */ {"BTMP", "Brightness temperature [K]"},
      /* 195 */ {"LGWS", "Latitudinal component of gravity wave stress [N m**-2 s]"},
      /* 196 */ {"MGWS", "Meridional component of gravity wave stress [N m**-2 s]"},
      /* 197 */ {"GWD", "Gravity wave dissipation [W m**-2 s]"},
      /* 198 */ {"SRC", "Skin reservoir content [m of water]"},
      /* 199 */ {"VEG", "Vegetation fraction [(0 - 1)]"},
      /* 200 */ {"VSO", "Variance of sub-gridscale orography [m**2]"},
      /* 201 */ {"MX2T", "Maximum temperature at 2 metres since previous post-processing [K]"},
      /* 202 */ {"MN2T", "Minimum temperature at 2 metres since previous post-processing [K]"},
      /* 203 */ {"O3", "Ozone mass mixing ratio [kg kg**-1]"},
      /* 204 */ {"PAW", "Precipitation analysis weights []"},
      /* 205 */ {"RO", "Runoff [m]"},
      /* 206 */ {"TCO3", "Total column ozone [kg m**-2]"},
      /* 207 */ {"10SI", "10 metre wind speed [m s**-1]"},
      /* 208 */ {"TSRC", "Top net solar radiation, clear sky [W m**-2 s]"},
      /* 209 */ {"TTRC", "Top net thermal radiation, clear sky [W m**-2 s]"},
      /* 210 */ {"SSRC", "Surface net solar radiation, clear sky [W m**-2 s]"},
      /* 211 */ {"STRC", "Surface net thermal radiation, clear sky [W m**-2 s]"},
      /* 212 */ {"TISR", "TOA incident solar radiation [W m**-2 s]"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"DHR", "Diabatic heating by radiation [K]"},
      /* 215 */ {"DHVD", "Diabatic heating by vertical diffusion [K]"},
      /* 216 */ {"DHCC", "Diabatic heating by cumulus convection [K]"},
      /* 217 */ {"DHLC", "Diabatic heating large-scale condensation [K]"},
      /* 218 */ {"VDZW", "Vertical diffusion of zonal wind [m s**-1]"},
      /* 219 */ {"VDMW", "Vertical diffusion of meridional wind [m s**-1]"},
      /* 220 */ {"EWGD", "East-West gravity wave drag tendency [m s**-1]"},
      /* 221 */ {"NSGD", "North-South gravity wave drag tendency [m s**-1]"},
      /* 222 */ {"CTZW", "Convective tendency of zonal wind [m s**-1]"},
      /* 223 */ {"CTMW", "Convective tendency of meridional wind [m s**-1]"},
      /* 224 */ {"VDH", "Vertical diffusion of humidity [kg kg**-1]"},
      /* 225 */ {"HTCC", "Humidity tendency by cumulus convection [kg kg**-1]"},
      /* 226 */ {"HTLC", "Humidity tendency by large-scale condensation [kg kg**-1]"},
      /* 227 */ {"CRNH", "Change from removal of negative humidity [kg kg**-1]"},
      /* 228 */ {"TP", "Total precipitation [m]"},
      /* 229 */ {"IEWS", "Instantaneous X surface stress [N m**-2]"},
      /* 230 */ {"INSS", "Instantaneous Y surface stress [N m**-2]"},
      /* 231 */ {"ISHF", "Instantaneous surface heat flux [W m**-2]"},
      /* 232 */ {"IE", "Instantaneous moisture flux [kg m**-2 s]"},
      /* 233 */ {"ASQ", "Apparent surface humidity [kg kg**-1]"},
      /* 234 */ {"LSRH", "Logarithm of surface roughness length for heat []"},
      /* 235 */ {"SKT", "Skin temperature [K]"},
      /* 236 */ {"STL4", "Soil temperature level 4 [K]"},
      /* 237 */ {"SWL4", "Soil wetness level 4 [m]"},
      /* 238 */ {"TSN", "Temperature of snow layer [K]"},
      /* 239 */ {"CSF", "Convective snowfall [m of water equivalent]"},
      /* 240 */ {"LSF", "Large-scale snowfall [m of water equivalent]"},
      /* 241 */ {"ACF", "Accumulated cloud fraction tendency [(-1 to 1)]"},
      /* 242 */ {"ALW", "Accumulated liquid water tendency [(-1 to 1)]"},
      /* 243 */ {"FAL", "Forecast albedo [(0 - 1)]"},
      /* 244 */ {"FSR", "Forecast surface roughness [m]"},
      /* 245 */ {"FLSR", "Forecast logarithm of surface roughness for heat []"},
      /* 246 */ {"CLWC", "Cloud liquid water content [kg kg**-1]"},
      /* 247 */ {"CIWC", "Cloud ice water content [kg kg**-1]"},
      /* 248 */ {"CC", "Cloud cover [(0 - 1)]"},
      /* 249 */ {"AIW", "Accumulated ice water tendency [(-1 to 1)]"},
      /* 250 */ {"ICE", "Ice age [(0 - 1)]"},
      /* 251 */ {"ATTE", "Adiabatic tendency of temperature [K]"},
      /* 252 */ {"ATHE", "Adiabatic tendency of humidity [kg kg**-1]"},
      /* 253 */ {"ATZE", "Adiabatic tendency of zonal wind [m s**-1]"},
      /* 254 */ {"ATMW", "Adiabatic tendency of meridional wind [m s**-1]"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_130[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"TSRU", "Top solar radiation upward [W m**-2]"},
      /* 209 */ {"TTRU", "Top thermal radiation upward [W m**-2]"},
      /* 210 */ {"TSUC", "Top solar radiation upward, clear sky [W m**-2]"},
      /* 211 */ {"TTUC", "Top thermal radiation upward, clear sky [W m**-2]"},
      /* 212 */ {"CLW", "Cloud liquid water [kg kg**-1]"},
      /* 213 */ {"CF", "Cloud fraction [(0 - 1)]"},
      /* 214 */ {"DHR", "Diabatic heating by radiation [K s**-1]"},
      /* 215 */ {"DHVD", "Diabatic heating by vertical diffusion [K s**-1]"},
      /* 216 */ {"DHCC", "Diabatic heating by cumulus convection [K s**-1]"},
      /* 217 */ {"DHLC", "Diabatic heating by large-scale condensation [K s**-1]"},
      /* 218 */ {"VDZW", "Vertical diffusion of zonal wind [m**2 s**-3]"},
      /* 219 */ {"VDMW", "Vertical diffusion of meridional wind [m**2 s**-3]"},
      /* 220 */ {"EWGD", "East-West gravity wave drag [m**2 s**-3]"},
      /* 221 */ {"NSGD", "North-South gravity wave drag [m**2 s**-3]"},
      /* 222 */ {"CTZW", "Convective tendency of zonal wind [m**2 s**-3]"},
      /* 223 */ {"CTMW", "Convective tendency of meridional wind [m**2 s**-3]"},
      /* 224 */ {"VDH", "Vertical diffusion of humidity [kg kg**-1 s**-1]"},
      /* 225 */ {"HTCC", "Humidity tendency by cumulus convection [kg kg**-1 s**-1]"},
      /* 226 */ {"HTLC", "Humidity tendency by large-scale condensation [kg kg**-1 s**-1]"},
      /* 227 */ {"CRNH", "Change from removal of negative humidity [kg kg**-1 s**-1]"},
      /* 228 */ {"ATT", "Adiabatic tendency of temperature [K s**-1]"},
      /* 229 */ {"ATH", "Adiabatic tendency of humidity [kg kg**-1 s**-1]"},
      /* 230 */ {"ATZW", "Adiabatic tendency of zonal wind [m**2 s**-3]"},
      /* 231 */ {"ATMWAX", "Adiabatic tendency of meridional wind [m**2 s**-3]"},
      /* 232 */ {"MVV", "Mean vertical velocity [Pa s**-1]"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_131[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"2TAG2", "2m temperature anomaly of at least +2K [%]"},
      /* 2 */ {"2TAG1", "2m temperature anomaly of at least +1K [%]"},
      /* 3 */ {"2TAG0", "2m temperature anomaly of at least 0K [%]"},
      /* 4 */ {"2TALM1", "2m temperature anomaly of at most -1K [%]"},
      /* 5 */ {"2TALM2", "2m temperature anomaly of at most -2K [%]"},
      /* 6 */ {"TPAG20", "Total precipitation anomaly of at least 20 mm [%]"},
      /* 7 */ {"TPAG10", "Total precipitation anomaly of at least 10 mm [%]"},
      /* 8 */ {"TPAG0", "Total precipitation anomaly of at least 0 mm [%]"},
      /* 9 */ {"STAG0", "Surface temperature anomaly of at least 0K [%]"},
      /* 10 */ {"MSLAG0", "Mean sea level pressure anomaly of at least 0 Pa [%]"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"H0DIP", "Heigth of 0 degree isotherm probability [percentage]"},
      /* 16 */ {"HSLP", "Heigth of snowfall limit probability [percentage]"},
      /* 17 */ {"SAIP", "Showalter index probability [percentage]"},
      /* 18 */ {"WHIP", "Whiting index probability [percentage]"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"TALM2K", "Temperature anomaly less than -2 K [%]"},
      /* 21 */ {"TAG2K", "Temperature anomaly of at least +2 K [%]"},
      /* 22 */ {"TALM8K", "Temperature anomaly less than -8 K [%]"},
      /* 23 */ {"TALM4K", "Temperature anomaly less than -4 K [%]"},
      /* 24 */ {"TAG4K", "Temperature anomaly greater than +4 K [%]"},
      /* 25 */ {"TAG8K", "Temperature anomaly greater than +8 K [%]"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"10GP", "10 metre wind gust probability [percentage]"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"CAPEP", "Convective available potential energy probability [percentage]"},
      /* 60 */ {"TPG1", "Total precipitation of at least 1 mm [%]"},
      /* 61 */ {"TPG5", "Total precipitation of at least 5 mm [%]"},
      /* 62 */ {"TPG10", "Total precipitation of at least 10 mm [%]"},
      /* 63 */ {"TPG20", "Total precipitation of at least 20 mm [%]"},
      /* 64 */ {"TPL01", "Total precipitation less than 0.1 mm [%]"},
      /* 65 */ {"TPRL1", "Total precipitation rate less than 1 mm per day [%]"},
      /* 66 */ {"TPRG3", "Total precipitation rate of at least 3 mm per day [%]"},
      /* 67 */ {"TPRG5", "Total precipitation rate of at least 5 mm per day [%]"},
      /* 68 */ {"10SPG10", "10 metre Wind speed of at least 10 metre per second [%]"},
      /* 69 */ {"10SPG15", "10 metre Wind speed of at least 15 metre per second [%]"},
      /* 70 */ {"10FGG15", "10 metre Wind gust of at least 15 metre per second [%]"},
      /* 71 */ {"10FGG20", "10 metre Wind gust of at least 20 metre per second [%]"},
      /* 72 */ {"10FGG25", "10 metre Wind gust of at least 25 metre per second [%]"},
      /* 73 */ {"2TL273", "2 metre temperature less than 273.15 K [%]"},
      /* 74 */ {"SWHG2", "Significant wave height of at least 2 m [%]"},
      /* 75 */ {"SWHG4", "Significant wave height of at least 4 m [%]"},
      /* 76 */ {"SWHG6", "Significant wave height of at least 6 m [%]"},
      /* 77 */ {"SWHG8", "Significant wave height of at least 8 m [%]"},
      /* 78 */ {"MWPG8", "Mean wave period of at least 8 s [%]"},
      /* 79 */ {"MWPG10", "Mean wave period of at least 10 s [%]"},
      /* 80 */ {"MWPG12", "Mean wave period of at least 12 s [%]"},
      /* 81 */ {"MWPG15", "Mean wave period of at least 15 s [%]"},
      /* 82 */ {"TPG40", "Total precipitation of at least 40 mm [%]"},
      /* 83 */ {"TPG60", "Total precipitation of at least 60 mm [%]"},
      /* 84 */ {"TPG80", "Total precipitation of at least 80 mm [%]"},
      /* 85 */ {"TPG100", "Total precipitation of at least 100 mm [%]"},
      /* 86 */ {"TPG150", "Total precipitation of at least 150 mm [%]"},
      /* 87 */ {"TPG200", "Total precipitation of at least 200 mm [%]"},
      /* 88 */ {"TPG300", "Total precipitation of at least 300 mm [%]"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"ZP", "Geopotential probability [%]"},
      /* 130 */ {"TAP", "Temperature anomaly probability [percentage]"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "2 metre temperature probability [%]"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"SFP", "Snowfall probability [percentage]"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "Total precipitation probability [%]"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"TCCP", "Total cloud cover probability [percentage]"},
      /* 165 */ {"10SP", "10 metre speed probability [percentage]"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"2TP", "2 metre temperature probability [percentage]"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"MX2TP", "Maximum 2 metre temperature probability [percentage]"},
      /* 202 */ {"MN2TP", "Minimum 2 metre temperature probability [percentage]"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"TPP", "Total precipitation probability [percentage]"},
      /* 229 */ {"SWHP", "Significant wave height probability [percentage]"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"MWPP", "Mean wave period probability [percentage]"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_132[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"10FGI", "10 metre wind gust index [(-1 to 1)]"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"SFI", "Snowfall index [(-1 to 1)]"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"10WSI", "10 metre speed index [(-1 to 1)]"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"2TI", "2 metre temperature index [(-1 to 1)]"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"MAXSWHI", "Maximum of significant wave height index [(-1 to 1)]"},
      /* 201 */ {"MX2TI", "Maximum temperature at 2 metres index [(-1 to 1)]"},
      /* 202 */ {"MN2TI", "Minimum temperature at 2 metres index [(-1 to 1)]"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"TPI", "Total precipitation index [(-1 to 1)]"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_133[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"2TPLM10", "2m temperature probability less than -10 C [%]"},
      /* 2 */ {"2TPLM5", "2m temperature probability less than -5 C [%]"},
      /* 3 */ {"2TPL0", "2m temperature probability less than 0 C [%]"},
      /* 4 */ {"2TPL5", "2m temperature probability less than 5 C [%]"},
      /* 5 */ {"2TPL10", "2m temperature probability less than 10 C [%]"},
      /* 6 */ {"2TPG25", "2m temperature probability greater than 25 C [%]"},
      /* 7 */ {"2TPG30", "2m temperature probability greater than 30 C [%]"},
      /* 8 */ {"2TPG35", "2m temperature probability greater than 35 C [%]"},
      /* 9 */ {"2TPG40", "2m temperature probability greater than 40 C [%]"},
      /* 10 */ {"2TPG45", "2m temperature probability greater than 45 C [%]"},
      /* 11 */ {"MN2TPLM10", "Minimum 2 metre temperature probability less than -10 C [%]"},
      /* 12 */ {"MN2TPLM5", "Minimum 2 metre temperature probability less than -5 C [%]"},
      /* 13 */ {"MN2TPL0", "Minimum 2 metre temperature probability less than 0 C [%]"},
      /* 14 */ {"MN2TPL5", "Minimum 2 metre temperature probability less than 5 C [%]"},
      /* 15 */ {"MN2TPL10", "Minimum 2 metre temperature probability less than 10 C [%]"},
      /* 16 */ {"MX2TPG25", "Maximum 2 metre temperature probability greater than 25 C [%]"},
      /* 17 */ {"MX2TPG30", "Maximum 2 metre temperature probability greater than 30 C [%]"},
      /* 18 */ {"MX2TPG35", "Maximum 2 metre temperature probability greater than 35 C [%]"},
      /* 19 */ {"MX2TPG40", "Maximum 2 metre temperature probability greater than 40 C [%]"},
      /* 20 */ {"MX2TPG45", "Maximum 2 metre temperature probability greater than 45 C [%]"},
      /* 21 */ {"10SPG10", "10 metre wind speed probability of at least 10 m/s [%]"},
      /* 22 */ {"10SPG15", "10 metre wind speed probability of at least 15 m/s [%]"},
      /* 23 */ {"10SPG20", "10 metre wind speed probability of at least 20 m/s [%]"},
      /* 24 */ {"10SPG35", "10 metre wind speed probability of at least 35 m/s [%]"},
      /* 25 */ {"10SPG50", "10 metre wind speed probability of at least 50 m/s [%]"},
      /* 26 */ {"10GPG20", "10 metre wind gust probability of at least 20 m/s [%]"},
      /* 27 */ {"10GPG35", "10 metre wind gust probability of at least 35 m/s [%]"},
      /* 28 */ {"10GPG50", "10 metre wind gust probability of at least 50 m/s [%]"},
      /* 29 */ {"10GPG75", "10 metre wind gust probability of at least 75 m/s [%]"},
      /* 30 */ {"10GPG100", "10 metre wind gust probability of at least 100 m/s [%]"},
      /* 31 */ {"TPPG1", "Total precipitation probability of at least 1 mm [%]"},
      /* 32 */ {"TPPG5", "Total precipitation probability of at least 5 mm [%]"},
      /* 33 */ {"TPPG10", "Total precipitation probability of at least 10 mm [%]"},
      /* 34 */ {"TPPG20", "Total precipitation probability of at least 20 mm [%]"},
      /* 35 */ {"TPPG40", "Total precipitation probability of at least 40 mm [%]"},
      /* 36 */ {"TPPG60", "Total precipitation probability of at least 60 mm [%]"},
      /* 37 */ {"TPPG80", "Total precipitation probability of at least 80 mm [%]"},
      /* 38 */ {"TPPG100", "Total precipitation probability of at least 100 mm [%]"},
      /* 39 */ {"TPPG150", "Total precipitation probability of at least 150 mm [%]"},
      /* 40 */ {"TPPG200", "Total precipitation probability of at least 200 mm [%]"},
      /* 41 */ {"TPPG300", "Total precipitation probability of at least 300 mm [%]"},
      /* 42 */ {"SFPG1", "Snowfall probability of at least 1 mm [%]"},
      /* 43 */ {"SFPG5", "Snowfall probability of at least 5 mm [%]"},
      /* 44 */ {"SFPG10", "Snowfall probability of at least 10 mm [%]"},
      /* 45 */ {"SFPG20", "Snowfall probability of at least 20 mm [%]"},
      /* 46 */ {"SFPG40", "Snowfall probability of at least 40 mm [%]"},
      /* 47 */ {"SFPG60", "Snowfall probability of at least 60 mm [%]"},
      /* 48 */ {"SFPG80", "Snowfall probability of at least 80 mm [%]"},
      /* 49 */ {"SFPG100", "Snowfall probability of at least 100 mm [%]"},
      /* 50 */ {"SFPG150", "Snowfall probability of at least 150 mm [%]"},
      /* 51 */ {"SFPG200", "Snowfall probability of at least 200 mm [%]"},
      /* 52 */ {"SFPG300", "Snowfall probability of at least 300 mm [%]"},
      /* 53 */ {"TCCPG10", "Total Cloud Cover probability greater than 10% [%]"},
      /* 54 */ {"TCCPG20", "Total Cloud Cover probability greater than 20% [%]"},
      /* 55 */ {"TCCPG30", "Total Cloud Cover probability greater than 30% [%]"},
      /* 56 */ {"TCCPG40", "Total Cloud Cover probability greater than 40% [%]"},
      /* 57 */ {"TCCPG50", "Total Cloud Cover probability greater than 50% [%]"},
      /* 58 */ {"TCCPG60", "Total Cloud Cover probability greater than 60% [%]"},
      /* 59 */ {"TCCPG70", "Total Cloud Cover probability greater than 70% [%]"},
      /* 60 */ {"TCCPG80", "Total Cloud Cover probability greater than 80% [%]"},
      /* 61 */ {"TCCPG90", "Total Cloud Cover probability greater than 90% [%]"},
      /* 62 */ {"TCCPG99", "Total Cloud Cover probability greater than 99% [%]"},
      /* 63 */ {"HCCPG10", "High Cloud Cover probability greater than 10% [%]"},
      /* 64 */ {"HCCPG20", "High Cloud Cover probability greater than 20% [%]"},
      /* 65 */ {"HCCPG30", "High Cloud Cover probability greater than 30% [%]"},
      /* 66 */ {"HCCPG40", "High Cloud Cover probability greater than 40% [%]"},
      /* 67 */ {"HCCPG50", "High Cloud Cover probability greater than 50% [%]"},
      /* 68 */ {"HCCPG60", "High Cloud Cover probability greater than 60% [%]"},
      /* 69 */ {"HCCPG70", "High Cloud Cover probability greater than 70% [%]"},
      /* 70 */ {"HCCPG80", "High Cloud Cover probability greater than 80% [%]"},
      /* 71 */ {"HCCPG90", "High Cloud Cover probability greater than 90% [%]"},
      /* 72 */ {"HCCPG99", "High Cloud Cover probability greater than 99% [%]"},
      /* 73 */ {"MCCPG10", "Medium Cloud Cover probability greater than 10% [%]"},
      /* 74 */ {"MCCPG20", "Medium Cloud Cover probability greater than 20% [%]"},
      /* 75 */ {"MCCPG30", "Medium Cloud Cover probability greater than 30% [%]"},
      /* 76 */ {"MCCPG40", "Medium Cloud Cover probability greater than 40% [%]"},
      /* 77 */ {"MCCPG50", "Medium Cloud Cover probability greater than 50% [%]"},
      /* 78 */ {"MCCPG60", "Medium Cloud Cover probability greater than 60% [%]"},
      /* 79 */ {"MCCPG70", "Medium Cloud Cover probability greater than 70% [%]"},
      /* 80 */ {"MCCPG80", "Medium Cloud Cover probability greater than 80% [%]"},
      /* 81 */ {"MCCPG90", "Medium Cloud Cover probability greater than 90% [%]"},
      /* 82 */ {"MCCPG99", "Medium Cloud Cover probability greater than 99% [%]"},
      /* 83 */ {"LCCPG10", "Low Cloud Cover probability greater than 10% [%]"},
      /* 84 */ {"LCCPG20", "Low Cloud Cover probability greater than 20% [%]"},
      /* 85 */ {"LCCPG30", "Low Cloud Cover probability greater than 30% [%]"},
      /* 86 */ {"LCCPG40", "Low Cloud Cover probability greater than 40% [%]"},
      /* 87 */ {"LCCPG50", "Low Cloud Cover probability greater than 50% [%]"},
      /* 88 */ {"LCCPG60", "Low Cloud Cover probability greater than 60% [%]"},
      /* 89 */ {"LCCPG70", "Low Cloud Cover probability greater than 70% [%]"},
      /* 90 */ {"LCCPG80", "Low Cloud Cover probability greater than 80% [%]"},
      /* 91 */ {"LCCPG90", "Low Cloud Cover probability greater than 90% [%]"},
      /* 92 */ {"LCCPG99", "Low Cloud Cover probability greater than 99% [%]"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_ecmwf_140[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"MAXSWH", "Maximum of significant wave height [m]"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"UST", "U-component stokes drift [m s**-1]"},
      /* 216 */ {"VST", "V-component stokes drift [m s**-1]"},
      /* 217 */ {"TMAX", "Period corresponding to maximum individual wave height [s]"},
      /* 218 */ {"HMAX", "Maximum individual wave height [m]"},
      /* 219 */ {"WMB", "Model bathymetry [m]"},
      /* 220 */ {"MP1", "Mean wave period based on first moment [s]"},
      /* 221 */ {"MP2", "Mean wave period based on second moment [s]"},
      /* 222 */ {"WDW", "Wave spectral directional width []"},
      /* 223 */ {"P1WW", "Mean wave period based on first moment for wind waves [s]"},
      /* 224 */ {"P2WW", "Mean wave period based on second moment for wind waves [s]"},
      /* 225 */ {"DWWW", "Wave spectral directional width for wind waves []"},
      /* 226 */ {"P1PS", "Mean wave period based on first moment for swell [s]"},
      /* 227 */ {"P2PS", "Mean wave period based on second moment for swell [s]"},
      /* 228 */ {"DWPS", "Wave spectral directional width for swell []"},
      /* 229 */ {"SWH", "Significant wave height [m]"},
      /* 230 */ {"MWD", "Mean wave direction [degrees]"},
      /* 231 */ {"PP1D", "Peak period of 1D spectra [s]"},
      /* 232 */ {"MWP", "Mean wave period [s]"},
      /* 233 */ {"CDWW", "Coefficient of drag with waves []"},
      /* 234 */ {"SHWW", "Significant height of wind waves [m]"},
      /* 235 */ {"MDWW", "Mean direction of wind waves [degrees]"},
      /* 236 */ {"MPWW", "Mean period of wind waves [s]"},
      /* 237 */ {"SHTS", "Significant height of total swell [m]"},
      /* 238 */ {"MDTS", "Mean direction of total swell [degrees]"},
      /* 239 */ {"MPTS", "Mean period of total swell [s]"},
      /* 240 */ {"SDHS", "Standard deviation wave height [m]"},
      /* 241 */ {"MU10", "Mean of 10 metre wind speed [m s**-1]"},
      /* 242 */ {"MDWI", "Mean wind direction [degrees]"},
      /* 243 */ {"SDU", "Standard deviation of 10 metre wind speed [m s**-1]"},
      /* 244 */ {"MSQS", "Mean square slope of waves [dimensionless]"},
      /* 245 */ {"WIND", "10 metre wind speed [m s**-1]"},
      /* 246 */ {"AWH", "Altimeter wave height [m]"},
      /* 247 */ {"ACWH", "Altimeter corrected wave height [m]"},
      /* 248 */ {"ARRC", "Altimeter range relative correction []"},
      /* 249 */ {"DWI", "10 metre wind direction [degrees]"},
      /* 250 */ {"2DSP", "2D wave spectra (multiple) [m**2 s radian**-1]"},
      /* 251 */ {"2DFD", "2D wave spectra (single) [m**2 s radian**-1]"},
      /* 252 */ {"WSK", "Wave spectral kurtosis []"},
      /* 253 */ {"BFI", "Benjamin-Feir index []"},
      /* 254 */ {"WSP", "Wave spectral peakedness [s**-1]"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_150[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "Ocean potential temperature [deg C]"},
      /* 130 */ {"var130", "Ocean salinity [psu]"},
      /* 131 */ {"var131", "Ocean potential density [kg m**-3 -1000]"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "Ocean U velocity [m s**-1]"},
      /* 134 */ {"var134", "Ocean V velocity [m s**-1]"},
      /* 135 */ {"var135", "Ocean W velocity [m s**-1]"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "Richardson number []"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "U*V product [m s**-2]"},
      /* 140 */ {"var140", "U*T product [m s**-1 deg C]"},
      /* 141 */ {"var141", "V*T product [m s**-1 deg C]"},
      /* 142 */ {"var142", "U*U product [m s**-2]"},
      /* 143 */ {"var143", "V*V product [m s**-2]"},
      /* 144 */ {"var144", "UV - U~V~ [m s**-2]"},
      /* 145 */ {"var145", "UT - U~T~ [m s**-1 deg C]"},
      /* 146 */ {"var146", "VT - V~T~ [m s**-1 deg C]"},
      /* 147 */ {"var147", "UU - U~U~ [m s**-2]"},
      /* 148 */ {"var148", "VV - V~V~ [m s**-2]"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "Sea level [m]"},
      /* 153 */ {"var153", "Barotropic stream function []"},
      /* 154 */ {"var154", "Mixed layer depth [m]"},
      /* 155 */ {"var155", "Depth [m]"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "U stress [Pa]"},
      /* 169 */ {"var169", "V stress [Pa]"},
      /* 170 */ {"var170", "Turbulent kinetic energy input []"},
      /* 171 */ {"var171", "Net surface heat flux []"},
      /* 172 */ {"var172", "Surface solar radiation []"},
      /* 173 */ {"var173", "P-E []"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "Diagnosed sea surface temperature error [deg C]"},
      /* 181 */ {"var181", "Heat flux correction [W m**-2]"},
      /* 182 */ {"var182", "Observed sea surface temperature [deg C]"},
      /* 183 */ {"var183", "Observed heat flux [W m**-2]"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_151[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "In situ Temperature [deg C]"},
      /* 129 */ {"OCPT", "Ocean potential temperature [deg C]"},
      /* 130 */ {"S", "Salinity [psu]"},
      /* 131 */ {"OCU", "Ocean current zonal component [m s**-1]"},
      /* 132 */ {"OCV", "Ocean current meridional component [m s**-1]"},
      /* 133 */ {"OCW", "Ocean current vertical component [m s**-1]"},
      /* 134 */ {"MST", "Modulus of strain rate tensor [s**-1]"},
      /* 135 */ {"VVS", "Vertical viscosity [m**2 s**-1]"},
      /* 136 */ {"VDF", "Vertical diffusivity [m**2 s**-1]"},
      /* 137 */ {"DEP", "Bottom level Depth [m]"},
      /* 138 */ {"STH", "Sigma-theta [kg m**-3]"},
      /* 139 */ {"RN", "Richardson number []"},
      /* 140 */ {"UV", "UV product [m**2 s**-2]"},
      /* 141 */ {"UT", "UT product [m s**-1 degC]"},
      /* 142 */ {"VT", "VT product [m s**-1 deg C]"},
      /* 143 */ {"UU", "UU product [m**2 s**-2]"},
      /* 144 */ {"VV", "VV product [m**2 s**-2]"},
      /* 145 */ {"SL", "Sea level [m]"},
      /* 146 */ {"SL_1", "Sea level previous timestep [m]"},
      /* 147 */ {"BSF", "Barotropic stream function [m**3 s**-1]"},
      /* 148 */ {"MLD", "Mixed layer depth [m]"},
      /* 149 */ {"BTP", "Bottom Pressure (equivalent height) [m]"},
      /* 150 */ {"SH", "Steric height [m]"},
      /* 151 */ {"CRL", "Curl of Wind Stress [N m**-3]"},
      /* 152 */ {"var152", "Divergence of wind stress [Nm**-3]"},
      /* 153 */ {"TAX", "U stress [N m**-2]"},
      /* 154 */ {"TAY", "V stress [N m**-2]"},
      /* 155 */ {"TKI", "Turbulent kinetic energy input [W m**-2]"},
      /* 156 */ {"NSF", "Net surface heat flux [W m**-2]"},
      /* 157 */ {"ASR", "Absorbed solar radiation [W m**-2]"},
      /* 158 */ {"PME", "Precipitation - evaporation [m s**-1]"},
      /* 159 */ {"SST", "Specified sea surface temperature [deg C]"},
      /* 160 */ {"SHF", "Specified surface heat flux [W m**-2]"},
      /* 161 */ {"DTE", "Diagnosed sea surface temperature error [deg C]"},
      /* 162 */ {"HFC", "Heat flux correction [W m**-2]"},
      /* 163 */ {"20D", "20 degrees isotherm depth [m]"},
      /* 164 */ {"TAV300", "Average potential temperature in the upper 300m [degrees C]"},
      /* 165 */ {"UBA1", "Vertically integrated zonal velocity (previous time step) [m**2 s**-1]"},
      /* 166 */ {"VBA1", "Vertically Integrated meridional velocity (previous time step) [m**2 s**-1]"},
      /* 167 */ {"ZTR", "Vertically integrated zonal volume transport [m**2 s**-1]"},
      /* 168 */ {"MTR", "Vertically integrated meridional volume transport [m**2 s**-1]"},
      /* 169 */ {"ZHT", "Vertically integrated zonal heat transport [J m**-1 s**-1]"},
      /* 170 */ {"MHT", "Vertically integrated meridional heat transport [J m**-1 s**-1]"},
      /* 171 */ {"UMAX", "U velocity maximum [m s**-1]"},
      /* 172 */ {"DUMAX", "Depth of the velocity maximum [m]"},
      /* 173 */ {"SMAX", "Salinity maximum [psu]"},
      /* 174 */ {"DSMAX", "Depth of salinity maximum [m]"},
      /* 175 */ {"SAV300", "Average salinity in the upper 300m [psu]"},
      /* 176 */ {"LDP", "Layer Thickness at scalar points [m]"},
      /* 177 */ {"LDU", "Layer Thickness at vector points [m]"},
      /* 178 */ {"PTI", "Potential temperature increment [deg C]"},
      /* 179 */ {"PTAE", "Potential temperature analysis error [deg C]"},
      /* 180 */ {"BPT", "Background potential temperature [deg C]"},
      /* 181 */ {"APT", "Analysed potential temperature [deg C]"},
      /* 182 */ {"PTBE", "Potential temperature background error [deg C]"},
      /* 183 */ {"AS", "Analysed salinity [psu]"},
      /* 184 */ {"SALI", "Salinity increment [psu]"},
      /* 185 */ {"EBT", "Estimated Bias in Temperature [deg C]"},
      /* 186 */ {"EBS", "Estimated Bias in Salinity [psu]"},
      /* 187 */ {"UVI", "Zonal Velocity increment (from balance operator) [m/s per time step]"},
      /* 188 */ {"VVI", "Meridional Velocity increment (from balance operator) []"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"SUBI", "Salinity increment (from salinity data) [psu per time step]"},
      /* 191 */ {"SALE", "Salinity analysis error [psu]"},
      /* 192 */ {"BSAL", "Background Salinity [psu]"},
      /* 193 */ {"var193", "Reserved []"},
      /* 194 */ {"SALBE", "Salinity background error [psu]"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"EBTA", "Estimated temperature bias from assimilation [deg C]"},
      /* 200 */ {"EBSA", "Estimated salinity bias from assimilation [psu]"},
      /* 201 */ {"LTI", "Temperature increment from relaxation term [deg C per time step]"},
      /* 202 */ {"LSI", "Salinity increment from relaxation term []"},
      /* 203 */ {"BZPGA", "Bias in the zonal pressure gradient (applied) [Pa**m-1]"},
      /* 204 */ {"BMPGA", "Bias in the meridional pressure gradient (applied) [Pa**m-1]"},
      /* 205 */ {"EBTL", "Estimated temperature bias from relaxation [deg C]"},
      /* 206 */ {"EBSL", "Estimated salinity bias from relaxation [psu]"},
      /* 207 */ {"FGBT", "First guess bias in temperature [deg C]"},
      /* 208 */ {"FGBS", "First guess bias in salinity [psu]"},
      /* 209 */ {"BPA", "Applied bias in pressure [Pa]"},
      /* 210 */ {"FGBP", "FG bias in pressure [Pa]"},
      /* 211 */ {"PTA", "Bias in temperature(applied) [deg C]"},
      /* 212 */ {"PSA", "Bias in salinity (applied) [psu]"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", " []"},
};

const struct ParmTable parm_table_ecmwf_160[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"10FG", "10 metre wind gust during averaging time [m s**-1]"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"AT", "Atmospheric tide []"},
      /* 128 */ {"BV", "Budget values []"},
      /* 129 */ {"Z", "Geopotential [m**2 s**-2]"},
      /* 130 */ {"T", "Temperature [K]"},
      /* 131 */ {"U", "U velocity [m s**-1]"},
      /* 132 */ {"V", "V velocity [m s**-1]"},
      /* 133 */ {"Q", "Specific humidity [kg kg**-1]"},
      /* 134 */ {"SP", "Surface pressure [Pa]"},
      /* 135 */ {"W", "Vertical velocity [Pa s**-1]"},
      /* 136 */ {"TCW", "Total column water [kg m**-2]"},
      /* 137 */ {"PWC", "Precipitable water content [kg m**-2]"},
      /* 138 */ {"VO", "Vorticity (relative) [s**-1]"},
      /* 139 */ {"STL1", "Soil temperature level 1 [K]"},
      /* 140 */ {"SWL1", "Soil wetness level 1 [m]"},
      /* 141 */ {"SD", "Snow depth [m of water]"},
      /* 142 */ {"LSP", "Large-scale precipitation [kg m**-2 s**-1]"},
      /* 143 */ {"CP", "Convective precipitation [kg m**-2 s**-1]"},
      /* 144 */ {"SF", "Snowfall [kg m**-2 s**-1]"},
      /* 145 */ {"BLD", "Boundary layer dissipation [W m**-2]"},
      /* 146 */ {"SSHF", "Surface sensible heat flux [W m**-2]"},
      /* 147 */ {"SLHF", "Surface latent heat flux [W m**-2]"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"MSL", "Mean sea level pressure [Pa]"},
      /* 152 */ {"LNSP", "Logarithm of surface pressure []"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"D", "Divergence [s**-1]"},
      /* 156 */ {"GH", "Height [m]"},
      /* 157 */ {"R", "Relative humidity [(0 - 1)]"},
      /* 158 */ {"TSP", "Tendency of surface pressure [Pa s**-1]"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"TCC", "Total cloud cover [(0 - 1)]"},
      /* 165 */ {"10U", "10 metre U wind component [m s**-1]"},
      /* 166 */ {"10V", "10 metre V wind component [m s**-1]"},
      /* 167 */ {"2T", "2 metre temperature [K]"},
      /* 168 */ {"2D", "2 metre dewpoint temperature [K]"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"STL2", "Soil temperature level 2 [K]"},
      /* 171 */ {"SWL2", "Soil wetness level 2 [m]"},
      /* 172 */ {"LSM", "Land-sea mask [(0 - 1)]"},
      /* 173 */ {"SR", "Surface roughness [m]"},
      /* 174 */ {"AL", "Albedo [(0 - 1)]"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"SSR", "Surface solar radiation [W m**-2]"},
      /* 177 */ {"STR", "Surface thermal radiation [W m**-2]"},
      /* 178 */ {"TSR", "Top solar radiation [W m**-2]"},
      /* 179 */ {"TTR", "Top thermal radiation [W m**-2]"},
      /* 180 */ {"EWSS", "East-West surface stress [N m**-2 s**-1]"},
      /* 181 */ {"NSSS", "North-South surface stress [N m**-2 s**-1]"},
      /* 182 */ {"E", "Evaporation [kg m**-2 s**-1]"},
      /* 183 */ {"STL3", "Soil temperature level 3 [K]"},
      /* 184 */ {"SWL3", "Soil wetness level 3 [m]"},
      /* 185 */ {"CCC", "Convective cloud cover [(0 - 1)]"},
      /* 186 */ {"LCC", "Low cloud cover [(0 - 1)]"},
      /* 187 */ {"MCC", "Medium cloud cover [(0 - 1)]"},
      /* 188 */ {"HCC", "High cloud cover [(0 - 1)]"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"EWOV", "East-West component of sub-gridscale orographic variance [m**2]"},
      /* 191 */ {"NSOV", "North-South component of sub-gridscale orographic variance [m**2]"},
      /* 192 */ {"NWOV", "North-West/South-East component of sub-gridscale orographic variance [m**2]"},
      /* 193 */ {"NEOV", "North-East/South-West component of sub-gridscale orographic variance [m**2]"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"LGWS", "Latitudinal component of gravity wave stress [N m**-2 s]"},
      /* 196 */ {"MGWS", "Meridional component of gravity wave stress [N m**-2 s]"},
      /* 197 */ {"GWD", "Gravity wave dissipation [W m**-2 s]"},
      /* 198 */ {"SRC", "Skin reservoir content [m of water]"},
      /* 199 */ {"VEG", "Percentage of vegetation [%]"},
      /* 200 */ {"VSO", "Variance of sub-gridscale orography [m**2]"},
      /* 201 */ {"MX2T", "Maximum temperature at 2 metres during averaging time [K]"},
      /* 202 */ {"MN2T", "Minimium temperature at 2 metres during averaging time [K]"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"PAW", "Precipitation analysis weights []"},
      /* 205 */ {"RO", "Runoff [kg m**-2 s**-1]"},
      /* 206 */ {"ZZ", "Standard deviation of geopotential [m**2 s**-2]"},
      /* 207 */ {"TZ", "Covariance of temperature and geopotential [K m**2 s**-2]"},
      /* 208 */ {"TT", "Standard deviation of temperature [K]"},
      /* 209 */ {"QZ", "Covariance of specific humidity and geopotential [m**2 s**-2]"},
      /* 210 */ {"QT", "Covariance of specific humidity and temperature [K]"},
      /* 211 */ {"QQ", "Standard deviation of specific humidity [(0 - 1)]"},
      /* 212 */ {"UZ", "Covariance of U component and geopotential [m**3 s**-3]"},
      /* 213 */ {"UT", "Covariance of U component and temperature [K m s**-1]"},
      /* 214 */ {"UQ", "Covariance of U component and specific humidity [m s**-1]"},
      /* 215 */ {"UU", "Standard deviation of U velocity [m s**-1]"},
      /* 216 */ {"VZ", "Covariance of V component and geopotential [m**3 s**-3]"},
      /* 217 */ {"VT", "Covariance of V component and temperature [K m s**-1]"},
      /* 218 */ {"VQ", "Covariance of V component and specific humidity [m s**-1]"},
      /* 219 */ {"VU", "Covariance of V component and U component [m**2 s**-2]"},
      /* 220 */ {"VV", "Standard deviation of V component [m s**-1]"},
      /* 221 */ {"WZ", "Covariance of W component and geopotential [Pa m**2 s**-3]"},
      /* 222 */ {"WT", "Covariance of W component and temperature [K Pa s**-1]"},
      /* 223 */ {"WQ", "Covariance of W component and specific humidity [Pa s**-1]"},
      /* 224 */ {"WU", "Covariance of W component and U component [Pa m s**-2]"},
      /* 225 */ {"WV", "Covariance of W component and V component [Pa m s**-2]"},
      /* 226 */ {"WW", "Standard deviation of vertical velocity [Pa s**-1]"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"TP", "Total precipitation [m]"},
      /* 229 */ {"IEWS", "Instantaneous X surface stress [N m**-2]"},
      /* 230 */ {"INSS", "Instantaneous Y surface stress [N m**-2]"},
      /* 231 */ {"ISHF", "Instantaneous surface heat flux [W m**-2]"},
      /* 232 */ {"IE", "Instantaneous moisture flux [kg m**-2 s**-1]"},
      /* 233 */ {"ASQ", "Apparent surface humidity [kg kg**-1]"},
      /* 234 */ {"LSRH", "Logarithm of surface roughness length for heat []"},
      /* 235 */ {"SKT", "Skin temperature [K]"},
      /* 236 */ {"STL4", "Soil temperature level 4 [K]"},
      /* 237 */ {"SWL4", "Soil wetness level 4 [m]"},
      /* 238 */ {"TSN", "Temperature of snow layer [K]"},
      /* 239 */ {"CSF", "Convective snowfall [kg m**-2 s**-1]"},
      /* 240 */ {"LSF", "Large-scale snowfall [kg m**-2 s**-1]"},
      /* 241 */ {"CLWCER", "Cloud liquid water content [kg kg**-1]"},
      /* 242 */ {"CC", "Cloud cover [(0 - 1)]"},
      /* 243 */ {"FAL", "Forecast albedo []"},
      /* 244 */ {"FSR", "Forecast surface roughness [m]"},
      /* 245 */ {"FLSR", "Forecast logarithm of surface roughness for heat []"},
      /* 246 */ {"10WS", "10 metre wind speed [m s**-1]"},
      /* 247 */ {"MOFL", "Momentum flux [N m**-2]"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "Gravity wave dissipation flux [W m**-2]"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"HSD", "Heaviside beta function [(0 - 1)]"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_162[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "Surface geopotential [m**2 s**-2]"},
      /* 52 */ {"var52", "Surface pressure [Pa]"},
      /* 53 */ {"var53", "Vertical integral of mass of atmosphere [kg m**-2]"},
      /* 54 */ {"var54", "Vertical integral of temperature [K kg m**-2]"},
      /* 55 */ {"var55", "Vertical integral of total column water vapour [kg m**-2]"},
      /* 56 */ {"var56", "Vertical integral of total column liquid cloud water [kg m**-2]"},
      /* 57 */ {"var57", "Vertical integral of total column frozen cloud water [kg m**-2]"},
      /* 58 */ {"var58", "Vertical integral of total column ozone [kg m**-2]"},
      /* 59 */ {"var59", "Vertical integral of kinetic energy [J m**-2]"},
      /* 60 */ {"var60", "Vertical integral of thermal energy [J m**-2]"},
      /* 61 */ {"var61", "Vertical integral of dry static energy [J m**-2]"},
      /* 62 */ {"var62", "Vertical integral of moist static energy [J m**-2]"},
      /* 63 */ {"var63", "Vertical integral of total energy [J m**-2]"},
      /* 64 */ {"var64", "Vertical integral of energy conversion [W m**-2]"},
      /* 65 */ {"var65", "Vertical integral of eastward mass flux [kg m**-1 s**-1]"},
      /* 66 */ {"var66", "Vertical integral of northward mass flux [kg m**-1 s**-1]"},
      /* 67 */ {"var67", "Vertical integral of eastward kinetic energy flux [W m**-2]"},
      /* 68 */ {"var68", "Vertical integral of northward kinetic energy flux [W m**-2]"},
      /* 69 */ {"var69", "Vertical integral of eastward heat flux [W m**-2]"},
      /* 70 */ {"var70", "Vertical integral of northward heat flux [W m**-2]"},
      /* 71 */ {"var71", "Vertical integral of eastward water vapour flux [kg m**-1 s**-1]"},
      /* 72 */ {"var72", "Vertical integral of northward water vapour flux [kg m**-1 s**-1]"},
      /* 73 */ {"var73", "Vertical integral of eastward geopotential flux [W m**-2]"},
      /* 74 */ {"var74", "Vertical integral of northward geopotential flux [W m**-2]"},
      /* 75 */ {"var75", "Vertical integral of eastward total energy flux [W m**-2]"},
      /* 76 */ {"var76", "Vertical integral of northward total energy flux [W m**-2]"},
      /* 77 */ {"var77", "Vertical integral of eastward ozone flux [kg m**-1 s**-1]"},
      /* 78 */ {"var78", "Vertical integral of northward ozone flux [kg m**-1 s**-1]"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "Vertical integral of divergence of mass flux [kg m**-2 s**-1]"},
      /* 82 */ {"var82", "Vertical integral of divergence of kinetic energy flux [W m**-2]"},
      /* 83 */ {"var83", "Vertical integral of divergence of thermal energy flux [W m**-2]"},
      /* 84 */ {"var84", "Vertical integral of divergence of moisture flux [kg m**-2 s**-1]"},
      /* 85 */ {"var85", "Vertical integral of divergence of geopotential flux [W m**-2]"},
      /* 86 */ {"var86", "Vertical integral of divergence of total energy flux [W m**-2]"},
      /* 87 */ {"var87", "Vertical integral of divergence of ozone flux [kg m**-2 s**-1]"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "Tendency of short wave radiation [K]"},
      /* 101 */ {"var101", "Tendency of long wave radiation [K]"},
      /* 102 */ {"var102", "Tendency of clear sky short wave radiation [K]"},
      /* 103 */ {"var103", "Tendency of clear sky long wave radiation [K]"},
      /* 104 */ {"var104", "Updraught mass flux [kg m**-2]"},
      /* 105 */ {"var105", "Downdraught mass flux [kg m**-2]"},
      /* 106 */ {"var106", "Updraught detrainment rate [kg m**-3]"},
      /* 107 */ {"var107", "Downdraught detrainment rate [kg m**-3]"},
      /* 108 */ {"var108", "Total precipitation flux [kg m**-2]"},
      /* 109 */ {"var109", "Turbulent diffusion coefficient for heat [m**2]"},
      /* 110 */ {"var110", "Tendency of temperature due to physics [K]"},
      /* 111 */ {"var111", "Tendency of specific humidity due to physics [kg kg**-1]"},
      /* 112 */ {"var112", "Tendency of u component due to physics [m s**-1]"},
      /* 113 */ {"var113", "Tendency of v component due to physics [m s**-1]"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "Variance of geopotential [m**4 s**-4]"},
      /* 207 */ {"var207", "Covariance of geopotential/temperature [m**2 K s**-2]"},
      /* 208 */ {"var208", "Variance of temperature [K**2]"},
      /* 209 */ {"var209", "Covariance of geopotential/specific humidity [m**2 s**-2]"},
      /* 210 */ {"var210", "Covariance of temperature/specific humidity [K]"},
      /* 211 */ {"var211", "Variance of specific humidity []"},
      /* 212 */ {"var212", "Covariance of u component/geopotential [M**3 s**-3]"},
      /* 213 */ {"var213", "Covariance of u component/temperature [m s**-1 K]"},
      /* 214 */ {"var214", "Covariance of u component/specific humidity [m s**-1]"},
      /* 215 */ {"var215", "Variance of u component [m**2 s**-2]"},
      /* 216 */ {"var216", "Covariance of v component/geopotential [M**3 s**-3]"},
      /* 217 */ {"var217", "Covariance of v component/temperaure [m s**-1 K]"},
      /* 218 */ {"var218", "Covariance of v component/specific humidity [m s**-1]"},
      /* 219 */ {"var219", "Covariance of v component/u component [m**2 s**-2]"},
      /* 220 */ {"var220", "Variance of v component [m**2 s**-2]"},
      /* 221 */ {"var221", "Covariance of omega/geopotential [m**2 Pa s**-3]"},
      /* 222 */ {"var222", "Covariance of omega/temperature [Pa s**-1 K]"},
      /* 223 */ {"var223", "Covariance of omega/specific humidity [Pa s**-1]"},
      /* 224 */ {"var224", "Covariance of omega/u component [m Pa s**-2]"},
      /* 225 */ {"var225", "Covariance of omega/v component [m Pa s**-2]"},
      /* 226 */ {"var226", "Variance of omega [Pa**2 s**-2]"},
      /* 227 */ {"var227", "Variance of surface pressure [Pa**2]"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "Variance of relative humidity [dimensionless]"},
      /* 230 */ {"var230", "Covariance of u component/ozone [m s**-1]"},
      /* 231 */ {"var231", "Covariance of v component/ozone [m s**-1]"},
      /* 232 */ {"var232", "Covariance of omega/ozone [Pa s**-1]"},
      /* 233 */ {"var233", "Variance of ozone [dimensionless]"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_170[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"Z", "Geopotential [m**2 s**-2]"},
      /* 130 */ {"T", "Temperature [K]"},
      /* 131 */ {"U", "U velocity [m s**-1]"},
      /* 132 */ {"V", "V velocity [m s**-1]"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"VO", "Vorticity (relative) [s**-1]"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"SWL1", "Soil wetness level 1 [m]"},
      /* 141 */ {"SD", "Snow depth [m of water equivalent]"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"TSW", "Total soil moisture [m]"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"MSL", "Mean sea level pressure [Pa]"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"D", "Divergence [s**-1]"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"SWL2", "Soil wetness level 2 [m]"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"TTR", "Top thermal radiation [W m-2]"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"SWL3", "Soil wetness level 3 [m]"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"MX2T", "Maximum temperature at 2 metres [K]"},
      /* 202 */ {"MN2T", "Minimum temperature at 2 metres [K]"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"TP", "Total precipitation [m]"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_171[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"STRFA", "Stream function anomaly [m**2 s**-1]"},
      /* 2 */ {"VPOTA", "Velocity potential anomaly [m**2 s**-1]"},
      /* 3 */ {"var3", "Potential temperature [K]"},
      /* 4 */ {"var4", "Equivalent potential temperature [K]"},
      /* 5 */ {"var5", "Saturated equivalent potential temperature [K]"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "U component of divergent wind [m s**-1]"},
      /* 12 */ {"var12", "V component of divergent wind [m s**-1]"},
      /* 13 */ {"var13", "U component of rotational wind [m s**-1]"},
      /* 14 */ {"var14", "V component of rotational wind [m s**-1]"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "Unbalanced component of temperature [K]"},
      /* 22 */ {"var22", "Unbalanced component of logarithm of surface pressure []"},
      /* 23 */ {"var23", "Unbalanced component of divergence [s**-1]"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "Lake cover [(0 - 1)]"},
      /* 27 */ {"var27", "Low vegetation cover [(0 - 1)]"},
      /* 28 */ {"var28", "High vegetation cover [(0 - 1)]"},
      /* 29 */ {"var29", "Type of low vegetation []"},
      /* 30 */ {"var30", "Type of high vegetation []"},
      /* 31 */ {"var31", "Sea-ice cover [(0 - 1)]"},
      /* 32 */ {"var32", "Snow albedo [(0 - 1)]"},
      /* 33 */ {"var33", "Snow density [kg m**-3]"},
      /* 34 */ {"var34", "Sea surface temperature [K]"},
      /* 35 */ {"var35", "Ice surface temperature layer 1 [K]"},
      /* 36 */ {"var36", "Ice surface temperature layer 2 [K]"},
      /* 37 */ {"var37", "Ice surface temperature layer 3 [K]"},
      /* 38 */ {"var38", "Ice surface temperature layer 4 [K]"},
      /* 39 */ {"var39", "Volumetric soil water layer 1 [m**3 m**-3]"},
      /* 40 */ {"var40", "Volumetric soil water layer 2 [m**3 m**-3]"},
      /* 41 */ {"var41", "Volumetric soil water layer 3 [m**3 m**-3]"},
      /* 42 */ {"var42", "Volumetric soil water layer 4 [m**3 m**-3]"},
      /* 43 */ {"var43", "Soil type []"},
      /* 44 */ {"var44", "Snow evaporation [m of water]"},
      /* 45 */ {"var45", "Snowmelt [m of water]"},
      /* 46 */ {"var46", "Solar duration [s]"},
      /* 47 */ {"var47", "Direct solar radiation [w m**-2]"},
      /* 48 */ {"var48", "Magnitude of surface stress [N m**-2 s]"},
      /* 49 */ {"var49", "10 metre wind gust [m s**-1]"},
      /* 50 */ {"var50", "Large-scale precipitation fraction [s]"},
      /* 51 */ {"var51", "Maximum 2 metre temperature [K]"},
      /* 52 */ {"var52", "Minimum 2 metre temperature [K]"},
      /* 53 */ {"var53", "Montgomery potential [m**2 s**-2]"},
      /* 54 */ {"var54", "Pressure [Pa]"},
      /* 55 */ {"var55", "Mean 2 metre temperature in past 24 hours [K]"},
      /* 56 */ {"var56", "Mean 2 metre dewpoint temperature in past 24 hours [K]"},
      /* 57 */ {"var57", "Downward UV radiation at the surface [w m**-2]"},
      /* 58 */ {"var58", "Photosynthetically active radiation at the surface [w m**-2]"},
      /* 59 */ {"var59", "Convective available potential energy [J kg**-1]"},
      /* 60 */ {"var60", "Potential vorticity [K m**2 kg**-1 s**-1]"},
      /* 61 */ {"var61", "Total precipitation from observations [Millimetres*100 + number of stations]"},
      /* 62 */ {"var62", "Observation count []"},
      /* 63 */ {"var63", "Start time for skin temperature difference [s]"},
      /* 64 */ {"var64", "Finish time for skin temperature difference [s]"},
      /* 65 */ {"var65", "Skin temperature difference [K]"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"TCLWA", "Total column liquid water anomaly [kg m**-2]"},
      /* 79 */ {"TCIWA", "Total column ice water anomaly [kg m**-2]"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "Vertically integrated total energy [J m**-2]"},
      /* 126 */ {"var126", "Generic parameter for sensitive area prediction [Various]"},
      /* 127 */ {"var127", "Atmospheric tide []"},
      /* 128 */ {"var128", "Budget values []"},
      /* 129 */ {"ZA", "Geopotential anomaly [m**2 s**-2]"},
      /* 130 */ {"TA", "Temperature anomaly [K]"},
      /* 131 */ {"UA", "U velocity anomaly [m s**-1]"},
      /* 132 */ {"VA", "V velocity anomaly [m s**-1]"},
      /* 133 */ {"var133", "Specific humidity [kg kg**-1]"},
      /* 134 */ {"var134", "Surface pressure [Pa]"},
      /* 135 */ {"var135", "Vertical velocity [Pa s**-1]"},
      /* 136 */ {"TCWA", "Total column water [kg m**-2]"},
      /* 137 */ {"TCWVA", "Total column water vapour [kg m**-2]"},
      /* 138 */ {"var138", "Vorticity (relative) [s**-1]"},
      /* 139 */ {"STAL1", "Soil temperature level 1 [K]"},
      /* 140 */ {"var140", "Soil wetness level 1 [m of water]"},
      /* 141 */ {"var141", "Snow depth [m of water equivalent]"},
      /* 142 */ {"var142", "Stratiform precipitation (Large-scale precipitation) [m]"},
      /* 143 */ {"var143", "Convective precipitation [m]"},
      /* 144 */ {"var144", "Snowfall (convective + stratiform) [m of water equivalent]"},
      /* 145 */ {"var145", "Boundary layer dissipation [W m**-2 s]"},
      /* 146 */ {"var146", "Surface sensible heat flux [W m**-2 s]"},
      /* 147 */ {"var147", "Surface latent heat flux [W m**-2 s]"},
      /* 148 */ {"var148", "Charnock []"},
      /* 149 */ {"var149", "Surface net radiation [W m**-2 s]"},
      /* 150 */ {"var150", "Top net radiation []"},
      /* 151 */ {"MSLA", "Mean sea level pressure anomaly [Pa]"},
      /* 152 */ {"var152", "Logarithm of surface pressure []"},
      /* 153 */ {"var153", "Short-wave heating rate [K]"},
      /* 154 */ {"var154", "Long-wave heating rate [K]"},
      /* 155 */ {"var155", "Divergence [s**-1]"},
      /* 156 */ {"var156", "Height [m]"},
      /* 157 */ {"var157", "Relative humidity [%]"},
      /* 158 */ {"var158", "Tendency of surface pressure [Pa s**-1]"},
      /* 159 */ {"var159", "Boundary layer height [m]"},
      /* 160 */ {"var160", "Standard deviation of orography []"},
      /* 161 */ {"var161", "Anisotropy of sub-gridscale orography []"},
      /* 162 */ {"var162", "Angle of sub-gridscale orography [rad]"},
      /* 163 */ {"var163", "Slope of sub-gridscale orography []"},
      /* 164 */ {"TCCA", "Total cloud cover anomaly [(0 - 1)]"},
      /* 165 */ {"10UA", "10 metre U wind component anomaly [m s**-1]"},
      /* 166 */ {"10VA", "10 metre V wind component anomaly [m s**-1]"},
      /* 167 */ {"2TA", "2 metre temperature anomaly [K]"},
      /* 168 */ {"var168", "2 metre dewpoint temperature [K]"},
      /* 169 */ {"var169", "Surface solar radiation downwards [W m**-2 s]"},
      /* 170 */ {"var170", "Soil temperature level 2 [K]"},
      /* 171 */ {"var171", "Soil wetness level 2 [m of water]"},
      /* 172 */ {"var172", "Land-sea mask [(0 - 1)]"},
      /* 173 */ {"var173", "Surface roughness [m]"},
      /* 174 */ {"var174", "Albedo [(0 - 1)]"},
      /* 175 */ {"var175", "Surface thermal radiation downwards [W m**-2 s]"},
      /* 176 */ {"var176", "Surface solar radiation [W m**-2 s]"},
      /* 177 */ {"var177", "Surface thermal radiation [W m**-2 s]"},
      /* 178 */ {"var178", "Top solar radiation [W m**-2 s]"},
      /* 179 */ {"var179", "Top thermal radiation [W m**-2 s]"},
      /* 180 */ {"var180", "East-West surface stress [N m**-2 s]"},
      /* 181 */ {"var181", "North-South surface stress [N m**-2 s]"},
      /* 182 */ {"var182", "Evaporation [m of water]"},
      /* 183 */ {"var183", "Soil temperature level 3 [K]"},
      /* 184 */ {"var184", "Soil wetness level 3 [m of water]"},
      /* 185 */ {"var185", "Convective cloud cover [(0 - 1)]"},
      /* 186 */ {"var186", "Low cloud cover [(0 - 1)]"},
      /* 187 */ {"var187", "Medium cloud cover [(0 - 1)]"},
      /* 188 */ {"var188", "High cloud cover [(0 - 1)]"},
      /* 189 */ {"SUNDA", "Sunshine duration anomaly [s]"},
      /* 190 */ {"var190", "East-West component of sub-gridscale orographic variance [m**2]"},
      /* 191 */ {"var191", "North-South component of sub-gridscale orographic variance [m**2]"},
      /* 192 */ {"var192", "North-West/South-East component of sub-gridscale orographic variance [m**2]"},
      /* 193 */ {"var193", "North-East/South-West component of sub-gridscale orographic variance [m**2]"},
      /* 194 */ {"var194", "Brightness temperature [K]"},
      /* 195 */ {"var195", "Latitudinal component of gravity wave stress [N m**-2 s]"},
      /* 196 */ {"var196", "Meridional component of gravity wave stress [N m**-2 s]"},
      /* 197 */ {"var197", "Gravity wave dissipation [W m**-2 s]"},
      /* 198 */ {"var198", "Skin reservoir content [m of water]"},
      /* 199 */ {"var199", "Vegetation fraction [(0 - 1)]"},
      /* 200 */ {"var200", "Variance of sub-gridscale orography [m**2]"},
      /* 201 */ {"MX2TA", "Maximum temperature at 2 metres anomaly [K]"},
      /* 202 */ {"MN2TA", "Minimum temperature at 2 metres anomaly [K]"},
      /* 203 */ {"var203", "Ozone mass mixing ratio [kg kg**-1]"},
      /* 204 */ {"var204", "Precipitation analysis weights []"},
      /* 205 */ {"var205", "Runoff [m]"},
      /* 206 */ {"var206", "Total column ozone [kg m**-2]"},
      /* 207 */ {"var207", "10 metre wind speed [m s**-1]"},
      /* 208 */ {"var208", "Top net solar radiation, clear sky [W m**-2 s]"},
      /* 209 */ {"var209", "Top net thermal radiation, clear sky [W m**-2 s]"},
      /* 210 */ {"var210", "Surface net solar radiation, clear sky [W m**-2 s]"},
      /* 211 */ {"var211", "Surface net thermal radiation, clear sky [W m**-2 s]"},
      /* 212 */ {"var212", "Solar insolation [W m**-2]"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "Diabatic heating by radiation [K]"},
      /* 215 */ {"var215", "Diabatic heating by vertical diffusion [K]"},
      /* 216 */ {"var216", "Diabatic heating by cumulus convection [K]"},
      /* 217 */ {"var217", "Diabatic heating by large-scale condensation [K]"},
      /* 218 */ {"var218", "Vertical diffusion of zonal wind [m s**-1]"},
      /* 219 */ {"var219", "Vertical diffusion of meridional wind [m s**-1]"},
      /* 220 */ {"var220", "East-West gravity wave drag tendency [m s**-1]"},
      /* 221 */ {"var221", "North-South gravity wave drag tendency [m s**-1]"},
      /* 222 */ {"var222", "Convective tendency of zonal wind [m s**-1]"},
      /* 223 */ {"var223", "Convective tendency of meridional wind [m s**-1]"},
      /* 224 */ {"var224", "Vertical diffusion of humidity [kg kg**-1]"},
      /* 225 */ {"var225", "Humidity tendency by cumulus convection [kg kg**-1]"},
      /* 226 */ {"var226", "Humidity tendency by large-scale condensation [kg kg**-1]"},
      /* 227 */ {"var227", "Change from removal of negative humidity [kg kg**-1]"},
      /* 228 */ {"TPA", "Total precipitation anomaly [m]"},
      /* 229 */ {"var229", "Instantaneous X surface stress [N m**-2]"},
      /* 230 */ {"var230", "Instantaneous Y surface stress [N m**-2]"},
      /* 231 */ {"var231", "Instantaneous surface heat flux [W m**-2]"},
      /* 232 */ {"var232", "Instantaneous moisture flux [kg m**-2 s]"},
      /* 233 */ {"var233", "Apparent surface humidity [kg kg**-1]"},
      /* 234 */ {"var234", "Logarithm of surface roughness length for heat []"},
      /* 235 */ {"var235", "Skin temperature [K]"},
      /* 236 */ {"var236", "Soil temperature level 4 [K]"},
      /* 237 */ {"var237", "Soil wetness level 4 [m]"},
      /* 238 */ {"var238", "Temperature of snow layer [K]"},
      /* 239 */ {"var239", "Convective snowfall [m of water equivalent]"},
      /* 240 */ {"var240", "Large-scale snowfall [m of water equivalent]"},
      /* 241 */ {"var241", "Accumulated cloud fraction tendency [(-1 to 1)]"},
      /* 242 */ {"var242", "Accumulated liquid water tendency [(-1 to 1)]"},
      /* 243 */ {"var243", "Forecast albedo [(0 - 1)]"},
      /* 244 */ {"var244", "Forecast surface roughness [m]"},
      /* 245 */ {"var245", "Forecast logarithm of surface roughness for heat []"},
      /* 246 */ {"var246", "Cloud liquid water content [kg kg**-1]"},
      /* 247 */ {"var247", "Cloud ice water content [kg kg**-1]"},
      /* 248 */ {"var248", "Cloud cover [(0 - 1)]"},
      /* 249 */ {"var249", "Accumulated ice water tendency [(-1 to 1)]"},
      /* 250 */ {"var250", "Ice age [(0 - 1)]"},
      /* 251 */ {"var251", "Adiabatic tendency of temperature [K]"},
      /* 252 */ {"var252", "Adiabatic tendency of humidity [kg kg**-1]"},
      /* 253 */ {"var253", "Adiabatic tendency of zonal wind [m s**-1]"},
      /* 254 */ {"var254", "Adiabatic tendency of meridional wind [m s**-1]"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

/* ectable 172 from Geert Jan van Oldenborgh  */

const struct ParmTable parm_table_ecmwf_172[256] = {
    /* 0 */ {"var0", "undefined"},
    /* 1 */ {"var1", "undefined"},
    /* 2 */ {"var2", "undefined"},
    /* 3 */ {"var3", "undefined"},
    /* 4 */ {"var4", "undefined"},
    /* 5 */ {"var5", "undefined"},
    /* 6 */ {"var6", "undefined"},
    /* 7 */ {"var7", "undefined"},
    /* 8 */ {"var8", "undefined"},
    /* 9 */ {"var9", "undefined"},
    /* 10 */ {"var10", "undefined"},
    /* 11 */ {"var11", "undefined"},
    /* 12 */ {"var12", "undefined"},
    /* 13 */ {"var13", "undefined"},
    /* 14 */ {"var14", "undefined"},
    /* 15 */ {"var15", "undefined"},
    /* 16 */ {"var16", "undefined"},
    /* 17 */ {"var17", "undefined"},
    /* 18 */ {"var18", "undefined"},
    /* 19 */ {"var19", "undefined"},
    /* 20 */ {"var20", "undefined"},
    /* 21 */ {"var21", "undefined"},
    /* 22 */ {"var22", "undefined"},
    /* 23 */ {"var23", "undefined"},
    /* 24 */ {"var24", "undefined"},
    /* 25 */ {"var25", "undefined"},
    /* 26 */ {"var26", "undefined"},
    /* 27 */ {"var27", "undefined"},
    /* 28 */ {"var28", "undefined"},
    /* 29 */ {"var29", "undefined"},
    /* 30 */ {"var30", "undefined"},
    /* 31 */ {"var31", "undefined"},
    /* 32 */ {"var32", "undefined"},
    /* 33 */ {"var33", "undefined"},
    /* 34 */ {"var34", "undefined"},
    /* 35 */ {"var35", "undefined"},
    /* 36 */ {"var36", "undefined"},
    /* 37 */ {"var37", "undefined"},
    /* 38 */ {"var38", "undefined"},
    /* 39 */ {"var39", "undefined"},
    /* 40 */ {"var40", "undefined"},
    /* 41 */ {"var41", "undefined"},
    /* 42 */ {"var42", "undefined"},
    /* 43 */ {"var43", "undefined"},
    /* 44 */ {"SNOE", "Snow evaporation m of water s**-1"},
    /* 45 */ {"SNOM", "Snow melt m of water s**-1"},
    /* 46 */ {"var46", "undefined"},
    /* 47 */ {"var47", "undefined"},
    /* 48 */ {"MSS", "magnitude of surface stress N m**-2"},
    /* 49 */ {"var49", "undefined"},
    /* 50 */ {"", "Large-scale precipitation fraction -"},
    /* 51 */ {"var51", "undefined"},
    /* 52 */ {"var52", "undefined"},
    /* 53 */ {"var53", "undefined"},
    /* 54 */ {"var54", "undefined"},
    /* 55 */ {"var55", "undefined"},
    /* 56 */ {"var56", "undefined"},
    /* 57 */ {"var57", "undefined"},
    /* 58 */ {"var58", "undefined"},
    /* 59 */ {"var59", "undefined"},
    /* 60 */ {"var60", "undefined"},
    /* 61 */ {"var61", "undefined"},
    /* 62 */ {"var62", "undefined"},
    /* 63 */ {"var63", "undefined"},
    /* 64 */ {"var64", "undefined"},
    /* 65 */ {"var65", "undefined"},
    /* 66 */ {"var66", "undefined"},
    /* 67 */ {"var67", "undefined"},
    /* 68 */ {"var68", "undefined"},
    /* 69 */ {"var69", "undefined"},
    /* 70 */ {"var70", "undefined"},
    /* 71 */ {"var71", "undefined"},
    /* 72 */ {"var72", "undefined"},
    /* 73 */ {"var73", "undefined"},
    /* 74 */ {"var74", "undefined"},
    /* 75 */ {"var75", "undefined"},
    /* 76 */ {"var76", "undefined"},
    /* 77 */ {"var77", "undefined"},
    /* 78 */ {"var78", "undefined"},
    /* 79 */ {"var79", "undefined"},
    /* 80 */ {"var80", "undefined"},
    /* 81 */ {"var81", "undefined"},
    /* 82 */ {"var82", "undefined"},
    /* 83 */ {"var83", "undefined"},
    /* 84 */ {"var84", "undefined"},
    /* 85 */ {"var85", "undefined"},
    /* 86 */ {"var86", "undefined"},
    /* 87 */ {"var87", "undefined"},
    /* 88 */ {"var88", "undefined"},
    /* 89 */ {"var89", "undefined"},
    /* 90 */ {"var90", "undefined"},
    /* 91 */ {"var91", "undefined"},
    /* 92 */ {"var92", "undefined"},
    /* 93 */ {"var93", "undefined"},
    /* 94 */ {"var94", "undefined"},
    /* 95 */ {"var95", "undefined"},
    /* 96 */ {"var96", "undefined"},
    /* 97 */ {"var97", "undefined"},
    /* 98 */ {"var98", "undefined"},
    /* 99 */ {"var99", "undefined"},
    /* 100 */ {"var100", "undefined"},
    /* 101 */ {"var101", "undefined"},
    /* 102 */ {"var102", "undefined"},
    /* 103 */ {"var103", "undefined"},
    /* 104 */ {"var104", "undefined"},
    /* 105 */ {"var105", "undefined"},
    /* 106 */ {"var106", "undefined"},
    /* 107 */ {"var107", "undefined"},
    /* 108 */ {"var108", "undefined"},
    /* 109 */ {"var109", "undefined"},
    /* 110 */ {"var110", "undefined"},
    /* 111 */ {"var111", "undefined"},
    /* 112 */ {"var112", "undefined"},
    /* 113 */ {"var113", "undefined"},
    /* 114 */ {"var114", "undefined"},
    /* 115 */ {"var115", "undefined"},
    /* 116 */ {"var116", "undefined"},
    /* 117 */ {"var117", "undefined"},
    /* 118 */ {"var118", "undefined"},
    /* 119 */ {"var119", "undefined"},
    /* 120 */ {"var120", "undefined"},
    /* 121 */ {"var121", "undefined"},
    /* 122 */ {"var122", "undefined"},
    /* 123 */ {"var123", "undefined"},
    /* 124 */ {"var124", "undefined"},
    /* 125 */ {"var125", "undefined"},
    /* 126 */ {"var126", "undefined"},
    /* 127 */ {"var127", "undefined"},
    /* 128 */ {"var128", "undefined"},
    /* 129 */ {"var129", "undefined"},
    /* 130 */ {"var130", "undefined"},
    /* 131 */ {"var131", "undefined"},
    /* 132 */ {"var132", "undefined"},
    /* 133 */ {"var133", "undefined"},
    /* 134 */ {"var134", "undefined"},
    /* 135 */ {"var135", "undefined"},
    /* 136 */ {"var136", "undefined"},
    /* 137 */ {"var137", "undefined"},
    /* 138 */ {"var138", "undefined"},
    /* 139 */ {"var139", "undefined"},
    /* 140 */ {"var140", "undefined"},
    /* 141 */ {"var141", "undefined"},
    /* 142 */ {"LSP", "Large scale precipitation m s**-1"},
    /* 143 */ {"CP", "Convective precipitation m s**-1"},
    /* 144 */ {"SF", "Snowfall (convective + stratiform) m of water equivalent s**-1"},
    /* 145 */ {"BLD", "Boundary layer dissipation W m**-2"},
    /* 146 */ {"SSHF", "Surface sensible heat flux W m**-2"},
    /* 147 */ {"SLHF", "Surface latent heat flux W m**-2"},
    /* 148 */ {"SNR", "Surface net radiation W m**-2"},
    /* 149 */ {"var149", "undefined"},
    /* 150 */ {"var150", "undefined"},
    /* 151 */ {"var151", "undefined"},
    /* 152 */ {"var152", "undefined"},
    /* 153 */ {"SWHR", "Short-wave heating rate	K s**-1"},
    /* 154 */ {"LWHR", "Long-wave heating rate K s**-1"},
    /* 155 */ {"var155", "undefined"},
    /* 156 */ {"var156", "undefined"},
    /* 157 */ {"var157", "undefined"},
    /* 158 */ {"var158", "undefined"},
    /* 159 */ {"var159", "undefined"},
    /* 160 */ {"var160", "undefined"},
    /* 161 */ {"var161", "undefined"},
    /* 162 */ {"var162", "undefined"},
    /* 163 */ {"var163", "undefined"},
    /* 164 */ {"var164", "undefined"},
    /* 165 */ {"var165", "undefined"},
    /* 166 */ {"var166", "undefined"},
    /* 167 */ {"var167", "undefined"},
    /* 168 */ {"var168", "undefined"},
    /* 169 */ {"SSRD", "Surface solar radiation downwards W m**-2"},
    /* 170 */ {"var170", "undefined"},
    /* 171 */ {"var171", "undefined"},
    /* 172 */ {"var172", "undefined"},
    /* 173 */ {"var173", "undefined"},
    /* 174 */ {"var174", "undefined"},
    /* 175 */ {"STRD", "Surface thermal radiation downwards W m**-2"},
    /* 176 */ {"SSR", "Surface solar radiation W m**-2"},
    /* 177 */ {"STR", "Surface thermal radiation W m**-2"},
    /* 178 */ {"TSR", "Top solar radiation W m**-2"},
    /* 179 */ {"TTR", "Top thermal radiation W m-2"},
    /* 180 */ {"EWSS", "East-West surface stress N m**-2"},
    /* 181 */ {"NSSS", "North-South surface stress N m**-2"},
    /* 182 */ {"E", "Evaporation m of water s**-1"},
    /* 183 */ {"var183", "undefined"},
    /* 184 */ {"var184", "undefined"},
    /* 185 */ {"var185", "undefined"},
    /* 186 */ {"var186", "undefined"},
    /* 187 */ {"var187", "undefined"},
    /* 188 */ {"var188", "undefined"},
    /* 189 */ {"SUND", "Sunshine duration"},
    /* 190 */ {"var190", "undefined"},
    /* 191 */ {"var191", "undefined"},
    /* 192 */ {"var192", "undefined"},
    /* 193 */ {"var193", "undefined"},
    /* 194 */ {"var194", "undefined"},
    /* 195 */ {"LGWS", "Latitudinal component of gravity wave stress N m**-2"},
    /* 196 */ {"MGWS", "Meridional component of gravity wave stress N m**-2"},
    /* 197 */ {"GWD", "Gravity wave dissipation W m**-2"},
    /* 198 */ {"var198", "undefined"},
    /* 199 */ {"var199", "undefined"},
    /* 200 */ {"var200", "undefined"},
    /* 201 */ {"var201", "undefined"},
    /* 202 */ {"var202", "undefined"},
    /* 203 */ {"var203", "undefined"},
    /* 204 */ {"var204", "undefined"},
    /* 205 */ {"RO", "Runoff m s**-1"},
    /* 206 */ {"var206", "undefined"},
    /* 207 */ {"var207", "undefined"},
    /* 208 */ {"TSRC", "Top net solar radiation, clear sky 	W m**-2"},
    /* 209 */ {"TTRC", "Top net thermal radiation, clear sky 	W m**-2"},
    /* 210 */ {"SSRC", "Surface net solar radiation, clear sky W m**-2"},
    /* 211 */ {"STRC", "Surface net thermal radiation, clear sky W m**-2"},
    /* 212 */ {"SI", "Solar insolation W m**-2"},
    /* 213 */ {"var213", "undefined"},
    /* 214 */ {"var214", "undefined"},
    /* 215 */ {"var215", "undefined"},
    /* 216 */ {"var216", "undefined"},
    /* 217 */ {"var217", "undefined"},
    /* 218 */ {"var218", "undefined"},
    /* 219 */ {"var219", "undefined"},
    /* 220 */ {"var220", "undefined"},
    /* 221 */ {"var221", "undefined"},
    /* 222 */ {"var222", "undefined"},
    /* 223 */ {"var223", "undefined"},
    /* 224 */ {"var224", "undefined"},
    /* 225 */ {"var225", "undefined"},
    /* 226 */ {"var226", "undefined"},
    /* 227 */ {"var227", "undefined"},
    /* 228 */ {"TP", "Total precipitation m s**-1"},
    /* 229 */ {"var229", "undefined"},
    /* 230 */ {"var230", "undefined"},
    /* 231 */ {"var231", "undefined"},
    /* 232 */ {"var232", "undefined"},
    /* 233 */ {"var233", "undefined"},
    /* 234 */ {"var234", "undefined"},
    /* 235 */ {"var235", "undefined"},
    /* 236 */ {"var236", "undefined"},
    /* 237 */ {"var237", "undefined"},
    /* 238 */ {"var238", "undefined"},
    /* 239 */ {"CSF", "Convective snowfall m of water equivalent s**-1"},
    /* 240 */ {"LSF", "Large-scale snowfall 	m of water equivalent s**-1"},
    /* 241 */ {"var241", "undefined"},
    /* 242 */ {"var242", "undefined"},
    /* 243 */ {"var243", "undefined"},
    /* 244 */ {"var244", "undefined"},
    /* 245 */ {"var245", "undefined"},
    /* 246 */ {"var246", "undefined"},
    /* 247 */ {"var247", "undefined"},
    /* 248 */ {"var248", "undefined"},
    /* 249 */ {"var249", "undefined"},
    /* 250 */ {"var250", "undefined"},
    /* 251 */ {"var251", "undefined"},
    /* 252 */ {"var252", "undefined"},
    /* 253 */ {"var253", "undefined"},
    /* 254 */ {"var254", "undefined"},
    /* 255 */ {"var255", "undefined"},
};


const struct ParmTable parm_table_ecmwf_173[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "Snow evaporation anomaly [m of water s**-1]"},
      /* 45 */ {"var45", "Snowmelt anomaly [m of water s**-1]"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "Magnitude of surface stress anomaly [N m**-2]"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "Large-scale precipitation fraction anomaly []"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "Stratiform precipitation (Large-scale precipitation) anomaly [m s**-1]"},
      /* 143 */ {"var143", "Convective precipitation anomaly [m s**-1]"},
      /* 144 */ {"SFARA", "Snowfall (convective + stratiform) anomalous rate of accumulation [m of water equivalent s**-1]"},
      /* 145 */ {"var145", "Boundary layer dissipation anomaly [W m**-2]"},
      /* 146 */ {"var146", "Surface sensible heat flux anomaly [W m**-2]"},
      /* 147 */ {"var147", "Surface latent heat flux anomaly [W m**-2]"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "Surface net radiation anomaly [W m**-2]"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "Short-wave heating rate anomaly [K s**-1]"},
      /* 154 */ {"var154", "Long-wave heating rate anomaly [K s**-1]"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "Surface solar radiation downwards anomaly [W m**-2]"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "Surface thermal radiation downwards anomaly [W m**-2]"},
      /* 176 */ {"var176", "Surface solar radiation anomaly [W m**-2]"},
      /* 177 */ {"var177", "Surface thermal radiation anomaly [W m**-2]"},
      /* 178 */ {"var178", "Top solar radiation anomaly [W m**-2]"},
      /* 179 */ {"var179", "Top thermal radiation anomaly [W m**-2]"},
      /* 180 */ {"var180", "East-West surface stress anomaly [N m**-2]"},
      /* 181 */ {"var181", "North-South surface stress anomaly [N m**-2]"},
      /* 182 */ {"var182", "Evaporation anomaly [m of water s**-1]"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"SUNDARA", "Sunshine duration anomalous rate of accumulation [dimensionless]"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "Latitudinal component of gravity wave stress anomaly [N m**-2]"},
      /* 196 */ {"var196", "Meridional component of gravity wave stress anomaly [N m**-2]"},
      /* 197 */ {"var197", "Gravity wave dissipation anomaly [W m**-2]"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "Runoff anomaly [m s**-1]"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "Top net solar radiation, clear sky anomaly [W m**-2]"},
      /* 209 */ {"var209", "Top net thermal radiation, clear sky anomaly [W m**-2]"},
      /* 210 */ {"var210", "Surface net solar radiation, clear sky anomaly [W m**-2]"},
      /* 211 */ {"var211", "Surface net thermal radiation, clear sky anomaly [W m**-2]"},
      /* 212 */ {"var212", "Solar insolation anomaly [W m**-2 s**-1]"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"TPARA", "Total precipitation anomalous rate of accumulation [m s**-1]"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "Convective snowfall anomaly [m of water equivalent s**-1]"},
      /* 240 */ {"var240", "Large-scale snowfall anomaly [m of water equivalent s**-1]"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_174[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "Total soil moisture [m]"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"SRO", "Surface runoff [kg m**-2]"},
      /* 9 */ {"SSRO", "Sub-surface runoff [kg m**-2]"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "Fraction of sea-ice in sea [(0 - 1)]"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "Open-sea surface temperature [K]"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "Volumetric soil water layer 1 [m**3 m**-3]"},
      /* 40 */ {"var40", "Volumetric soil water layer 2 [m**3 m**-3]"},
      /* 41 */ {"var41", "Volumetric soil water layer 3 [m**3 m**-3]"},
      /* 42 */ {"var42", "Volumetric soil water layer 4 [m**3 m**-3]"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "10 metre wind gust over last 24 hours [m s**-1]"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "1.5m temperature - mean over last 24 hours [K]"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "Net primary productivity [kg C m**-2 s**-1]"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "10m U wind over land [m s**-1]"},
      /* 86 */ {"var86", "10m V wind over land [m s**-1]"},
      /* 87 */ {"var87", "1.5m temperature over land [K]"},
      /* 88 */ {"var88", "1.5m dewpoint temperature over land [K]"},
      /* 89 */ {"var89", "Top incoming solar radiation [W m**-2 s]"},
      /* 90 */ {"var90", "Top outgoing solar radiation [W m**-2 s]"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "Mean sea surface temperature [K]"},
      /* 95 */ {"var95", "1.5m specific humidity [kg kg**-1]"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "Sea-ice thickness [m]"},
      /* 99 */ {"var99", "Liquid water potential temperature [K]"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "Ocean ice concentration [(0 - 1)]"},
      /* 111 */ {"var111", "Ocean mean ice depth [m]"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "Soil temperature layer 1 [K]"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "Average potential temperature in upper 293.4m [degrees C]"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "1.5m temperature [K]"},
      /* 168 */ {"var168", "1.5m dewpoint temperature [K]"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "Soil temperature layer 2 [K]"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "Fractional land mask [(0 - 1)]"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "Average salinity in upper 293.4m [psu]"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "Soil temperature layer 3 [K]"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "1.5m temperature - maximum over last 24 hours [K]"},
      /* 202 */ {"var202", "1.5m temperature - minimum over last 24 hours [K]"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "Soil temperature layer 4 [K]"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_180[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"Z", "Geopotential [m**2 s**-2]"},
      /* 130 */ {"T", "Temperature [K]"},
      /* 131 */ {"U", "U velocity [m s**-1]"},
      /* 132 */ {"V", "V velocity [m s**-1]"},
      /* 133 */ {"Q", "Specific humidity [kg kg**-1]"},
      /* 134 */ {"SP", "Surface pressure [Pa]"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"TCWV", "Total column water vapour [kg m**-2]"},
      /* 138 */ {"VO", "Vorticity (relative) [s**-1]"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"SD", "Snow depth [m of water equivalent]"},
      /* 142 */ {"LSP", "Large-scale precipitation [m]"},
      /* 143 */ {"CP", "Convective precipitation [m]"},
      /* 144 */ {"SF", "Snowfall [m of water equivalent]"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"SSHF", "Surface sensible heat flux [W m**-2 s]"},
      /* 147 */ {"SLHF", "Surface latent heat flux [W m**-2 s]"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"TSW", "Total soil wetness [m]"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"MSL", "Mean sea level pressure [Pa]"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"D", "Divergence [s**-1]"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"TCC", "Total cloud cover [(0 - 1)]"},
      /* 165 */ {"10U", "10 metre U wind component [m s**-1]"},
      /* 166 */ {"10V", "10 metre V wind component [m s**-1]"},
      /* 167 */ {"2T", "2 metre temperature [K]"},
      /* 168 */ {"2D", "2 metre dewpoint temperature [K]"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"LSM", "Land-sea mask [(0 - 1)]"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"SSR", "Surface solar radiation [J m**-2 s]"},
      /* 177 */ {"STR", "Surface thermal radiation [J m**-2 s]"},
      /* 178 */ {"TSR", "Top solar radiation [J m**-2 s]"},
      /* 179 */ {"TTR", "Top thermal radiation [J m**-2 s]"},
      /* 180 */ {"EWSS", "East-West surface stress [N m**-2 s]"},
      /* 181 */ {"NSSS", "North-South surface stress [N m**-2 s]"},
      /* 182 */ {"E", "Evaporation [m of water]"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"RO", "Runoff [m]"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_190[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"Z", "Geopotential [m**2 s**-2]"},
      /* 130 */ {"T", "Temperature [K]"},
      /* 131 */ {"U", "U velocity [m s**-1]"},
      /* 132 */ {"V", "V velocity [m s**-1]"},
      /* 133 */ {"Q", "Specific humidity [kg kg**-1]"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"STL1", "Soil temperature level 1 [K]"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"SD", "Snow depth [m of water]"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"SSHF", "Surface sensible heat flux [W m**-2 s]"},
      /* 147 */ {"SLHF", "Surface latent heat flux [W m**-2 s]"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"MSL", "Mean sea level pressure [Pa]"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"TCC", "Total cloud cover [(0 - 1)]"},
      /* 165 */ {"10U", "10 metre U wind component [m s**-1]"},
      /* 166 */ {"10V", "10 metre V wind component [m s**-1]"},
      /* 167 */ {"2T", "2 metre temperature [K]"},
      /* 168 */ {"2D", "2 metre dewpoint temperature [K]"},
      /* 169 */ {"SSRD", "Downward surface solar radiation [W m**-2 s (W m**-2 for monthly means)]"},
      /* 170 */ {"CAP", "Field capacity [(0 - 1)]"},
      /* 171 */ {"WILT", "Wilting point [(0 - 1)]"},
      /* 172 */ {"LSM", "Land-sea mask [(0 - 1)]"},
      /* 173 */ {"SR", "Roughness length [(0 - 1)]"},
      /* 174 */ {"AL", "Albedo [(0 - 1)]"},
      /* 175 */ {"STRD", "Downward surface long wave radiation [W m**-2 s (W m**-2 for monthly means)]"},
      /* 176 */ {"SSR", "Surface net solar radiation [W m**-2 s (W m**-2 for monthly means)]"},
      /* 177 */ {"STR", "Surface net long wave radiation [W m**-2 s (W m**-2 for monthly means)]"},
      /* 178 */ {"TSR", "Top net solar radiation [W m**-2 s (W m**-2 for monthly means)]"},
      /* 179 */ {"TTR", "Top net long wave radiation [W m**-2 s (W m**-2 for monthly means)]"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"E", "Evaporation [m (m s**-1 for monthly means)]"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"MX2T", "Maximum 2 metre temperature [K]"},
      /* 202 */ {"MN2T", "Minimum 2 metre temperature [K]"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"TP", "Total precipitation [m (m s**-1 for monthly means)]"},
      /* 229 */ {"TSM", "Total soil moisture [m**3 m**-3]"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", " []"},
};

const struct ParmTable parm_table_ecmwf_200[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"STRF", "Stream function [m**2 s**-1]"},
      /* 2 */ {"VPOT", "Velocity potential [m**2 s**-1]"},
      /* 3 */ {"PT", "Potential temperature [K]"},
      /* 4 */ {"EQPT", "Equivalent potential temperature [K]"},
      /* 5 */ {"SEPT", "Saturated equivalent potential temperature [K]"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"UDVW", "U component of divergent wind [m s**-1]"},
      /* 12 */ {"VDVW", "V component of divergent wind [m s**-1]"},
      /* 13 */ {"URTW", "U component of rotational wind [m s**-1]"},
      /* 14 */ {"VRTW", "V component of rotational wind [m s**-1]"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"UCTP", "Unbalanced component of temperature [K]"},
      /* 22 */ {"UCLN", "Unbalanced component of logarithm of surface pressure []"},
      /* 23 */ {"UCDV", "Unbalanced component of divergence [s**-1]"},
      /* 24 */ {"var24", "Reserved for future unbalanced components []"},
      /* 25 */ {"var25", "Reserved for future unbalanced components []"},
      /* 26 */ {"CL", "Lake cover [(0 - 1)]"},
      /* 27 */ {"CVL", "Low vegetation cover [(0 - 1)]"},
      /* 28 */ {"CVH", "High vegetation cover [(0 - 1)]"},
      /* 29 */ {"TVL", "Type of low vegetation []"},
      /* 30 */ {"TVH", "Type of high vegetation []"},
      /* 31 */ {"CI", "Sea-ice cover [(0 - 1)]"},
      /* 32 */ {"ASN", "Snow albedo [(0 - 1)]"},
      /* 33 */ {"RSN", "Snow density [kg m**-3]"},
      /* 34 */ {"SSTK", "Sea surface temperature [K]"},
      /* 35 */ {"ISTL1", "Ice surface temperature layer 1 [K]"},
      /* 36 */ {"ISTL2", "Ice surface temperature layer 2 [K]"},
      /* 37 */ {"ISTL3", "Ice surface temperature layer 3 [K]"},
      /* 38 */ {"ISTL4", "Ice surface temperature layer 4 [K]"},
      /* 39 */ {"SWVL1", "Volumetric soil water layer 1 [m**3 m**-3]"},
      /* 40 */ {"SWVL2", "Volumetric soil water layer 2 [m**3 m**-3]"},
      /* 41 */ {"SWVL3", "Volumetric soil water layer 3 [m**3 m**-3]"},
      /* 42 */ {"SWVL4", "Volumetric soil water layer 4 [m**3 m**-3]"},
      /* 43 */ {"SLT", "Soil type []"},
      /* 44 */ {"ES", "Snow evaporation [m of water]"},
      /* 45 */ {"SMLT", "Snowmelt [m of water]"},
      /* 46 */ {"SDUR", "Solar duration [s]"},
      /* 47 */ {"DSRP", "Direct solar radiation [w m**-2]"},
      /* 48 */ {"MAGSS", "Magnitude of surface stress [N m**-2 s]"},
      /* 49 */ {"10FG", "10 metre wind gust [m s**-1]"},
      /* 50 */ {"LSPF", "Large-scale precipitation fraction [s]"},
      /* 51 */ {"MX2T24", "Maximum 2 metre temperature [K]"},
      /* 52 */ {"MN2T24", "Minimum 2 metre temperature [K]"},
      /* 53 */ {"MONT", "Montgomery potential [m**2 s**-2]"},
      /* 54 */ {"PRES", "Pressure [Pa]"},
      /* 55 */ {"MEAN2T24", "Mean 2 metre temperature in past 24 hours [K]"},
      /* 56 */ {"MN2D24", "Mean 2 metre dewpoint temperature in past 24 hours [K]"},
      /* 57 */ {"UVB", "Downward UV radiation at the surface [w m**-2 s]"},
      /* 58 */ {"PAR", "Photosynthetically active radiation at the surface [w m**-2 s]"},
      /* 59 */ {"CAPE", "Convective available potential energy [J kg**-1]"},
      /* 60 */ {"PV", "Potential vorticity [K m**2 kg**-1 s**-1]"},
      /* 61 */ {"TPO", "Total precipitation from observations [Millimetres*100 + number of stations]"},
      /* 62 */ {"OBCT", "Observation count []"},
      /* 63 */ {"var63", "Start time for skin temperature difference [s]"},
      /* 64 */ {"var64", "Finish time for skin temperature difference [s]"},
      /* 65 */ {"var65", "Skin temperature difference [K]"},
      /* 66 */ {"var66", "Leaf area index, low vegetation [m**2 / m**2]"},
      /* 67 */ {"var67", "Leaf area index, high vegetation [m**2 / m**2]"},
      /* 68 */ {"var68", "Minimum stomatal resistance, low vegetation [s m**-1]"},
      /* 69 */ {"var69", "Minimum stomatal resistance, high vegetation [s m**-1]"},
      /* 70 */ {"var70", "Biome cover, low vegetation [(0 - 1)]"},
      /* 71 */ {"var71", "Biome cover, high vegetation [(0 - 1)]"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "Total column liquid water [kg m**-2]"},
      /* 79 */ {"var79", "Total column ice water [kg m**-2]"},
      /* 80 */ {"var80", "Experimental product []"},
      /* 81 */ {"var81", "Experimental product []"},
      /* 82 */ {"var82", "Experimental product []"},
      /* 83 */ {"var83", "Experimental product []"},
      /* 84 */ {"var84", "Experimental product []"},
      /* 85 */ {"var85", "Experimental product []"},
      /* 86 */ {"var86", "Experimental product []"},
      /* 87 */ {"var87", "Experimental product []"},
      /* 88 */ {"var88", "Experimental product []"},
      /* 89 */ {"var89", "Experimental product []"},
      /* 90 */ {"var90", "Experimental product []"},
      /* 91 */ {"var91", "Experimental product []"},
      /* 92 */ {"var92", "Experimental product []"},
      /* 93 */ {"var93", "Experimental product []"},
      /* 94 */ {"var94", "Experimental product []"},
      /* 95 */ {"var95", "Experimental product []"},
      /* 96 */ {"var96", "Experimental product []"},
      /* 97 */ {"var97", "Experimental product []"},
      /* 98 */ {"var98", "Experimental product []"},
      /* 99 */ {"var99", "Experimental product []"},
      /* 100 */ {"var100", "Experimental product []"},
      /* 101 */ {"var101", "Experimental product []"},
      /* 102 */ {"var102", "Experimental product []"},
      /* 103 */ {"var103", "Experimental product []"},
      /* 104 */ {"var104", "Experimental product []"},
      /* 105 */ {"var105", "Experimental product []"},
      /* 106 */ {"var106", "Experimental product []"},
      /* 107 */ {"var107", "Experimental product []"},
      /* 108 */ {"var108", "Experimental product []"},
      /* 109 */ {"var109", "Experimental product []"},
      /* 110 */ {"var110", "Experimental product []"},
      /* 111 */ {"var111", "Experimental product []"},
      /* 112 */ {"var112", "Experimental product []"},
      /* 113 */ {"var113", "Experimental product []"},
      /* 114 */ {"var114", "Experimental product []"},
      /* 115 */ {"var115", "Experimental product []"},
      /* 116 */ {"var116", "Experimental product []"},
      /* 117 */ {"var117", "Experimental product []"},
      /* 118 */ {"var118", "Experimental product []"},
      /* 119 */ {"var119", "Experimental product []"},
      /* 120 */ {"var120", "Experimental product []"},
      /* 121 */ {"MX2T6", "Maximum temperature at 2 metres [K]"},
      /* 122 */ {"MN2T6", "Minimum temperature at 2 metres [K]"},
      /* 123 */ {"10FG6", "10 metre wind gust in the past 6 hours [m s**-1]"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "Vertically integrated total energy [J m**-2]"},
      /* 126 */ {"var126", "Generic parameter for sensitive area prediction [Various]"},
      /* 127 */ {"AT", "Atmospheric tide []"},
      /* 128 */ {"BV", "Budget values []"},
      /* 129 */ {"Z", "Geopotential [m**2 s**-2]"},
      /* 130 */ {"T", "Temperature [K]"},
      /* 131 */ {"U", "U velocity [m s**-1]"},
      /* 132 */ {"V", "V velocity [m s**-1]"},
      /* 133 */ {"Q", "Specific humidity [kg kg**-1]"},
      /* 134 */ {"SP", "Surface pressure [Pa]"},
      /* 135 */ {"W", "Vertical velocity [Pa s**-1]"},
      /* 136 */ {"TCW", "Total column water [kg m**-2]"},
      /* 137 */ {"TCWV", "Total column water vapour [kg m**-2]"},
      /* 138 */ {"VO", "Vorticity (relative) [s**-1]"},
      /* 139 */ {"STL1", "Soil temperature level 1 [K]"},
      /* 140 */ {"SWL1", "Soil wetness level 1 [m of water]"},
      /* 141 */ {"SD", "Snow depth [m of water equivalent]"},
      /* 142 */ {"LSP", "Stratiform precipitation (Large-scale precipitation) [m]"},
      /* 143 */ {"CP", "Convective precipitation [m]"},
      /* 144 */ {"SF", "Snowfall (convective + stratiform) [m of water equivalent]"},
      /* 145 */ {"BLD", "Boundary layer dissipation [W m**-2 s]"},
      /* 146 */ {"SSHF", "Surface sensible heat flux [W m**-2 s]"},
      /* 147 */ {"SLHF", "Surface latent heat flux [W m**-2 s]"},
      /* 148 */ {"CHNK", "Charnock []"},
      /* 149 */ {"SNR", "Surface net radiation [W m**-2 s]"},
      /* 150 */ {"TNR", "Top net radiation []"},
      /* 151 */ {"MSL", "Mean sea level pressure [Pa]"},
      /* 152 */ {"LNSP", "Logarithm of surface pressure []"},
      /* 153 */ {"SWHR", "Short-wave heating rate [K]"},
      /* 154 */ {"LWHR", "Long-wave heating rate [K]"},
      /* 155 */ {"D", "Divergence [s**-1]"},
      /* 156 */ {"GH", "Height [m]"},
      /* 157 */ {"R", "Relative humidity [%]"},
      /* 158 */ {"TSP", "Tendency of surface pressure [Pa s**-1]"},
      /* 159 */ {"BLH", "Boundary layer height [m]"},
      /* 160 */ {"SDOR", "Standard deviation of orography []"},
      /* 161 */ {"ISOR", "Anisotropy of sub-gridscale orography []"},
      /* 162 */ {"ANOR", "Angle of sub-gridscale orography [rad]"},
      /* 163 */ {"SLOR", "Slope of sub-gridscale orography []"},
      /* 164 */ {"TCC", "Total cloud cover [(0 - 1)]"},
      /* 165 */ {"10U", "10 metre U wind component [m s**-1]"},
      /* 166 */ {"10V", "10 metre V wind component [m s**-1]"},
      /* 167 */ {"2T", "2 metre temperature [K]"},
      /* 168 */ {"2D", "2 metre dewpoint temperature [K]"},
      /* 169 */ {"SSRD", "Surface solar radiation downwards [W m**-2 s]"},
      /* 170 */ {"STL2", "Soil temperature level 2 [K]"},
      /* 171 */ {"SWL2", "Soil wetness level 2 [m of water]"},
      /* 172 */ {"LSM", "Land-sea mask [(0 - 1)]"},
      /* 173 */ {"SR", "Surface roughness [m]"},
      /* 174 */ {"AL", "Albedo [(0 - 1)]"},
      /* 175 */ {"STRD", "Surface thermal radiation downwards [W m**-2 s]"},
      /* 176 */ {"SSR", "Surface solar radiation [W m**-2 s]"},
      /* 177 */ {"STR", "Surface thermal radiation [W m**-2 s]"},
      /* 178 */ {"TSR", "Top solar radiation [W m**-2 s]"},
      /* 179 */ {"TTR", "Top thermal radiation [W m**-2 s]"},
      /* 180 */ {"EWSS", "East-West surface stress [N m**-2 s]"},
      /* 181 */ {"NSSS", "North-South surface stress [N m**-2 s]"},
      /* 182 */ {"E", "Evaporation [m of water]"},
      /* 183 */ {"STL3", "Soil temperature level 3 [K]"},
      /* 184 */ {"SWL3", "Soil wetness level 3 [m of water]"},
      /* 185 */ {"CCC", "Convective cloud cover [(0 - 1)]"},
      /* 186 */ {"LCC", "Low cloud cover [(0 - 1)]"},
      /* 187 */ {"MCC", "Medium cloud cover [(0 - 1)]"},
      /* 188 */ {"HCC", "High cloud cover [(0 - 1)]"},
      /* 189 */ {"SUND", "Sunshine duration [s]"},
      /* 190 */ {"EWOV", "East-West component of sub-gridscale orographic variance [m**2]"},
      /* 191 */ {"NSOV", "North-South component of sub-gridscale orographic variance [m**2]"},
      /* 192 */ {"NWOV", "North-West/South-East component of sub-gridscale orographic variance [m**2]"},
      /* 193 */ {"NEOV", "North-East/South-West component of sub-gridscale orographic variance [m**2]"},
      /* 194 */ {"BTMP", "Brightness temperature [K]"},
      /* 195 */ {"LGWS", "Latitudinal component of gravity wave stress [N m**-2 s]"},
      /* 196 */ {"MGWS", "Meridional component of gravity wave stress [N m**-2 s]"},
      /* 197 */ {"GWD", "Gravity wave dissipation [W m**-2 s]"},
      /* 198 */ {"SRC", "Skin reservoir content [m of water]"},
      /* 199 */ {"VEG", "Vegetation fraction [(0 - 1)]"},
      /* 200 */ {"VSO", "Variance of sub-gridscale orography [m**2]"},
      /* 201 */ {"MX2T", "Maximum temperature at 2 metres since previous post-processing [K]"},
      /* 202 */ {"MN2T", "Minimum temperature at 2 metres since previous post-processing [K]"},
      /* 203 */ {"O3", "Ozone mass mixing ratio [kg kg**-1]"},
      /* 204 */ {"PAW", "Precipitation analysis weights []"},
      /* 205 */ {"RO", "Runoff [m]"},
      /* 206 */ {"TCO3", "Total column ozone [kg m**-2]"},
      /* 207 */ {"10SI", "10 metre wind speed [m s**-1]"},
      /* 208 */ {"TSRC", "Top net solar radiation, clear sky [W m**-2 s]"},
      /* 209 */ {"TTRC", "Top net thermal radiation, clear sky [W m**-2 s]"},
      /* 210 */ {"SSRC", "Surface net solar radiation, clear sky [W m**-2 s]"},
      /* 211 */ {"STRC", "Surface net thermal radiation, clear sky [W m**-2 s]"},
      /* 212 */ {"TISR", "TOA incident solar radiation [W m**-2 s]"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"DHR", "Diabatic heating by radiation [K]"},
      /* 215 */ {"DHVD", "Diabatic heating by vertical diffusion [K]"},
      /* 216 */ {"DHCC", "Diabatic heating by cumulus convection [K]"},
      /* 217 */ {"DHLC", "Diabatic heating large-scale condensation [K]"},
      /* 218 */ {"VDZW", "Vertical diffusion of zonal wind [m s**-1]"},
      /* 219 */ {"VDMW", "Vertical diffusion of meridional wind [m s**-1]"},
      /* 220 */ {"EWGD", "East-West gravity wave drag tendency [m s**-1]"},
      /* 221 */ {"NSGD", "North-South gravity wave drag tendency [m s**-1]"},
      /* 222 */ {"CTZW", "Convective tendency of zonal wind [m s**-1]"},
      /* 223 */ {"CTMW", "Convective tendency of meridional wind [m s**-1]"},
      /* 224 */ {"VDH", "Vertical diffusion of humidity [kg kg**-1]"},
      /* 225 */ {"HTCC", "Humidity tendency by cumulus convection [kg kg**-1]"},
      /* 226 */ {"HTLC", "Humidity tendency by large-scale condensation [kg kg**-1]"},
      /* 227 */ {"CRNH", "Change from removal of negative humidity [kg kg**-1]"},
      /* 228 */ {"TP", "Total precipitation [m]"},
      /* 229 */ {"IEWS", "Instantaneous X surface stress [N m**-2]"},
      /* 230 */ {"INSS", "Instantaneous Y surface stress [N m**-2]"},
      /* 231 */ {"ISHF", "Instantaneous surface heat flux [W m**-2]"},
      /* 232 */ {"IE", "Instantaneous moisture flux [kg m**-2 s]"},
      /* 233 */ {"ASQ", "Apparent surface humidity [kg kg**-1]"},
      /* 234 */ {"LSRH", "Logarithm of surface roughness length for heat []"},
      /* 235 */ {"SKT", "Skin temperature [K]"},
      /* 236 */ {"STL4", "Soil temperature level 4 [K]"},
      /* 237 */ {"SWL4", "Soil wetness level 4 [m]"},
      /* 238 */ {"TSN", "Temperature of snow layer [K]"},
      /* 239 */ {"CSF", "Convective snowfall [m of water equivalent]"},
      /* 240 */ {"LSF", "Large-scale snowfall [m of water equivalent]"},
      /* 241 */ {"ACF", "Accumulated cloud fraction tendency [(-1 to 1)]"},
      /* 242 */ {"ALW", "Accumulated liquid water tendency [(-1 to 1)]"},
      /* 243 */ {"FAL", "Forecast albedo [(0 - 1)]"},
      /* 244 */ {"FSR", "Forecast surface roughness [m]"},
      /* 245 */ {"FLSR", "Forecast logarithm of surface roughness for heat []"},
      /* 246 */ {"CLWC", "Cloud liquid water content [kg kg**-1]"},
      /* 247 */ {"CIWC", "Cloud ice water content [kg kg**-1]"},
      /* 248 */ {"CC", "Cloud cover [(0 - 1)]"},
      /* 249 */ {"AIW", "Accumulated ice water tendency [(-1 to 1)]"},
      /* 250 */ {"ICE", "Ice age [(0 - 1)]"},
      /* 251 */ {"ATTE", "Adiabatic tendency of temperature [K]"},
      /* 252 */ {"ATHE", "Adiabatic tendency of humidity [kg kg**-1]"},
      /* 253 */ {"ATZE", "Adiabatic tendency of zonal wind [m s**-1]"},
      /* 254 */ {"ATMW", "Adiabatic tendency of meridional wind [m s**-1]"},
      /* 255 */ {"var255", "Indicates a missing value []"},
};

const struct ParmTable parm_table_ecmwf_210[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"AERMR01", "Sea Salt Aerosol (0.03 - 0.5 um) Mixing Ratio [kg kg**-1]"},
      /* 2 */ {"AERMR02", "Sea Salt Aerosol (0.5 - 5 um) Mixing Ratio [kg kg**-1]"},
      /* 3 */ {"AERMR03", "Sea Salt Aerosol (5 - 20 um) Mixing Ratio [kg kg**-1]"},
      /* 4 */ {"AERMR04", "Dust Aerosol (0.03 - 0.55 um) Mixing Ratio [kg kg**-1]"},
      /* 5 */ {"AERMR05", "Dust Aerosol (0.55 - 0.9 um) Mixing Ratio [kg kg**-1]"},
      /* 6 */ {"AERMR06", "Dust Aerosol (0.9 - 20 um) Mixing Ratio [kg kg**-1]"},
      /* 7 */ {"AERMR07", "Hydrophobic Organic Matter Aerosol Mixing Ratio [kg kg**-1]"},
      /* 8 */ {"AERMR08", "Hydrophilic Organic Matter Aerosol Mixing Ratio [kg kg**-1]"},
      /* 9 */ {"AERMR09", "Hydrophobic Black Carbon Aerosol Mixing Ratio [kg kg**-1]"},
      /* 10 */ {"AERMR10", "Hydrophilic Black Carbon Aerosol Mixing Ratio [kg kg**-1]"},
      /* 11 */ {"AERMR11", "Sulphate Aerosol Mixing Ratio [kg kg**-1]"},
      /* 12 */ {"AERMR12", "Aerosol type 12 mixing ratio [kg kg**-1]"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"AERGN01", "Aerosol type 1 source/gain accumulated [kg m**-2]"},
      /* 17 */ {"AERGN02", "Aerosol type 2 source/gain accumulated [kg m**-2]"},
      /* 18 */ {"AERGN03", "Aerosol type 3 source/gain accumulated [kg m**-2]"},
      /* 19 */ {"AERGN04", "Aerosol type 4 source/gain accumulated [kg m**-2]"},
      /* 20 */ {"AERGN05", "Aerosol type 5 source/gain accumulated [kg m**-2]"},
      /* 21 */ {"AERGN06", "Aerosol type 6 source/gain accumulated [kg m**-2]"},
      /* 22 */ {"AERGN07", "Aerosol type 7 source/gain accumulated [kg m**-2]"},
      /* 23 */ {"AERGN08", "Aerosol type 8 source/gain accumulated [kg m**-2]"},
      /* 24 */ {"AERGN09", "Aerosol type 9 source/gain accumulated [kg m**-2]"},
      /* 25 */ {"AERGN10", "Aerosol type 10 source/gain accumulated [kg m**-2]"},
      /* 26 */ {"AERGN11", "Aerosol type 11 source/gain accumulated [kg m**-2]"},
      /* 27 */ {"AERGN12", "Aerosol type 12 source/gain accumulated [kg m**-2]"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"AERLS01", "Aerosol type 1 sink/loss accumulated [kg m**-2]"},
      /* 32 */ {"AERLS02", "Aerosol type 2 sink/loss accumulated [kg m**-2]"},
      /* 33 */ {"AERLS03", "Aerosol type 3 sink/loss accumulated [kg m**-2]"},
      /* 34 */ {"AERLS04", "Aerosol type 4 sink/loss accumulated [kg m**-2]"},
      /* 35 */ {"AERLS05", "Aerosol type 5 sink/loss accumulated [kg m**-2]"},
      /* 36 */ {"AERLS06", "Aerosol type 6 sink/loss accumulated [kg m**-2]"},
      /* 37 */ {"AERLS07", "Aerosol type 7 sink/loss accumulated [kg m**-2]"},
      /* 38 */ {"AERLS08", "Aerosol type 8 sink/loss accumulated [kg m**-2]"},
      /* 39 */ {"AERLS09", "Aerosol type 9 sink/loss accumulated [kg m**-2]"},
      /* 40 */ {"AERLS10", "Aerosol type 10 sink/loss accumulated [kg m**-2]"},
      /* 41 */ {"AERLS11", "Aerosol type 11 sink/loss accumulated [kg m**-2]"},
      /* 42 */ {"AERLS12", "Aerosol type 12 sink/loss accumulated [kg m**-2]"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"AERPR", "Aerosol precursor mixing ratio [kg kg**-1]"},
      /* 47 */ {"AERSM", "Aerosol small mode mixing ratio [kg kg**-1]"},
      /* 48 */ {"AERLG", "Aerosol large mode mixing ratio [kg kg**-1]"},
      /* 49 */ {"AODPR", "Aerosol precursor optical depth [dimensionless]"},
      /* 50 */ {"AODSM", "Aerosol small mode optical depth [dimensionless]"},
      /* 51 */ {"AODLG", "Aerosol large mode optical depth [dimensionless]"},
      /* 52 */ {"AERDEP", "Dust emission potential [kg s**2 m**-5]"},
      /* 53 */ {"AERLTS", "Lifting threshold speed [m s**-1]"},
      /* 54 */ {"AERSCC", "Soil clay content [%]"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"CO2", "Carbon Dioxide [kg kg**-1]"},
      /* 62 */ {"CH4", "Methane [kg kg**-1]"},
      /* 63 */ {"N2O", "Nitrous oxide [kg kg**-1]"},
      /* 64 */ {"TCCO2", "Total column Carbon Dioxide [kg m**-2]"},
      /* 65 */ {"TCCH4", "Total column Methane [kg m**-2]"},
      /* 66 */ {"TCN2O", "Total column Nitrous oxide [kg m**-2]"},
      /* 67 */ {"CO2OF", "Ocean flux of Carbon Dioxide [kg m**-2 s**-1]"},
      /* 68 */ {"CO2NBF", "Natural biosphere flux of Carbon Dioxide [kg m**-2 s**-1]"},
      /* 69 */ {"CO2APF", "Anthropogenic emissions of Carbon Dioxide [kg m**-2 s**-1]"},
      /* 70 */ {"CH4F", "Methane Surface Fluxes [kg m**-2 s**-1]"},
      /* 71 */ {"kCH4", "Methane loss rate due to radical hydroxyl (OH) [s**-1]"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"CO2FIRE", "Wildfire flux of Carbon Dioxide [kg m**-2 s**-1]"},
      /* 81 */ {"COFIRE", "Wildfire flux of Carbon Monoxide [kg m**-2 s**-1]"},
      /* 82 */ {"CH4FIRE", "Wildfire flux of Methane [kg m**-2 s**-1]"},
      /* 83 */ {"NMHCFIRE", "Wildfire flux of Non-Methane Hydro-Carbons [kg m**-2 s**-1]"},
      /* 84 */ {"H2FIRE", "Wildfire flux of Hydrogen [kg m**-2 s**-1]"},
      /* 85 */ {"NOXFIRE", "Wildfire flux of Nitrogen Oxides NOx [kg m**-2 s**-1]"},
      /* 86 */ {"N2OFIRE", "Wildfire flux of Nitrous Oxide [kg m**-2 s**-1]"},
      /* 87 */ {"PM2P5FIRE", "Wildfire flux of Particulate Matter PM2.5 [kg m**-2 s**-1]"},
      /* 88 */ {"TPMFIRE", "Wildfire flux of Total Particulate Matter [kg m**-2 s**-1]"},
      /* 89 */ {"TCFIRE", "Wildfire flux of Total Carbon in Aerosols [kg m**-2 s**-1]"},
      /* 90 */ {"OCFIRE", "Wildfire flux of Organic Carbon [kg m**-2 s**-1]"},
      /* 91 */ {"BCFIRE", "Wildfire flux of Black Carbon [kg m**-2 s**-1]"},
      /* 92 */ {"CFIRE", "Wildfire overall flux of burnt Carbon [kg m**-2 s**-1]"},
      /* 93 */ {"C4FFIRE", "Wildfire fraction of C4 plants [dimensionless]"},
      /* 94 */ {"VEGFIRE", "Wildfire vegetation map index [dimensionless]"},
      /* 95 */ {"CCFIRE", "Wildfire Combustion Completeness [dimensionless]"},
      /* 96 */ {"FLFIRE", "Wildfire Fuel Load"},
      /* 97 */ {"BFFIRE", "Wildfire fraction of area burnt [dimensionless]"},
      /* 98 */ {"OAFIRE", "Wildfire observed area [m**2]"},
      /* 99 */ {"FRPFIRE", "Wildfire radiative power [W m**-2]"},
      /* 100 */ {"CRFIRE", "Wildfire combustion rate [kg m**-2 s**-1]"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"NO2", "Nitrogen dioxide [kg kg**-1]"},
      /* 122 */ {"SO2", "Sulphur dioxide [kg kg**-1]"},
      /* 123 */ {"CO", "Carbon monoxide [kg kg**-1]"},
      /* 124 */ {"HCHO", "Formaldehyde [kg kg**-1]"},
      /* 125 */ {"TCNO2", "Total column Nitrogen dioxide [kg m**-2]"},
      /* 126 */ {"TCSO2", "Total column Sulphur dioxide [kg m**-2]"},
      /* 127 */ {"TCCO", "Total column Carbon monoxide [kg m**-2]"},
      /* 128 */ {"TCHCHO", "Total column Formaldehyde [kg m**-2]"},
      /* 129 */ {"NOX", "Nitrogen Oxides [kg kg**-1]"},
      /* 130 */ {"TCNOX", "Total Column Nitrogen Oxides [kg m**-2]"},
      /* 131 */ {"GRG1", "Reactive tracer 1 mass mixing ratio [kg kg**-1]"},
      /* 132 */ {"TCGRG1", "Total column GRG tracer 1 [kg m**-2]"},
      /* 133 */ {"GRG2", "Reactive tracer 2 mass mixing ratio [kg kg**-1]"},
      /* 134 */ {"TCGRG2", "Total column GRG tracer 2 [kg m**-2]"},
      /* 135 */ {"GRG3", "Reactive tracer 3 mass mixing ratio [kg kg**-1]"},
      /* 136 */ {"TCGRG3", "Total column GRG tracer 3 [kg m**-2]"},
      /* 137 */ {"GRG4", "Reactive tracer 4 mass mixing ratio [kg kg**-1]"},
      /* 138 */ {"TCGRG4", "Total column GRG tracer 4 [kg m**-2]"},
      /* 139 */ {"GRG5", "Reactive tracer 5 mass mixing ratio [kg kg**-1]"},
      /* 140 */ {"TCGRG5", "Total column GRG tracer 5 [kg m**-2]"},
      /* 141 */ {"GRG6", "Reactive tracer 6 mass mixing ratio [kg kg**-1]"},
      /* 142 */ {"TCGRG6", "Total column GRG tracer 6 [kg m**-2]"},
      /* 143 */ {"GRG7", "Reactive tracer 7 mass mixing ratio [kg kg**-1]"},
      /* 144 */ {"TCGRG7", "Total column GRG tracer 7 [kg m**-2]"},
      /* 145 */ {"GRG8", "Reactive tracer 8 mass mixing ratio [kg kg**-1]"},
      /* 146 */ {"TCGRG8", "Total column GRG tracer 8 [kg m**-2]"},
      /* 147 */ {"GRG9", "Reactive tracer 9 mass mixing ratio [kg kg**-1]"},
      /* 148 */ {"TCGRG9", "Total column GRG tracer 9 [kg m**-2]"},
      /* 149 */ {"GRG10", "Reactive tracer 10 mass mixing ratio [kg kg**-1]"},
      /* 150 */ {"TCGRG10", "Total column GRG tracer 10 [kg m**-2]"},
      /* 151 */ {"SFNOX", "Surface flux Nitrogen oxides [kg m**-2 s**-1]"},
      /* 152 */ {"SFNO2", "Surface flux Nitrogen dioxide [kg m**-2 s**-1]"},
      /* 153 */ {"SFSO2", "Surface flux Sulphur dioxide [kg m**-2 s**-1]"},
      /* 154 */ {"SFCO2", "Surface flux Carbon monoxide [kg m**-2 s**-1]"},
      /* 155 */ {"SFHCHO", "Surface flux Formaldehyde [kg m**-2 s**-1]"},
      /* 156 */ {"SFGO3", "Surface flux GEMS Ozone [kg m**-2 s**-1]"},
      /* 157 */ {"SFGR1", "Surface flux reactive tracer 1 [kg m**-2 s**-1]"},
      /* 158 */ {"SFGR2", "Surface flux reactive tracer 2 [kg m**-2 s**-1]"},
      /* 159 */ {"SFGR3", "Surface flux reactive tracer 3 [kg m**-2 s**-1]"},
      /* 160 */ {"SFGR4", "Surface flux reactive tracer 4 [kg m**-2 s**-1]"},
      /* 161 */ {"SFGR5", "Surface flux reactive tracer 5 [kg m**-2 s**-1]"},
      /* 162 */ {"SFGR6", "Surface flux reactive tracer 6 [kg m**-2 s**-1]"},
      /* 163 */ {"SFGR7", "Surface flux reactive tracer 7 [kg m**-2 s**-1]"},
      /* 164 */ {"SFGR8", "Surface flux reactive tracer 8 [kg m**-2 s**-1]"},
      /* 165 */ {"SFGR9", "Surface flux reactive tracer 9 [kg m**-2 s**-1]"},
      /* 166 */ {"SFGR10", "Surface flux reactive tracer 10 [kg m**-2 s**-1]"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"Ra", "Radon [kg kg**-1]"},
      /* 182 */ {"SF6", "Sulphur Hexafluoride [kg kg**-1]"},
      /* 183 */ {"TCRa", "Total column Radon [kg m**-2]"},
      /* 184 */ {"TCSF6", "Total column Sulphur Hexafluoride [kg m**-2]"},
      /* 185 */ {"SF6APF", "Anthropogenic Emissions of Sulphur Hexafluoride [kg m**-2 s**-1]"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"GO3", "GEMS Ozone [kg kg**-1]"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"GTCO3", "GEMS Total column ozone [kg m**-2]"},
      /* 207 */ {"AOD550", "Total Aerosol Optical Depth at 550nm [-]"},
      /* 208 */ {"SSAOD550", "Sea Salt Aerosol Optical Depth at 550nm [-]"},
      /* 209 */ {"DUAOD550", "Dust Aerosol Optical Depth at 550nm [-]"},
      /* 210 */ {"OMAOD550", "Organic Matter Aerosol Optical Depth at 550nm [-]"},
      /* 211 */ {"BCAOD550", "Black Carbon Aerosol Optical Depth at 550nm [-]"},
      /* 212 */ {"SUAOD550", "Sulphate Aerosol Optical Depth at 550nm [-]"},
      /* 213 */ {"AOD469", "Total Aerosol Optical Depth at 469nm [-]"},
      /* 214 */ {"AOD670", "Total Aerosol Optical Depth at 670nm [-]"},
      /* 215 */ {"AOD865", "Total Aerosol Optical Depth at 865nm [-]"},
      /* 216 */ {"AOD1240", "Total Aerosol Optical Depth at 1240nm [-]"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_ecmwf_211[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"var1", "undefined"},
      /* 2 */ {"var2", "undefined"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"var4", "undefined"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"var6", "undefined"},
      /* 7 */ {"var7", "undefined"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"var10", "undefined"},
      /* 11 */ {"var11", "undefined"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"var13", "undefined"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"var15", "undefined"},
      /* 16 */ {"var16", "undefined"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"var18", "undefined"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"var39", "undefined"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"AERPR", "Aerosol precursor mixing ratio [kg kg**-1]"},
      /* 47 */ {"AERSM", "Aerosol small mode mixing ratio [kg kg**-1]"},
      /* 48 */ {"AERLG", "Aerosol large mode mixing ratio [kg kg**-1]"},
      /* 49 */ {"AODPR", "Aerosol precursor optical depth [dimensionless]"},
      /* 50 */ {"AODSM", "Aerosol small mode optical depth [dimensionless]"},
      /* 51 */ {"AODLG", "Aerosol large mode optical depth [dimensionless]"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"CO2", "Carbon Dioxide [kg kg**-1]"},
      /* 62 */ {"CH4", "Methane [kg kg**-1]"},
      /* 63 */ {"N2O", "Nitrous oxide [kg kg**-1]"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"NO2", "Nitrogen dioxide [kg kg**-1]"},
      /* 122 */ {"SO2", "Sulphur dioxide [kg kg**-1]"},
      /* 123 */ {"CO", "Carbon monoxide [kg kg**-1]"},
      /* 124 */ {"HCHO", "Formaldehyde [kg kg**-1]"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"GO3", "GEMS Ozone [kg kg**-1]"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_ecmwf_228[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"CIN", "Convective inhibition [J kg**-1]"},
      /* 2 */ {"OROG", "Orography [m]"},
      /* 3 */ {"ZUST", "Friction velocity [m s**-1]"},
      /* 4 */ {"MEAN2T", "Mean temperature at 2 metres [K]"},
      /* 5 */ {"MEAN10WS", "Mean of 10 metre wind speed [m s**-1]"},
      /* 6 */ {"MEANTCC", "Mean total cloud cover [(0 - 1)]"},
      /* 7 */ {"DL", "Lake depth [m]"},
      /* 8 */ {"LMLT", "Lake mix-layer temperature [K]"},
      /* 9 */ {"LMLD", "Lake mix-layer depth [m]"},
      /* 10 */ {"LBLT", "Lake bottom temperature [K]"},
      /* 11 */ {"LTLT", "Lake total layer temperature [K]"},
      /* 12 */ {"LSHF", "Lake shape factor [dimensionless]"},
      /* 13 */ {"LICT", "Lake ice temperature [K]"},
      /* 14 */ {"LICD", "Lake ice depth [m]"},
      /* 15 */ {"DNDZN", "Minimum vertical gradient of refractivity inside trapping layer [m**-1]"},
      /* 16 */ {"DNDZA", "Mean vertical gradient of refractivity inside trapping layer [m**-1]"},
      /* 17 */ {"DCTB", "Duct base height [m]"},
      /* 18 */ {"TPLB", "Trapping layer base height [m]"},
      /* 19 */ {"TPLT", "Trapping layer top height [m]"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"var33", "undefined"},
      /* 34 */ {"var34", "undefined"},
      /* 35 */ {"var35", "undefined"},
      /* 36 */ {"var36", "undefined"},
      /* 37 */ {"var37", "undefined"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"SM", "Soil Moisture [kg m**-3]"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"var43", "undefined"},
      /* 44 */ {"var44", "undefined"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"var51", "undefined"},
      /* 52 */ {"var52", "undefined"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"var54", "undefined"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"var57", "undefined"},
      /* 58 */ {"var58", "undefined"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"var61", "undefined"},
      /* 62 */ {"var62", "undefined"},
      /* 63 */ {"var63", "undefined"},
      /* 64 */ {"var64", "undefined"},
      /* 65 */ {"var65", "undefined"},
      /* 66 */ {"var66", "undefined"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"var71", "undefined"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"var73", "undefined"},
      /* 74 */ {"var74", "undefined"},
      /* 75 */ {"var75", "undefined"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"var81", "undefined"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"var83", "undefined"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"var85", "undefined"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"var90", "undefined"},
      /* 91 */ {"var91", "undefined"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"var118", "undefined"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"var121", "undefined"},
      /* 122 */ {"var122", "undefined"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"var124", "undefined"},
      /* 125 */ {"var125", "undefined"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"U10N", "Neutral wind at 10 m x-component [m s**-1]"},
      /* 132 */ {"V10N", "Neutral wind at 10 m y-component [m s**-1]"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"VTNOWD", "V-tendency from non-orographic wave drag [m s**-2]"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"UTNOWD", "U-tendency from non-orographic wave drag [m s**-2]"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"ST", "Soil Temperature [K]"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"SD", "Snow Depth water equivalent [m]"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"SF", "Snow Fall water equivalent [kg m**-2]"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"TCC", "Total Cloud Cover [%]"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"CAP", "Field capacity [kg m**-3]"},
      /* 171 */ {"WILT", "Wilting point [kg m**-3]"},
      /* 172 */ {"var172", "undefined"},
      /* 173 */ {"var173", "undefined"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"TP", "Total Precipitation [kg m**-2]"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_nceptab_129[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land cover (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"PAOT", "Probability anomaly of temp [%]"},
      /* 129 */ {"PAOP", "Probability anomaly of precip [%]"},
      /* 130 */ {"CWR", "Probability of wetting rain > 0.1 in [%]"},
      /* 131 */ {"FRAIN", "Rain fraction of total liquid water []"},
      /* 132 */ {"FICE", "Ice fraction of total condensate []"},
      /* 133 */ {"FRIME", "Rime factor []"},
      /* 134 */ {"CUEFI", "Convective cloud efficiency []"},
      /* 135 */ {"TCOND", "Total condensate [kg/kg]"},
      /* 136 */ {"TCOLW", "Total column cloud water [kg/m/m]"},
      /* 137 */ {"TCOLI", "Total column cloud ice [kg/m/m]"},
      /* 138 */ {"TCOLR", "Total column rain [kg/m/m]"},
      /* 139 */ {"TCOLS", "Total column snow [kg/m/m]"},
      /* 140 */ {"TCOLC", "Total column condensate [kg/m/m]"},
      /* 141 */ {"PLPL", "Pressure of level from which parcel was lifted [Pa]"},
      /* 142 */ {"HLPL", "Height of level from which parcel was lifted [m]"},
      /* 143 */ {"CEMS", "Cloud Emissivity [fraction]"},
      /* 144 */ {"COPD", "Cloud Optical Depth [non-dim]"},
      /* 145 */ {"PSIZ", "Effective Particle size [microns]"},
      /* 146 */ {"TCWAT", "Total Water Cloud [%]"},
      /* 147 */ {"TCICE", "Total Ice Cloud [%]"},
      /* 148 */ {"WDIF", "Wind Difference [m/s]"},
      /* 149 */ {"WSTP", "Wave Steepness [non-dim]"},
      /* 150 */ {"PTAN", "Probability of Temp. above normal [%]"},
      /* 151 */ {"PTNN", "Probability of Temp. near normal [%]"},
      /* 152 */ {"PTBN", "Probability of Temp. below normal [%]"},
      /* 153 */ {"PPAN", "Probability of Precip. above normal [%]"},
      /* 154 */ {"PPNN", "Probability of Precip. near normal [%]"},
      /* 155 */ {"PPBN", "Probability of Precip. below normal [%]"},
      /* 156 */ {"PMTC", "Particulate matter (coarse) [ug/m^3]"},
      /* 157 */ {"PMTF", "Particulate matter (fine) [ug/m^3]"},
      /* 158 */ {"AETMP", "Analysis Error of Temperature [K]"},
      /* 159 */ {"AEDPT", "Analysis Error of Dew Point [K]"},
      /* 160 */ {"AESPH", "Analysis Error of Specific Humidity [kg/kg] wne"},
      /* 161 */ {"AEUWD", "Analysis Error of U-wind [m/s]"},
      /* 162 */ {"AEVWD", "Analysis Error of V-wind [m/s]"},
      /* 163 */ {"LPMTF", "Particulate matter (fine) [log10(ug/m^3)]"},
      /* 164 */ {"LIPMF", "Integrated Column Particulate matter (fine) [log10(ug/m^2)] wne"},
      /* 165 */ {"REFZR", "Derived radar reflectivity backscatter from rain [mm^6/m^3]"},
      /* 166 */ {"REFZI", "Derived radar reflectivity backscatter from ice [mm^6/m^3]"},
      /* 167 */ {"REFZC", "Derived radar reflectivity backscatter from parameterized convection [mm^6/m^3]"},
      /* 168 */ {"TCLSW", "Integrated supercooled liquid water [kg/m^2]"},
      /* 169 */ {"TCOLM", "Total Column Integrated Melting Ice [kg/m^2]"},
      /* 170 */ {"ELRDI", "Ellrod Index [non-dim]"},
      /* 171 */ {"TSEC", "Seconds prior to initial reference time [sec]"},
      /* 172 */ {"TSECA", "Seconds after initial reference time [sec]"},
      /* 173 */ {"NUM", "Number of samples/observations [non-dim]"},
      /* 174 */ {"AEPRS", "Analysis Error of Pressure [Pa]"},
      /* 175 */ {"ICSEV", "Icing Severity [non-dim]"},
      /* 176 */ {"ICPRB", "Icing Probability [non-dim]"},
      /* 177 */ {"LAVNI", "Low-level Aviation Interest [non-dim]"},
      /* 178 */ {"HAVNI", "High-level Aviation Interest [non-dim]"},
      /* 179 */ {"FLGHT", "Flight Category [non-dim]"},
      /* 180 */ {"OZCON", "Ozone concentration [ppb]"},
      /* 181 */ {"OZCAT", "Categorical ozone concentration [?]"},
      /* 182 */ {"VEDH", "vertical heat eddy diffusivity [m^2/s]"},
      /* 183 */ {"SIGV", "Sigma level value [non-dim]"},
      /* 184 */ {"EWGT", "Ensemble Weight [non-dim]"},
      /* 185 */ {"CICEL", "Confidence indicator - Ceiling [non-dim]"},
      /* 186 */ {"CIVIS", "Confidence indicator - Visibility [non-dim]"},
      /* 187 */ {"CIFLT", "Confidence indicator - Flight Category [non-dim]"},
      /* 188 */ {"LAVV", "Latitude of V wind component of velocity [deg]"},
      /* 189 */ {"LOVV", "Longitude of V wind component of velocity [deg]"},
      /* 190 */ {"USCT", "Scatterometer est. U wind component [m/s]"},
      /* 191 */ {"VSCT", "Scatterometer est. V wind component [m/s]"},
      /* 192 */ {"LAUV", "Latitude of U wind component of velocity [deg]"},
      /* 193 */ {"LOUV", "Longitude of U wind component of velocity [deg]"},
      /* 194 */ {"TCHP", "Tropical Cyclone Heat Potential [J/m^2]"},
      /* 195 */ {"DBSS", "Geometric Depth Below Sea Surface [m]"},
      /* 196 */ {"ODHA", "Ocean Dynamic Heat Anomaly [dynamic m]"},
      /* 197 */ {"OHC", "Ocean Heat Content [J/m^2]"},
      /* 198 */ {"SSHG", "Sea Surface Height Relative to Geoid [m]"},
      /* 199 */ {"SLTFL", "Salt flux [g/cm^2/s]"},
      /* 200 */ {"DUVB", "UV-B Downward Solar Flux [W/m^2]"},
      /* 201 */ {"CDUVB", "Clear Sky UV-B Downward Solar Flux [W/m^2]"},
      /* 202 */ {"THFLX", "Total downward heat flux at surface [W/m^2]"},
      /* 203 */ {"UVAR", "U velocity variance [m^2/s^2]"},
      /* 204 */ {"VVAR", "V velocity variance [m^2/s^2]"},
      /* 205 */ {"UVVCC", "UV Velocity Cross Correlation [m^2/s^2]"},
      /* 206 */ {"MCLS", "Meteorological Correlation Length Scale [m]"},
      /* 207 */ {"LAPP", "Latitude of pressure point [deg]"},
      /* 208 */ {"LOPP", "Longitude of pressure point [deg]"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"REFO", "Observed radar reflectivity [dbZ]"},
      /* 211 */ {"REFD", "Derived radar reflectivity [dbZ]"},
      /* 212 */ {"REFC", "Maximum/Composite radar reflectivity [dbZ]"},
      /* 213 */ {"SBT122", "Simulated Brightness Temperature for GOES12, Channel 2 [K]"},
      /* 214 */ {"SBT123", "Simulated Brightness Temperature for GOES12, Channel 3 [K]"},
      /* 215 */ {"SBT124", "Simulated Brightness Temperature for GOES12, Channel 4 [K]"},
      /* 216 */ {"SBT125", "Simulated Brightness Temperature for GOES12, Channel 5 [K]"},
      /* 217 */ {"MINRH", "Minimum Relative Humumidity [%]"},
      /* 218 */ {"MAXRH", "Maximum Relative Humumidity [%]"},
      /* 219 */ {"CEIL", "Ceiling [m]"},
      /* 220 */ {"PBLREG", "Planetary boundary layer regime []"},
      /* 221 */ {"SBC123", "Simulated brightness counts for GOES12, Channel 3 [byte]"},
      /* 222 */ {"SBC124", "Simulated brightness counts for GOES12, Channel 4 [byte]"},
      /* 223 */ {"RPRATE", "Rain precipitation rate [kg/m^2/s]"},
      /* 224 */ {"SPRATE", "Snow precipitation rate [kg/m^2/s]"},
      /* 225 */ {"FPRATE", "Freezing rain precipitation rate [kg/m^2/s]"},
      /* 226 */ {"IPRATE", "Ice pellets precipitation rate [kg/m^2/s]"},
      /* 227 */ {"UPHL", "Updraft Helicity [m^2/s^2]"},
      /* 228 */ {"SURGE", "Storm Surge [m]"},
      /* 229 */ {"ETSRG", "Extra-tropical storm Surge [m]"},
      /* 230 */ {"RHPW", "Relative humidity with respect to precip water [%]"},
      /* 231 */ {"OZMAX1", "Ozone daily max from 1-hour ave [ppbV]"},
      /* 232 */ {"OZMAX8", "Ozone daily max from 8-hour ave [ppbV]"},
      /* 233 */ {"PDMAX1", "PM 2.5 daily max from 1-hour ave [ug/m^3]"},
      /* 234 */ {"PDMAX24", "PM 2.5 daily max from 24-hour ave [ug/m^3]"},
      /* 235 */ {"MAXREF", "Hourly max of sim. reflect at 1km AGL [dbZ]"},
      /* 236 */ {"MXUPHL", "Hourly max updraft helicity 2-5km AGL [m^2/s^2]"},
      /* 237 */ {"MAXUVV", "Hourly max upward vert vel in lowest 400mb [m/s]"},
      /* 238 */ {"MAXDVV", "Hourly max downward vert fel in lowest 400mb [m/s]"},
      /* 239 */ {"MAXVIG", "Hourly max column graupel [kg/m^2]"},
      /* 240 */ {"RETOP", "Radar echo top (18.3 dbZ) [m]"},
      /* 241 */ {"VRATE", "Ventilation rate [m^2/s]"},
      /* 242 */ {"TCSRG20", "20% tropical cyclone storm exceedance [m]"},
      /* 243 */ {"TCSRG30", "30% tropical cyclone storm exceedance [m]"},
      /* 244 */ {"TCSRG40", "40% tropical cyclone storm exceedance [m]"},
      /* 245 */ {"TCSRG50", "50% tropical cyclone storm exceedance [m]"},
      /* 246 */ {"TCSRG60", "60% tropical cyclone storm exceedance [m]"},
      /* 247 */ {"TCSRG70", "70% tropical cyclone storm exceedance [m]"},
      /* 248 */ {"TCSRG80", "80% tropical cyclone storm exceedance [m]"},
      /* 249 */ {"TCSRG90", "90% tropical cyclone storm exceedance [m]"},
      /* 250 */ {"HINDEX", "Haines index []"},
      /* 251 */ {"DIFTEN", "Difference between 2 states in total energy norm [J/kg]"},
      /* 252 */ {"PSPCP", "Pseudo-precipitation [kg/m^2]"},
      /* 253 */ {"MAXUW", "U of hourly max 10m wind speed [m/s]"},
      /* 254 */ {"MAXVW", "V of hourly max 10m wind speed [m/s]"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_nceptab_140[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land cover (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"var132", "undefined"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"var144", "undefined"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"var146", "undefined"},
      /* 147 */ {"var147", "undefined"},
      /* 148 */ {"var148", "undefined"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"var159", "undefined"},
      /* 160 */ {"var160", "undefined"},
      /* 161 */ {"var161", "undefined"},
      /* 162 */ {"var162", "undefined"},
      /* 163 */ {"var163", "undefined"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"MEIP", "Mean icing potential []"},
      /* 169 */ {"MAIP", "Maximum icing potential []"},
      /* 170 */ {"MECTP", "Mean in-cloud turbulence potential []"},
      /* 171 */ {"MACTP", "Maximum in-cloud turbulence potential []"},
      /* 172 */ {"MECAT", "Mean cloud air turbulence potential []"},
      /* 173 */ {"MACAT", "Maximum cloud air turbulence potential []"},
      /* 174 */ {"CBHE", "Cumulonimbus horizontal extent [%]"},
      /* 175 */ {"PCBB", "Pressure at cumblonimbus base [Pa]"},
      /* 176 */ {"PCBT", "Pressure at cumblonimbus top [Pa]"},
      /* 177 */ {"PECBB", "Pressure at embedded cumblonimbus base [Pa]"},
      /* 178 */ {"PECBT", "Pressure at embedded cumblonimbus top [Pa]"},
      /* 179 */ {"HCBB", "ICAO height at cumblonimbus base [m]"},
      /* 180 */ {"HCBT", "ICAO height at cumblonimbus top [m]"},
      /* 181 */ {"HECBB", "ICAO height at embedded cumblonimbus base [m]"},
      /* 182 */ {"HECBT", "ICAO height at embedded cumblonimbus top [m]"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"var202", "undefined"},
      /* 203 */ {"var203", "undefined"},
      /* 204 */ {"var204", "undefined"},
      /* 205 */ {"var205", "undefined"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"var219", "undefined"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"var222", "undefined"},
      /* 223 */ {"var223", "undefined"},
      /* 224 */ {"var224", "undefined"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"var228", "undefined"},
      /* 229 */ {"var229", "undefined"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"var240", "undefined"},
      /* 241 */ {"var241", "undefined"},
      /* 242 */ {"var242", "undefined"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"var246", "undefined"},
      /* 247 */ {"var247", "undefined"},
      /* 248 */ {"var248", "undefined"},
      /* 249 */ {"var249", "undefined"},
      /* 250 */ {"var250", "undefined"},
      /* 251 */ {"var251", "undefined"},
      /* 252 */ {"var252", "undefined"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_nceptab_141[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land cover (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"EXTNC", "Aerosol Extinction Coefficient [1/km]"},
      /* 129 */ {"AOD", "Aerosol Optical Depth [-]"},
      /* 130 */ {"ASFTR", "Aerosol Asymmetry Factor [-]"},
      /* 131 */ {"SSALBD", "Aerosol Single Scatter Albedo [-]"},
      /* 132 */ {"BSCTRS", "Aerosol Back Scattering [1/km/sr]"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"NOy", "Total Inorganic and Organic Nitrates [ppbV]"},
      /* 141 */ {"NO", "Nitrogen Oxide [ppbV]"},
      /* 142 */ {"NO2", "Nitrogen Dioxide [ppbV]"},
      /* 143 */ {"N2O5", "Nitrogen Pentoxide [ppbV]"},
      /* 144 */ {"HNO3", "Nitric Acid [ppbV]"},
      /* 145 */ {"NO3", "Nitrogen Trioxide [ppbV]"},
      /* 146 */ {"PNA", "Peroxynitric Acid [ppbV]"},
      /* 147 */ {"HONO", "Nitrous Acid [ppbV]"},
      /* 148 */ {"CO", "Carbon Monoxide [ppbV]"},
      /* 149 */ {"NH3", "Ammonia [ppbV]"},
      /* 150 */ {"HCL", "Hydrogen Chloride [ppbV]"},
      /* 151 */ {"var151", "undefined"},
      /* 152 */ {"var152", "undefined"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"var154", "undefined"},
      /* 155 */ {"var155", "undefined"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"var157", "undefined"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"PAR", "Lumped Single-Bond Carbon Specie [ppbV]"},
      /* 160 */ {"ETHE", "Ethene [ppbV]"},
      /* 161 */ {"OLE", "Lumped Double-Bond Carbon Species Less Ethene [ppbV]"},
      /* 162 */ {"TOL", "Toluene [ppbV]"},
      /* 163 */ {"XYL", "Xylene [ppbV]"},
      /* 164 */ {"ISOP", "Isoprene [ppbV]"},
      /* 165 */ {"var165", "undefined"},
      /* 166 */ {"FORM", "Formaldehyde [ppbV]"},
      /* 167 */ {"ALD2", "Acetaldehyde & Higher Aldehydes [ppbV]"},
      /* 168 */ {"MGLY", "Methyl Glyoxal [ppbV]"},
      /* 169 */ {"CRES", "Cresol and Higher Molecular Weight Phenols [ppbV]"},
      /* 170 */ {"var170", "undefined"},
      /* 171 */ {"var171", "undefined"},
      /* 172 */ {"PAN", "Peroxyacyl Nitrate [ppbV]"},
      /* 173 */ {"NTR", "Lumped Gaseous Organic Nitrate [ppbV]"},
      /* 174 */ {"var174", "undefined"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"ROOH", "Esters [ppbV]"},
      /* 178 */ {"ETHOH", "Ethanol [ppbV]"},
      /* 179 */ {"METHOH", "Methanol [ppbV]"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"H2O2", "Hydrogen Peroxide [ppbV]"},
      /* 187 */ {"OH", "Hydroxyl Radical [ppbV]"},
      /* 188 */ {"HO2", "Hydroperoxyl Radical [ppbV]"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"var190", "undefined"},
      /* 191 */ {"var191", "undefined"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"ASO4", "Sulfate (SO4) Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 201 */ {"ANH4", "Ammonia (NH4) Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 202 */ {"ANO3", "Nitrate (NO3) Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 203 */ {"AORGA", "Organic Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 204 */ {"AORGPA", "Primarily Organic Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 205 */ {"AORGB", "Biogenically Originated Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 206 */ {"AEC", "Elemental Carbon Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 207 */ {"A25", "Unspecified Anthropogenic Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 208 */ {"AH2O", "Water Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 209 */ {"ANA", "Sodium Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 210 */ {"ACL", "Chloride Particulates ≤ 2.5 μm Diameter [μg/m^3]"},
      /* 211 */ {"var211", "undefined"},
      /* 212 */ {"var212", "undefined"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"ASO4K", "Sulfate (SO4) Particulates between 2.5 and 10 μm Diameter [μg/m^3]"},
      /* 217 */ {"ANAK", "Sodium (NA) Particulates between 2.5 and 10 μm Diameter [μg/m^3]"},
      /* 218 */ {"ACLK", "Chloride (CL) Particulates between 2.5 and 10 μm Diameter [μg/m^3]"},
      /* 219 */ {"ASEAS", "Sea Salt Originated Particulates between 2.5 and 10 μm Diameter [μg/m^3]"},
      /* 220 */ {"ASOIL", "Crustal Material Orginated Particulates between 2.5 and 10 μm Diameter [μg/m^3]"},
      /* 221 */ {"ACORS", "Particulates between 2.5 and 10 μm Diameter [μg/m^3]"},
      /* 222 */ {"NUMATKN", "Number Concentration Particulates between 2.5 and 0.1 μm Diameter [number/m^3]"},
      /* 223 */ {"NUMACC", "Number Concentration Particulates between 2.5 and 2.5 μm Diameter [number/m^3]"},
      /* 224 */ {"NUMCOR", "Number Concentration Particulates between 2.5 and 10 μm Diameter [number/m^3]"},
      /* 225 */ {"var225", "undefined"},
      /* 226 */ {"var226", "undefined"},
      /* 227 */ {"var227", "undefined"},
      /* 228 */ {"SRFATKN", "Surface Area Contributed by Particulates ≤ 0.1 μm Diameter [m2/m^3]"},
      /* 229 */ {"SRFACC", "Surface Area Contributed by Particulates between 0.1 and 2.5 μm Diameter [m2/m^3]"},
      /* 230 */ {"var230", "undefined"},
      /* 231 */ {"var231", "undefined"},
      /* 232 */ {"SO2", "Sulfur Dioxide [ppbV]"},
      /* 233 */ {"MSA", "Methanesulfonic Acid [Kg/Kg]"},
      /* 234 */ {"TSO4", "Total Sulfate Particulates (Fine ands Coarse) [μg/m^3]"},
      /* 235 */ {"DMS", "Dimethylsulfide [Kg/Kg]"},
      /* 236 */ {"var236", "undefined"},
      /* 237 */ {"var237", "undefined"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"var239", "undefined"},
      /* 240 */ {"DU1", "Dust Particulates between 0.2 - 2.0 μm Diameter [Kg/Kg]"},
      /* 241 */ {"DU2", "Dust Particulates between 2.0 - 3.6 μm Diameter [Kg/Kg]"},
      /* 242 */ {"DU3", "Dust Particulates between 3.6 - 6.0 μm Diameter [Kg/Kg]"},
      /* 243 */ {"DU4", "Dust Particulates between 6.0 - 12.0 μm Diameter [Kg/Kg]"},
      /* 244 */ {"DU5", "Dust Particulates between 12.0 - 20.0 μm Diameter [Kg/Kg]"},
      /* 245 */ {"SS1", "Sea Salt Particulates between 0.2 - 1.0 μm Diameter [Kg/Kg]"},
      /* 246 */ {"SS2", "Sea Salt Particulates between 1.0 - 3.0 μm Diameter [Kg/Kg]"},
      /* 247 */ {"SS3", "Sea Salt Particulates between 3.0 - 10.0 μm Diameter [Kg/Kg]"},
      /* 248 */ {"SS4", "Sea Salt Particulates between 10.0 - 20.0 μm Diameter [Kg/Kg]"},
      /* 249 */ {"OCDRY", "Hydrophobic Organic Carbon [Kg/Kg]"},
      /* 250 */ {"OCWET", "Hydrophilic Organic Carbon [Kg/Kg]"},
      /* 251 */ {"BCDRY", "Hydrophobic Black Carbon [Kg/Kg]"},
      /* 252 */ {"BCWET", "Hydrophilic Black Carbon [Kg/Kg]"},
      /* 253 */ {"var253", "undefined"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_mdl_nceptab[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"PTEND", "Pressure tendency [Pa/s]"},
      /* 4 */ {"PVORT", "Pot. vorticity [km^2/kg/s]"},
      /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height [M]"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"DIST", "Geometric height [m]"},
      /* 9 */ {"HSTDV", "Std dev of height [m]"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temp. [K]"},
      /* 12 */ {"VTMP", "Virtual temp. [K]"},
      /* 13 */ {"POT", "Potential temp. [K]"},
      /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temp. [K]"},
      /* 15 */ {"TMAX", "Max. temp. [K]"},
      /* 16 */ {"TMIN", "Min. temp. [K]"},
      /* 17 */ {"DPT", "Dew point temp. [K]"},
      /* 18 */ {"DEPR", "Dew point depression [K]"},
      /* 19 */ {"LAPR", "Lapse rate [K/m]"},
      /* 20 */ {"VIS", "Visibility [m]"},
      /* 21 */ {"RDSP1", "Radar spectra (1) [non-dim]"},
      /* 22 */ {"RDSP2", "Radar spectra (2) [non-dim]"},
      /* 23 */ {"RDSP3", "Radar spectra (3) [non-dim]"},
      /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa) [K]"},
      /* 25 */ {"TMPA", "Temp. anomaly [K]"},
      /* 26 */ {"PRESA", "Pressure anomaly [Pa]"},
      /* 27 */ {"GPA", "Geopotential height anomaly [gpm]"},
      /* 28 */ {"WVSP1", "Wave spectra (1) [non-dim]"},
      /* 29 */ {"WVSP2", "Wave spectra (2) [non-dim]"},
      /* 30 */ {"WVSP3", "Wave spectra (3) [non-dim]"},
      /* 31 */ {"WDIR", "Wind direction [deg]"},
      /* 32 */ {"WIND", "Wind speed [m/s]"},
      /* 33 */ {"UGRD", "u wind [m/s]"},
      /* 34 */ {"VGRD", "v wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"SGCVV", "Sigma coord. vertical velocity [/s]"},
      /* 39 */ {"VVEL", "Pressure vertical velocity [Pa/s]"},
      /* 40 */ {"DZDT", "Geometric vertical velocity [m/s]"},
      /* 41 */ {"ABSV", "Absolute vorticity [/s]"},
      /* 42 */ {"ABSD", "Absolute divergence [/s]"},
      /* 43 */ {"RELV", "Relative vorticity [/s]"},
      /* 44 */ {"RELD", "Relative divergence [/s]"},
      /* 45 */ {"VUCSH", "Vertical u shear [/s]"},
      /* 46 */ {"VVCSH", "Vertical v shear [/s]"},
      /* 47 */ {"DIRC", "Direction of current [deg]"},
      /* 48 */ {"SPC", "Speed of current [m/s]"},
      /* 49 */ {"UOGRD", "u of current [m/s]"},
      /* 50 */ {"VOGRD", "v of current [m/s]"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"MIXR", "Humidity mixing ratio [kg/kg]"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"VAPP", "Vapor pressure [Pa]"},
      /* 56 */ {"SATD", "Saturation deficit [Pa]"},
      /* 57 */ {"EVP", "Evaporation [kg/m^2]"},
      /* 58 */ {"CICE", "Cloud Ice [kg/m^2]"},
      /* 59 */ {"PRATE", "Precipitation rate [kg/m^2/s]"},
      /* 60 */ {"TSTM", "Thunderstorm probability [%]"},
      /* 61 */ {"APCP", "Total precipitation [kg/m^2]"},
      /* 62 */ {"NCPCP", "Large scale precipitation [kg/m^2]"},
      /* 63 */ {"ACPCP", "Convective precipitation [kg/m^2]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equiv. [kg/m^2/s]"},
      /* 65 */ {"WEASD", "Accum. snow [kg/m^2]"},
      /* 66 */ {"SNOD", "Snow depth [m]"},
      /* 67 */ {"MIXHT", "Mixed layer depth [m]"},
      /* 68 */ {"TTHDP", "Transient thermocline depth [m]"},
      /* 69 */ {"MTHD", "Main thermocline depth [m]"},
      /* 70 */ {"MTHA", "Main thermocline anomaly [m]"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"CDCON", "Convective cloud cover [%]"},
      /* 73 */ {"LCDC", "Low level cloud cover [%]"},
      /* 74 */ {"MCDC", "Mid level cloud cover [%]"},
      /* 75 */ {"HCDC", "High level cloud cover [%]"},
      /* 76 */ {"CWAT", "Cloud water [kg/m^2]"},
      /* 77 */ {"BLI", "Best lifted index (to 500 hPa) [K]"},
      /* 78 */ {"SNOC", "Convective snow [kg/m^2]"},
      /* 79 */ {"SNOL", "Large scale snow [kg/m^2]"},
      /* 80 */ {"WTMP", "Water temp. [K]"},
      /* 81 */ {"LAND", "Land cover (land=1;sea=0) [fraction]"},
      /* 82 */ {"DSLM", "Deviation of sea level from mean [m]"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"ALBDO", "Albedo [%]"},
      /* 85 */ {"TSOIL", "Soil temp. [K]"},
      /* 86 */ {"SOILM", "Soil moisture content [kg/m^2]"},
      /* 87 */ {"VEG", "Vegetation [%]"},
      /* 88 */ {"SALTY", "Salinity [kg/kg]"},
      /* 89 */ {"DEN", "Density [kg/m^3]"},
      /* 90 */ {"WATR", "Water runoff [kg/m^2]"},
      /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0) [fraction]"},
      /* 92 */ {"ICETK", "Ice thickness [m]"},
      /* 93 */ {"DICED", "Direction of ice drift [deg]"},
      /* 94 */ {"SICED", "Speed of ice drift [m/s]"},
      /* 95 */ {"UICE", "u of ice drift [m/s]"},
      /* 96 */ {"VICE", "v of ice drift [m/s]"},
      /* 97 */ {"ICEG", "Ice growth rate [m/s]"},
      /* 98 */ {"ICED", "Ice divergence [/s]"},
      /* 99 */ {"SNOM", "Snow melt [kg/m^2]"},
      /* 100 */ {"HTSGW", "Sig height of wind waves and swell [m]"},
      /* 101 */ {"WVDIR", "Direction of wind waves [deg]"},
      /* 102 */ {"WVHGT", "Sig height of wind waves [m]"},
      /* 103 */ {"WVPER", "Mean period of wind waves [s]"},
      /* 104 */ {"SWDIR", "Direction of swell waves [deg]"},
      /* 105 */ {"SWELL", "Sig height of swell waves [m]"},
      /* 106 */ {"SWPER", "Mean period of swell waves [s]"},
      /* 107 */ {"DIRPW", "Primary wave direction [deg]"},
      /* 108 */ {"PERPW", "Primary wave mean period [s]"},
      /* 109 */ {"DIRSW", "Secondary wave direction [deg]"},
      /* 110 */ {"PERSW", "Secondary wave mean period [s]"},
      /* 111 */ {"NSWRS", "Net short wave (surface) [W/m^2]"},
      /* 112 */ {"NLWRS", "Net long wave (surface) [W/m^2]"},
      /* 113 */ {"NSWRT", "Net short wave (top) [W/m^2]"},
      /* 114 */ {"NLWRT", "Net long wave (top) [W/m^2]"},
      /* 115 */ {"LWAVR", "Long wave [W/m^2]"},
      /* 116 */ {"SWAVR", "Short wave [W/m^2]"},
      /* 117 */ {"GRAD", "Global radiation [W/m^2]"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"LWRAD", "Radiance with respect to wave no. [W/m/sr]"},
      /* 120 */ {"SWRAD", "Radiance with respect ot wave len. [W/m^3/sr]"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"BLYDP", "Boundary layer dissipation [W/m^2]"},
      /* 124 */ {"UFLX", "Zonal momentum flux [N/m^2]"},
      /* 125 */ {"VFLX", "Meridional momentum flux [N/m^2]"},
      /* 126 */ {"WMIXE", "Wind mixing energy [J]"},
      /* 127 */ {"IMGD", "Image data []"},
      /* 128 */ {"TMPF", "TEMPERATURE (Fahrenheit) [F]"},
      /* 129 */ {"MAXK", "DAYTIME MAX TEMP (MAX) (Kelvin) [K]"},
      /* 130 */ {"MAXF", "DAYTIME MAX TEMP (MAX) (deg F) [F]"},
      /* 131 */ {"NMAXK", "NORMAL MAX TEMPERATURE (Kelvin) [K]"},
      /* 132 */ {"NMAXF", "NORMAL MAX TEMPERATURE (deg F) [F]"},
      /* 133 */ {"DMAXK", "DEPARTURE FROM NORMAL MAX (K) [K]"},
      /* 134 */ {"DMAXF", "DEPARTURE FROM NORMAL MAX (deg F) [F]"},
      /* 135 */ {"MINK", "NIGHTTIME MIN TEMP (MIN) (Kelvin) [K]"},
      /* 136 */ {"MINF", "NIGHTTIME MIN TEMP (MIN) (deg F) [F]"},
      /* 137 */ {"NMINK", "NORMAL MIN TEMPERATURE (Kelvin) [K]"},
      /* 138 */ {"NMINF", "NORMAL NIGHTTIME MIN TEMP (deg F) [F]"},
      /* 139 */ {"DMINK", "DEPARTURE FROM NORMAL MIN (K) [K]"},
      /* 140 */ {"DMINF", "DEPARTURE FROM NORMAL MIN (deg F) [F]"},
      /* 141 */ {"DWPF", "DEW POINT TEMPERATURE (deg F) [F]"},
      /* 142 */ {"DPDF", "DEW POINT DEPRESSION (deg F) [F]"},
      /* 143 */ {"HTINF", "HEAT INDEX (deg F) [F]"},
      /* 144 */ {"WNCHF", "WIND CHILL (deg F) [F]"},
      /* 145 */ {"var145", "undefined"},
      /* 146 */ {"POP", "PROB OF 0.01 IN. OF PRECIP (PoP) [%]"},
      /* 147 */ {"PQPF2", "PROB OF QPF >= 0.10 INCHES [%]"},
      /* 148 */ {"PQPF3", "PROB OF QPF >= 0.25 INCHES [%]"},
      /* 149 */ {"PQPF4", "PROB OF QPF >= 0.50 INCHES [%]"},
      /* 150 */ {"PQPF5", "PROB OF QPF >= 1.00 INCHES [%]"},
      /* 151 */ {"PQPF6", "PROB OF QPF >= 2.00 INCHES [%]"},
      /* 152 */ {"PQPF7", "PROB OF QPF >= 3.00 INCHES FUTURE [%]"},
      /* 153 */ {"BQPF", "BEST CATEGORY OF QPF [num]"},
      /* 154 */ {"NPOP", "NML REL. FREQ. OF 0.01 IN OF PCP [%]"},
      /* 155 */ {"DPOP", "DEPARTURE FROM NML OF 0.01 POP [%]"},
      /* 156 */ {"PCPM", "EXPECTED VALUE OF PRECIPITATION [mm]"},
      /* 157 */ {"PCPI", "EXPECTED VALUE OF PRECIPITATION [in]"},
      /* 158 */ {"CPCPM", "CONDITIONAL EXPECTED PRECIP AMT [mm]"},
      /* 159 */ {"CPCPI", "CONDITIONAL EXPECTED PRECIP AMT [in]"},
      /* 160 */ {"PSNA1", "PROB OF SNOW AMOUNT >= 0.10 [%]"},
      /* 161 */ {"PSNA2", "PROB OF SNOW AMOUNT >= 2 INCHES [%]"},
      /* 162 */ {"PSNA3", "PROB OF SNOW AMOUNT >= 4 INCHES [%]"},
      /* 163 */ {"PSNA4", "PROB OF SNOW AMOUNT >= 6 INCHES [%]"},
      /* 164 */ {"PSNA5", "PROB OF SNOW AMOUNT >= 8 INCHES [%]"},
      /* 165 */ {"BSNA", "BEST CATEGORY FOR SNOW AMOUNT [num]"},
      /* 166 */ {"SNWM", "EXPECTED VALUE OF SNOW AMOUNT [mm]"},
      /* 167 */ {"SNWI", "EXPECTED VALUE OF SNOW AMOUNT [in]"},
      /* 168 */ {"MWSPK", "INFLATED MAX WIND SPEED (knots) [kts]"},
      /* 169 */ {"IWSPM", "INFLATED WIND SPEED (meter/sec) [m/s]"},
      /* 170 */ {"SKNT", "INFLATED WIND SPEED (knots) [kts]"},
      /* 171 */ {"PWSP1", "PROB OF MAX WIND SPEED 0-12 kts [%]"},
      /* 172 */ {"PWSP2", "PROB OF MAX WIND SPEED 13-21 kts [%]"},
      /* 173 */ {"PWSP3", "PROB OF MAX WIND SPEED 22-31 kts [%]"},
      /* 174 */ {"PWSP4", "PROB OF MAX WIND SPEED >=32 kts [%]"},
      /* 175 */ {"WSPDC", "CATEGORICAL MAX WIND SPEED [num]"},
      /* 176 */ {"XSPDM", "EXPECTED VALUE OF MAX WIND SPEED [m/s]"},
      /* 177 */ {"XSPDK", "EXPECTED VALUE OF MAX WIND SPEED [kts]"},
      /* 178 */ {"PWDRN", "PROB OF WIND DIRECTION NORTH [%]"},
      /* 179 */ {"PWDRNE", "PROB OF WIND DIRECTION NORTHEAST [%]"},
      /* 180 */ {"PWDRE", "PROB OF WIND DIRECTION EAST [%]"},
      /* 181 */ {"PWDRSE", "PROB OF WIND DIRECTION SOUTHEAST [%]"},
      /* 182 */ {"PWDRS", "PROB OF WIND DIRECTION SOUTH [%]"},
      /* 183 */ {"PWDRSW", "PROB OF WIND DIRECTION SOUTHWEST [%]"},
      /* 184 */ {"PWDRW", "PROB OF WIND DIRECTION WEST [%]"},
      /* 185 */ {"PWDRNW", "PROB OF WIND DIRECTION NORTHWEST [%]"},
      /* 186 */ {"WDIRC", "CATEGORICAL WIND DIRECTION [num]"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"PSKCL", "PROB OF TOTAL SKY"},
      /* 189 */ {"PSKFW", "PROB OF TOTAL SKY"},
      /* 190 */ {"PSKSC", "PROB OF TOTAL SKY"},
      /* 191 */ {"PSKBK", "PROB OF TOTAL SKY"},
      /* 192 */ {"PSKOV", "PROB OF TOTAL SKY"},
      /* 193 */ {"SKYC", "CATEGORICAL TOTAL SKY COVER [num]"},
      /* 194 */ {"MSKCL", "PROB MEAN SKY CVR"},
      /* 195 */ {"MSKOV", "PROB MEAN SKY CVR"},
      /* 196 */ {"MSKMC", "PROB MEAN SKY CVR"},
      /* 197 */ {"MSKPC", "PROB MEAN SKY CVR"},
      /* 198 */ {"MSKMO", "PROB MEAN SKY CVR"},
      /* 199 */ {"MSKYC", "CATEGORICAL MEAN SKY COVER [num]"},
      /* 200 */ {"PCIG1", "PROB OF CIG HGT < 200 FT [%]"},
      /* 201 */ {"PCIG2", "PROB OF CIG HGT 200-400 FT [%]"},
      /* 202 */ {"PCIG3", "PROB OF CIG HGT 500-900 FT [%]"},
      /* 203 */ {"PCIG4", "PROB OF CIG HGT 1000-3000 FT [%]"},
      /* 204 */ {"PCIG5", "PROB OF CIG HGT 3100-6500 FT [%]"},
      /* 205 */ {"PCIG6", "PROB OF CIG HGT 6600-12000 FT [%]"},
      /* 206 */ {"PCIG7", "PROB OF CIG HGT > 12000 FT [%]"},
      /* 207 */ {"BCIG", "BEST CATEGORY OF CEILING HEIGHT [num]"},
      /* 208 */ {"PVIS1", "PROB OF VIS <=1/4 MILE [%]"},
      /* 209 */ {"PVIS2", "PROB OF VIS <=1/2 MILE [%]"},
      /* 210 */ {"PVIS3", "PROB OF VIS <=7/8 MILE [%]"},
      /* 211 */ {"PVIS4", "PROB OF VIS <=2 3/4 MILES [%]"},
      /* 212 */ {"PVIS5", "PROB OF VIS <=5 MILES [%]"},
      /* 213 */ {"PVIS6", "PROB OF VIS <=6 MILES [%]"},
      /* 214 */ {"VISC", "CATEGORICAL VISIBILITY [num]"},
      /* 215 */ {"POBVN", "PROB OF OBSTRUCTION TO VIS"},
      /* 216 */ {"POBVH", "PROB OF OBSTRUCTION TO VIS"},
      /* 217 */ {"POBVM", "PROB OF OBSTRUCTION TO VIS"},
      /* 218 */ {"POBVF", "PROB OF OBSTRUCTION TO VIS"},
      /* 219 */ {"POVBL", "PROB OF BLOWING OBVIS [%]"},
      /* 220 */ {"OBVC", "BEST CATEGORY OF OBVIS [num]"},
      /* 221 */ {"var221", "undefined"},
      /* 222 */ {"NTSM", "NORMAL PROB OF THUNDERSTORMS [%]"},
      /* 223 */ {"CSVR", "COND PROB OF SEVERE WEATHER [%]"},
      /* 224 */ {"USVR", "UNCOND PROB OF SEVERE WX [%]"},
      /* 225 */ {"NSVR", "NORMAL PROB OF SEVERE WX [%]"},
      /* 226 */ {"UHAI", "UNCONDITIONAL PROB OF HAIL [%]"},
      /* 227 */ {"UTOR", "UNCONDITIONAL PROB OF TORNADO [%]"},
      /* 228 */ {"UTSW", "UNCOND PROB OF DAMAGING WIND [%]"},
      /* 229 */ {"CFZI", "COND PROB FRZING PRECIP (INSTANT) [%]"},
      /* 230 */ {"UFZI", "UNCND PROB FRZING PRECIP (INSTNT) [%]"},
      /* 231 */ {"CZNI", "COND PROB FROZEN PRECIP (INSTANT) [%]"},
      /* 232 */ {"UZNI", "UNCND PROB FROZEN PRECIP (INSTNT) [%]"},
      /* 233 */ {"CLQI", "COND PROB LIQUID PRECIP (INSTANT) [%]"},
      /* 234 */ {"ULQI", "UNCND PROB LIQUID PRECIP (INSTNT) [%]"},
      /* 235 */ {"PTYPI", "CATEGORICAL PRECIP TYPE (INSTANT) [num]"},
      /* 236 */ {"CPOZP", "COND PROB OF FRZING PRECIP [%]"},
      /* 237 */ {"UPOZP", "UNCOND PROB OF FRZING PRECIP [%]"},
      /* 238 */ {"CPOS", "COND PROB OF SNOW (CPoS) [%]"},
      /* 239 */ {"UPOS", "UNCOND PROB OF SNOW (CPoS) [%]"},
      /* 240 */ {"CPORS", "COND PROB OF RAIN/SNOW MIXED [%]"},
      /* 241 */ {"UPORS", "UNCOND PROB OF RAIN/SNOW MIXED [%]"},
      /* 242 */ {"CPORA", "COND PROB OF RAIN [%]"},
      /* 243 */ {"var243", "undefined"},
      /* 244 */ {"BPCPT", "BEST CATEGORY OF PRECIP TYPE [num]"},
      /* 245 */ {"POPOH", "POPO PRECIP OCCURRING AT AN HOUR [%]"},
      /* 246 */ {"POPOP", "POPO PRECIP DURING A PERIOD [%]"},
      /* 247 */ {"CPDRZ", "COND PROB OF DRIZZLE [%]"},
      /* 248 */ {"CPSTY", "COND PROB OF CONT (STEADY) PRECIP [%]"},
      /* 249 */ {"CPSHW", "COND PROB OF SHOWERS [%]"},
      /* 250 */ {"BPCHR", "BEST CAT PRECIP CHARACTERISTIC [num]"},
      /* 251 */ {"SUNSH", "PERCENT OF POSSIBLE SUNSHINE [%]"},
      /* 252 */ {"HRSUN", "HOURS OF SUNSHINE [hrs]"},
      /* 253 */ {"SCQP", "SCAN 0-3H CATEGORICAL QPF [num]"},
      /* 254 */ {"SCTS", "SCAN 0-3H C-G LIGHTNING PROB [%]"},
      /* 255 */ {"var255", "undefined"},
};


/*
 * EC_ext	v1.0 wesley ebisuzaki
 *
 * prints something readable from the EC stream parameter
 *
 * prefix and suffix are only printed if EC_ext has text
 */

void EC_ext(unsigned char *pds, char *prefix, char *suffix, int verbose) {

    int local_id, ec_type, ec_class, ec_stream;
    char string[200];

    if (PDS_Center(pds) != ECMWF) return;

    local_id = PDS_EcLocalId(pds);
    if (local_id  == 0) return;
    ec_class = PDS_EcClass(pds);
    ec_type = PDS_EcType(pds);
    ec_stream = PDS_EcStream(pds);

    if (verbose == 2) printf("%sECext=%d%s", prefix, local_id, suffix);

    if (verbose == 2) {
	switch(ec_class) {
	    case 1: strcpy(string, "operations"); break;
	    case 2: strcpy(string, "research"); break;
	    case 3: strcpy(string, "ERA-15"); break;
	    case 4: strcpy(string, "Euro clim support network"); break;
	    case 5: strcpy(string, "ERA-40"); break;
	    case 6: strcpy(string, "DEMETER"); break;
	    case 7: strcpy(string, "PROVOST"); break;
	    case 8: strcpy(string, "ELDAS"); break;
            default: sprintf(string, "%d", ec_class); break;
	}
        printf("%sclass=%s%s",prefix,string,suffix);
    }
    /*
     10/03/2000: R.Rudsar : subroutine changed.
                 Tests for EcType and extra test for EcStream 1035
    */


/*    if (verbose == 2) { */
        switch(ec_type) {
            case 1: strcpy(string, "first guess"); break;
            case 2: strcpy(string, "analysis"); break;
            case 3: strcpy(string, "init analysis"); break;
            case 4: strcpy(string, "OI analysis"); break;
            /* case 10: strcpy(string, "Control forecast"); break; */
            case 10: sprintf(string, "Control forecast %d",PDS_EcFcstNo(pds)); 
		break;
            case 11:
		/* 5/23/2023 added ec_stream == 1122, thanks S. Villaume (ECMWF) **/
		/* 6/2023 added ec_stream 1120 */
		/* 9/2023 added ec_stream 1081 */
		if (ec_stream == 1035 || ec_stream == 1033 || ec_stream == 1122 || ec_stream == 1120 || ec_stream == 1081) 
              	   sprintf(string, "Perturbed forecast %d",
                   PDS_EcFcstNo(pds)); 
		else
		    strcpy(string, "Perturbed forecasts"); break;
		break;
            case 14: strcpy(string, "Cluster means"); break;
            case 15: strcpy(string, "Cluster std. dev."); break;
            case 16: strcpy(string, "Forecast probability"); break;
            case 17: strcpy(string, "Ensemble means"); break;
            case 18: strcpy(string, "Ensemble std. dev."); break;
    	    case 20: strcpy(string, "Climatology"); break;
            case 21: strcpy(string, "Climatology simulation"); break;
            case 80: strcpy(string, "Fcst seasonal mean"); break;
            default: sprintf(string, "%d", ec_type); break;
        }
        printf("%stype=%s%s",prefix,string,suffix);
/*    }  */
    if (verbose == 2) {
        switch(ec_stream) {
	    case 1033: strcpy(string, "ensemble hindcasts"); break;
	    case 1035: strcpy(string, "ensemble forecasts"); break;
	    case 1043: strcpy(string, "mon mean"); break;
	    case 1070: strcpy(string, "mon (co)var"); break;
	    case 1071: strcpy(string, "mon mean from daily"); break;
	    case 1090: strcpy(string, "EC ensemble fcsts"); break;
	    case 1091: strcpy(string, "EC seasonal fcst mon means"); break;
	    default:   sprintf(string, "%d", ec_stream); break;
        }
        printf("%sstream=%s%s",prefix,string,suffix);
    }
    if (verbose == 2) {
        printf("%sVersion=%c%c%c%c%s", prefix, *(PDS_Ec16Version(pds)), *(PDS_Ec16Version(pds)+1),
		*(PDS_Ec16Version(pds)+2), *(PDS_Ec16Version(pds)+3), suffix);
        if (local_id == 16) {
	    printf("%sSysVersion=%d%s", prefix, PDS_Ec16SysNum(pds), suffix);
	    printf("%sAvgPeriod=%d%s", prefix, PDS_Ec16AvePeriod(pds), suffix);
	    printf("%sFcstMon=%d%s", prefix, PDS_Ec16FcstMon(pds), suffix);

        }
    }

        if (local_id == 16) {
	    printf("%sEnsem_mem=%d%s", prefix, PDS_Ec16Number(pds), suffix);
	    printf("%sVerfDate=%d%s", prefix, PDS_Ec16VerfMon(pds), suffix);
        }

}

/*
 * get grid size from GDS
 *
 * added calculation of nxny of spectral data and clean up of triangular
 * grid nnxny calculation     l. kornblueh 
 * 7/25/03 wind fix Dusan Jovic
 * 9/17/03 fix scan mode
 */
extern int ec_large_grib, len_ec_bds;

int GDS_grid(unsigned char *gds, unsigned char *bds, int *nx, int *ny, 
             long int *nxny) {

    int i, d, ix, iy, pl;
    long int isum;

    *nx = ix = GDS_LatLon_nx(gds);
    *ny = iy = GDS_LatLon_ny(gds);
    *nxny = ix * iy;

    /* thin grid */

    if (GDS_Gaussian(gds) || GDS_LatLon(gds)) {
	if (ix == 65535) {
	    *nx = -1;
	    /* reduced grid */
	    isum = 0;
	    pl = GDS_PL(gds);
	    for (i = 0; i < iy; i++) {
		isum += gds[pl+i*2]*256 + gds[pl+i*2+1];
	    }
	    *nxny = isum;
	}
	return 0;
    }
    if (GDS_Triangular(gds)) {
        i = GDS_Triangular_ni(gds);
        d = GDS_Triangular_nd(gds);
	*nx = *nxny = d * (i + 1) * (i + 1);
        *ny = 1;
	return 0;
    }
    if (GDS_Harmonic(gds)) {
	if (BDS_ComplexPacking(bds)) {
	    *nx = BDS_NValues(bds);
	    *ny = -1;
	}
	else {
        /* this code assumes j, k, m are consistent with bds */
            *nx = *nxny = (8*(BDS_LEN(bds)-15)-BDS_UnusedBits(bds))/
		BDS_NumBits(bds)+1;
            if ((8*(BDS_LEN(bds)-15)-BDS_UnusedBits(bds)) % BDS_NumBits(bds)) {
	       fprintf(stderr,"inconsistent harmonic BDS\n");
            }
            *ny = 1;
	}
    }
    return 0;
}

#define NCOL 15
void GDS_prt_thin_lon(unsigned char *gds) {
    int iy, i, col, pl;

    iy = GDS_LatLon_ny(gds);
    iy = (iy + 1) / 2;
    iy = GDS_LatLon_ny(gds);

    if ((pl = GDS_PL(gds)) == -1) {
	fprintf(stderr,"\nprogram error: GDS_prt_thin\n");
	return;
    }
    for (col = i = 0; i < iy; i++) {
	if (col == 0) printf("   ");
	printf("%5d", (gds[pl+i*2] << 8) + gds[pl+i*2+1]);
	col++;
	if (col == NCOL) {
	    col = 0;
	    printf("\n");
	}
    }
    if (col != 0) printf("\n");
}

/*
 * prints out wind rel to grid or earth
 */

static char *scan_mode[8] = {
	"WE:NS",
	"NS:WE",

	"WE:SN",
	"SN:WE",

        "EW:NS",
	"NS:EW",

	"EW:SN",
	"SN:EW" };


void GDS_winds(unsigned char *gds, int verbose) {
    int scan = -1, mode = -1;

    if (gds != NULL) {
        if (GDS_LatLon(gds)) {
	    scan = GDS_LatLon_scan(gds);
	    mode = GDS_LatLon_mode(gds);
	}
	else if (GDS_Mercator(gds)) {
	    scan =GDS_Merc_scan(gds);
	    mode =GDS_Merc_mode(gds);
	}
	/* else if (GDS_Gnomonic(gds)) { */
	else if (GDS_Lambert(gds)) {
	    scan = GDS_Lambert_scan(gds);
	    mode = GDS_Lambert_mode(gds);
	}
	else if (GDS_Gaussian(gds)) {
	    scan = GDS_LatLon_scan(gds);
	    mode = GDS_LatLon_mode(gds);
	}
	else if (GDS_Polar(gds)) {
	    scan = GDS_Polar_scan(gds);
	    mode = GDS_Polar_mode(gds);
	}
	else if (GDS_RotLL(gds)) {
	    scan = GDS_RotLL_scan(gds);
	    mode = GDS_RotLL_mode(gds);
	}
	/* else if (GDS_Triangular(gds)) { */
	else if (GDS_ssEgrid(gds)) {
	    scan = GDS_ssEgrid_scan(gds);
	    mode = GDS_ssEgrid_mode(gds);
	}
	else if (GDS_fEgrid(gds)) {
	    scan = GDS_fEgrid_scan(gds);
	    mode = GDS_fEgrid_mode(gds);
	}
	else if (GDS_ss2dEgrid(gds)) {
	    scan = GDS_ss2dEgrid_scan(gds);
	    mode = GDS_ss2dEgrid_mode(gds);
	}
        else if (GDS_ss2dBgrid(gds)) {
           scan = GDS_ss2dBgrid_scan(gds);
           mode = GDS_ss2dBgrid_mode(gds); 
	}
    }
    if (verbose == 1) {
	if (mode != -1) {
	    if (mode & 8) printf("winds in grid direction:");
	    else printf("winds are N/S:"); 
	}
    }
    else if (verbose == 2) {
	if (scan != -1) {
	    printf(" scan: %s", scan_mode[(scan >> 5) & 7]);
        }
	if (mode != -1) {
	    if (mode & 8) printf(" winds(grid) ");
	    else printf(" winds(N/S) "); 
	}
    }
}



#define START -1

static int user_center = 0, user_subcenter = 0, user_ptable = 0;
static enum {filled, not_found, not_checked, no_file, init} status = init;

struct ParmTable parm_table_user[256];

/*
 * sets up user parameter table
 * v1.1 12/2005 w. ebisuzaki
 * v1.2  3/2007 w. ebisuzaki add FAST_GRIBTAB option
 */

int setup_user_table(int center, int subcenter, int ptable) {

    int i, j, c0, c1, c2;
    static FILE *input;
    static int file_open = 0;
    char *filename, line[300];

    if (status == init) {
	for (i = 0; i < 256; i++) {
	    parm_table_user[i].name = parm_table_user[i].comment = NULL;
	}
	status = not_checked;
    }

    if (status == no_file) return 0;

    if ((user_center == -1 || center == user_center) &&
	    (user_subcenter == -1 || subcenter == user_subcenter) &&
	    (user_ptable == -1 || ptable == user_ptable)) {

	if (status == filled) return 1;
	if (status == not_found) return 0;
    }

    /* open gribtab file if not open */

    if (!file_open) {
#ifdef FAST_GRIBTAB
        filename = getenv("GRIBTAB");
#else
        filename = getenv("GRIBTAB");
        if (filename == NULL) filename = getenv("gribtab");
        if (filename == NULL) filename = "gribtab";
#endif
        if (filename == NULL || (input = fopen(filename,"r")) == NULL) {
            status = no_file;
            return 0;
        }
	file_open = 1;
    }
    else {
	rewind(input);
    }

    user_center = center;
    user_subcenter = subcenter;
    user_ptable = ptable;

    /* scan for center & subcenter and ptable */
    for (;;) {
        if (fgets(line, 299, input) == NULL) {
	    status = not_found;
            return 0;
        }
	if (atoi(line) != START) continue;
	i = sscanf(line,"%d:%d:%d:%d", &j, &center, &subcenter, &ptable);
        if (i != 4) {
	    fprintf(stderr,"illegal gribtab center/subcenter/ptable line: %s\n", line);
            continue;
        }
	if ((center == -1 || center == user_center) &&
	    (subcenter == -1 || subcenter == user_subcenter) &&
	    (ptable == -1 || ptable == user_ptable)) break;
    }

    user_center = center;
    user_subcenter = subcenter;
    user_ptable = ptable;

    /* free any used memory */
    for (i = 0; i < 256; i++) {
        if (parm_table_user[i].name != NULL) free(parm_table_user[i].name);
        if (parm_table_user[i].comment != NULL) free(parm_table_user[i].comment);
	parm_table_user[i].name = parm_table_user[i].comment = NULL;
    }

    /* read definitions */

    for (;;) {
        if (fgets(line, 299, input) == NULL) break;
	if ((i = atoi(line)) == START) break;
	line[299] = 0;

	/* find the colons and end-of-line */
	for (c0 = 0; line[c0] != ':' && line[c0] != 0; c0++) ;
        /* skip blank lines */
        if (line[c0] == 0) continue;

	for (c1 = c0 + 1; line[c1] != ':' && line[c1] != 0; c1++) ;
	c2 = strlen(line);
        if (line[c2-1] == '\n') line[--c2] = '\0';
        if (c2 <= c1) {
	    fprintf(stderr,"illegal gribtab line:%s\n", line);
	    continue;
	}
	line[c0] = 0;
	line[c1] = 0;

	parm_table_user[i].name = (char *) malloc(c1 - c0);
	parm_table_user[i].comment = (char *) malloc(c2 - c1);
	strcpy(parm_table_user[i].name, line+c0+1);
	strcpy(parm_table_user[i].comment, line+c1+1);
    }

    /* now to fill in undefined blanks */
    for (i = 0; i < 255; i++) {
	if (parm_table_user[i].name == NULL) {
	    parm_table_user[i].name = (char *) malloc(7);
	    sprintf(parm_table_user[i].name, "var%d", i);
	    parm_table_user[i].comment = (char *) malloc(strlen("undefined")+1);
	    strcpy(parm_table_user[i].comment, "undefined");
        }
    }
    status = filled;
    return 1;
}

/*
 * PDS_date.c  v1.2 wesley ebisuzaki
 *
 * prints a string with a date code
 *
 * PDS_date(pds,option, v_time)
 *   options=0  .. 2 digit year
 *   options=1  .. 4 digit year
 *
 *   v_time=0   .. initial time
 *   v_time=1   .. verification time
 *
 * assumption: P1 and P2 are unsigned integers (not clear from doc)
 *
 * v1.2 years that are multiple of 400 are leap years, not 500
 * v1.2.1  make the change to the source code for v1.2
 * v1.2.2  add 3/6/12 hour forecast time units
 * v1.2.3  Jan 31 + 1 month => Feb 31 .. change to Feb 28/29
 */

static int msg_count = 0;
extern int minute;

int PDS_date(unsigned char *pds, int option, int v_time) {

    int year, month, day, hour, min;

    if (v_time == 0) {
        year = PDS_Year4(pds);
        month = PDS_Month(pds);
        day  = PDS_Day(pds);
        hour = PDS_Hour(pds);
    }
    else {
        if (verf_time(pds, &year, &month, &day, &hour) != 0) {
	    if (msg_count++ < 5) fprintf(stderr, "PDS_date: problem\n");
	}
    }
    min =  PDS_Minute(pds);

    switch(option) {
	case 0:
	    printf("%2.2d%2.2d%2.2d%2.2d", year % 100, month, day, hour);
	    if (minute) printf("-%2.2d", min);
	    break;
	case 1:
	    printf("%4.4d%2.2d%2.2d%2.2d", year, month, day, hour);
	    if (minute) printf("-%2.2d", min);
	    break;
	default:
	    fprintf(stderr,"missing code\n");
	    exit(8);
    }
    return 0;
}

#define  FEB29   (31+29)
static int monthjday[13] = {
        0,31,59,90,120,151,181,212,243,273,304,334,365};

static int leap(int year) {
	if (year % 4 != 0) return 0;
	if (year % 100 != 0) return 1;
	return (year % 400 == 0);
}


int add_time(int *year, int *month, int *day, int *hour, int dtime, int unit) {
    int y, m, d, h, jday, i, days_in_month;

    y = *year;
    m = *month;
    d = *day;
    h = *hour;

    if (unit == YEAR) {
	*year = y + dtime;
	return 0;
    }
    if (unit == DECADE) {
	*year =  y + (10 * dtime);
	return 0;
    }
    if (unit == CENTURY) {
	*year = y + (100 * dtime);
	return 0;
    }
    if (unit == NORMAL) {
	*year = y + (30 * dtime);
	return 0;
    }
    if (unit == MONTH) {
        if (dtime < 0) {
           i = (-dtime) / 12 + 1;
           y -= i;
           dtime += (i * 12);
        }
	dtime += (m - 1);
	*year =  y = y + (dtime / 12);
	*month = m = 1 + (dtime % 12);

        /* check if date code if valid */
	days_in_month = monthjday[m] - monthjday[m-1];
	if (m == 2 && leap(y)) {
	    days_in_month++;
	}
	if (days_in_month < d) *day = days_in_month;

	return 0;
    }

    if (unit == SECOND) {
	dtime /= 60;
	unit = MINUTE;
    }
    if (unit == MINUTE) {
	dtime /= 60;
	unit = HOUR;
    }

    if (unit == HOURS3) {
        dtime *= 3;
        unit = HOUR;
    }
    else if (unit == HOURS6) {
        dtime *= 6;
        unit = HOUR;
    }
    else if (unit == HOURS12) {
        dtime *= 12;
        unit = HOUR;
    }

    if (unit == HOUR) {
	dtime += h;

        *hour = dtime % 24;
        dtime = dtime / 24;
        if (*hour < 0) {
            *hour += 24;
            dtime--;
        }
        unit = DAY;
    }

    /* this is the hard part */

    if (unit == DAY) {
	/* set m and day to Jan 0, and readjust dtime */
	jday = d + monthjday[m-1];
	if (leap(y) && m > 2) jday++;
        dtime += jday;

        while (dtime < 1) {
            y--;
	    dtime += 365 + leap(y);
        }

	/* one year chunks */
	while (dtime > 365 + leap(y)) {
	    dtime -= (365 + leap(y));
	    y++;
	}

	/* calculate the month and day */

	if (leap(y) && dtime == FEB29) {
	    m = 2;
	    d = 29;
	}
	else {
	    if (leap(y) && dtime > FEB29) dtime--;
	    for (i = 11; monthjday[i] >= dtime; --i);
	    m = i + 1;
	    d = dtime - monthjday[i];
	}
	*year = y;
	*month = m;
	*day = d;
	return 0;
   }
   fprintf(stderr,"add_time: undefined time unit %d\n", unit);
   return 1;
}


/*
 * verf_time:
 *
 * this routine returns the "verification" time
 * should have behavior similar to gribmap
 *
 */

int verf_time(unsigned char *pds, int *year, int *month, int *day, int *hour) {
    int tr, dtime, unit;

    *year = PDS_Year4(pds);
    *month = PDS_Month(pds);
    *day  = PDS_Day(pds);
    *hour = PDS_Hour(pds);

    /* find time increment */

    dtime = PDS_P1(pds);
    tr = PDS_TimeRange(pds);
    unit = PDS_ForecastTimeUnit(pds);

    if (tr == 10) dtime = PDS_P1(pds) * 256 + PDS_P2(pds);
    if (tr > 1 && tr < 6 ) dtime = PDS_P2(pds);
    if (tr == 6 || tr == 7) dtime = - PDS_P1(pds);

    if (dtime == 0) return 0;

    return add_time(year, month, day, hour, dtime, unit);
}


/*
 * ensemble.c   v0.1 wesley ebisuzaki
 *
 * prints ensemble meta-data
 *
 * only for NCEP and ECMWF
 *
 * output format:
 *
 *       ECMWF
 *  ens=n/N:       n:  0=ctl, +/-ve
 *                 N:  total number of members
 *
 *       NCEP
 *  ens=n/type:    n:  0=ctl, +/-ve, CLUST, PROD/
 *                 type: Mn, WtdMn, SDev, NSDev
 *
 * updated 8/06 w. ebisuzaki
 */

extern int ncep_ens;

void ensemble(unsigned char *pds, int mode) {

    int pdslen;
    unsigned char ctmp;
    char char_end;

    pdslen = PDS_LEN(pds);
    char_end = mode == 2 ? ' ' : ':';

    if ((PDS_Center(pds) == NMC || ncep_ens) && pdslen >= 45 && pds[40] == 1) {

	/* control run */

	if (pds[41] == 1) {
	    if (mode != 2) {
		printf("ens%c0:%c", pds[42] == 1 ? '+' : '-', char_end);
	    }
	    else {
		printf("%s-res_ens_control ", pds[42] == 1 ? "hi" : "low");
	    }
	}

	/* perturbation run */

	else if (pds[41] == 2 || pds[41] == 3) {
	    if (mode != 2) {
	        printf("ens%c%d%c", pds[41] == 3 ? '+' : '-', pds[42],char_end);
	    }
	    else {
		printf("ens_perturbation=%c%d ",pds[41] == 3 ? '+' : '-', 
		    pds[42]);
	    }
	}

	/* cluster mean */

	else if (pds[41] == 4) {
	    if (mode != 2) printf("cluster%c", char_end);
	    else printf("cluster(%d members) ",pds[60]);
	}


	/* ensemble mean */

	else if (pds[41] == 5) {
	    if (mode != 2) printf("ensemble%c", char_end);
	    else printf("ensemble(%d members) ",pds[60]);
	}

	/* other case .. debug code */

	else {
		printf("ens %d/%d/%d/%d%c", pds[41],pds[42],pds[43],pds[44],char_end);
	}


	if (pdslen >= 44) {
	    if (pds[43] == 1 && pds[41] >= 4) printf("mean%c", char_end);
	    else if (pds[43] == 2) printf("weighted mean%c",char_end);
	    else if (pds[43] == 3) printf("no bias%c",char_end);
	    else if (pds[43] == 4) printf("weighted mean no bias%c",char_end);
	    else if (pds[43] == 5) printf("weight%c",char_end);
	    else if (pds[43] == 6) printf("climate percentile%c",char_end);
	    else if (pds[43] == 7) printf("daily climate mean%c",char_end);
	    else if (pds[43] == 8) printf("daily climate std dev%c",char_end);
	    else if (pds[43] == 11) printf("std dev%c",char_end);
	    else if (pds[43] == 12) printf("norm std dev%c",char_end);
	    else if (pds[43] == 21) printf("max val%c",char_end);
	    else if (pds[43] == 22) printf("min val%c",char_end);
	}

	/* NCEP probability limits */

	if ((PDS_PARAM(pds) == 191 || PDS_PARAM(pds) == 192) && pdslen >= 47) {
	    ctmp = PDS_PARAM(pds);
	    PDS_PARAM(pds) = pds[45];
	    if (pds[46] == 1 && pdslen >= 51) {
		printf("prob(%s<%f)%c", k5toa(pds), ibm2flt(pds+47),char_end);
	    }
	    else if (pds[46] == 2 && pdslen >= 54) {
		printf("prob(%s>%f)%c", k5toa(pds), ibm2flt(pds+51), char_end);
	    }
	    else if (pds[46] == 3 && pdslen >= 54) {
		printf("prob(%f<%s<%f)%c", ibm2flt(pds+47), k5toa(pds), 
			ibm2flt(pds+51), char_end);
	    }
            PDS_PARAM(pds) = ctmp;
	}
    }
}

/*
 * GRIB table 2 at DWD
 *     Helmut P. Frank, 30.08.2001
 * updated 24.07.2003: PMSL, DD, FF, W, FR_ICE, H_ICE
 * updated 28.11.2005: H_SNOW
 */

const struct ParmTable parm_table_dwd_002[256] = {
    /* 0 */ {"var0", "undefined"},
    /* 1 */ {"PS", "pressure [Pa]"},
    /* 2 */ {"PMSL", "pressure reduced to MSL [Pa]"},
    /* 3 */ {"DPSDT", "pressure tendency [Pa/s]"},
    /* 4 */ {"var4", "undefined"},
    /* 5 */ {"var5", "undefined"},
    /* 6 */ {"FI", "geopotential [(m**2)/(s**2)]"},
    /* 7 */ {"geopot h", "geopotential height [gpm]"},
    /* 8 */ {"HH", "geometrical height [m]"},
    /* 9 */ {"dev of h", "standard deviation of height [m]"},
    /* 10 */ {"TO3", "total ozone [Dobson Units]"},
    /* 11 */ {"T", "temperature [K]"},
    /* 12 */ {"virt.temp.", "virtual temperature [K]"},
    /* 13 */ {"pot. temp.", "potential temperature [K]"},
    /* 14 */ {"pseudo-pot", "pseudo-adiabatic potential temperature [K]"},
    /* 15 */ {"TMAX", "maximum temperature [K]"},
    /* 16 */ {"TMIN", "minimum temperature [K]"},
    /* 17 */ {"TD", "dew-point temperature [K]"},
    /* 18 */ {"dew-pnt de", "dew-point depression (or deficit) [K]"},
    /* 19 */ {"lapse rate", "laps rate [K/m]"},
    /* 20 */ {"visibility", "visibility [m]"},
    /* 21 */ {"radar sp 1", "radar spectra (1) [non-dim]"},
    /* 22 */ {"radar sp 2", "radar spectra (2) [non-dim]"},
    /* 23 */ {"radar sp 3", "radar spectra (3) [non-dim]"},
    /* 24 */ {"pli to 500", "parcel lifted index (to 500 hPa) [K]"},
    /* 25 */ {"temp anom", "temperature anomaly [K]"},
    /* 26 */ {"pres anom", "pressure anomaly [Pa]"},
    /* 27 */ {"geop anom", "geopotential height anomaly [gpm]"},
    /* 28 */ {"wave sp 1", "wave spaectra(1) [non-dim]"},
    /* 29 */ {"wave sp 2", "wave spaectra(2) [non-dim]"},
    /* 30 */ {"wave sp 3", "wave spaectra(3) [non-dim]"},
    /* 31 */ {"DD", "wind direction [degree true]"},
    /* 32 */ {"FF", "wind speed [m/s]"},
    /* 33 */ {"U", "u-component (zonal) of wind [m/s]"},
    /* 34 */ {"V", "v-component (merdional) of wind [m/s]"},
    /* 35 */ {"stream fun", "stream function [(m**2)/s]"},
    /* 36 */ {"vel potent", "velocity potential [(m**2)/s]"},
    /* 37 */ {"M.stream f", "Montgomery stream function [(m**2)/(s**2)]"},
    /* 38 */ {"sigma vert", "sigma co-ordinate vertical velocity [1/s]"},
    /* 39 */ {"OMEGA", "vertical velocity [Pa/s]"},
    /* 40 */ {"W", "vertical velocity [m/s]"},
    /* 41 */ {"abs vortic", "absolute vorticity [1/s]"},
    /* 42 */ {"abs diverg", "absolute divergence [1/s]"},
    /* 43 */ {"rel vortic", "relative vorticity [1/s]"},
    /* 44 */ {"rel diverg", "relative divergence [1/s]"},
    /* 45 */ {"vert.u-shr", "vertical u-component shear [1/s]"},
    /* 46 */ {"vert.v-shr", "vertical v-component shear [1/s]"},
    /* 47 */ {"dir of cur", "direction of current [degree true]"},
    /* 48 */ {"spd of cur", "speed of current [m/s]"},
    /* 49 */ {"currcomp U", "u-component of current [m/s]"},
    /* 50 */ {"currcomp V", "v-component of current [m/s]"},
    /* 51 */ {"QV", "specific humidity [kg/kg]"},
    /* 52 */ {"RELHUM", "relative humidity [%]"},
    /* 53 */ {"hum mixrat", "humidity mixing ratio [kg/kg]"},
    /* 54 */ {"TQV", "total precipitable water [kg/m**2]"},
    /* 55 */ {"vapor pres", "vapor pressure [Pa]"},
    /* 56 */ {"sat.defic.", "saturation deficit [Pa]"},
    /* 57 */ {"AEVAP_S", "evaporation [kg/(m**2)]"},
    /* 58 */ {"TQI", "total cloud ice content [kg/m**2]"},
    /* 59 */ {"prec. rate", "precipitation rate [kg/((m**2)*s)]"},
    /* 60 */ {"thunderst.", "thunderstorm probability [%]"},
    /* 61 */ {"TOT_PREC", "total precipitation [kg/(m**2)]"},
    /* 62 */ {"PREC_GSP", "large scale precipitation [kg/(m**2)]"},
    /* 63 */ {"PREC_CON", "convective precipitation [kg/(m**2)]"},
    /* 64 */ {"snowf.rate", "snowfall rate water equivalent [kg/((m**2)*s)]"},
    /* 65 */ {"W_SNOW", "water equivalent of accumulated snow depth [kg/(m**2)]"},
    /* 66 */ {"H_SNOW", "snow depth [m]"},
    /* 67 */ {"mix lay de", "mixed layer depth [m]"},
    /* 68 */ {"tr therm d", "transient thermocline depth [m]"},
    /* 69 */ {"ma therm d", "main thermocline depth [m]"},
    /* 70 */ {"m therm da", "main thermocline depth anomaly [m]"},
    /* 71 */ {"CLCT", "total cloud cover [%]"},
    /* 72 */ {"CLC_CON", "convective cloud cover [%]"},
    /* 73 */ {"CLCL", "low cloud cover [%]"},
    /* 74 */ {"CLCM", "medium cloud cover [%]"},
    /* 75 */ {"CLCH", "high cloud cover [%]"},
    /* 76 */ {"TQC", "total cloud water content [kg/m**2]"},
    /* 77 */ {"bli to 500", "best lifted index (to 500 hPa) [K]"},
    /* 78 */ {"SNOW_CON", "convective snow [kg/(m**2)]"},
    /* 79 */ {"SNOW_GSP", "large scale snow [kg/(m**2)]"},
    /* 80 */ {"water temp", "water temperature [K]"},
    /* 81 */ {"FR_LAND", "land cover (1=land, 0=sea) [1]"},
    /* 82 */ {"dev sea-le", "deviation of sea-level from mean [m]"},
    /* 83 */ {"Z0", "surface roughness [m]"},
    /* 84 */ {"ALB_RAD", "albedo [%]"},
    /* 85 */ {"T_soil", "soil temperature [K]"},
    /* 86 */ {"W_soil", "soil moisture content [kg/(m**2)]"},
    /* 87 */ {"PLCOV", "vegetation (plant cover) [%]"},
    /* 88 */ {"salinity", "salinity [kg/kg]"},
    /* 89 */ {"density", "density [kg/(m**3)]"},
    /* 90 */ {"RUNOFF", "water run-off [kg/(m**2)]"},
    /* 91 */ {"FR_ICE", "ice cover (1=ice, 0=no ice) [1]"},
    /* 92 */ {"H_ICE", "ice thickness [m]"},
    /* 93 */ {"dir ice dr", "direction of ice drift [degree true]"},
    /* 94 */ {"sp ice dr", "speed of ice drift [m/s]"},
    /* 95 */ {"ice dr u", "u-component of ice drift [m/s]"},
    /* 96 */ {"ice dr v", "v-component of ice drift [m/s]"},
    /* 97 */ {"ice growth", "ice growth rate [m/s]"},
    /* 98 */ {"ice diverg", "ice divergence [1/s]"},
    /* 99 */ {"snow melt", "snow melt [kg/(m**2)]"},
    /* 100 */ {"winwav/swe", "significant height of comb. wind waves and swell [m]"},
    /* 101 */ {"dir of wav", "direction of wind waves [degree true]"},
    /* 102 */ {"hei of wav", "significant height of wind waves [m]"},
    /* 103 */ {"MP of wiwa", "mean period of wind waves [s]"},
    /* 104 */ {"dir of swe", "direction of swell [degree true]"},
    /* 105 */ {"hei of swe", "significant height of swell [m]"},
    /* 106 */ {"MP of swel", "mean period of swell [s]"},
    /* 107 */ {"pr wave di", "primary wave direction [degree true]"},
    /* 108 */ {"pr wave pe", "primary wave period [s]"},
    /* 109 */ {"se wave di", "secondary wave direction [degree true]"},
    /* 110 */ {"se wave pe", "secondary wave period [s]"},
    /* 111 */ {"ASOB_S", "net short-wave radiation (surface) [W/(m**2)]"},
    /* 112 */ {"ATHB_S", "net long-wave radiation (surface) [W/(m**2)]"},
    /* 113 */ {"ASOB_T", "net short-wave radiation (top of atmosphere) [W/(m**2)]"},
    /* 114 */ {"ATHB_T", "net long-wave radiation (top of atmosphere) [W/(m**2)]"},
    /* 115 */ {"l-w rad.", "long-wave radiation [W/(m**2)]"},
    /* 116 */ {"s-w rad.", "short-wave radiation [W/(m**2)]"},
    /* 117 */ {"global rad", "global radiation [W/(m**2)]"},
    /* 118 */ {"var118", "undefined"},
    /* 119 */ {"var119", "undefined"},
    /* 120 */ {"var120", "undefined"},
    /* 121 */ {"ALHFL_S", "latent heat flux [W/(m**2)]"},
    /* 122 */ {"ASHFL_S", "sensible heat flux [W/(m**2)]"},
    /* 123 */ {"bound l di", "boundary layer dissipation [W/(m**2)]"},
    /* 124 */ {"AUMFL_S", "momentum flux, u component [N/(m**2)]"},
    /* 125 */ {"AVMFL_S", "momentum flux, v component [N/(m**2)]"},
    /* 126 */ {"wind mix e", "wind mixing energy [J]"},
    /* 127 */ {"image data", "image data []"},
    /* 128 */ {"var128", "undefined"},
    /* 129 */ {"geopot h", "geopotential height (ECMF) [gpm]"},
    /* 130 */ {"temperatur", "temperature (ECMF) [K]"},
    /* 131 */ {"wind compU", "u-component of wind (ECMF) [m/s]"},
    /* 132 */ {"wind compV", "v-component of wind (ECMF) [m/s]"},
    /* 133 */ {"var133", "undefined"},
    /* 134 */ {"var134", "undefined"},
    /* 135 */ {"var135", "undefined"},
    /* 136 */ {"var136", "undefined"},
    /* 137 */ {"var137", "undefined"},
    /* 138 */ {"var138", "undefined"},
    /* 139 */ {"soil temp.", "soil temperature (ECMF) [K]"},
    /* 140 */ {"var140", "undefined"},
    /* 141 */ {"var141", "undefined"},
    /* 142 */ {"ls precip.", "large scale precipitation (ECMF) [kg/(m**2)]"},
    /* 143 */ {"conv prec.", "convective precipitation (ECMF) [kg/(m**2)]"},
    /* 144 */ {"snowfall", "snowfall (ECMF) [m of water equivalent]"},
    /* 145 */ {"var145", "undefined"},
    /* 146 */ {"var146", "undefined"},
    /* 147 */ {"var147", "undefined"},
    /* 148 */ {"var148", "undefined"},
    /* 149 */ {"var149", "undefined"},
    /* 150 */ {"var150", "undefined"},
    /* 151 */ {"pressure", "pressure reduced to MSL (ECMF) [Pa]"},
    /* 152 */ {"var152", "undefined"},
    /* 153 */ {"var153", "undefined"},
    /* 154 */ {"var154", "undefined"},
    /* 155 */ {"var155", "undefined"},
    /* 156 */ {"geopot h", "geopotential height (ECMF) [gpm]"},
    /* 157 */ {"rel. humid", "relative humidity (ECMF) [%]"},
    /* 158 */ {"var158", "undefined"},
    /* 159 */ {"var159", "undefined"},
    /* 160 */ {"var160", "undefined"},
    /* 161 */ {"var161", "undefined"},
    /* 162 */ {"var162", "undefined"},
    /* 163 */ {"var163", "undefined"},
    /* 164 */ {"cloud cov.", "total cloud cover (ECMF) [%]"},
    /* 165 */ {"10m-wind U", "u-component of 10m-wind (ECMF) [m/s]"},
    /* 166 */ {"10m-wind V", "v-component of 10m-wind (ECMF) [m/s]"},
    /* 167 */ {"2m temper", "2m temperature (ECMF) [K]"},
    /* 168 */ {"2m due-p.", "2m due-point temperature (ECMF) [K]"},
    /* 169 */ {"var169", "undefined"},
    /* 170 */ {"var170", "undefined"},
    /* 171 */ {"var171", "undefined"},
    /* 172 */ {"var172", "undefined"},
    /* 173 */ {"var173", "undefined"},
    /* 174 */ {"var174", "undefined"},
    /* 175 */ {"var175", "undefined"},
    /* 176 */ {"var176", "undefined"},
    /* 177 */ {"var177", "undefined"},
    /* 178 */ {"var178", "undefined"},
    /* 179 */ {"var179", "undefined"},
    /* 180 */ {"var180", "undefined"},
    /* 181 */ {"var181", "undefined"},
    /* 182 */ {"var182", "undefined"},
    /* 183 */ {"var183", "undefined"},
    /* 184 */ {"var184", "undefined"},
    /* 185 */ {"var185", "undefined"},
    /* 186 */ {"var186", "undefined"},
    /* 187 */ {"var187", "undefined"},
    /* 188 */ {"var188", "undefined"},
    /* 189 */ {"var189", "undefined"},
    /* 190 */ {"var190", "undefined"},
    /* 191 */ {"var191", "undefined"},
    /* 192 */ {"var192", "undefined"},
    /* 193 */ {"var193", "undefined"},
    /* 194 */ {"var194", "undefined"},
    /* 195 */ {"var195", "undefined"},
    /* 196 */ {"var196", "undefined"},
    /* 197 */ {"var197", "undefined"},
    /* 198 */ {"var198", "undefined"},
    /* 199 */ {"var199", "undefined"},
    /* 200 */ {"var200", "undefined"},
    /* 201 */ {"var201", "undefined"},
    /* 202 */ {"var202", "undefined"},
    /* 203 */ {"var203", "undefined"},
    /* 204 */ {"var204", "undefined"},
    /* 205 */ {"var205", "undefined"},
    /* 206 */ {"var206", "undefined"},
    /* 207 */ {"var207", "undefined"},
    /* 208 */ {"var208", "undefined"},
    /* 209 */ {"var209", "undefined"},
    /* 210 */ {"var210", "undefined"},
    /* 211 */ {"var211", "undefined"},
    /* 212 */ {"var212", "undefined"},
    /* 213 */ {"var213", "undefined"},
    /* 214 */ {"var214", "undefined"},
    /* 215 */ {"var215", "undefined"},
    /* 216 */ {"var216", "undefined"},
    /* 217 */ {"var217", "undefined"},
    /* 218 */ {"var218", "undefined"},
    /* 219 */ {"var219", "undefined"},
    /* 220 */ {"var220", "undefined"},
    /* 221 */ {"var221", "undefined"},
    /* 222 */ {"var222", "undefined"},
    /* 223 */ {"var223", "undefined"},
    /* 224 */ {"var224", "undefined"},
    /* 225 */ {"var225", "undefined"},
    /* 226 */ {"var226", "undefined"},
    /* 227 */ {"var227", "undefined"},
    /* 228 */ {"total prec", "total precipitation (ECMF) [m]"},
    /* 229 */ {"seaway 01", "seaway 01 (ECMF) []"},
    /* 230 */ {"seaway 02", "seaway 02 (ECMF) []"},
    /* 231 */ {"seaway 03", "seaway 03 (ECMF) []"},
    /* 232 */ {"seaway 04", "seaway 04 (ECMF) []"},
    /* 233 */ {"seaway 05", "seaway 05 (ECMF) []"},
    /* 234 */ {"seaway 06", "seaway 06 (ECMF) []"},
    /* 235 */ {"seaway 07", "seaway 07 (ECMF) []"},
    /* 236 */ {"seaway 08", "seaway 08 (ECMF) []"},
    /* 237 */ {"seaway 09", "seaway 09 (ECMF) []"},
    /* 238 */ {"seaway 10", "seaway 10 (ECMF) []"},
    /* 239 */ {"seaway 11", "seaway 11 (ECMF) []"},
    /* 240 */ {"var240", "undefined"},
    /* 241 */ {"var241", "undefined"},
    /* 242 */ {"var242", "undefined"},
    /* 243 */ {"var243", "undefined"},
    /* 244 */ {"var244", "undefined"},
    /* 245 */ {"var245", "undefined"},
    /* 246 */ {"var246", "undefined"},
    /* 247 */ {"var247", "undefined"},
    /* 248 */ {"var248", "undefined"},
    /* 249 */ {"var249", "undefined"},
    /* 250 */ {"var250", "undefined"},
    /* 251 */ {"var251", "undefined"},
    /* 252 */ {"var252", "undefined"},
    /* 253 */ {"var253", "undefined"},
    /* 254 */ {"var254", "undefined"},
    /* 255 */ {"var255", "undefined"},
};

/*
 * GRIB table 201 at DWD
 *     Helmut P. Frank, 30.08.2001
 * updated 24.07.2003:  DQC_GSP, DQI_GSP, T_SO, W_SO, W_SO_ICE, T_ICE
 *         19.10.2005:  SOTR_RA, QRS_GSP, RHO_SNOW to table 201, and others
 */

const struct ParmTable parm_table_dwd_201[256] = {
    /* 0 */ {"var0", "undefined"},
    /* 1 */ {"dw sw flux", "downward shortwave radiant flux density [W/m**2]"},
    /* 2 */ {"uw sw flux", "upward shortwave radiant flux density [W/m**2]"},
    /* 3 */ {"dw lw flux", "downward longwave radiant flux density [W/m**2]"},
    /* 4 */ {"uw lw flux", "upward longwave radiant flux density [W/m**2]"},
    /* 5 */ {"APAB_S", "downwd photosynthetic active radiant flux density [W/m**2]"},
    /* 6 */ {"net s flux", "net shortwave flux [W/m**2]"},
    /* 7 */ {"net l flux", "net longwave flux [W/m**2]"},
    /* 8 */ {"net flux", "total net radiative flux density [W/m**2]"},
    /* 9 */ {"dw sw clfr", "downw shortw radiant flux density, cloudfree part [W/m**2]"},
    /* 10 */ {"uw sw cldy", "upw shortw radiant flux density, cloudy part [W/m**2]"},
    /* 11 */ {"dw lw clfr", "downw longw radiant flux density, cloudfree part [W/m**2]"},
    /* 12 */ {"uw lw cldy", "upw longw radiant flux density, cloudy part [W/m**2]"},
    /* 13 */ {"SOHR_RAD", "shortwave radiative heating rate [K/s]"},
    /* 14 */ {"THHR_RAD", "longwave radiative heating rate [K/s]"},
    /* 15 */ {"rad heat", "total radiative heating rate [K/s]"},
    /* 16 */ {"soilheat S", "soil heat flux, surface [W/m**2]"},
    /* 17 */ {"soilheat L", "soil heat flux, bottom of layer [W/m**2]"},
    /* 18 */ {"var18", "undefined"},
    /* 19 */ {"var19", "undefined"},
    /* 20 */ {"var20", "undefined"},
    /* 21 */ {"var21", "undefined"},
    /* 22 */ {"var22", "undefined"},
    /* 23 */ {"var23", "undefined"},
    /* 24 */ {"var24", "undefined"},
    /* 25 */ {"var25", "undefined"},
    /* 26 */ {"var26", "undefined"},
    /* 27 */ {"var27", "undefined"},
    /* 28 */ {"var28", "undefined"},
    /* 29 */ {"CLC", "cloud cover, grid scale + convective [1]"},
    /* 30 */ {"clc gr sc", "cloud cover, grid scale  (0...1) [1]"},
    /* 31 */ {"QC", "specific cloud water content, grid scale [kg/kg]"},
    /* 32 */ {"clw gs vi", "cloud water content, grid scale, vert integrated [kg/m**2]"},
    /* 33 */ {"QI", "specific cloud ice content, grid scale [kg/kg]"},
    /* 34 */ {"cli gs vi", "cloud ice content, grid scale, vert integrated [kg/m**2]"},
    /* 35 */ {"QR", "specific rainwater content, grid scale [kg/kg]"},
    /* 36 */ {"QS", "specific snow content, grid scale [kg/kg]"},
    /* 37 */ {"src gs vi", "specific rainwater content, gs, vert. integrated [kg/m**2]"},
    /* 38 */ {"ssc gs vi", "specific snow content, gs, vert. integrated [kg/m**2]"},
    /* 39 */ {"QG", "specific graupel content, grid scale [kg/kg]"},
    /* 40 */ {"var40", "undefined"},
    /* 41 */ {"TWATER", "vert. integral of humidity, cloud water (and ice) [kg/(m**2)]"},
    /* 42 */ {"TDIV_HUM", "vert. integral of divergence of tot. water content [kg/(m**2)]"},
    /* 43 */ {"var43", "undefined"},
    /* 44 */ {"var44", "undefined"},
    /* 45 */ {"var45", "undefined"},
    /* 46 */ {"var46", "undefined"},
    /* 47 */ {"var47", "undefined"},
    /* 48 */ {"var48", "undefined"},
    /* 49 */ {"var49", "undefined"},
    /* 50 */ {"CH_CM_CL", "cloud covers CH_CM_CL (000...888) [1]"},
    /* 51 */ {"cl cov. CH", "cloud cover CH (0..8) [1]"},
    /* 52 */ {"cl cov. CM", "cloud cover CM (0..8) [1]"},
    /* 53 */ {"cl cov. CL", "cloud cover CL (0..8) [1]"},
    /* 54 */ {"cloud cov.", "total cloud cover (0..8) [1]"},
    /* 55 */ {"fog", "fog (0..8) [1]"},
    /* 56 */ {"fog", "fog [1]"},
    /* 57 */ {"var57", "undefined"},
    /* 58 */ {"var58", "undefined"},
    /* 59 */ {"var59", "undefined"},
    /* 60 */ {"clc con ci", "cloud cover, convective cirrus  (0...1) [1]"},
    /* 61 */ {"CLW_CON", "specific cloud water content, convective clouds [kg/kg]"},
    /* 62 */ {"clw con vi", "cloud water content, conv clouds, vert integrated [kg/m**2]"},
    /* 63 */ {"cli con", "specific cloud ice content, convective clouds [kg/kg]"},
    /* 64 */ {"cli con vi", "cloud ice content, conv clouds, vert integrated [kg/m**2]"},
    /* 65 */ {"mass fl co", "convective mass flux [kg/(s*m**2)]"},
    /* 66 */ {"upd vel co", "updraft velocity, convection [m/s]"},
    /* 67 */ {"entr p co", "entrainment parameter, convection [m**(-1)]"},
    /* 68 */ {"HBAS_CON", "cloud base, convective clouds (above msl) [m]"},
    /* 69 */ {"HTOP_CON", "cloud top, convective clouds (above msl) [m]"},
    /* 70 */ {"con layers", "convective layers (00...77)  (BKE) [1]"},
    /* 71 */ {"KO-index", "KO-index [1]"},
    /* 72 */ {"BAS_CON", "convection base index [1]"},
    /* 73 */ {"TOP_CON", "convection top index [1]"},
    /* 74 */ {"DT_CON", "convective temperature tendency [K/s]"},
    /* 75 */ {"DQV_CON", "convective tendency of specific humidity [s**(-1)]"},
    /* 76 */ {"H ten co", "convective tendency of total heat [J/(kg*s)]"},
    /* 77 */ {"QDW ten co", "convective tendency of total water [s**(-1)]"},
    /* 78 */ {"DU_CON", "convective momentum tendency (X-component) [m/s**2]"},
    /* 79 */ {"DV_CON", "convective momentum tendency (Y-component) [m/s**2]"},
    /* 80 */ {"vor ten co", "convective vorticity tendency [s**(-2)]"},
    /* 81 */ {"div ten co", "convective divergence tendency [s**(-2)]"},
    /* 82 */ {"HTOP_DC", "top of dry convection (above msl) [m]"},
    /* 83 */ {"top ind dc", "dry convection top index [1]"},
    /* 84 */ {"HZEROCL", "height of 0 degree Celsius isotherm above msl [m]"},
    /* 85 */ {"SNOWLMT", "height of snowfall limit above msl [m]"},
    /* 86 */ {"var86", "undefined"},
    /* 87 */ {"var87", "undefined"},
    /* 88 */ {"var88", "undefined"},
    /* 89 */ {"var89", "undefined"},
    /* 90 */ {"var90", "undefined"},
    /* 91 */ {"var91", "undefined"},
    /* 92 */ {"var92", "undefined"},
    /* 93 */ {"var93", "undefined"},
    /* 94 */ {"var94", "undefined"},
    /* 95 */ {"var95", "undefined"},
    /* 96 */ {"var96", "undefined"},
    /* 97 */ {"var97", "undefined"},
    /* 98 */ {"var98", "undefined"},
    /* 99 */ {"QRS_GSP", "spec water cont of rain/snow needed for w loading [kg/kg]"},
    /* 100 */ {"PRR_GSP", "surface precipitation rate, rain, grid scale [kg/(s*m**2)]"},
    /* 101 */ {"PRS_GSP", "surface precipitation rate, snow, grid scale [kg/(s*m**2)]"},
    /* 102 */ {"RAIN_GSP", "surface precipitation amount, rain, grid scale [kg/m**2]"},
    /* 103 */ {"condens gs", "condensation rate, grid scale [kg/(kg*s)]"},
    /* 104 */ {"autocon gs", "autoconversion rate, grid scale   (C+C  --> R) [kg/(kg*s)]"},
    /* 105 */ {"accret gs", "accretion rate, grid scale        (R+C  --> R) [kg/(kg*s)]"},
    /* 106 */ {"nucleat gs", "nucleation rate, grid scale       (C+C  --> S) [kg/(kg*s)]"},
    /* 107 */ {"riming gs", "riming rate, grid scale           (S+C  --> S) [kg/(kg*s)]"},
    /* 108 */ {"deposit gs", "deposition rate, grid scale       (S+V <--> S) [kg/(kg*s)]"},
    /* 109 */ {"melting gs", "melting rate, grid scale          (S    --> R) [kg/(kg*s)]"},
    /* 110 */ {"evapor gs", "evaporation rate, grid scale      (R+V <--  R) [kg/(kg*s)]"},
    /* 111 */ {"PRR_CON", "surface precipitation rate, rain, convective [kg/(s*m**2)]"},
    /* 112 */ {"PRS_CON", "surface precipitation rate, snow, convective [kg/(s*m**2)]"},
    /* 113 */ {"RAIN_CON", "surface precipitation amount, rain, convective [kg/m**2]"},
    /* 114 */ {"condens co", "condensation rate, convective [kg/(kg*s)]"},
    /* 115 */ {"autocon co", "autoconversion rate, convective [kg/(kg*s)]"},
    /* 116 */ {"accret co", "accretion rate, convective [kg/(kg*s)]"},
    /* 117 */ {"nucleat co", "nucleation rate, convective [kg/(kg*s)]"},
    /* 118 */ {"riming co", "riming rate, convective [kg/(kg*s)]"},
    /* 119 */ {"sublim co", "sublimation rate, convective [kg/(kg*s)]"},
    /* 120 */ {"melting co", "melting rate, convective [kg/(kg*s)]"},
    /* 121 */ {"evapor co", "evaporation rate, convective [kg/(kg*s)]"},
    /* 122 */ {"rain am", "rain amount, grid-scale plus convective [kg/m**2]"},
    /* 123 */ {"snow am", "snow amount, grid-scale plus convective [kg/m**2]"},
    /* 124 */ {"DT_GSP", "temperature tendency, grid-scale condensation [K/s]"},
    /* 125 */ {"DQV_GSP", "tendency of specific humidity, grid-scale condens [s**(-1)]"},
    /* 126 */ {"H ten gs", "tendency of total heat, grid-scale condensation [J/(kg*s)]"},
    /* 127 */ {"DQC_GSP", "tendency of total water, grid-scale condensation [s**(-1)]"},
    /* 128 */ {"snowfall", "snowfall  (dimension"},
    /* 129 */ {"FRESHSNW", "fresh snow factor [1]"},
    /* 130 */ {"DQI_GSP", "tend of the sp cl ice cont due to gs precipitation [kg/(kg*s)]"},
    /* 131 */ {"PRG_GSP", "surface precipitation rate, graupel, grid scale [kg/(s*m**2)]"},
    /* 132 */ {"GRAU_GSP", "surface precipitation amount, graupel, grid scale [kg/(m**2)]"},
    /* 133 */ {"RHO_SNOW", "snow density [kg/m**3"},
    /* 134 */ {"var134", "undefined"},
    /* 135 */ {"var135", "undefined"},
    /* 136 */ {"var136", "undefined"},
    /* 137 */ {"var137", "undefined"},
    /* 138 */ {"var138", "undefined"},
    /* 139 */ {"PP", "deviation of pressure from reference value [Pa]"},
    /* 140 */ {"var140", "undefined"},
    /* 141 */ {"var141", "undefined"},
    /* 142 */ {"var142", "undefined"},
    /* 143 */ {"var143", "undefined"},
    /* 144 */ {"var144", "undefined"},
    /* 145 */ {"var145", "undefined"},
    /* 146 */ {"var146", "undefined"},
    /* 147 */ {"var147", "undefined"},
    /* 148 */ {"var148", "undefined"},
    /* 149 */ {"KE", "kinetic energy ((u**2 + v**2) / 2) [(m**2/s**2)]"},
    /* 150 */ {"hdi coeff", "coefficient of horizontal diffusion [m**2/s]"},
    /* 151 */ {"dissp rate", "dissipation rate [W/(Pa*m**2)]"},
    /* 152 */ {"TKE", "turbulent kinetic energy [(m/s)**2]"},
    /* 153 */ {"TKVM", "coefficient of vertical diffusion, momentum [m**2/s]"},
    /* 154 */ {"TKVH", "coefficient of vertical diffusion, heat [m**2/s]"},
    /* 155 */ {"vdi coe cw", "coefficient of vertical diffusion, cloud water [m**2/s]"},
    /* 156 */ {"vdi coe ci", "coefficient of vertical diffusion, cloud ice [m**2/s]"},
    /* 157 */ {"vdi coe vp", "coefficient of vertical diffusion, water vapour [m**2/s]"},
    /* 158 */ {"dis len m", "turbulent dissipation length for momentum [m]"},
    /* 159 */ {"dis len h", "turbulent dissipation length for heat [m]"},
    /* 160 */ {"var u mom", "variance of u-component of momentum [(m/s)**2]"},
    /* 161 */ {"var v mom", "variance of v-component of momentum [(m/s)**2]"},
    /* 162 */ {"var w mom", "variance of w-component of momentum [(m/s)**2]"},
    /* 163 */ {"var temp", "variance of temperature [K**2]"},
    /* 164 */ {"var cl wat", "variance of specific cloud water content [(kg/kg)**2]"},
    /* 165 */ {"var cl ice", "variance of specific cloud ice content [(kg/kg)**2]"},
    /* 166 */ {"var vap mr", "variance of water vapour mixing ratio [(kg/kg)**2]"},
    /* 167 */ {"c wat flux", "turbulent vertical flux of spec cloud water [m/s]"},
    /* 168 */ {"c ice flux", "turbulent vertical flux of spec cloud ice [m/s]"},
    /* 169 */ {"w vap flux", "turbulent vertical flux of water vapour mix ratio [m/s]"},
    /* 170 */ {"TCM", "drag coefficient CD [1]"},
    /* 171 */ {"TCH", "transfer coefficient CH (sensible heat) [1]"},
    /* 172 */ {"tr coef CQ", "transfer coefficient CQ (latent heat) [1]"},
    /* 173 */ {"PBL-top h", "PBL-top h [m]"},
    /* 174 */ {"T-jump  h", "temperature jump at PBL-top [K]"},
    /* 175 */ {"q-jump  h", "specific humidity jump at PBL-top [kg/kg]"},
    /* 176 */ {"entr at h", "entrainment at PBL-top [kg/(s*m**2)]"},
    /* 177 */ {"mass fl h", "upward mass flux at PBL-top [kg/(s*m**2)]"},
    /* 178 */ {"cl cov PBL", "cloud cover of PBL-clouds (0...1) [1]"},
    /* 179 */ {"cl wat PBL", "specific cloud water content of PBL-clouds [kg/kg]"},
    /* 180 */ {"cl top PBL", "cloud top of PBL-clouds [m]"},
    /* 181 */ {"cl bas PBL", "cloud base of PBL-clouds [m]"},
    /* 182 */ {"moun wav X", "vertical mountain wave momentum flux (X component) [kg/(m*s**2)]"},
    /* 183 */ {"moun wav Y", "vertical mountain wave momentum flux (Y component) [kg/(m*s**2)]"},
    /* 184 */ {"wave Ri", "wave Richardson number [1]"},
    /* 185 */ {"wav div X", "mountain wave momentum flux divergence (X comp) [m/s**2]"},
    /* 186 */ {"wav div Y", "mountain wave momentum flux divergence (Y comp) [m/s**2]"},
    /* 187 */ {"VMAX_10M", "maximum wind velocity [m/s]"},
    /* 188 */ {"wav dis vi", "mountain wave dissipation, vert integrated [W/m**2]"},
    /* 189 */ {"wv en flux", "vertical wave energy flux [kg*m/s**4]"},
    /* 190 */ {"var190", "undefined"},
    /* 191 */ {"var191", "undefined"},
    /* 192 */ {"var192", "undefined"},
    /* 193 */ {"var193", "undefined"},
    /* 194 */ {"var194", "undefined"},
    /* 195 */ {"var195", "undefined"},
    /* 196 */ {"var196", "undefined"},
    /* 197 */ {"T_SO", "temperature of soil layers [K]"},
    /* 198 */ {"W_SO", "water + ice content of soil layers [kg/(m**2)]"},
    /* 199 */ {"W_SO_ICE", "ice content of soil layers [kg/(m**2)]"},
    /* 200 */ {"W_I", "water content of interception store [kg/(m**2)]"},
    /* 201 */ {"interc ice", "icebit for interception store [1]"},
    /* 202 */ {"snow fract", "snow fraction [1]"},
    /* 203 */ {"T_SNOW", "snow temperature [K]"},
    /* 204 */ {"foliag tem", "foliage temperature [K]"},
    /* 205 */ {"infiltrat", "infiltration [m/s]"},
    /* 206 */ {"runoff", "runoff [m/s]"},
    /* 207 */ {"soil evap", "bare soil evaporation [m/s]"},
    /* 208 */ {"plant tran", "plant transpiration [m/s]"},
    /* 209 */ {"inter evap", "interception store evaporation [m/s]"},
    /* 210 */ {"water evap", "evaporation from water surfaces [m/s]"},
    /* 211 */ {"aero resis", "aerodynamic resistance [s/m]"},
    /* 212 */ {"plant res", "plant resistance [s/m]"},
    /* 213 */ {"soil res", "soil resistance [s/m]"},
    /* 214 */ {"total evap", "total evaporation (water, soil, plants) [m/s]"},
    /* 215 */ {"T_ICE", "temperature of sea ice [K]"},
    /* 216 */ {"var216", "undefined"},
    /* 217 */ {"max wind m", "maximum wind velocity (modified) [m/s]"},
    /* 218 */ {"var218", "undefined"},
    /* 219 */ {"var219", "undefined"},
    /* 220 */ {"var220", "undefined"},
    /* 221 */ {"var221", "undefined"},
    /* 222 */ {"var222", "undefined"},
    /* 223 */ {"var223", "undefined"},
    /* 224 */ {"var224", "undefined"},
    /* 225 */ {"var225", "undefined"},
    /* 226 */ {"var226", "undefined"},
    /* 227 */ {"var227", "undefined"},
    /* 228 */ {"var228", "undefined"},
    /* 229 */ {"var229", "undefined"},
    /* 230 */ {"XYZ", "S1 [1]"},
    /* 231 */ {"RHS_SI", "S2 [1]"},
    /* 232 */ {"DTTDIV", "S3 [1]"},
    /* 233 */ {"SOTR_RAD", "effective transmissivity of solar rad. [1]"},
    /* 234 */ {"GEN_TEN1", "averaged tendencies [x/s]"},
    /* 235 */ {"GEN_TEN2", "averaged tendencies [x/s]"},
    /* 236 */ {"S7", "S7 [1]"},
    /* 237 */ {"S8", "S8 [1]"},
    /* 238 */ {"S9", "S9 [1]"},
    /* 239 */ {"S10", "S10 [1]"},
    /* 240 */ {"MFLX_CON", "cloud base mass flux kg/(s*m**2)"},
    /* 241 */ {"CAPE_CON", "convective available potential energy [J/kg]"},
    /* 242 */ {"QCVG_CON", "moisture convergence for Kuo-type closure [1/s]"},
    /* 243 */ {"TKE_CON", "convective turbulent energy [J/kg]"},
    /* 244 */ {"MOS pTS fq", "MOS Gewitter-Wahrscheinlichkeit (frequent) [1]"},
    /* 245 */ {"MOS TS cov", "MOS Gewitteranteil (occasional - frequent (1 - 2)) [1]"},
    /* 246 */ {"S17", "S17 [1]"},
    /* 247 */ {"S18", "S18 [1]"},
    /* 248 */ {"S19", "S19 [1]"},
    /* 249 */ {"S20", "S20 [1]"},
    /* 250 */ {"MOS TSISO1", "MOS Wahrscheinlichkeit mindestens ein Blitz [1]"},
    /* 251 */ {"MOS TSISO2", "MOS Wahrscheinlichkeit mindestens zehn Blitze [1]"},
    /* 252 */ {"MOS TSISO3", "MOS Wahrscheinlichkeit mindestens hundert Blitze [1]"},
    /* 253 */ {"MOS TS DEN", "MOS Vorhersage der Blitzanzahl [1]"},
    /* 254 */ {"MOS TS OCC", "MOS Gewitter-Wahrscheinlichkeit (occasional) [1]"},
    /* 255 */ {"MOS TS FRQ", "MOS Gewitter-Wahrscheinlichkeit (frequent) [1]"},
};

/*
 * GRIB table 202 at DWD
 *     Helmut P. Frank, 30.08.2001
 * updated 24.07.2003: UV_Ind_F_h, BasicUV_IF, UV_Ind_W_h, UV_IndmaxF,
 *                     "gesamt O3", UV_IndmaxW, "h UV_IndMx"
 *         19.10.2005: AER_SEA, AER_LAN, AER_URB, AER_DES, and others
 *          2.11.2005: Use RLAT, RLON instead of PHI, RLA.
 */

const struct ParmTable parm_table_dwd_202[256] = {
    /* 0 */ {"var0", "undefined"},
    /* 1 */ {"Seeg_peak", "jonswap parameter fm [s**(-1)]"},
    /* 2 */ {"Seeg_alpha", "jonswap parameter alpha [1]"},
    /* 3 */ {"Seeg_gamma", "jonswap parameter gamma [1]"},
    /* 4 */ {"Seeg_dir", "Seegang direction [degree true]"},
    /* 5 */ {"Seeg_energ", "Seegang energy densitiy [(m**2)*(s**2)]"},
    /* 6 */ {"Seeg_icemk", "Seegang ice mask [1]"},
    /* 7 */ {"peak p sw", "peak period of swell [s]"},
    /* 8 */ {"peak p ww", "peak period of wind waves [s]"},
    /* 9 */ {"var9", "undefined"},
    /* 10 */ {"var10", "undefined"},
    /* 11 */ {"var11", "undefined"},
    /* 12 */ {"var12", "undefined"},
    /* 13 */ {"var13", "undefined"},
    /* 14 */ {"var14", "undefined"},
    /* 15 */ {"var15", "undefined"},
    /* 16 */ {"var16", "undefined"},
    /* 17 */ {"var17", "undefined"},
    /* 18 */ {"var18", "undefined"},
    /* 19 */ {"var19", "undefined"},
    /* 20 */ {"Var. Geop.", "Varianz Geopotential [(m/s)**4]"},
    /* 21 */ {"Var. T", "Varianz Temperatur [K**2]"},
    /* 22 */ {"Var. u", "Varianz Zonalwind [(m/s)**2]"},
    /* 23 */ {"Var. v", "Varianz Meridionalwind [(m/s)**2]"},
    /* 24 */ {"Var. q", "Varianz spezifische Feuchte [(kg/kg)**2]"},
    /* 25 */ {"Mer. Imptr", "Meridionaler Impulstransport [(m/s)**2]"},
    /* 26 */ {"Mer. TrEpt", "Meridionaler Transport potentieller Energie [(m/s)**3]"},
    /* 27 */ {"Mer. TrsW", "Meridionaler Transport sensibler Waerme [K*(m/s)]"},
    /* 28 */ {"Mer. TrlW", "Meridionaler Transport latenter Waerme [(kg/kg)*(m/s)]"},
    /* 29 */ {"Ver. TrEpt", "Vertikaler Transport potentieller Energie [(m/s)**2*(Pa/s)]"},
    /* 30 */ {"Ver. TrsW", "Vertikaler Transport sensibler Waerme [K*(Pa/s)]"},
    /* 31 */ {"Ver.TrlW", "Vertikaler Transport latenter Waerme [(kg/kg)*(Pa/s)]"},
    /* 32 */ {"var32", "undefined"},
    /* 33 */ {"var33", "undefined"},
    /* 34 */ {"var34", "undefined"},
    /* 35 */ {"var35", "undefined"},
    /* 36 */ {"var36", "undefined"},
    /* 37 */ {"var37", "undefined"},
    /* 38 */ {"var38", "undefined"},
    /* 39 */ {"var39", "undefined"},
    /* 40 */ {"VarAF Geop", "Varianz des Analyse-Fehlers Geopotential [(m/s)**4]"},
    /* 41 */ {"VarAF u", "Varianz des Analyse-Fehlers Zonalwind [(m/s)**2]"},
    /* 42 */ {"VarAF v", "Varianz des Analyse-Fehlers Meridionalwind [(m/s)**2]"},
    /* 43 */ {"var43", "undefined"},
    /* 44 */ {"DU_SSO", "undefined"},
    /* 45 */ {"DV_SSO", "undefined"},
    /* 46 */ {"SSO_STDH", "standard deviation of subgrid scale orogr. height [m]"},
    /* 47 */ {"SSO_GAMMA", "anisotropy of topography [1]"},
    /* 48 */ {"SSO_THETA", "angle betw. principal axis of orogr. and global E [1]"},
    /* 49 */ {"SSO_SIGMA", "mean slope of subgrid scale orography [1]"},
    /* 50 */ {"oro varian", "subgrid-scale variance of orography [m**2]"},
    /* 51 */ {"E-W oro va", "E-W component of subgrid-scale variance of orogr [m**2]"},
    /* 52 */ {"N-S oro va", "N-S component of subgrid-scale variance of orogr [m**2]"},
    /* 53 */ {"NW-SE o va", "NW-SE component of subgrid-scale variance of orogr [m**2]"},
    /* 54 */ {"NE-SW o va", "NE-SW component of subgrid-scale variance of orogr [m**2]"},
    /* 55 */ {"inl w frac", "fraction of inland water [1]"},
    /* 56 */ {"EMISS_RAD", "surface emissivity [1]"},
    /* 57 */ {"SOILTYP", "soil texture [1]"},
    /* 58 */ {"soil color", "soil color [1]"},
    /* 59 */ {"soil drain", "soil drainage [1]"},
    /* 60 */ {"ground wat", "ground water table [m]"},
    /* 61 */ {"LAI", "leaf area index [1]"},
    /* 62 */ {"ROOTDP", "root depth [m]"},
    /* 63 */ {"root dens", "root density [1]"},
    /* 64 */ {"HMO3", "height of maximum of ozone concentration [Pa]"},
    /* 65 */ {"VIO3", "total vertically integrated ozone content [Pa]"},
    /* 66 */ {"ld-sea msk", "land-sea mask [1]"},
    /* 67 */ {"PLCOV_MX", "ground fraction covered by plants (vegetation p.) [1]"},
    /* 68 */ {"PLCOV_MN", "ground fraction covered by plants (time of rest) [1]"},
    /* 69 */ {"LAI_MX", "leaf area index (vegetation period) [1]"},
    /* 70 */ {"LAI_MN", "leaf area index (time of rest) [1]"},
    /* 71 */ {"Orographie", "Orographie + Land-Meer-Verteilung [m]"},
    /* 72 */ {"r length m", "roughness length momentum [m]"},
    /* 73 */ {"r length h", "roughness length heat [m]"},
    /* 74 */ {"var smc", "variance of soil moisture content [kg**2/m**4]"},
    /* 75 */ {"FOR_E", "ground fraction covered by evergreen forest [1]"},
    /* 76 */ {"FOR_D", "ground fraction covered by deciduous forest [1]"},
    /* 77 */ {"NDVI", "normalized differential vegetation index [1]"},
    /* 78 */ {"NDVI_MAX", "annual max. of norm. differential vegetation index [1]"},
    /* 79 */ {"NDVIRATIO", "proportion of act.value/max. norm.diff.veg.index [1]"},
    /* 80 */ {"AER_SEA", "aerosol optical depth, type sea [1]"},
    /* 81 */ {"AER_LAN", "aerosol optical depth, type land [1]"},
    /* 82 */ {"AER_URB", "aerosol optical depth, type urban [1]"},
    /* 83 */ {"AER_DES", "aerosol optical depth, type desert [1]"},
    /* 84 */ {"var84", "undefined"},
    /* 85 */ {"var85", "undefined"},
    /* 86 */ {"var86", "undefined"},
    /* 87 */ {"var87", "undefined"},
    /* 88 */ {"var88", "undefined"},
    /* 89 */ {"var89", "undefined"},
    /* 90 */ {"var90", "undefined"},
    /* 91 */ {"var91", "undefined"},
    /* 92 */ {"var92", "undefined"},
    /* 93 */ {"var93", "undefined"},
    /* 94 */ {"var94", "undefined"},
    /* 95 */ {"var95", "undefined"},
    /* 96 */ {"var96", "undefined"},
    /* 97 */ {"var97", "undefined"},
    /* 98 */ {"var98", "undefined"},
    /* 99 */ {"var99", "undefined"},
    /* 100 */ {"var100", "undefined"},
    /* 101 */ {"tidal tend", "tidal tendencies [(m/s)**2]"},
    /* 102 */ {"diab heatg", "sum of diabatic heating terms [K/s]"},
    /* 103 */ {"adiab heat", "total adiabatic heating [K/s]"},
    /* 104 */ {"adv q tend", "advective tendency of specific humidity [s**(-1)]"},
    /* 105 */ {"nadv q ten", "non-advective tendency of specific humidity [s**(-1)]"},
    /* 106 */ {"adv m te X", "advective momentum tendency (X component) [m/s**2]"},
    /* 107 */ {"adv m te Y", "advective momentum tendency (Y component) [m/s**2]"},
    /* 108 */ {"nad m te X", "non-advective momentum tendency (X component) [m/s**2]"},
    /* 109 */ {"nad m te Y", "non-advective momentum tendency (Y component) [m/s**2]"},
    /* 110 */ {"torque", "sum of mountain and frictional torque [kg*(m/s)**2]"},
    /* 111 */ {"budget val", "budget values [1]"},
    /* 112 */ {"scale fact", "scale factor [1]"},
    /* 113 */ {"FC", "Coriolis parameter [s**(-1)]"},
    /* 114 */ {"RLAT", "latitude [degr N]"},
    /* 115 */ {"RLON", "longitude [degr E]"},
    /* 116 */ {"relax fact", "relaxation factor (lateral boundary, LAM) [1]"},
    /* 117 */ {"climsstint", "climatic sea surface temp interpolated in time [degr C]"},
    /* 118 */ {"pot vortic", "potential vorticity [K*m**2/(s*kg)]"},
    /* 119 */ {"ln ps", "log surface pressure [1]"},
    /* 120 */ {"var120", "undefined"},
    /* 121 */ {"ZTD", "delay of the GPS signal through the atmosphere [m]"},
    /* 122 */ {"ZWD", "delay of the GPS signal through a wet atmosphere [m]"},
    /* 123 */ {"ZHD", "delay of the GPS signal through a dry atmosphere [m]"},
    /* 124 */ {"var124", "undefined"},
    /* 125 */ {"var125", "undefined"},
    /* 126 */ {"var126", "undefined"},
    /* 127 */ {"var127", "undefined"},
    /* 128 */ {"var128", "undefined"},
    /* 129 */ {"var129", "undefined"},
    /* 130 */ {"var130", "undefined"},
    /* 131 */ {"var131", "undefined"},
    /* 132 */ {"var132", "undefined"},
    /* 133 */ {"var133", "undefined"},
    /* 134 */ {"var134", "undefined"},
    /* 135 */ {"var135", "undefined"},
    /* 136 */ {"var136", "undefined"},
    /* 137 */ {"var137", "undefined"},
    /* 138 */ {"var138", "undefined"},
    /* 139 */ {"var139", "undefined"},
    /* 140 */ {"var140", "undefined"},
    /* 141 */ {"var141", "undefined"},
    /* 142 */ {"var142", "undefined"},
    /* 143 */ {"var143", "undefined"},
    /* 144 */ {"var144", "undefined"},
    /* 145 */ {"var145", "undefined"},
    /* 146 */ {"var146", "undefined"},
    /* 147 */ {"var147", "undefined"},
    /* 148 */ {"var148", "undefined"},
    /* 149 */ {"var149", "undefined"},
    /* 150 */ {"SO2-conc", "SO2-concentration [10**(-6)*g/m**3]"},
    /* 151 */ {"SO2-dryd", "SO2-dry deposition [10**(-3)*g/m**2]"},
    /* 152 */ {"SO2-wetd", "SO2-wet deposition [10**(-3)*g/m**2]"},
    /* 153 */ {"SO4-conc", "SO4-concentration [10**(-6)*g/m**3]"},
    /* 154 */ {"SO4-dryd", "SO4-dry deposition [10**(-3)*g/m**2]"},
    /* 155 */ {"SO4-wetd", "SO4-wet deposition [10**(-3)*g/m**2]"},
    /* 156 */ {"NO-conc", "NO-concentration [10**(-6)*g/m**3]"},
    /* 157 */ {"NO-dryd", "NO-dry deposition [10**(-3)*g/m**2]"},
    /* 158 */ {"NO-wetd", "NO-wet deposition [10**(-3)*g/m**2]"},
    /* 159 */ {"NO2-conc", "NO2-concentration [10**(-6)*g/m**3]"},
    /* 160 */ {"NO2-dryd", "NO2-dry deposition [10**(-3)*g/m**2]"},
    /* 161 */ {"NO2-wetd", "NO2-wet deposition [10**(-3)*g/m**2]"},
    /* 162 */ {"NO3-conc", "NO3-concentration [10**(-6)*g/m**3]"},
    /* 163 */ {"NO3-dryd", "NO3-dry deposition [10**(-3)*g/m**2]"},
    /* 164 */ {"NO3-wetd", "NO3-wet deposition [10**(-3)*g/m**2]"},
    /* 165 */ {"HNO3-conc", "HNO3-concentration [10**(-6)*g/m**3]"},
    /* 166 */ {"HNO3-dryd", "HNO3-dry deposition [10**(-3)*g/m**2]"},
    /* 167 */ {"HNO3-wetd", "HNO3-wet deposition [10**(-3)*g/m**2]"},
    /* 168 */ {"NH3-conc", "NH3-concentration [10**(-6)*g/m**3]"},
    /* 169 */ {"NH3-dryd", "NH3-dry deposition [10**(-3)*g/m**2]"},
    /* 170 */ {"NH3-wetd", "NH3-wet deposition [10**(-3)*g/m**2]"},
    /* 171 */ {"NH4-conc", "NH4-concentration [10**(-6)*g/m**3]"},
    /* 172 */ {"NH4-dryd", "NH4-dry deposition [10**(-3)*g/m**2]"},
    /* 173 */ {"NH4-wetd", "NH4-wet deposition [10**(-3)*g/m**2]"},
    /* 174 */ {"O3-conc", "O3-concentration [10**(-6)*g/m**3]"},
    /* 175 */ {"PAN-conc", "PAN-concentration [10**(-6)*g/m**3]"},
    /* 176 */ {"PAN-dryd", "PAN-dry deposition [10**(-3)*g/m**2]"},
    /* 177 */ {"OH-conc", "OH-concentration [10**(-6)*g/m**3]"},
    /* 178 */ {"O3-dryd", "O3-dry deposition [10**(-3)*g/m**2]"},
    /* 179 */ {"O3-wetd", "O3-wet deposition [10**(-3)*g/m**2]"},
    /* 180 */ {"O3", "O3-mixing ratio [kg/kg]"},
    /* 181 */ {"var181", "undefined"},
    /* 182 */ {"var182", "undefined"},
    /* 183 */ {"var183", "undefined"},
    /* 184 */ {"var184", "undefined"},
    /* 185 */ {"var185", "undefined"},
    /* 186 */ {"var186", "undefined"},
    /* 187 */ {"var187", "undefined"},
    /* 188 */ {"var188", "undefined"},
    /* 189 */ {"var189", "undefined"},
    /* 190 */ {"var190", "undefined"},
    /* 191 */ {"var191", "undefined"},
    /* 192 */ {"var192", "undefined"},
    /* 193 */ {"var193", "undefined"},
    /* 194 */ {"var194", "undefined"},
    /* 195 */ {"var195", "undefined"},
    /* 196 */ {"var196", "undefined"},
    /* 197 */ {"var197", "undefined"},
    /* 198 */ {"var198", "undefined"},
    /* 199 */ {"var199", "undefined"},
    /* 200 */ {"I131-conc", "I131-concentration [Bq/m**3]"},
    /* 201 */ {"I131-dryd", "I131-dry deposition [Bq/m**2]"},
    /* 202 */ {"I131-wetd", "I131-wet deposition [Bq/m**2]"},
    /* 203 */ {"Cs137-conc", "Cs137-concentration [Bq/m**3]"},
    /* 204 */ {"Cs137-dryd", "Cs1370dry deposition [Bq/m**2]"},
    /* 205 */ {"Cs137-wetd", "Cs137-wet deposition [Bq/m**2]"},
    /* 206 */ {"Te132-conc", "Te132-concentration [Bq/m**3]"},
    /* 207 */ {"Te132-dryd", "Te132-dry deposition [Bq/m**2]"},
    /* 208 */ {"Te132-wetd", "Te132-wet deposition [Bq/m**2]"},
    /* 209 */ {"Zr95-conc", "Zr95-concentration [Bq/m**3]"},
    /* 210 */ {"Zr95-dryd", "Zr95-dry deposition [Bq/m**2]"},
    /* 211 */ {"Zr95-wetd", "Zr95-wet deposition [Bq/m**2]"},
    /* 212 */ {"var212", "undefined"},
    /* 213 */ {"var213", "undefined"},
    /* 214 */ {"var214", "undefined"},
    /* 215 */ {"var215", "undefined"},
    /* 216 */ {"var216", "undefined"},
    /* 217 */ {"var217", "undefined"},
    /* 218 */ {"var218", "undefined"},
    /* 219 */ {"var219", "undefined"},
    /* 220 */ {"var220", "undefined"},
    /* 221 */ {"var221", "undefined"},
    /* 222 */ {"var222", "undefined"},
    /* 223 */ {"var223", "undefined"},
    /* 224 */ {"var224", "undefined"},
    /* 225 */ {"var225", "undefined"},
    /* 226 */ {"var226", "undefined"},
    /* 227 */ {"var227", "undefined"},
    /* 228 */ {"var228", "undefined"},
    /* 229 */ {"var229", "undefined"},
    /* 230 */ {"var230", "undefined"},
    /* 231 */ {"USTR_SSO", "Mom. flux, u component, due to SSO-effects [(N/(m**2)]"},
    /* 232 */ {"VSTR_SSO", "Mom. flux, v component, due to SSO-effects [(N/(m**2)]"},
    /* 233 */ {"VDIS_SSO", "Dissipation of kinetic energy due to SSO-effects [(W/(m**2)]"},
    /* 234 */ {"var234", "undefined"},
    /* 235 */ {"var235", "undefined"},
    /* 236 */ {"var236", "undefined"},
    /* 237 */ {"var237", "undefined"},
    /* 238 */ {"var238", "undefined"},
    /* 239 */ {"var239", "undefined"},
    /* 240 */ {"UV_Ind_F_h", "UV_Index corr. for albedo+altitude,cloudless(F), h [1]"},
    /* 241 */ {"BasicUV_IF", "Basic UV_Index m.s.l.,fixed albedo,cloudless(F), h [1]"},
    /* 242 */ {"UV_Ind_W_h", "UV_Index corrected for albedo+altitude+clouds(W),h [1]"},
    /* 243 */ {"UV_IndmaxF", "UV_Index cloudless (F), daily maximum [1]"},
    /* 244 */ {"SB-Index", "Sonnenbrand-Index [(W*10**(-3))/m**2]"},
    /* 245 */ {"SB-Index W", "Sonnenbrand-Index bei mittl. Bewoelkung (08z-12z) [(W*10**(-3))/m**2]"},
    /* 246 */ {"Kan.UVB-WI", "Kanadischer UVB-Warnindex (bew|lkungsreduziert) [(W*10**(-3))/m**2]"},
    /* 247 */ {"gesamt O3", "total column ozone (Gesamtozon) [Dobson Unit, DU]"},
    /* 248 */ {"UV_IndmaxW", "UV_Index clouded (W), daily maximum [1]"},
    /* 249 */ {"h UV_IndMx", "time of UV_Index maximum [h UTC]"},
    /* 250 */ {"var250", "undefined"},
    /* 251 */ {"var251", "undefined"},
    /* 252 */ {"var252", "undefined"},
    /* 253 */ {"var253", "undefined"},
    /* 254 */ {"var254", "undefined"},
    /* 255 */ {"var255", "undefined"},
};

/*
 * GRIB table 203 at DWD
 *     Helmut P. Frank, 30.08.2001
 *     updated: 19.10.2005
 */

const struct ParmTable parm_table_dwd_203[256] = {
    /* 0 */ {"var0", "undefined"},
    /* 1 */ {"pressure", "pressure [hPa]"},
    /* 2 */ {"geopot h", "geopotential height [10 * gpm]"},
    /* 3 */ {"var3", "undefined"},
    /* 4 */ {"temperatur", "temperature [1*degree Celsius]"},
    /* 5 */ {"dew-pnt te", "dew-point temperature [1*degree Celsius]"},
    /* 6 */ {"windcompXY", "wind components X/Y (X*100000 + ((Y*10)+5000)) [m/s]"},
    /* 7 */ {"geomet h", "geometrical height [kft]"},
    /* 8 */ {"geomet h", "geometrical height [hft]"},
    /* 9 */ {"wind di/sp", "wind direction and speed (dd*1000 + ff) [1*degree, 1*kt]"},
    /* 10 */ {"3 h pr cha", "3 hour pressure change [Pa/(3*h)]"},
    /* 11 */ {"Schnee-Mge", "Schneemenge [mm]"},
    /* 12 */ {"var12", "undefined"},
    /* 13 */ {"Bod-Wass-G", "Bodenwassergehalt [mm]"},
    /* 14 */ {"var14", "undefined"},
    /* 15 */ {"stab. ind.", "stability index [K]"},
    /* 16 */ {"var16", "undefined"},
    /* 17 */ {"var17", "undefined"},
    /* 18 */ {"max wind", "maximum wind velocity [km/h]"},
    /* 19 */ {"max wind", "maximum wind velocity [kt]"},
    /* 20 */ {"wind di/sp", "wind direction and speed (dd*1000 + ff) [5*degrees, 1*(m/s)]"},
    /* 21 */ {"wind di/sp", "wind direction and speed (dd*1000 + ff) [5*degrees, 1*kt]"},
    /* 22 */ {"wave di/he", "direction and height of wind waves (dd*1000 + h) [1*degree, 1*cm]"},
    /* 23 */ {"swe. di/he", "direction and height of swell (dd*1000 + h) [1*degree, 1*cm]"},
    /* 24 */ {"wave m d/h", "mean direction and height of waves (dd*1000 + h) [1*degree, 1*cm]"},
    /* 25 */ {"wind speed", "wind speed [kt]"},
    /* 26 */ {"var26", "undefined"},
    /* 27 */ {"wind compX", "wind component X-direction [kt]"},
    /* 28 */ {"wind compY", "wind component Y-direction [kt]"},
    /* 29 */ {"var29", "undefined"},
    /* 30 */ {"var30", "undefined"},
    /* 31 */ {"var31", "undefined"},
    /* 32 */ {"var32", "undefined"},
    /* 33 */ {"abs voradv", "absolute vorticity advection [1/(s**2)]"},
    /* 34 */ {"var34", "undefined"},
    /* 35 */ {"var35", "undefined"},
    /* 36 */ {"var36", "undefined"},
    /* 37 */ {"var37", "undefined"},
    /* 38 */ {"var38", "undefined"},
    /* 39 */ {"var39", "undefined"},
    /* 40 */ {"var40", "undefined"},
    /* 41 */ {"var41", "undefined"},
    /* 42 */ {"vert. vel.", "vertical velocity [hPa/h]"},
    /* 43 */ {"var43", "undefined"},
    /* 44 */ {"var44", "undefined"},
    /* 45 */ {"var45", "undefined"},
    /* 46 */ {"var46", "undefined"},
    /* 47 */ {"var47", "undefined"},
    /* 48 */ {"var48", "undefined"},
    /* 49 */ {"var49", "undefined"},
    /* 50 */ {"var50", "undefined"},
    /* 51 */ {"var51", "undefined"},
    /* 52 */ {"var52", "undefined"},
    /* 53 */ {"var53", "undefined"},
    /* 54 */ {"var54", "undefined"},
    /* 55 */ {"max. temp.", "maximum temperature [1*degree Celsius]"},
    /* 56 */ {"min. temp.", "minimum temperature [1*degree Celsius]"},
    /* 57 */ {"sul_prob", "probability to perceive sultriness [1]"},
    /* 58 */ {"clo", "value of isolation of clothes [1]"},
    /* 59 */ {"pmva", "predected mean vote (angepasst) [1]"},
    /* 60 */ {"feeled t", "feeled temperature [1*degree Celsius]"},
    /* 61 */ {"sea temper", "sea temperature [1*degree Celsius]"},
    /* 62 */ {"var62", "undefined"},
    /* 63 */ {"var63", "undefined"},
    /* 64 */ {"var64", "undefined"},
    /* 65 */ {"var65", "undefined"},
    /* 66 */ {"var66", "undefined"},
    /* 67 */ {"var67", "undefined"},
    /* 68 */ {"var68", "undefined"},
    /* 69 */ {"var69", "undefined"},
    /* 70 */ {"var70", "undefined"},
    /* 71 */ {"var71", "undefined"},
    /* 72 */ {"var72", "undefined"},
    /* 73 */ {"var73", "undefined"},
    /* 74 */ {"var74", "undefined"},
    /* 75 */ {"var75", "undefined"},
    /* 76 */ {"var76", "undefined"},
    /* 77 */ {"var77", "undefined"},
    /* 78 */ {"var78", "undefined"},
    /* 79 */ {"var79", "undefined"},
    /* 80 */ {"var80", "undefined"},
    /* 81 */ {"var81", "undefined"},
    /* 82 */ {"var82", "undefined"},
    /* 83 */ {"var83", "undefined"},
    /* 84 */ {"var84", "undefined"},
    /* 85 */ {"var85", "undefined"},
    /* 86 */ {"Globalstr.", "Summe der Globalstrahlung ueber einen Zeitraum [kWh/m**2]"},
    /* 87 */ {"Nied-GW-GE", "Niederschlagsart+Gewitter+Glatteis (T23-i) (0..99) [1]"},
    /* 88 */ {"NiedGW-Art", "Niederschlagsart+Gewitter (T23-intern)     (0..99) [1]"},
    /* 89 */ {"NiedGE-Art", "Niederschlagsart+Glatteis (T23-intern)     (0..99) [1]"},
    /* 90 */ {"NiedBewArt", "Kombination Niederschl.-Bew.-Blautherm. (283..407) [1]"},
    /* 91 */ {"Konv.U-Gr.", "Hoehe der Konvektionsuntergrenze ueber Grund [m]"},
    /* 92 */ {"Nied.-Art", "Niederschlagsart -ww- (T23-intern)         (0..99) [1]"},
    /* 93 */ {"Konv.-Art", "Konvektionsart                              (0..4) [1]"},
    /* 94 */ {"Konv.UG-nn", "Hoehe der Konvektionsuntergrenze ueber nn [m]"},
    /* 95 */ {"var95", "undefined"},
    /* 96 */ {"var96", "undefined"},
    /* 97 */ {"var97", "undefined"},
    /* 98 */ {"var98", "undefined"},
    /* 99 */ {"WW", "Wetter (verschluesselt nach ww-Tabelle"},
    /* 100 */ {"geostr Vor", "geostrophische Vorticity [1/s]"},
    /* 101 */ {"Geo VorAdv", "geostrophische  Vorticityadvektion [1/s**2]"},
    /* 102 */ {"VerGraVoAd", "vert. Gradient der geostr. Vorticityadvektion [m/(kg*s)]"},
    /* 103 */ {"Geo TemAdv", "geostrophische Schichtdickenadvektion [m**3/(kg*s)]"},
    /* 104 */ {"Lap TemAdv", "Kruemmung der geostr. Schichtdickenadvektion [m/(kg*s)]"},
    /* 105 */ {"Omega Forc", "Forcing rechte Seite Omegagleichung [m/(kg*s)]"},
    /* 106 */ {"var106", "undefined"},
    /* 107 */ {"Schichtd.A", "Schichtdicken-Advektion [m**3/(kg*s)]"},
    /* 108 */ {"AdGeVoThWi", "Advektion von geostr. Vorticity mit dem therm Wind [m/(kg*s)]"},
    /* 109 */ {"Wind-Div.", "Winddivergenz [1/s]"},
    /* 110 */ {"Q", "Q-vector direction and speed (dd*1000 + fff*1E13) [5*deg,1E13*m**2/kg/s]"},
    /* 111 */ {"Qx", "Q-Vektor X-Komponente [m**2/(kg*s)]"},
    /* 112 */ {"Qy", "Q-Vektor Y-Komponente [m**2/(kg*s)]"},
    /* 113 */ {"Div Q", "Divergenz Q [m/(kg*s)]"},
    /* 114 */ {"FrontoGeQn", "Frontogenesefunktion, Q isother-senkrecht-Kompon. [m**2/(kg*s)]"},
    /* 115 */ {"Qs (geo)", "Qs (geo),Komp. Q-Vektor parallel zu den Isothermen [m**2/(kg*s)]"},
    /* 116 */ {"DivQn(geo)", "Divergenz Qn  geostrophisch [m/(kg*s)]"},
    /* 117 */ {"DivQs(geo)", "Divergenz Qs  geostrophisch [m/(kg*s)]"},
    /* 118 */ {"Fronto Gen", "Frontogenesefunktion [K**2/(m**2*s)]"},
    /* 119 */ {"var119", "undefined"},
    /* 120 */ {"var120", "undefined"},
    /* 121 */ {"var121", "undefined"},
    /* 122 */ {"var122", "undefined"},
    /* 123 */ {"var123", "undefined"},
    /* 124 */ {"FrontoGenP", "Frontogenese-Parameter [1]"},
    /* 125 */ {"Qs-Vektor", "Qs, Komp. Q-Vektor parallel zu den Isothermen [m**2/(kg*s)]"},
    /* 126 */ {"var126", "undefined"},
    /* 127 */ {"Div Qs", "Divergenz Qs [m/(kg*s)]"},
    /* 128 */ {"var128", "undefined"},
    /* 129 */ {"var129", "undefined"},
    /* 130 */ {"IPV", "Isentrope potentielle Vorticity [K*m**2/(s*kg)]"},
    /* 131 */ {"Wind KompX", "Wind X-Komponente auf isentropen Flaechen [m/s]"},
    /* 132 */ {"Wind KompY", "Wind Y-Komponente auf isentropen Flaechen [m/s]"},
    /* 133 */ {"Druck-Ise.", "Druck einer isentropen Flaeche [hPa]"},
    /* 134 */ {"var134", "undefined"},
    /* 135 */ {"var135", "undefined"},
    /* 136 */ {"var136", "undefined"},
    /* 137 */ {"var137", "undefined"},
    /* 138 */ {"var138", "undefined"},
    /* 139 */ {"var139", "undefined"},
    /* 140 */ {"KO-Index", "KO-Index [K]"},
    /* 141 */ {"TT-Index", "Totals-Totals-Index [K]"},
    /* 142 */ {"S-Index", "S-Index [K]"},
    /* 143 */ {"Stein-Ind", "Steinbeck-Index [1]"},
    /* 144 */ {"Baily-Ind", "Baily-Index [1]"},
    /* 145 */ {"Microburst", "Microburst-Index [1]"},
    /* 146 */ {"Cat-Index", "Clear Air Turbulence Index [1/s]"},
    /* 147 */ {"var147", "undefined"},
    /* 148 */ {"Lab-Energ", "Labilit{tsenergie [J/g]"},
    /* 149 */ {"var149", "undefined"},
    /* 150 */ {"Virt T", "Virtuelle Temperatur [K]"},
    /* 151 */ {"Pseudo T", "Pseudo-Temperatur [K]"},
    /* 152 */ {"Pseudo Pot", "Pseudopotentielle Temperatur [K]"},
    /* 153 */ {"Aequi T", "Aequivalent-Temperatur [K]"},
    /* 154 */ {"Aequi Pot", "Aequivalentpotentielle Temperatur [K]"},
    /* 155 */ {"var155", "undefined"},
    /* 156 */ {"var156", "undefined"},
    /* 157 */ {"var157", "undefined"},
    /* 158 */ {"var158", "undefined"},
    /* 159 */ {"var159", "undefined"},
    /* 160 */ {"Bas St Wol", "Untergrenze strat. Bew|lkung [hft]"},
    /* 161 */ {"Bas St Wol", "Untergrenze strat. Bew|lkung [hPa]"},
    /* 162 */ {"Bas Cu Wol", "Untergrenze cumul. Bew|lkung [hft]"},
    /* 163 */ {"Bas Cu Wol", "Untergrenze cumul. Bew|lkung [hPa]"},
    /* 164 */ {"Top St Wol", "Obergrenze strat. Bew|lkung [hft]"},
    /* 165 */ {"Top St Wol", "Obergrenze strat. Bew|lkung [hPa]"},
    /* 166 */ {"Top Cu Wol", "Obergrenze cumul. Bew|lkung [hft]"},
    /* 167 */ {"Top Cu Wol", "Obergrenze cumul. Bew|lkung [hPa]"},
    /* 168 */ {"var168", "undefined"},
    /* 169 */ {"var169", "undefined"},
    /* 170 */ {"Bas Tur Wo", "Untergrenze Wolkenturbulenz [hft]"},
    /* 171 */ {"Bas Tur Wo", "Untergrenze Wolkenturbulenz [hPa]"},
    /* 172 */ {"Top Tur Wo", "Obergrenze Wolkenturbulenz [hft]"},
    /* 173 */ {"Top Tur Wo", "Obergrenze Wolkenturbulenz [hPa]"},
    /* 174 */ {"Bas Eis Wo", "Untergrenze Vereisung in Wolken [hft]"},
    /* 175 */ {"Bas Eis Wo", "Untergrenze Vereisung in Wolken [hPa]"},
    /* 176 */ {"Top Eis Wo", "Obergrenze Vereisung in Wolken [hft]"},
    /* 177 */ {"Top Eis Wo", "Obergrenze Vereisung in Wolken [hPa]"},
    /* 178 */ {"Int Tur Wo", "Intensitaet der Turbulenz in Wolken  (0..4) [1]"},
    /* 179 */ {"Int Eis Wo", "Intensitaet der Vereisung  (0..4) [1]"},
    /* 180 */ {"var180", "undefined"},
    /* 181 */ {"var181", "undefined"},
    /* 182 */ {"var182", "undefined"},
    /* 183 */ {"var183", "undefined"},
    /* 184 */ {"var184", "undefined"},
    /* 185 */ {"var185", "undefined"},
    /* 186 */ {"var186", "undefined"},
    /* 187 */ {"var187", "undefined"},
    /* 188 */ {"var188", "undefined"},
    /* 189 */ {"var189", "undefined"},
    /* 190 */ {"Sichtweite", "Sichtweite [m]"},
    /* 191 */ {"PIP_degree", "Prognostic Icing"},
    /* 192 */ {"PIP_scenar", "Prog Icing"},
    /* 193 */ {"DIP_degree", "Diagnostic Icing"},
    /* 194 */ {"DIP_scenar", "Diag Icing"},
    /* 195 */ {"IcingGuess", "Icing Regime 1.Guess(1=gen,2=conv,3=strat,4=freez) [1]"},
    /* 196 */ {"IcingGrade", "Icing Grade (1=LGT,2=MOD,3=SEV) [1]"},
    /* 197 */ {"IcingRegim", "Icing Regime(1=general,2=convect,3=strat,4=freez) [1]"},
    /* 198 */ {"var198", "undefined"},
    /* 199 */ {"var199", "undefined"},
    /* 200 */ {"Gru Wetter", "Wetter - Grundzustand   (ww"},
    /* 201 */ {"Lok Wetter", "Wetter - 1. lokale Abweichung  (ww"},
    /* 202 */ {"Lok Wetter", "Wetter - 2. lokale Abweichung  (ww"},
    /* 203 */ {"CLDEPTH", "cloud depth (grey scale"},
    /* 204 */ {"CLCT_MOD", "modified total cloud cover  (0..1) [1]"},
    /* 205 */ {"curr weath", "current weather (symbol number"},
    /* 206 */ {"var206", "undefined"},
    /* 207 */ {"var207", "undefined"},
    /* 208 */ {"var208", "undefined"},
    /* 209 */ {"var209", "undefined"},
    /* 210 */ {"var210", "undefined"},
    /* 211 */ {"Cu", "Cumulus  (0..1) [1]"},
    /* 212 */ {"Cb", "Cumulimbus  (0..1) [1]"},
    /* 213 */ {"Sc", "Stratocumulus  (0..1) [1]"},
    /* 214 */ {"Ac", "Altocumulus  (0..1) [1]"},
    /* 215 */ {"Ci", "Cirrus  (0..1) [1]"},
    /* 216 */ {"St", "Stratus  (0..1) [1]"},
    /* 217 */ {"As", "Altostratus  (0..1) [1]"},
    /* 218 */ {"var218", "undefined"},
    /* 219 */ {"var219", "undefined"},
    /* 220 */ {"var220", "undefined"},
    /* 221 */ {"Bedeckung", "Bedeckung in Stufen [1]"},
    /* 222 */ {"Konvektion", "Konvektion  ja/nein [1]"},
    /* 223 */ {"MN >90%", "Gesamtbedeckung > 90%  ja/nein [1]"},
    /* 224 */ {"RF700 >89%", "relative Feuchte 700 hPa >= 90%  ja/nein [1]"},
    /* 225 */ {"RR12 zentr", "Niederschlag 12 std. zentriert [mm]"},
    /* 226 */ {"RR12 <=0.5", "Niederschlag 12 std. zentriert, Werte <= 0.5mm [mm]"},
    /* 227 */ {"RR12 SA>60", "RR12 zentriert, Schneeanteil > 60%  ja/nein [1]"},
    /* 228 */ {"RR12 Kv>60", "RR12 zentriert, konvektiver Anteil > 60%  ja/nein [1]"},
    /* 229 */ {"SRR12ff", "Starkniederschlag in Stufen (12 std. Folgezeitr) [1]"},
    /* 230 */ {"RRMAX/STD", "Maximaler Starkniederschlag / std [mm/h]"},
    /* 231 */ {"RRMAX/MIN", "Maximaler Starkniederschlag / min [mm/min]"},
    /* 232 */ {"SN12ff >15", "Schneefall (12std. Folgezeitraum) > 15 mm  ja/nein [1]"},
    /* 233 */ {"RRgefr12ff", "gefrierender Regen (12std. Folgezeitraum)  ja/nein [1]"},
    /* 234 */ {"FFboe", "Boeenstaerke in Stufen [1]"},
    /* 235 */ {"Gewitter", "Gewitter in Stufen [1]"},
    /* 236 */ {"Tx2m12h ze", "2m Maximumtemperatur 12h zentriert [Grad Celsius]"},
    /* 237 */ {"Tn2m12h ze", "2m Minimumtemperatur 12h zentriert [Grad Celsius]"},
    /* 238 */ {"var238", "undefined"},
    /* 239 */ {"var239", "undefined"},
    /* 240 */ {"var240", "undefined"},
    /* 241 */ {"var241", "undefined"},
    /* 242 */ {"var242", "undefined"},
    /* 243 */ {"var243", "undefined"},
    /* 244 */ {"var244", "undefined"},
    /* 245 */ {"var245", "undefined"},
    /* 246 */ {"var246", "undefined"},
    /* 247 */ {"var247", "undefined"},
    /* 248 */ {"var248", "undefined"},
    /* 249 */ {"var249", "undefined"},
    /* 250 */ {"var250", "undefined"},
    /* 251 */ {"SCHWUELIND", "Schwuele-Index [1]"},
    /* 252 */ {"SMOGSTUFEN", "Smog-Intensitaetsstufen [1]"},
    /* 253 */ {"var253", "undefined"},
    /* 254 */ {"SMOGHOEHE", "Obergrenze Smog  ( Inversionshoehe ) [m]"},
    /* 255 */ {"var255", "undefined"},
};

/*
 * GRIB table 204 at DWD
 *     Helmut P. Frank, 27.10.2004
 */

const struct ParmTable parm_table_dwd_204[256] = {
    /* 0 */ {"var0", "undefined"},
    /* 1 */ {"p RMS fg-a", "pressure RMS-error first guess - analysis [Pa]"},
    /* 2 */ {"p RMS ia-a", "pressure RMS-error initialised analysis - analysis [Pa]"},
    /* 3 */ {"u RMS fg-a", "u RMS-error first guess - analysis [m/s]"},
    /* 4 */ {"u RMS ia-a", "u RMS-error initialised analysis - analysis [m/s]"},
    /* 5 */ {"v RMS fg-a", "v RMS-error first guess - analysis [m/s]"},
    /* 6 */ {"v RMS ia-a", "v RMS-error initialised analysis - analysis [m/s]"},
    /* 7 */ {"fi E fg-a", "geopotential RMS-error first guess - analysis [(m**2)/(s**2)]"},
    /* 8 */ {"fi E ia-a", "geopotential RMS-error init. analysis - analysis [(m**2)/(s**2)]"},
    /* 9 */ {"rh E fg-a", "relative humidity RMS-error first guess - analysis [1]"},
    /* 10 */ {"rh E ia-a", "rel. hum. RMS-error init. analysis - analysis [1]"},
    /* 11 */ {"t RMS fg-a", "temperature RMS-error first guess - analysis [K]"},
    /* 12 */ {"t RMS ia-a", "temperature RMS-error init. analysis - analysis [K]"},
    /* 13 */ {"om E fg-a", "omega RMS-error first guess - analysis [m/s]"},
    /* 14 */ {"om E ia-a", "omega RMS-error initialised analysis - analysis [m/s]"},
    /* 15 */ {"E fg-a KE", "kinetic energy RMS-error first guess - analysis [(m**2)/(s**2)]"},
    /* 16 */ {"E ig-a KE", "kinetic energy RMS-error init. analysis [(m**2)/(s**2)]"},
    /* 17 */ {"var17", "undefined"},
    /* 18 */ {"var18", "undefined"},
    /* 19 */ {"var19", "undefined"},
    /* 20 */ {"var20", "undefined"},
    /* 21 */ {"var21", "undefined"},
    /* 22 */ {"var22", "undefined"},
    /* 23 */ {"var23", "undefined"},
    /* 24 */ {"var24", "undefined"},
    /* 25 */ {"var25", "undefined"},
    /* 26 */ {"var26", "undefined"},
    /* 27 */ {"var27", "undefined"},
    /* 28 */ {"var28", "undefined"},
    /* 29 */ {"var29", "undefined"},
    /* 30 */ {"var30", "undefined"},
    /* 31 */ {"var31", "undefined"},
    /* 32 */ {"var32", "undefined"},
    /* 33 */ {"var33", "undefined"},
    /* 34 */ {"var34", "undefined"},
    /* 35 */ {"var35", "undefined"},
    /* 36 */ {"var36", "undefined"},
    /* 37 */ {"var37", "undefined"},
    /* 38 */ {"var38", "undefined"},
    /* 39 */ {"var39", "undefined"},
    /* 40 */ {"var40", "undefined"},
    /* 41 */ {"var41", "undefined"},
    /* 42 */ {"var42", "undefined"},
    /* 43 */ {"var43", "undefined"},
    /* 44 */ {"var44", "undefined"},
    /* 45 */ {"var45", "undefined"},
    /* 46 */ {"var46", "undefined"},
    /* 47 */ {"var47", "undefined"},
    /* 48 */ {"var48", "undefined"},
    /* 49 */ {"var49", "undefined"},
    /* 50 */ {"var50", "undefined"},
    /* 51 */ {"var51", "undefined"},
    /* 52 */ {"var52", "undefined"},
    /* 53 */ {"var53", "undefined"},
    /* 54 */ {"var54", "undefined"},
    /* 55 */ {"var55", "undefined"},
    /* 56 */ {"var56", "undefined"},
    /* 57 */ {"var57", "undefined"},
    /* 58 */ {"var58", "undefined"},
    /* 59 */ {"var59", "undefined"},
    /* 60 */ {"var60", "undefined"},
    /* 61 */ {"var61", "undefined"},
    /* 62 */ {"var62", "undefined"},
    /* 63 */ {"var63", "undefined"},
    /* 64 */ {"var64", "undefined"},
    /* 65 */ {"var65", "undefined"},
    /* 66 */ {"var66", "undefined"},
    /* 67 */ {"var67", "undefined"},
    /* 68 */ {"var68", "undefined"},
    /* 69 */ {"var69", "undefined"},
    /* 70 */ {"var70", "undefined"},
    /* 71 */ {"var71", "undefined"},
    /* 72 */ {"var72", "undefined"},
    /* 73 */ {"var73", "undefined"},
    /* 74 */ {"var74", "undefined"},
    /* 75 */ {"var75", "undefined"},
    /* 76 */ {"var76", "undefined"},
    /* 77 */ {"var77", "undefined"},
    /* 78 */ {"var78", "undefined"},
    /* 79 */ {"var79", "undefined"},
    /* 80 */ {"var80", "undefined"},
    /* 81 */ {"var81", "undefined"},
    /* 82 */ {"var82", "undefined"},
    /* 83 */ {"var83", "undefined"},
    /* 84 */ {"var84", "undefined"},
    /* 85 */ {"var85", "undefined"},
    /* 86 */ {"var86", "undefined"},
    /* 87 */ {"var87", "undefined"},
    /* 88 */ {"var88", "undefined"},
    /* 89 */ {"var89", "undefined"},
    /* 90 */ {"var90", "undefined"},
    /* 91 */ {"var91", "undefined"},
    /* 92 */ {"var92", "undefined"},
    /* 93 */ {"var93", "undefined"},
    /* 94 */ {"var94", "undefined"},
    /* 95 */ {"var95", "undefined"},
    /* 96 */ {"var96", "undefined"},
    /* 97 */ {"var97", "undefined"},
    /* 98 */ {"var98", "undefined"},
    /* 99 */ {"var99", "undefined"},
    /* 100 */ {"var100", "undefined"},
    /* 101 */ {"var101", "undefined"},
    /* 102 */ {"var102", "undefined"},
    /* 103 */ {"var103", "undefined"},
    /* 104 */ {"var104", "undefined"},
    /* 105 */ {"var105", "undefined"},
    /* 106 */ {"var106", "undefined"},
    /* 107 */ {"var107", "undefined"},
    /* 108 */ {"var108", "undefined"},
    /* 109 */ {"var109", "undefined"},
    /* 110 */ {"var110", "undefined"},
    /* 111 */ {"var111", "undefined"},
    /* 112 */ {"var112", "undefined"},
    /* 113 */ {"var113", "undefined"},
    /* 114 */ {"var114", "undefined"},
    /* 115 */ {"var115", "undefined"},
    /* 116 */ {"var116", "undefined"},
    /* 117 */ {"var117", "undefined"},
    /* 118 */ {"var118", "undefined"},
    /* 119 */ {"var119", "undefined"},
    /* 120 */ {"var120", "undefined"},
    /* 121 */ {"var121", "undefined"},
    /* 122 */ {"var122", "undefined"},
    /* 123 */ {"var123", "undefined"},
    /* 124 */ {"var124", "undefined"},
    /* 125 */ {"var125", "undefined"},
    /* 126 */ {"var126", "undefined"},
    /* 127 */ {"var127", "undefined"},
    /* 128 */ {"var128", "undefined"},
    /* 129 */ {"var129", "undefined"},
    /* 130 */ {"var130", "undefined"},
    /* 131 */ {"RR20", "probability of total precipitation > 20mm [1]"},
    /* 132 */ {"RR50", "probability of total precipitation > 50mm [1]"},
    /* 133 */ {"RR100", "probability of total precipitation > 100mm [1]"},
    /* 134 */ {"var134", "undefined"},
    /* 135 */ {"var135", "undefined"},
    /* 136 */ {"var136", "undefined"},
    /* 137 */ {"var137", "undefined"},
    /* 138 */ {"var138", "undefined"},
    /* 139 */ {"var139", "undefined"},
    /* 140 */ {"var140", "undefined"},
    /* 141 */ {"FF10", "probability of maximum wind speed > 10m/s [1]"},
    /* 142 */ {"FF15", "probability of maximum wind speed > 15m/s [1]"},
    /* 143 */ {"FF20", "probability of maximum wind speed > 20m/s [1]"},
    /* 144 */ {"var144", "undefined"},
    /* 145 */ {"var145", "undefined"},
    /* 146 */ {"var146", "undefined"},
    /* 147 */ {"var147", "undefined"},
    /* 148 */ {"var148", "undefined"},
    /* 149 */ {"var149", "undefined"},
    /* 150 */ {"var150", "undefined"},
    /* 151 */ {"var151", "undefined"},
    /* 152 */ {"var152", "undefined"},
    /* 153 */ {"var153", "undefined"},
    /* 154 */ {"var154", "undefined"},
    /* 155 */ {"var155", "undefined"},
    /* 156 */ {"var156", "undefined"},
    /* 157 */ {"var157", "undefined"},
    /* 158 */ {"var158", "undefined"},
    /* 159 */ {"var159", "undefined"},
    /* 160 */ {"var160", "undefined"},
    /* 161 */ {"var161", "undefined"},
    /* 162 */ {"var162", "undefined"},
    /* 163 */ {"var163", "undefined"},
    /* 164 */ {"var164", "undefined"},
    /* 165 */ {"var165", "undefined"},
    /* 166 */ {"var166", "undefined"},
    /* 167 */ {"var167", "undefined"},
    /* 168 */ {"var168", "undefined"},
    /* 169 */ {"var169", "undefined"},
    /* 170 */ {"var170", "undefined"},
    /* 171 */ {"var171", "undefined"},
    /* 172 */ {"var172", "undefined"},
    /* 173 */ {"var173", "undefined"},
    /* 174 */ {"var174", "undefined"},
    /* 175 */ {"var175", "undefined"},
    /* 176 */ {"var176", "undefined"},
    /* 177 */ {"var177", "undefined"},
    /* 178 */ {"var178", "undefined"},
    /* 179 */ {"var179", "undefined"},
    /* 180 */ {"var180", "undefined"},
    /* 181 */ {"var181", "undefined"},
    /* 182 */ {"var182", "undefined"},
    /* 183 */ {"var183", "undefined"},
    /* 184 */ {"var184", "undefined"},
    /* 185 */ {"var185", "undefined"},
    /* 186 */ {"var186", "undefined"},
    /* 187 */ {"var187", "undefined"},
    /* 188 */ {"var188", "undefined"},
    /* 189 */ {"var189", "undefined"},
    /* 190 */ {"var190", "undefined"},
    /* 191 */ {"var191", "undefined"},
    /* 192 */ {"var192", "undefined"},
    /* 193 */ {"var193", "undefined"},
    /* 194 */ {"var194", "undefined"},
    /* 195 */ {"var195", "undefined"},
    /* 196 */ {"var196", "undefined"},
    /* 197 */ {"var197", "undefined"},
    /* 198 */ {"var198", "undefined"},
    /* 199 */ {"var199", "undefined"},
    /* 200 */ {"var200", "undefined"},
    /* 201 */ {"var201", "undefined"},
    /* 202 */ {"var202", "undefined"},
    /* 203 */ {"var203", "undefined"},
    /* 204 */ {"var204", "undefined"},
    /* 205 */ {"var205", "undefined"},
    /* 206 */ {"var206", "undefined"},
    /* 207 */ {"var207", "undefined"},
    /* 208 */ {"var208", "undefined"},
    /* 209 */ {"var209", "undefined"},
    /* 210 */ {"var210", "undefined"},
    /* 211 */ {"var211", "undefined"},
    /* 212 */ {"var212", "undefined"},
    /* 213 */ {"var213", "undefined"},
    /* 214 */ {"var214", "undefined"},
    /* 215 */ {"var215", "undefined"},
    /* 216 */ {"var216", "undefined"},
    /* 217 */ {"var217", "undefined"},
    /* 218 */ {"var218", "undefined"},
    /* 219 */ {"var219", "undefined"},
    /* 220 */ {"var220", "undefined"},
    /* 221 */ {"var221", "undefined"},
    /* 222 */ {"var222", "undefined"},
    /* 223 */ {"var223", "undefined"},
    /* 224 */ {"var224", "undefined"},
    /* 225 */ {"var225", "undefined"},
    /* 226 */ {"var226", "undefined"},
    /* 227 */ {"var227", "undefined"},
    /* 228 */ {"var228", "undefined"},
    /* 229 */ {"var229", "undefined"},
    /* 230 */ {"var230", "undefined"},
    /* 231 */ {"var231", "undefined"},
    /* 232 */ {"var232", "undefined"},
    /* 233 */ {"var233", "undefined"},
    /* 234 */ {"var234", "undefined"},
    /* 235 */ {"var235", "undefined"},
    /* 236 */ {"var236", "undefined"},
    /* 237 */ {"var237", "undefined"},
    /* 238 */ {"var238", "undefined"},
    /* 239 */ {"var239", "undefined"},
    /* 240 */ {"var240", "undefined"},
    /* 241 */ {"var241", "undefined"},
    /* 242 */ {"var242", "undefined"},
    /* 243 */ {"var243", "undefined"},
    /* 244 */ {"var244", "undefined"},
    /* 245 */ {"var245", "undefined"},
    /* 246 */ {"var246", "undefined"},
    /* 247 */ {"var247", "undefined"},
    /* 248 */ {"var248", "undefined"},
    /* 249 */ {"var249", "undefined"},
    /* 250 */ {"var250", "undefined"},
    /* 251 */ {"var251", "undefined"},
    /* 252 */ {"var252", "undefined"},
    /* 253 */ {"var253", "undefined"},
    /* 254 */ {"var254", "undefined"},
    /* 255 */ {"var255", "undefined"},
};

/*
 * GRIB table 205 at DWD
 *     Helmut P. Frank, 27.10.2004
 * updated 19.10.2005
 */

const struct ParmTable parm_table_dwd_205[256] = {
    /* 0 */ {"var0", "undefined"},
    /* 1 */ {"SYNME5", "METEOSAT-5 mit Instrument MVIRI [1]"},
    /* 2 */ {"SYNME6", "METEOSAT-6 mit Instrument MVIRI [1]"},
    /* 3 */ {"SYNME7", "METEOSAT-7 mit Instrument MVIRI [1]"},
    /* 4 */ {"SYNMSG", "MSG mit Instrument SEVIRI [1]"},
    /* 5 */ {"var5", "undefined"},
    /* 6 */ {"var6", "undefined"},
    /* 7 */ {"var7", "undefined"},
    /* 8 */ {"var8", "undefined"},
    /* 9 */ {"var9", "undefined"},
    /* 10 */ {"var10", "undefined"},
    /* 11 */ {"var11", "undefined"},
    /* 12 */ {"var12", "undefined"},
    /* 13 */ {"var13", "undefined"},
    /* 14 */ {"var14", "undefined"},
    /* 15 */ {"var15", "undefined"},
    /* 16 */ {"var16", "undefined"},
    /* 17 */ {"var17", "undefined"},
    /* 18 */ {"var18", "undefined"},
    /* 19 */ {"var19", "undefined"},
    /* 20 */ {"var20", "undefined"},
    /* 21 */ {"var21", "undefined"},
    /* 22 */ {"var22", "undefined"},
    /* 23 */ {"var23", "undefined"},
    /* 24 */ {"var24", "undefined"},
    /* 25 */ {"var25", "undefined"},
    /* 26 */ {"var26", "undefined"},
    /* 27 */ {"var27", "undefined"},
    /* 28 */ {"var28", "undefined"},
    /* 29 */ {"var29", "undefined"},
    /* 30 */ {"var30", "undefined"},
    /* 31 */ {"var31", "undefined"},
    /* 32 */ {"var32", "undefined"},
    /* 33 */ {"var33", "undefined"},
    /* 34 */ {"var34", "undefined"},
    /* 35 */ {"var35", "undefined"},
    /* 36 */ {"var36", "undefined"},
    /* 37 */ {"var37", "undefined"},
    /* 38 */ {"var38", "undefined"},
    /* 39 */ {"var39", "undefined"},
    /* 40 */ {"var40", "undefined"},
    /* 41 */ {"var41", "undefined"},
    /* 42 */ {"var42", "undefined"},
    /* 43 */ {"var43", "undefined"},
    /* 44 */ {"var44", "undefined"},
    /* 45 */ {"var45", "undefined"},
    /* 46 */ {"var46", "undefined"},
    /* 47 */ {"var47", "undefined"},
    /* 48 */ {"var48", "undefined"},
    /* 49 */ {"var49", "undefined"},
    /* 50 */ {"var50", "undefined"},
    /* 51 */ {"var51", "undefined"},
    /* 52 */ {"var52", "undefined"},
    /* 53 */ {"var53", "undefined"},
    /* 54 */ {"var54", "undefined"},
    /* 55 */ {"var55", "undefined"},
    /* 56 */ {"var56", "undefined"},
    /* 57 */ {"var57", "undefined"},
    /* 58 */ {"var58", "undefined"},
    /* 59 */ {"var59", "undefined"},
    /* 60 */ {"var60", "undefined"},
    /* 61 */ {"var61", "undefined"},
    /* 62 */ {"var62", "undefined"},
    /* 63 */ {"var63", "undefined"},
    /* 64 */ {"var64", "undefined"},
    /* 65 */ {"var65", "undefined"},
    /* 66 */ {"var66", "undefined"},
    /* 67 */ {"var67", "undefined"},
    /* 68 */ {"var68", "undefined"},
    /* 69 */ {"var69", "undefined"},
    /* 70 */ {"var70", "undefined"},
    /* 71 */ {"var71", "undefined"},
    /* 72 */ {"var72", "undefined"},
    /* 73 */ {"var73", "undefined"},
    /* 74 */ {"var74", "undefined"},
    /* 75 */ {"var75", "undefined"},
    /* 76 */ {"var76", "undefined"},
    /* 77 */ {"var77", "undefined"},
    /* 78 */ {"var78", "undefined"},
    /* 79 */ {"var79", "undefined"},
    /* 80 */ {"var80", "undefined"},
    /* 81 */ {"var81", "undefined"},
    /* 82 */ {"var82", "undefined"},
    /* 83 */ {"var83", "undefined"},
    /* 84 */ {"var84", "undefined"},
    /* 85 */ {"var85", "undefined"},
    /* 86 */ {"var86", "undefined"},
    /* 87 */ {"var87", "undefined"},
    /* 88 */ {"var88", "undefined"},
    /* 89 */ {"var89", "undefined"},
    /* 90 */ {"var90", "undefined"},
    /* 91 */ {"var91", "undefined"},
    /* 92 */ {"var92", "undefined"},
    /* 93 */ {"var93", "undefined"},
    /* 94 */ {"var94", "undefined"},
    /* 95 */ {"var95", "undefined"},
    /* 96 */ {"var96", "undefined"},
    /* 97 */ {"var97", "undefined"},
    /* 98 */ {"var98", "undefined"},
    /* 99 */ {"var99", "undefined"},
    /* 100 */ {"var100", "undefined"},
    /* 101 */ {"var101", "undefined"},
    /* 102 */ {"var102", "undefined"},
    /* 103 */ {"var103", "undefined"},
    /* 104 */ {"var104", "undefined"},
    /* 105 */ {"var105", "undefined"},
    /* 106 */ {"var106", "undefined"},
    /* 107 */ {"var107", "undefined"},
    /* 108 */ {"var108", "undefined"},
    /* 109 */ {"var109", "undefined"},
    /* 110 */ {"var110", "undefined"},
    /* 111 */ {"var111", "undefined"},
    /* 112 */ {"var112", "undefined"},
    /* 113 */ {"var113", "undefined"},
    /* 114 */ {"var114", "undefined"},
    /* 115 */ {"var115", "undefined"},
    /* 116 */ {"var116", "undefined"},
    /* 117 */ {"var117", "undefined"},
    /* 118 */ {"var118", "undefined"},
    /* 119 */ {"var119", "undefined"},
    /* 120 */ {"var120", "undefined"},
    /* 121 */ {"var121", "undefined"},
    /* 122 */ {"var122", "undefined"},
    /* 123 */ {"var123", "undefined"},
    /* 124 */ {"var124", "undefined"},
    /* 125 */ {"var125", "undefined"},
    /* 126 */ {"var126", "undefined"},
    /* 127 */ {"var127", "undefined"},
    /* 128 */ {"var128", "undefined"},
    /* 129 */ {"var129", "undefined"},
    /* 130 */ {"var130", "undefined"},
    /* 131 */ {"var131", "undefined"},
    /* 132 */ {"var132", "undefined"},
    /* 133 */ {"var133", "undefined"},
    /* 134 */ {"var134", "undefined"},
    /* 135 */ {"var135", "undefined"},
    /* 136 */ {"var136", "undefined"},
    /* 137 */ {"var137", "undefined"},
    /* 138 */ {"var138", "undefined"},
    /* 139 */ {"var139", "undefined"},
    /* 140 */ {"var140", "undefined"},
    /* 141 */ {"var141", "undefined"},
    /* 142 */ {"var142", "undefined"},
    /* 143 */ {"var143", "undefined"},
    /* 144 */ {"var144", "undefined"},
    /* 145 */ {"var145", "undefined"},
    /* 146 */ {"var146", "undefined"},
    /* 147 */ {"var147", "undefined"},
    /* 148 */ {"var148", "undefined"},
    /* 149 */ {"var149", "undefined"},
    /* 150 */ {"var150", "undefined"},
    /* 151 */ {"var151", "undefined"},
    /* 152 */ {"var152", "undefined"},
    /* 153 */ {"var153", "undefined"},
    /* 154 */ {"var154", "undefined"},
    /* 155 */ {"var155", "undefined"},
    /* 156 */ {"var156", "undefined"},
    /* 157 */ {"var157", "undefined"},
    /* 158 */ {"var158", "undefined"},
    /* 159 */ {"var159", "undefined"},
    /* 160 */ {"var160", "undefined"},
    /* 161 */ {"var161", "undefined"},
    /* 162 */ {"var162", "undefined"},
    /* 163 */ {"var163", "undefined"},
    /* 164 */ {"var164", "undefined"},
    /* 165 */ {"var165", "undefined"},
    /* 166 */ {"var166", "undefined"},
    /* 167 */ {"var167", "undefined"},
    /* 168 */ {"var168", "undefined"},
    /* 169 */ {"var169", "undefined"},
    /* 170 */ {"var170", "undefined"},
    /* 171 */ {"var171", "undefined"},
    /* 172 */ {"var172", "undefined"},
    /* 173 */ {"var173", "undefined"},
    /* 174 */ {"var174", "undefined"},
    /* 175 */ {"var175", "undefined"},
    /* 176 */ {"var176", "undefined"},
    /* 177 */ {"var177", "undefined"},
    /* 178 */ {"var178", "undefined"},
    /* 179 */ {"var179", "undefined"},
    /* 180 */ {"var180", "undefined"},
    /* 181 */ {"var181", "undefined"},
    /* 182 */ {"var182", "undefined"},
    /* 183 */ {"var183", "undefined"},
    /* 184 */ {"var184", "undefined"},
    /* 185 */ {"var185", "undefined"},
    /* 186 */ {"var186", "undefined"},
    /* 187 */ {"var187", "undefined"},
    /* 188 */ {"var188", "undefined"},
    /* 189 */ {"var189", "undefined"},
    /* 190 */ {"var190", "undefined"},
    /* 191 */ {"var191", "undefined"},
    /* 192 */ {"var192", "undefined"},
    /* 193 */ {"var193", "undefined"},
    /* 194 */ {"var194", "undefined"},
    /* 195 */ {"var195", "undefined"},
    /* 196 */ {"var196", "undefined"},
    /* 197 */ {"var197", "undefined"},
    /* 198 */ {"var198", "undefined"},
    /* 199 */ {"var199", "undefined"},
    /* 200 */ {"var200", "undefined"},
    /* 201 */ {"var201", "undefined"},
    /* 202 */ {"var202", "undefined"},
    /* 203 */ {"var203", "undefined"},
    /* 204 */ {"var204", "undefined"},
    /* 205 */ {"var205", "undefined"},
    /* 206 */ {"var206", "undefined"},
    /* 207 */ {"var207", "undefined"},
    /* 208 */ {"var208", "undefined"},
    /* 209 */ {"var209", "undefined"},
    /* 210 */ {"var210", "undefined"},
    /* 211 */ {"var211", "undefined"},
    /* 212 */ {"var212", "undefined"},
    /* 213 */ {"var213", "undefined"},
    /* 214 */ {"var214", "undefined"},
    /* 215 */ {"var215", "undefined"},
    /* 216 */ {"var216", "undefined"},
    /* 217 */ {"var217", "undefined"},
    /* 218 */ {"var218", "undefined"},
    /* 219 */ {"var219", "undefined"},
    /* 220 */ {"var220", "undefined"},
    /* 221 */ {"var221", "undefined"},
    /* 222 */ {"var222", "undefined"},
    /* 223 */ {"var223", "undefined"},
    /* 224 */ {"var224", "undefined"},
    /* 225 */ {"var225", "undefined"},
    /* 226 */ {"var226", "undefined"},
    /* 227 */ {"var227", "undefined"},
    /* 228 */ {"var228", "undefined"},
    /* 229 */ {"var229", "undefined"},
    /* 230 */ {"var230", "undefined"},
    /* 231 */ {"var231", "undefined"},
    /* 232 */ {"var232", "undefined"},
    /* 233 */ {"var233", "undefined"},
    /* 234 */ {"var234", "undefined"},
    /* 235 */ {"var235", "undefined"},
    /* 236 */ {"var236", "undefined"},
    /* 237 */ {"var237", "undefined"},
    /* 238 */ {"var238", "undefined"},
    /* 239 */ {"var239", "undefined"},
    /* 240 */ {"var240", "undefined"},
    /* 241 */ {"var241", "undefined"},
    /* 242 */ {"var242", "undefined"},
    /* 243 */ {"var243", "undefined"},
    /* 244 */ {"var244", "undefined"},
    /* 245 */ {"var245", "undefined"},
    /* 246 */ {"var246", "undefined"},
    /* 247 */ {"var247", "undefined"},
    /* 248 */ {"var248", "undefined"},
    /* 249 */ {"var249", "undefined"},
    /* 250 */ {"var250", "undefined"},
    /* 251 */ {"var251", "undefined"},
    /* 252 */ {"var252", "undefined"},
    /* 253 */ {"var253", "undefined"},
    /* 254 */ {"var254", "undefined"},
    /* 255 */ {"var255", "undefined"},
};

const struct ParmTable parm_table_cptec_254[256] = {
   /* 0 */ {"var0", "undefined"},
   /* 1 */ {"PRES", "Pressure [hPa]"},
   /* 2 */ {"psnm", "Pressure reduced to MSL [hPa]"},
   /* 3 */ {"tsps", "Pressure tendency [Pa/s]"},
   /* 4 */ {"var4", "undefined"},
   /* 5 */ {"var5", "undefined"},
   /* 6 */ {"geop", "Geopotential [dam]"},
   /* 7 */ {"zgeo", "Geopotential height [gpm]"},
   /* 8 */ {"gzge", "Geometric height [m]"},
   /* 9 */ {"var9", "undefined"},
   /* 10 */ {"var10", "undefined"},
   /* 11 */ {"temp", "ABSOLUTE TEMPERATURE [K]"},
   /* 12 */ {"vtmp", "VIRTUAL TEMPERATURE [K]"},
   /* 13 */ {"ptmp", "POTENTIAL TEMPERATURE [K]"},
   /* 14 */ {"psat", "PSEUDO-ADIABATIC POTENTIAL TEMPERATURE [K]"},
   /* 15 */ {"mxtp", "MAXIMUM TEMPERATURE [K]"},
   /* 16 */ {"mntp", "MINIMUM TEMPERATURE [K]"},
   /* 17 */ {"tpor", "DEW POINT TEMPERATURE [K]"},
   /* 18 */ {"dptd", "DEW POINT DEPRESSION [K]"},
   /* 19 */ {"lpsr", "LAPSE RATE [K/m]"},
   /* 20 */ {"var20", "undefined"},
   /* 21 */ {"rds1", "RADAR SPECTRA(1) [non-dim]"},
   /* 22 */ {"rds2", "RADAR SPECTRA(2) [non-dim]"},
   /* 23 */ {"rds3", "RADAR SPECTRA(3) [non-dim]"},
   /* 24 */ {"var24", "undefined"},
   /* 25 */ {"tpan", "TEMPERATURE ANOMALY [K]"},
   /* 26 */ {"psan", "PRESSURE ANOMALY [Pa hPa]"},
   /* 27 */ {"zgan", "GEOPOT HEIGHT ANOMALY [m]"},
   /* 28 */ {"wvs1", "WAVE SPECTRA(1) [non-dim]"},
   /* 29 */ {"wvs2", "WAVE SPECTRA(2) [non-dim]"},
   /* 30 */ {"wvs3", "WAVE SPECTRA(3) [non-dim]"},
   /* 31 */ {"wind", "WIND DIRECTION  [deg]"},
   /* 32 */ {"wins", "WIND SPEED [m/s]"},
   /* 33 */ {"uvel", "ZONAL WIND (U) [m/s]"},
   /* 34 */ {"vvel", "MERIDIONAL WIND (V) [m/s]"},
   /* 35 */ {"fcor", "STREAM FUNCTION [m2/s]"},
   /* 36 */ {"potv", "VELOCITY POTENTIAL [m2/s]"},
   /* 37 */ {"var37", "undefined"},
   /* 38 */ {"sgvv", "SIGMA COORD VERT VEL [sec/sec]"},
   /* 39 */ {"omeg", "OMEGA [Pa/s]"},
   /* 40 */ {"omg2", "VERTICAL VELOCITY [m/s]"},
   /* 41 */ {"abvo", "ABSOLUTE VORTICITY        [10**5/sec]"},
   /* 42 */ {"abdv", "ABSOLUTE DIVERGENCE [10**5/sec]"},
   /* 43 */ {"vort", "VORTICITY  [1/s]"},
   /* 44 */ {"divg", "DIVERGENCE [1/s]"},
   /* 45 */ {"vucs", "VERTICAL U-COMP SHEAR [1/sec]"},
   /* 46 */ {"vvcs", "VERT V-COMP SHEAR [1/sec]"},
   /* 47 */ {"dirc", "DIRECTION OF CURRENT [deg]"},
   /* 48 */ {"spdc", "SPEED OF CURRENT [m/s]"},
   /* 49 */ {"ucpc", "U-COMPONENT OF CURRENT [m/s]"},
   /* 50 */ {"vcpc", "V-COMPONENT OF CURRENT [m/s]"},
   /* 51 */ {"umes", "SPECIFIC HUMIDITY [kg/kg]"},
   /* 52 */ {"umrl", "RELATIVE HUMIDITY [no Dim]"},
   /* 53 */ {"hmxr", "HUMIDITY MIXING RATIO [kg/kg]"},
   /* 54 */ {"agpl", "INST. PRECIPITABLE WATER [Kg/m2]"},
   /* 55 */ {"vapp", "VAPOUR PRESSURE [Pa hpa]"},
   /* 56 */ {"sadf", "SATURATION DEFICIT        [Pa hPa]"},
   /* 57 */ {"evap", "EVAPORATION [Kg/m2/day]"},
   /* 58 */ {"var58", "undefined"},
   /* 59 */ {"prcr", "PRECIPITATION RATE        [kg/m2/day]"},
   /* 60 */ {"thpb", "THUNDER PROBABILITY [%]"},
   /* 61 */ {"prec", "TOTAL PRECIPITATION [Kg/m2/day]"},
   /* 62 */ {"prge", "LARGE SCALE PRECIPITATION [Kg/m2/day]"},
   /* 63 */ {"prcv", "CONVECTIVE PRECIPITATION [Kg/m2/day]"},
   /* 64 */ {"neve", "SNOWFALL [Kg/m2/day]"},
   /* 65 */ {"wenv", "WAT EQUIV ACC SNOW DEPTH [kg/m2]"},
   /* 66 */ {"nvde", "SNOW DEPTH        [cm]"},
   /* 67 */ {"mxld", "MIXED LAYER DEPTH [m cm]"},
   /* 68 */ {"tthd", "TRANS THERMOCLINE DEPTH [m cm]"},
   /* 69 */ {"mthd", "MAIN THERMOCLINE DEPTH [m cm]"},
   /* 70 */ {"mtha", "MAIN THERMOCLINE ANOM [m cm]"},
   /* 71 */ {"cbnv", "CLOUD COVER [0-1]"},
   /* 72 */ {"cvnv", "CONVECTIVE CLOUD COVER [0-1]"},
   /* 73 */ {"lwnv", "LOW CLOUD COVER [0-1]"},
   /* 74 */ {"mdnv", "MEDIUM CLOUD COVER        [0-1]"},
   /* 75 */ {"hinv", "HIGH CLOUD COVER [0-1]"},
   /* 76 */ {"wtnv", "CLOUD WATER [kg/m2]"},
   /* 77 */ {"bli", "BEST LIFTED INDEX (TO 500 HPA) [K]"},
   /* 78 */ {"var78", "undefined"},
   /* 79 */ {"var79", "undefined"},
   /* 80 */ {"var80", "undefined"},
   /* 81 */ {"lsmk", "LAND SEA MASK [0,1]"},
   /* 82 */ {"dslm", "DEV SEA_LEV FROM MEAN [m]"},
   /* 83 */ {"zorl", "ROUGHNESS LENGTH [m]"},
   /* 84 */ {"albe", "ALBEDO [%]"},
   /* 85 */ {"dstp", "DEEP SOIL TEMPERATURE [K]"},
   /* 86 */ {"soic", "SOIL MOISTURE CONTENT [Kg/m2]"},
   /* 87 */ {"vege", "VEGETATION        [%]"},
   /* 88 */ {"var88", "undefined"},
   /* 89 */ {"dens", "DENSITY [kg/m3]"},
   /* 90 */ {"var90", "Undefined"},
   /* 91 */ {"icec", "ICE CONCENTRATION [fraction]"},
   /* 92 */ {"icet", "ICE THICKNESS [m]"},
   /* 93 */ {"iced", "DIRECTION OF ICE DRIFT [deg]"},
   /* 94 */ {"ices", "SPEED OF ICE DRIFT        [m/s]"},
   /* 95 */ {"iceu", "U-COMP OF ICE DRIFT [m/s]"},
   /* 96 */ {"icev", "V-COMP OF ICE DRIFT [m/s]"},
   /* 97 */ {"iceg", "ICE GROWTH        [m]"},
   /* 98 */ {"icdv", "ICE DIVERGENCE [sec/sec]"},
   /* 99 */ {"var99", "undefined"},
   /* 100 */ {"shcw", "SIG HGT COM WAVE/SWELL [m]"},
   /* 101 */ {"wwdi", "DIRECTION OF WIND WAVE [deg]"},
   /* 102 */ {"wwsh", "SIG HGHT OF WIND WAVES [m]"},
   /* 103 */ {"wwmp", "MEAN PERIOD WIND WAVES [sec]"},
   /* 104 */ {"swdi", "DIRECTION OF SWELL WAVE [deg]"},
   /* 105 */ {"swsh", "SIG HEIGHT SWELL WAVES [m]"},
   /* 106 */ {"swmp", "MEAN PERIOD SWELL WAVES [sec]"},
   /* 107 */ {"prwd", "PRIMARY WAVE DIRECTION [deg]"},
   /* 108 */ {"prmp", "PRIM WAVE MEAN PERIOD [s]"},
   /* 109 */ {"swdi", "SECOND WAVE DIRECTION [deg]"},
   /* 110 */ {"swmp", "SECOND WAVE MEAN PERIOD [s]"},
   /* 111 */ {"ocas", "SHORT WAVE ABSORBED AT GROUND [W/m2]"},
   /* 112 */ {"slds", "NET LONG WAVE AT BOTTOM [W/m2]"},
   /* 113 */ {"nswr", "NET SHORT-WAV RAD(TOP) [W/m2]"},
   /* 114 */ {"role", "OUTGOING LONG WAVE AT TOP [W/m2]"},
   /* 115 */ {"lwrd", "LONG-WAV RAD [W/m2]"},
   /* 116 */ {"swea", "SHORT WAVE ABSORBED BY EARTH/ATMOSPHERE  [W/m2]"},
   /* 117 */ {"glbr", "GLOBAL RADIATION [W/m2 ]"},
   /* 118 */ {"var118", "undefined"},
   /* 119 */ {"var119", "undefined"},
   /* 120 */ {"var120", "undefined"},
   /* 121 */ {"clsf", "LATENT HEAT FLUX FROM SURFACE [W/m2]"},
   /* 122 */ {"cssf", "SENSIBLE HEAT FLUX FROM SURFACE [W/m2]"},
   /* 123 */ {"blds", "BOUND LAYER DISSIPATION [W/m2]"},
   /* 124 */ {"var124", "undefined"},
   /* 125 */ {"var125", "undefined"},
   /* 126 */ {"var126", "undefined"},
   /* 127 */ {"imag", "IMAGE [image^data]"},
   /* 128 */ {"tp2m", "2 METRE TEMPERATURE [K]"},
   /* 129 */ {"dp2m", "2 METRE DEWPOINT TEMPERATURE [K]"},
   /* 130 */ {"u10m", "10 METRE U-WIND COMPONENT [m/s]"},
   /* 131 */ {"v10m", "10 METRE V-WIND COMPONENT [m/s]"},
   /* 132 */ {"topo", "TOPOGRAPHY [m]"},
   /* 133 */ {"gsfp", "GEOMETRIC MEAN SURFACE PRESSURE [hPa]"},
   /* 134 */ {"lnsp", "LN SURFACE PRESSURE [hPa]"},
   /* 135 */ {"pslc", "SURFACE PRESSURE [hPa]"},
   /* 136 */ {"pslm", "M S L PRESSURE (MESINGER METHOD) [hPa]"},
   /* 137 */ {"mask", "MASK  [-/+]"},
   /* 138 */ {"mxwu", "MAXIMUM U-WIND [m/s]"},
   /* 139 */ {"mxwv", "MAXIMUM V-WIND [m/s]"},
   /* 140 */ {"cape", "CONVECTIVE AVAIL. POT.ENERGY [m2/s2]"},
   /* 141 */ {"cine", "CONVECTIVE INHIB. ENERGY [m2/s2]"},
   /* 142 */ {"lhcv", "CONVECTIVE LATENT HEATING [K/s]"},
   /* 143 */ {"mscv", "CONVECTIVE MOISTURE SOURCE [1/s]"},
   /* 144 */ {"scvm", "SHALLOW CONV. MOISTURE SOURCE [1/s]"},
   /* 145 */ {"scvh", "SHALLOW CONVECTIVE HEATING [K/s]"},
   /* 146 */ {"mxwp", "MAXIMUM WIND PRESS. LVL  [hPa]"},
   /* 147 */ {"ustr", "STORM MOTION U-COMPONENT [m/s]"},
   /* 148 */ {"vstr", "STORM MOTION V-COMPONENT [m/s]"},
   /* 149 */ {"cbnt", "MEAN CLOUD COVER [0-1]"},
   /* 150 */ {"pcbs", "PRESSURE AT CLOUD BASE [hPa]"},
   /* 151 */ {"pctp", "PRESSURE AT CLOUD TOP [hPa]"},
   /* 152 */ {"fzht", "FREEZING LEVEL HEIGHT [m]"},
   /* 153 */ {"fzrh", "FREEZING LEVEL RELATIVE HUMIDITY [%]"},
   /* 154 */ {"fdlt", "FLIGHT LEVELS TEMPERATURE [K]"},
   /* 155 */ {"fdlu", "FLIGHT LEVELS U-WIND [m/s]"},
   /* 156 */ {"fdlv", "FLIGHT LEVELS V-WIND [m/s]"},
   /* 157 */ {"tppp", "TROPOPAUSE PRESSURE   [hPa]"},
   /* 158 */ {"tppt", "TROPOPAUSE TEMPERATURE [K]"},
   /* 159 */ {"tppu", "TROPOPAUSE U-WIND COMPONENT [m/s]"},
   /* 160 */ {"tppv", "TROPOPAUSE v-WIND COMPONENT [m/s]"},
   /* 161 */ {"var161", "undefined"},
   /* 162 */ {"gvdu", "GRAVITY WAVE DRAG DU/DT [m/s2]"},
   /* 163 */ {"gvdv", "GRAVITY WAVE DRAG DV/DT [m/s2]"},
   /* 164 */ {"gvus", "GRAVITY WAVE DRAG SFC ZONAL STRESS  [Pa]"},
   /* 165 */ {"gvvs", "GRAVITY WAVE DRAG SFC MERIDIONAL STRESS [Pa]"},
   /* 166 */ {"var166", "undefined"},
   /* 167 */ {"dvsh", "DIVERGENCE OF SPECIFIC HUMIDITY [1/s]"},
   /* 168 */ {"hmfc", "HORIZ. MOISTURE FLUX CONV.  [1/s]"},
   /* 169 */ {"vmfl", "VERT. INTEGRATED MOISTURE FLUX CONV. [kg/(m2*s)]"},
   /* 170 */ {"vadv", "VERTICAL MOISTURE ADVECTION  [kg/(kg*s)]"},
   /* 171 */ {"nhcm", "NEG. HUM. CORR. MOISTURE SOURCE [kg/(kg*s)]"},
   /* 172 */ {"lglh", "LARGE SCALE LATENT HEATING   [K/s]"},
   /* 173 */ {"lgms", "LARGE SCALE MOISTURE SOURCE  [1/s]"},
   /* 174 */ {"smav", "SOIL MOISTURE AVAILABILITY  [0-1]"},
   /* 175 */ {"tgrz", "SOIL TEMPERATURE OF ROOT ZONE [K]"},
   /* 176 */ {"bslh", "BARE SOIL LATENT HEAT [Ws/m2]"},
   /* 177 */ {"evpp", "POTENTIAL SFC EVAPORATION [m]"},
   /* 178 */ {"rnof", "RUNOFF [kg/m2/s)]"},
   /* 179 */ {"pitp", "INTERCEPTION LOSS [W/m2]"},
   /* 180 */ {"vpca", "VAPOR PRESSURE OF CANOPY AIR SPACE [mb]"},
   /* 181 */ {"qsfc", "SURFACE SPEC HUMIDITY   [kg/kg]"},
   /* 182 */ {"ussl", "SOIL WETNESS OF SURFACE [0-1]"},
   /* 183 */ {"uzrs", "SOIL WETNESS OF ROOT ZONE [0-1]"},
   /* 184 */ {"uzds", "SOIL WETNESS OF DRAINAGE ZONE [0-1]"},
   /* 185 */ {"amdl", "STORAGE ON CANOPY [m]"},
   /* 186 */ {"amsl", "STORAGE ON GROUND [m]"},
   /* 187 */ {"tsfc", "SURFACE TEMPERATURE [K]"},
   /* 188 */ {"tems", "SURFACE ABSOLUTE TEMPERATURE [K]"},
   /* 189 */ {"tcas", "TEMPERATURE OF CANOPY AIR SPACE [K]"},
   /* 190 */ {"ctmp", "TEMPERATURE AT CANOPY [K]"},
   /* 191 */ {"tgsc", "GROUND/SURFACE COVER TEMPERATURE [K]"},
   /* 192 */ {"uves", "SURFACE ZONAL WIND (U) [m/s]"},
   /* 193 */ {"usst", "SURFACE ZONAL WIND STRESS [Pa]"},
   /* 194 */ {"vves", "SURFACE MERIDIONAL WIND (V) [m/s]"},
   /* 195 */ {"vsst", "SURFACE MERIDIONAL WIND STRESS [Pa]"},
   /* 196 */ {"suvf", "SURFACE MOMENTUM FLUX [W/m2]"},
   /* 197 */ {"iswf", "INCIDENT SHORT WAVE FLUX [W/m2]"},
   /* 198 */ {"ghfl", "TIME AVE GROUND HT FLX   [W/m2]"},
   /* 199 */ {"var199", "undefined"},
   /* 200 */ {"lwbc", "NET LONG WAVE AT BOTTOM (CLEAR) [W/m2]"},
   /* 201 */ {"lwtc", "OUTGOING LONG WAVE AT TOP (CLEAR) [W/m2]"},
   /* 202 */ {"swec", "SHORT WV ABSRBD BY EARTH/ATMOS (CLEAR) [W/m2]"},
   /* 203 */ {"ocac", "SHORT WAVE ABSORBED AT GROUND (CLEAR) [W/m2]"},
   /* 204 */ {"var204", "undefined"},
   /* 205 */ {"lwrh", "LONG WAVE RADIATIVE HEATING  [K/s]"},
   /* 206 */ {"swrh", "SHORT WAVE RADIATIVE HEATING [K/s]"},
   /* 207 */ {"olis", "DOWNWARD LONG WAVE AT BOTTOM [W/m2]"},
   /* 208 */ {"olic", "DOWNWARD LONG WAVE AT BOTTOM (CLEAR) [W/m2]"},
   /* 209 */ {"ocis", "DOWNWARD SHORT WAVE AT GROUND [W/m2]"},
   /* 210 */ {"ocic", "DOWNWARD SHORT WAVE AT GROUND (CLEAR) [W/m2]"},
   /* 211 */ {"oles", "UPWARD LONG WAVE AT BOTTOM [W/m2]"},
   /* 212 */ {"oces", "UPWARD SHORT WAVE AT GROUND [W/m2]"},
   /* 213 */ {"swgc", "UPWARD SHORT WAVE AT GROUND (CLEAR) [W/m2]"},
   /* 214 */ {"roce", "UPWARD SHORT WAVE AT TOP [W/m2]"},
   /* 215 */ {"swtc", "UPWARD SHORT WAVE AT TOP (CLEAR) [W/m2]"},
   /* 216 */ {"var216", "undefined"},
   /* 217 */ {"var217", "undefined"},
   /* 218 */ {"hhdf", "HORIZONTAL HEATING DIFFUSION [K/s]"},
   /* 219 */ {"hmdf", "HORIZONTAL MOISTURE DIFFUSION [1/s]"},
   /* 220 */ {"hddf", "HORIZONTAL DIVERGENCE DIFFUSION [1/s2]"},
   /* 221 */ {"hvdf", "HORIZONTAL VORTICITY DIFFUSION [1/s2]"},
   /* 222 */ {"vdms", "VERTICAL DIFF. MOISTURE SOURCE [1/s]"},
   /* 223 */ {"vdfu", "VERTICAL DIFFUSION DU/DT [m/s2]"},
   /* 224 */ {"vdfv", "VERTICAL DIFFUSION DV/DT [m/s2]"},
   /* 225 */ {"vdfh", "VERTICAL DIFFUSION HEATING [K/s]"},
   /* 226 */ {"umrs", "SURFACE RELATIVE HUMIDITY [no Dim]"},
   /* 227 */ {"vdcc", "VERTICAL DIST TOTAL CLOUD COVER [no Dim]"},
   /* 228 */ {"var228", "undefined"},
   /* 229 */ {"var229", "undefined"},
   /* 230 */ {"usmt", "TIME MEAN SURFACE ZONAL WIND (U) [m/s]"},
   /* 231 */ {"vsmt", "TIME MEAN SURFACE MERIDIONAL WIND (V) [m/s]"},
   /* 232 */ {"tsmt", "TIME MEAN SURFACE ABSOLUTE TEMPERATURE [K]"},
   /* 233 */ {"rsmt", "TIME MEAN SURFACE RELATIVE HUMIDITY [no Dim]"},
   /* 234 */ {"atmt", "TIME MEAN ABSOLUTE TEMPERATURE [K]"},
   /* 235 */ {"stmt", "TIME MEAN DEEP SOIL TEMPERATURE [K]"},
   /* 236 */ {"ommt", "TIME MEAN DERIVED OMEGA [Pa/s]"},
   /* 237 */ {"dvmt", "TIME MEAN DIVERGENCE [1/s]"},
   /* 238 */ {"zhmt", "TIME MEAN GEOPOTENTIAL HEIGHT [m]"},
   /* 239 */ {"lnmt", "TIME MEAN LOG SURFACE PRESSURE [ln(cbar)]"},
   /* 240 */ {"mkmt", "TIME MEAN MASK [-/+]"},
   /* 241 */ {"vvmt", "TIME MEAN MERIDIONAL WIND (V) [m/s]"},
   /* 242 */ {"omtm", "TIME MEAN OMEGA  [cbar/s]"},
   /* 243 */ {"ptmt", "TIME MEAN POTENTIAL TEMPERATURE [K]"},
   /* 244 */ {"pcmt", "TIME MEAN PRECIP. WATER  [kg/m2]"},
   /* 245 */ {"rhmt", "TIME MEAN RELATIVE HUMIDITY [%]"},
   /* 246 */ {"mpmt", "TIME MEAN SEA LEVEL PRESSURE [hPa]"},
   /* 247 */ {"simt", "TIME MEAN SIGMADOT [1/s]"},
   /* 248 */ {"uemt", "TIME MEAN SPECIFIC HUMIDITY [kg/kg]"},
   /* 249 */ {"fcmt", "TIME MEAN STREAM FUNCTION| m2/s]"},
   /* 250 */ {"psmt", "TIME MEAN SURFACE PRESSURE [hPa]"},
   /* 251 */ {"tmmt", "TIME MEAN SURFACE TEMPERATURE [K]"},
   /* 252 */ {"pvmt", "TIME MEAN VELOCITY POTENTIAL [m2/s]"},
   /* 253 */ {"tvmt", "TIME MEAN VIRTUAL TEMPERATURE [K]"},
   /* 254 */ {"vtmt", "TIME MEAN VORTICITY [1/s]"},
   /* 255 */ {"uvmt", "TIME MEAN ZONAL WIND (U) [m/s]"},
};


const struct ParmTable parm_table_jra55_200[256] = {
      /* 0 */ {"var0", "undefined"},
      /* 1 */ {"PRES", "Pressure [Pa]"},
      /* 2 */ {"PRMSL", "Pressure reduced to MSL [Pa]"},
      /* 3 */ {"var3", "undefined"},
      /* 4 */ {"pVOR", "Potential vorticity [K*m^2/kg/s]"},
      /* 5 */ {"var5", "undefined"},
      /* 6 */ {"GP", "Geopotential [m^2/s^2]"},
      /* 7 */ {"HGT", "Geopotential height [gpm]"},
      /* 8 */ {"var8", "undefined"},
      /* 9 */ {"var9", "undefined"},
      /* 10 */ {"TOZNE", "Total ozone [Dobson]"},
      /* 11 */ {"TMP", "Temperature [K]"},
      /* 12 */ {"var12", "undefined"},
      /* 13 */ {"POT", "Potential temperature [K]"},
      /* 14 */ {"var14", "undefined"},
      /* 15 */ {"TMAX", "Maximum temperature [K]"},
      /* 16 */ {"TMIN", "Minimum temperature [K]"},
      /* 17 */ {"var17", "undefined"},
      /* 18 */ {"DEPR", "Dew-point depression [K]"},
      /* 19 */ {"var19", "undefined"},
      /* 20 */ {"var20", "undefined"},
      /* 21 */ {"var21", "undefined"},
      /* 22 */ {"var22", "undefined"},
      /* 23 */ {"var23", "undefined"},
      /* 24 */ {"var24", "undefined"},
      /* 25 */ {"var25", "undefined"},
      /* 26 */ {"var26", "undefined"},
      /* 27 */ {"var27", "undefined"},
      /* 28 */ {"var28", "undefined"},
      /* 29 */ {"var29", "undefined"},
      /* 30 */ {"var30", "undefined"},
      /* 31 */ {"var31", "undefined"},
      /* 32 */ {"var32", "undefined"},
      /* 33 */ {"UGRD", "u-component of wind [m/s]"},
      /* 34 */ {"VGRD", "v-component of wind [m/s]"},
      /* 35 */ {"STRM", "Stream function [m^2/s]"},
      /* 36 */ {"VPOT", "Velocity potential [m^2/s]"},
      /* 37 */ {"MNTSF", "Montgomery stream function [m^2/s^2]"},
      /* 38 */ {"var38", "undefined"},
      /* 39 */ {"VVEL", "Vertical velocity [Pa/s]"},
      /* 40 */ {"var40", "undefined"},
      /* 41 */ {"var41", "undefined"},
      /* 42 */ {"var42", "undefined"},
      /* 43 */ {"RELV", "Relative vorticity [1/s]"},
      /* 44 */ {"RELD", "Relative divergence [1/s]"},
      /* 45 */ {"var45", "undefined"},
      /* 46 */ {"var46", "undefined"},
      /* 47 */ {"var47", "undefined"},
      /* 48 */ {"var48", "undefined"},
      /* 49 */ {"var49", "undefined"},
      /* 50 */ {"var50", "undefined"},
      /* 51 */ {"SPFH", "Specific humidity [kg/kg]"},
      /* 52 */ {"RH", "Relative humidity [%]"},
      /* 53 */ {"var53", "undefined"},
      /* 54 */ {"PWAT", "Precipitable water [kg/m^2]"},
      /* 55 */ {"var55", "undefined"},
      /* 56 */ {"var56", "undefined"},
      /* 57 */ {"EVP", "Evaporation [mm/day]"},
      /* 58 */ {"CICE", "Cloud ice [kg/m^2]"},
      /* 59 */ {"var59", "undefined"},
      /* 60 */ {"var60", "undefined"},
      /* 61 */ {"TPRAT", "Total precipitation [mm/day]"},
      /* 62 */ {"LPRAT", "Large scale precipitation [mm/day]"},
      /* 63 */ {"CPRAT", "Convective precipitation [mm/day]"},
      /* 64 */ {"SRWEQ", "Snowfall rate water equivalent [mm/day]"},
      /* 65 */ {"SnWe", "Water equivalent of accumulated snow depth [kg/m^2]"},
      /* 66 */ {"SnowD", "Snow depth [m]"},
      /* 67 */ {"var67", "undefined"},
      /* 68 */ {"var68", "undefined"},
      /* 69 */ {"var69", "undefined"},
      /* 70 */ {"var70", "undefined"},
      /* 71 */ {"TCDC", "Total cloud cover [%]"},
      /* 72 */ {"var72", "undefined"},
      /* 73 */ {"LCDC", "Low cloud cover [%]"},
      /* 74 */ {"MCDC", "Medium cloud cover [%]"},
      /* 75 */ {"HCDC", "High cloud cover [%]"},
      /* 76 */ {"var76", "undefined"},
      /* 77 */ {"var77", "undefined"},
      /* 78 */ {"var78", "undefined"},
      /* 79 */ {"var79", "undefined"},
      /* 80 */ {"var80", "undefined"},
      /* 81 */ {"LAND", "Land cover (1 = land, 0 = sea) [Proportion]"},
      /* 82 */ {"var82", "undefined"},
      /* 83 */ {"SFCR", "Surface roughness [m]"},
      /* 84 */ {"var84", "undefined"},
      /* 85 */ {"SoilT", "Soil temperature [K]"},
      /* 86 */ {"var86", "undefined"},
      /* 87 */ {"var87", "undefined"},
      /* 88 */ {"var88", "undefined"},
      /* 89 */ {"var89", "undefined"},
      /* 90 */ {"ROF", "Water run-off [mm/day]"},
      /* 91 */ {"ICEC", "Ice cover (1 = ice, 0 = no ice) [Proportion]"},
      /* 92 */ {"var92", "undefined"},
      /* 93 */ {"var93", "undefined"},
      /* 94 */ {"var94", "undefined"},
      /* 95 */ {"var95", "undefined"},
      /* 96 */ {"var96", "undefined"},
      /* 97 */ {"var97", "undefined"},
      /* 98 */ {"var98", "undefined"},
      /* 99 */ {"var99", "undefined"},
      /* 100 */ {"var100", "undefined"},
      /* 101 */ {"var101", "undefined"},
      /* 102 */ {"var102", "undefined"},
      /* 103 */ {"var103", "undefined"},
      /* 104 */ {"var104", "undefined"},
      /* 105 */ {"var105", "undefined"},
      /* 106 */ {"var106", "undefined"},
      /* 107 */ {"var107", "undefined"},
      /* 108 */ {"var108", "undefined"},
      /* 109 */ {"var109", "undefined"},
      /* 110 */ {"var110", "undefined"},
      /* 111 */ {"var111", "undefined"},
      /* 112 */ {"var112", "undefined"},
      /* 113 */ {"var113", "undefined"},
      /* 114 */ {"var114", "undefined"},
      /* 115 */ {"var115", "undefined"},
      /* 116 */ {"var116", "undefined"},
      /* 117 */ {"var117", "undefined"},
      /* 118 */ {"BRTMP", "Brightness temperature [K]"},
      /* 119 */ {"var119", "undefined"},
      /* 120 */ {"var120", "undefined"},
      /* 121 */ {"LHTFL", "Latent heat flux [W/m^2]"},
      /* 122 */ {"SHTFL", "Sensible heat flux [W/m^2]"},
      /* 123 */ {"var123", "undefined"},
      /* 124 */ {"UFLX", "Momentum flux, u-component [N/m^2]"},
      /* 125 */ {"VFLX", "Momentum flux, v-component [N/m^2]"},
      /* 126 */ {"var126", "undefined"},
      /* 127 */ {"var127", "undefined"},
      /* 128 */ {"var128", "undefined"},
      /* 129 */ {"var129", "undefined"},
      /* 130 */ {"var130", "undefined"},
      /* 131 */ {"var131", "undefined"},
      /* 132 */ {"BVF2", "Square of Brunt-Vaisala frequency [1/s^2]"},
      /* 133 */ {"var133", "undefined"},
      /* 134 */ {"var134", "undefined"},
      /* 135 */ {"var135", "undefined"},
      /* 136 */ {"var136", "undefined"},
      /* 137 */ {"var137", "undefined"},
      /* 138 */ {"var138", "undefined"},
      /* 139 */ {"var139", "undefined"},
      /* 140 */ {"var140", "undefined"},
      /* 141 */ {"var141", "undefined"},
      /* 142 */ {"var142", "undefined"},
      /* 143 */ {"var143", "undefined"},
      /* 144 */ {"TSC", "Canopy temperature [K]"},
      /* 145 */ {"TSG", "Ground temperature [K]"},
      /* 146 */ {"CWORK", "Cloud work function [J/kg]"},
      /* 147 */ {"FGLU", "Zonal momentum flux by long gravity wave [N/m^2]"},
      /* 148 */ {"FGLV", "Meridional momentum flux by long gravity wave [N/m^2]"},
      /* 149 */ {"var149", "undefined"},
      /* 150 */ {"var150", "undefined"},
      /* 151 */ {"ADUA", "Adiabatic zonal acceleration [m/s/day]"},
      /* 152 */ {"VWV", "Meridional water vapour flux [Kg/m/s]"},
      /* 153 */ {"var153", "undefined"},
      /* 154 */ {"FGSV", "Meridional momentum flux by short gravity wave [N/m^2]"},
      /* 155 */ {"GFLX", "Ground heat flux [W/m^2]"},
      /* 156 */ {"var156", "undefined"},
      /* 157 */ {"UWV", "Zonal water vapour flux [kg/m/s]"},
      /* 158 */ {"var158", "undefined"},
      /* 159 */ {"FGSU", "Zonal momentum flux by short gravity wave [N/m^2]"},
      /* 160 */ {"CSUSF", "Clear sky upward solar radiation flux [W/m^2]"},
      /* 161 */ {"CSDSF", "Clear sky downward solar radiation flux [W/m^2]"},
      /* 162 */ {"CSULF", "Clear sky upward long wave radiation flux [W/m^2]"},
      /* 163 */ {"CSDLF", "Clear sky downward long wave radiation flux [W/m^2]"},
      /* 164 */ {"var164", "undefined"},
      /* 165 */ {"ADVA", "Adiabatic meridional acceleration [m/s/day]"},
      /* 166 */ {"var166", "undefined"},
      /* 167 */ {"var167", "undefined"},
      /* 168 */ {"var168", "undefined"},
      /* 169 */ {"var169", "undefined"},
      /* 170 */ {"FRCV", "Frequency of deep convection [%]"},
      /* 171 */ {"FRCVS", "Frequency of shallow convection [%]"},
      /* 172 */ {"FRSC", "Frequency of stratocumulus parameterisation [%]"},
      /* 173 */ {"GWDUA", "Gravity wave zonal acceleration [m/s/day]"},
      /* 174 */ {"GWDVA", "Gravity wave meridional acceleration [m/s/day]"},
      /* 175 */ {"var175", "undefined"},
      /* 176 */ {"var176", "undefined"},
      /* 177 */ {"var177", "undefined"},
      /* 178 */ {"var178", "undefined"},
      /* 179 */ {"var179", "undefined"},
      /* 180 */ {"var180", "undefined"},
      /* 181 */ {"var181", "undefined"},
      /* 182 */ {"var182", "undefined"},
      /* 183 */ {"var183", "undefined"},
      /* 184 */ {"var184", "undefined"},
      /* 185 */ {"var185", "undefined"},
      /* 186 */ {"var186", "undefined"},
      /* 187 */ {"var187", "undefined"},
      /* 188 */ {"var188", "undefined"},
      /* 189 */ {"var189", "undefined"},
      /* 190 */ {"UTHE", "Zonal thermal energy flux [W/m]"},
      /* 191 */ {"VTHE", "Meridional thermal energy flux [W/m]"},
      /* 192 */ {"var192", "undefined"},
      /* 193 */ {"var193", "undefined"},
      /* 194 */ {"var194", "undefined"},
      /* 195 */ {"var195", "undefined"},
      /* 196 */ {"var196", "undefined"},
      /* 197 */ {"var197", "undefined"},
      /* 198 */ {"var198", "undefined"},
      /* 199 */ {"var199", "undefined"},
      /* 200 */ {"var200", "undefined"},
      /* 201 */ {"var201", "undefined"},
      /* 202 */ {"LTRS", "Evapotranspiration [W/m^2]"},
      /* 203 */ {"LINT", "Interception loss [W/m^2]"},
      /* 204 */ {"DSWRF", "Downward solar radiation flux [W/m^2]"},
      /* 205 */ {"DLWRF", "Downward long wave radiation flux [W/m^2]"},
      /* 206 */ {"var206", "undefined"},
      /* 207 */ {"var207", "undefined"},
      /* 208 */ {"var208", "undefined"},
      /* 209 */ {"var209", "undefined"},
      /* 210 */ {"var210", "undefined"},
      /* 211 */ {"USWRF", "Upward solar radiation flux [W/m^2]"},
      /* 212 */ {"ULWRF", "Upward long wave radiation flux [W/m^2]"},
      /* 213 */ {"var213", "undefined"},
      /* 214 */ {"var214", "undefined"},
      /* 215 */ {"var215", "undefined"},
      /* 216 */ {"var216", "undefined"},
      /* 217 */ {"var217", "undefined"},
      /* 218 */ {"var218", "undefined"},
      /* 219 */ {"WSMX", "Maximum wind speed [m/s]"},
      /* 220 */ {"var220", "undefined"},
      /* 221 */ {"CWAT", "Cloud water [kg/kg]"},
      /* 222 */ {"ADHR", "Adiabatic heating rate [K/day]"},
      /* 223 */ {"MSC", "Moisture storage on canopy [m]"},
      /* 224 */ {"MSG", "Moisture storage on ground/cover [m]"},
      /* 225 */ {"SoilW", "Soil wetness [Proportion]"},
      /* 226 */ {"SMC", "Mass concentration of condensed water in soil [kg/m^3]"},
      /* 227 */ {"CW", "Cloud liquid water [kg/m^2]"},
      /* 228 */ {"CLWC", "Cloud liquid water [kg/kg]"},
      /* 229 */ {"CIWC", "Cloud ice [kg/kg]"},
      /* 230 */ {"MFLXB", "Upward mass flux at cloud base [kg/m^2/s]"},
      /* 231 */ {"MFLUX", "Upward mass flux [kg/m^2/s]"},
      /* 232 */ {"var232", "undefined"},
      /* 233 */ {"var233", "undefined"},
      /* 234 */ {"var234", "undefined"},
      /* 235 */ {"var235", "undefined"},
      /* 236 */ {"ADMR", "Adiabatic moistening rate [kg/kg/day]"},
      /* 237 */ {"OZONE", "Ozone mixing ratio [mg/kg]"},
      /* 238 */ {"var238", "undefined"},
      /* 239 */ {"CNVUA", "Convective zonal acceleration [m/s/day]"},
      /* 240 */ {"CNVVA", "Convective meridional acceleration [m/s/day]"},
      /* 241 */ {"LRGHR", "Large scale condensation heating rate [K/day]"},
      /* 242 */ {"CNVHR", "Convective heating rate [k/day]"},
      /* 243 */ {"CNVMR", "Convective moistening rate [kg/kg/day]"},
      /* 244 */ {"var244", "undefined"},
      /* 245 */ {"var245", "undefined"},
      /* 246 */ {"VDFHR", "Vertical diffusion heating rate [K/day]"},
      /* 247 */ {"VDFUA", "Vertical diffusion zonal acceleration [m/s/day]"},
      /* 248 */ {"VDFVA", "Vertical diffusion meridional acceleration [m/s/day]"},
      /* 249 */ {"VDFMR", "Vertical diffusion moistening rate [kg/kg/day]"},
      /* 250 */ {"SWHR", "Solar radiative heating rate [k/day]"},
      /* 251 */ {"LWHR", "Long wave radiative heating rate [K/day]"},
      /* 252 */ {"VTYPE", "Type of vegetation"},
      /* 253 */ {"LRGMR", "Large scale moistening rate [kg/kg/day]"},
      /* 254 */ {"var254", "undefined"},
      /* 255 */ {"var255", "undefined"},
};

/*
 * support for complex packing
 *  determine the number of data points in the BDS
 *  does not handle matrix values
 */

extern int ec_large_grib,  len_ec_bds;


int BDS_NValues(unsigned char *bds) {

    /* returns number of grid points as determined from the BDS */

    int i = 0;

    if (BDS_SimplePacking(bds) && BDS_Grid(bds)) {
	i = ((BDS_LEN(bds) - BDS_DataStart(bds))*8 -
		BDS_UnusedBits(bds)) / (BDS_NumBits(bds));
    }
    else if (BDS_ComplexPacking(bds) && BDS_Grid(bds)) {
	i = BDS_P2(bds);
    }
    return i;
}
