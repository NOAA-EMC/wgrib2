/* grb2.h  10/2024  Public Domain   Wesley Ebisuzaki */

#ifndef INT3
#define INT3(a,b,c) ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 127) << 16)+(b<<8)+c))
#endif
#ifndef INT2
#define INT2(a,b)   ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 127) << 8) + b))
#endif
#ifndef INT1
#define INT1(a)   ((a & 0x80) ? - (int) (a & 127) : (int) (a & 127))
#endif

#ifndef UINT4
#define UINT4(a,b,c,d) ((int) ((a <<24) + (b << 16) + (c << 8) + (d)))
#endif

#ifndef UINT3
#define UINT3(a,b,c) ((int) ((a << 16) + (b << 8) + (c)))
#endif

#ifndef UINT2
#define UINT2(a,b) ((int) ((a << 8) + (b)))
#endif

/* Section 0 */
#define	GB2_Sec0_size			16
#define GB2_Discipline(sec)		((int) (sec[0][6]))
#define GB2_Edition(sec)		((int) (sec[0][7]))
#define GB2_MsgLen(sec)			uint8(&(sec[0][8]))

/* Section 1 */
#define GB2_Sec1_size(sec)		(sec[1] ? uint4(sec[1]+0) : 0)
#define GB2_Center(sec)			UINT2(sec[1][5], sec[1][6])
#define GB2_Subcenter(sec)		UINT2(sec[1][7], sec[1][8])
#define GB2_MasterTable(sec)		((int) (sec[1][9]))
// #define GB2_LocalTable(sec)		((int) (sec[1][10]))
#define GB2_LocalTable(sec)		sec[1][10]

/* Section 2 */
#define GB2_Sec2_size(sec)		(sec[2] ? uint4(sec[2]+0) : 0)

/* Section 3 */
#define GB2_Sec3_size(sec)		(sec[3] ? uint4(sec[3]+0) : 0)
#define GB2_Sec3_num(sec)		((int) (sec[3][4]))
#define GB2_Sec3_gdef(sec)		((int) (sec[3][5]))
#define GB2_Sec3_npts(sec)		uint4(sec[3]+6)
// #define GB2_gds_npts(gds)		uint4(gds+6)
/*      #define GB2_Sec3_GridDefTemplateNo(sec)	UINT2(sec[3][12], sec[3][13]) */
/*      #define GB2_GridDefTemplateNo(sec)	UINT2(gds[12], gds[13]) */

#define GDS_Lambert_La1(gds)		(int4(gds+38) * 0.000001)
#define GDS_Lambert_Lo1(gds)		(int4(gds+42) * 0.000001)
#define GDS_Lambert_LatD(gds)		(int4(gds+47) * 0.000001)
#define GDS_Lambert_Lov(gds)		(int4(gds+51) * 0.000001)
#define GDS_Lambert_Latin1(gds)		(int4(gds+65) * 0.000001)
#define GDS_Lambert_Latin2(gds)		(int4(gds+69) * 0.000001)
#define GDS_Lambert_LatSP(gds)		(int4(gds+73) * 0.000001)
#define GDS_Lambert_LonSP(gds)		(int4(gds+77) * 0.000001)

#define GDS_Lambert_NP(gds)		(((gds[63]) & 128) == 0)
#define GDS_Lambert_nx(gds)		(uint4_missing(gds+30))
#define GDS_Lambert_ny(gds)		(uint4_missing(gds+34))
#define GDS_Lambert_dx(gds)		(int4(gds+55) * 0.001) 
#define GDS_Lambert_dy(gds)		(int4(gds+59) * 0.001) 

#define GDS_Albers_La1(gds)		(int4(gds+38) * 0.000001)
#define GDS_Albers_Lo1(gds)		(int4(gds+42) * 0.000001)
#define GDS_Albers_LatD(gds)		(int4(gds+47) * 0.000001)
#define GDS_Albers_Lov(gds)		(int4(gds+51) * 0.000001)
#define GDS_Albers_Latin1(gds)		(int4(gds+65) * 0.000001)
#define GDS_Albers_Latin2(gds)		(int4(gds+69) * 0.000001)
#define GDS_Albers_LatSP(gds)		(int4(gds+73) * 0.000001)
#define GDS_Albers_LonSP(gds)		(int4(gds+77) * 0.000001)

