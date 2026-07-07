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