#define GDS_Albers_NP(gds)		(((gds[63]) & 128) == 0)
#define GDS_Albers_nx(gds)		(uint4_missing(gds+30))
#define GDS_Albers_ny(gds)		(uint4_missing(gds+34))
#define GDS_Albers_dx(gds)		(int4(gds+55) * 0.001) 
#define GDS_Albers_dy(gds)		(int4(gds+59) * 0.001) 

#define GDS_LatLon_basic_ang(gds)	int4(gds+38)
#define GDS_LatLon_sub_ang(gds)		sub_angle(gds+42)
#define GDS_LatLon_lat1(gds)		int4(gds+46)
#define GDS_LatLon_lon1(gds)		uint4(gds+50)
#define GDS_LatLon_lat2(gds)		int4(gds+55)
#define GDS_LatLon_lon2(gds)		uint4(gds+59)
#define GDS_LatLon_dlon(gds)		int4(gds+63)
#define GDS_LatLon_dlat(gds)		int4(gds+67)
#define GDS_LatLon_nx(gds)		(uint4(gds+30))
#define GDS_LatLon_ny(gds)		(uint4(gds+34))

#define GDS_RotLatLon_sp_lat(gds)	(int4(gds+72))
#define GDS_RotLatLon_sp_lon(gds)	(uint4(gds+76))
#define GDS_RotLatLon_rotation(gds)	(int4(gds+80))

#define GDS_NCEP_B_LatLon_nx(gds)           (uint4(gds+30))
#define GDS_NCEP_B_LatLon_ny(gds)           (uint4(gds+34))
#define GDS_NCEP_B_LatLon_basic_ang(gds)    int4(gds+38)
#define GDS_NCEP_B_LatLon_sub_ang(gds)      sub_angle(gds+42)
#define GDS_NCEP_B_LatLon_lat1(gds)         int4(gds+46)
#define GDS_NCEP_B_LatLon_lon1(gds)         uint4(gds+50)
#define GDS_NCEP_B_LatLon_tph0d(gds)        int4(gds+55)
#define GDS_NCEP_B_LatLon_tlm0d(gds)        uint4(gds+59)
#define GDS_NCEP_B_LatLon_dlon(gds)         int4(gds+63)
#define GDS_NCEP_B_LatLon_dlat(gds)         int4(gds+67)
#define GDS_NCEP_B_LatLon_lat2(gds)         (int4(gds+72))
#define GDS_NCEP_B_LatLon_lon2(gds)         (uint4(gds+76))

#define GDS_NCEP_E_LatLon_nx(gds)           (uint4(gds+30))
#define GDS_NCEP_E_LatLon_ny(gds)           (uint4(gds+34))
#define GDS_NCEP_E_LatLon_basic_ang(gds)    int4(gds+38)
#define GDS_NCEP_E_LatLon_sub_ang(gds)      sub_angle(gds+42)
#define GDS_NCEP_E_LatLon_lat1(gds)         int4(gds+46)
#define GDS_NCEP_E_LatLon_lon1(gds)         uint4(gds+50)
#define GDS_NCEP_E_LatLon_tph0d(gds)        int4(gds+55)
#define GDS_NCEP_E_LatLon_tlm0d(gds)        uint4(gds+59)
#define GDS_NCEP_E_LatLon_dlon(gds)         int4(gds+63)
#define GDS_NCEP_E_LatLon_dlat(gds)         int4(gds+67)


#define GDS_Mercator_nx(gds)		(uint4(gds+30))
#define GDS_Mercator_ny(gds)		(uint4(gds+34))
#define GDS_Mercator_dx(gds)		((uint4(gds+64))*0.001)
#define GDS_Mercator_dy(gds)		((uint4(gds+68))*0.001)
#define GDS_Mercator_lat1(gds)		(int4(gds+38)*0.000001)
#define GDS_Mercator_lon1(gds)		(uint4(gds+42)*0.000001)
#define GDS_Mercator_lat2(gds)		(int4(gds+51)*0.000001)
#define GDS_Mercator_lon2(gds)		(uint4(gds+55)*0.000001)
#define GDS_Mercator_latD(gds)		(int4(gds+47)*0.000001)
#define GDS_Mercator_ori_angle(gds)	(uint4(gds+60)*0.000001)

#define GDS_Polar_nx(gds)		(uint4_missing(gds+30))
#define GDS_Polar_ny(gds)		(uint4_missing(gds+34))
#define GDS_Polar_lat1(gds)		(int4(gds+38)*0.000001)
#define GDS_Polar_lon1(gds)		(uint4(gds+42)*0.000001)
#define GDS_Polar_lad(gds)		(int4(gds+47)*0.000001)
#define GDS_Polar_lov(gds)		(uint4(gds+51)*0.000001)
#define GDS_Polar_dx(gds)		(uint4(gds+55)*0.001)
#define GDS_Polar_dy(gds)		(uint4(gds+59)*0.001)
#define GDS_Polar_nps(gds)		((gds[63] & 128) == 0)
#define GDS_Polar_sps(gds)		((gds[63] & 128) == 128)

#define GDS_Gaussian_nx(gds)		(uint4_missing(gds+30))
#define GDS_Gaussian_ny(gds)		(uint4(gds+34))
#define GDS_Gaussian_nlat(gds)		(uint4(gds+67))
#define GDS_Gaussian_basic_ang(gds)	int4(gds+38)
#define GDS_Gaussian_sub_ang(gds)	sub_angle(gds+42)
#define GDS_Gaussian_lat1(gds)		int4(gds+46)
#define GDS_Gaussian_lon1(gds)		uint4(gds+50)
#define GDS_Gaussian_lat2(gds)		int4(gds+55)
#define GDS_Gaussian_lon2(gds)		uint4(gds+59)
#define GDS_Gaussian_dlon(gds)		int4(gds+63)

#define GDS_Harmonic_j(gds)		int4(gds+14)
#define GDS_Harmonic_k(gds)		int4(gds+18)
#define GDS_Harmonic_m(gds)		int4(gds+22)
#define GDS_Harmonic_code_3_6(gds)	((int) gds[26])
#define GDS_Harmonic_code_3_7(gds)	((int) gds[27])

#define GDS_Space_lap(gds)		(int4(gds+38)*1e-6)
#define GDS_Space_lop(gds)		(int4(gds+42)*1e-6)
#define GDS_Space_dx(gds)		uint4(gds+47)
#define GDS_Space_dy(gds)		uint4(gds+51)
#define GDS_Space_xp(gds)		(int4(gds+55)*1e-3)
#define GDS_Space_yp(gds)		(int4(gds+59)*1e-3)
#define GDS_Space_ori(gds)		(int4(gds+64)*1e-6)
// #define GDS_Space_altitude(gds)		(uint4_missing(gds+68) == -1 ? -1 : int4(gds+68)*1e-6)
#define GDS_Space_altitude(gds)		(uint4_missing(gds+68) == 0 ? -1 : int4(gds+68)*1e-6)
#define GDS_Space_x0(gds)		(int4(gds+72))
#define GDS_Space_y0(gds)		(int4(gds+76))

#define GDS_AzRan_lat1(gds)		(int4(gds+22)*1e-6)
#define GDS_AzRan_lon1(gds)		(uint4(gds+26)*1e-6)
#define GDS_AzRan_dx(gds)		(uint4(gds+30)*1e-3)
#define GDS_AzRan_dstart(gds)		(uint4(gds+34)*1e-3)


#define GDS_Lambert_Az_La1(gds)         (int4(gds+38) * 0.000001)
// #define GDS_Lambert_Az_Lo1(gds)         (uint4(gds+42) * 0.000001)
#define GDS_Lambert_Az_Lo1(gds)         (int4(gds+42) * 0.000001)
#define GDS_Lambert_Az_Std_Par(gds)     (int4(gds+46) * 0.000001)
#define GDS_Lambert_Az_Cen_Lon(gds)     (int4(gds+50) * 0.000001)
#define GDS_Lambert_Az_nx(gds)          (uint4_missing(gds+30))
#define GDS_Lambert_Az_ny(gds)          (uint4_missing(gds+34))
#define GDS_Lambert_Az_dx(gds)          (int4(gds+55) * 0.001)
#define GDS_Lambert_Az_dy(gds)          (int4(gds+59) * 0.001)

// #ifdef WMO_VALIDATION
#define GDS_Gnom_face_size(gds)         uint4(gds+38)
#define GDS_Gnom_i_offset(gds)		uint4(gds+42)
#define GDS_Gnom_j_offset(gds)		uint4(gds+46)
#define GDS_Gnom_tile(gds)              gds[50]
#define GDS_Gnom_SP_Lat(gds)            (int4(gds+51) * 0.000001)
#define GDS_Gnom_SP_Lon(gds)            (uint4(gds+55) * 0.000001)
// not sure if rotation angle can be negative
#define GDS_Gnom_SP_Rot(gds)            (int4(gds+59) * 0.000001)
#define GDS_Gnom_Stretch(gds)           (int4(gds+63) * 0.000001)
#define GDS_Gnom_B(gds)                 (int4(gds+67) * 0.000001)
// #endif


#define GDS_CrossSec_basic_ang(gds)	int4(gds+34)
#define GDS_CrossSec_sub_ang(gds)	int4(gds+38)
#define GDS_CrossSec_lat1(gds)		int4(gds+42)
#define GDS_CrossSec_lon1(gds)		uint4(gds+46)
#define GDS_CrossSec_lat2(gds)		int4(gds+51)
#define GDS_CrossSec_lon2(gds)		uint4(gds+55)

/* GDS_Scan_x -> +ve x scanning */
#define GDS_Scan_x(scan)		((scan & 128) == 0)
/* GDS_Scan_y -> +ve y scanning */
#define GDS_Scan_y(scan)		((scan & 64) == 64)
/* GDS_Scan_fortran -> fortran storage order */
#define GDS_Scan_fortran(scan)		((scan & 32) == 32)
/* GDS_Scan_row_rev -> row reversing order */
#define GDS_Scan_row_rev(scan)		((scan & 16) == 16)
/* GDS_Scan_staggered test for staggered grid*/
#define GDS_Scan_staggered(scan)	(((scan) & 15) != 0)
/* GDS_Scan_staggered_storage test for grid size != nx*ny */
#define GDS_Scan_staggered_storage(scan)	(((scan) & (1)) != 0)

/* Section 4 */
#define GB2_Sec4_size(sec)		(sec[4] ? uint4(sec[4]+0) : 0)
#define GB2_Sec4_num(sec)		((int) (sec[4][4]))
#define GB2_ProdDefTemplateNo(sec)	(UINT2(sec[4][7],sec[4][8]))

#define GB2_ParmCat(sec)		(sec[4][9])

#define GB2_ParmNum(sec)		(sec[4][10])

// #define GB2_ForecastTime(sec)		(UINT4(sec[4][18],sec[4][19],sec[4][20],sec[4][21]))
// replaced by forecast_time_in_units
// #define GB2_TimeUnit2(sec)		(sec[4][48])
// #define GB2_ForecastTime2(sec)		(UINT4(sec[4][49],sec[4][50],sec[4][51],sec[4][52]))
// #define GB2_StatProcess(sec)            UINT2(sec[4][45], sec[4][46])

/* Section 5 */
#define GB2_Sec5_size(sec)		(sec[5] ? uint4(sec[5]+0) : 0)
#define GB2_Sec5_nval(sec)		(sec[5] ? uint4(sec[5]+5) : 0)

/* Section 6 */
#define GB2_Sec6_size(sec)		(sec[6] ? uint4(sec[6]+0) : 0)

/* Section 7 */
#define GB2_Sec7_size(sec)		(sec[7] ? uint4(sec[7]+0) : 0)

/* Section 8 */
#define	GB2_Sec8_size			4

/* some center codes */
#define NCEP 7
#define ECMWF 98
#define JMA1 34
#define JMA2 35
#define KMA 40
#define DWD1 78
#define DWD2 79
