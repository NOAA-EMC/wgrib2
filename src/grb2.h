/** @file
 * @brief Header file for decoding GRIB sections.
 * @author Public Domain: Wesley Ebisuzaki @date 2005
 */

/** Convert 3 bytes to a signed integer. */
#ifndef INT3
#define INT3(a,b,c) ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 127) << 16)+(b<<8)+c))
#endif

/** Convert 2 bytes to a signed integer. */
#ifndef INT2
#define INT2(a,b)   ((1-(int) ((unsigned) (a & 0x80) >> 6)) * (int) (((a & 127) << 8) + b))
#endif

/** Convert 1 byte to a signed integer. */
#ifndef INT1
#define INT1(a)   ((a & 0x80) ? - (int) (a & 127) : (int) (a & 127))
#endif

/** Convert 4 bytes to an unsigned integer. */
#ifndef UINT4
#define UINT4(a,b,c,d) ((int) ((a <<24) + (b << 16) + (c << 8) + (d)))
#endif

/** Convert 3 bytes to an unsigned integer. */
#ifndef UINT3
#define UINT3(a,b,c) ((int) ((a << 16) + (b << 8) + (c)))
#endif

/** Convert 2 bytes to an unsigned integer. */
#ifndef UINT2
#define UINT2(a,b) ((int) ((a << 8) + (b)))
#endif

/* Section 0 */
#define	GB2_Sec0_size			16                      /**< Size of Section 0 in bytes. */
#define GB2_Discipline(sec)		((int) (sec[0][6]))     /**< GRIB discipline. */
#define GB2_Edition(sec)		((int) (sec[0][7]))     /**< GRIB edition. */
#define GB2_MsgLen(sec)			uint8(&(sec[0][8]))    /**< Length of the GRIB message. */

/* Section 1 */
#define GB2_Sec1_size(sec)		(sec[1] ? uint4(sec[1]+0) : 0)  /**< Size of Section 1 in bytes. */
#define GB2_Center(sec)			UINT2(sec[1][5], sec[1][6])     /**< GRIB center. */
#define GB2_Subcenter(sec)		UINT2(sec[1][7], sec[1][8])     /**< GRIB subcenter. */
#define GB2_MasterTable(sec)		((int) (sec[1][9]))         /**< GRIB master table. */
// #define GB2_LocalTable(sec)		((int) (sec[1][10]))
#define GB2_LocalTable(sec)		sec[1][10]                      /**< GRIB local table. */

/* Section 2 */
#define GB2_Sec2_size(sec)		(sec[2] ? uint4(sec[2]+0) : 0)  /**< Size of Section 2 in bytes. */

/* Section 3 */
#define GB2_Sec3_size(sec)		(sec[3] ? uint4(sec[3]+0) : 0)  /**< Size of Section 3 in bytes. */
#define GB2_Sec3_num(sec)		((int) (sec[3][4]))             /**< Number of Section 3. */
#define GB2_Sec3_gdef(sec)		((int) (sec[3][5]))             /**< Source of Grid Definition. */
#define GB2_Sec3_npts(sec)		uint4(sec[3]+6)                 /** Number of data points. */
// #define GB2_gds_npts(gds)		uint4(gds+6)
/*      #define GB2_Sec3_GridDefTemplateNo(sec)	UINT2(sec[3][12], sec[3][13]) */
/*      #define GB2_GridDefTemplateNo(sec)	UINT2(gds[12], gds[13]) */

#define GDS_Lambert_La1(gds)		(int4(gds+38) * 0.000001)   /**< Lambert Conformal - Latitude of first grid point. */
#define GDS_Lambert_Lo1(gds)		(int4(gds+42) * 0.000001)   /**< Lambert Conformal - Longitude of first grid point. */
#define GDS_Lambert_LatD(gds)		(int4(gds+47) * 0.000001)   /**< Lambert Conformal - Latitude where dx and dy are specified. */
#define GDS_Lambert_Lov(gds)		(int4(gds+51) * 0.000001)   /**< Lambert Conformal - Longitude of merididian parallel to y-axis. */
#define GDS_Lambert_Latin1(gds)		(int4(gds+65) * 0.000001)   /**< Lambert Conformal - First latitude from the pole at which the secant cone cuts the sphere. */
#define GDS_Lambert_Latin2(gds)		(int4(gds+69) * 0.000001)   /**< Lambert Conformal - Second latitude from the pole at which the secant cone cuts the sphere. */
#define GDS_Lambert_LatSP(gds)		(int4(gds+73) * 0.000001)   /**< Lambert Conformal - Latitude of Southern Pole. */
#define GDS_Lambert_LonSP(gds)		(int4(gds+77) * 0.000001)   /**< Lambert Conformal - Longitude of Southern Pole. */

#define GDS_Lambert_NP(gds)		(((gds[63]) & 128) == 0)    /**< Lambert Conformal - North Pole is on projection plane. */
#define GDS_Lambert_nx(gds)		(uint4_missing(gds+30))     /**< Lambert Conformal - Number of grid points in x-direction. */
#define GDS_Lambert_ny(gds)		(uint4_missing(gds+34))     /**< Lambert Conformal - Number of grid points in y-direction. */
#define GDS_Lambert_dx(gds)		(int4(gds+55) * 0.001)      /**< Lambert Conformal - Grid spacing in x-direction. */
#define GDS_Lambert_dy(gds)		(int4(gds+59) * 0.001)      /**< Lambert Conformal - Grid spacing in y-direction. */

#define GDS_Albers_La1(gds)		(int4(gds+38) * 0.000001)       /**< Albers - Latitude of first grid point. */
#define GDS_Albers_Lo1(gds)		(int4(gds+42) * 0.000001)       /**< Albers - Longitude of first grid point. */
#define GDS_Albers_LatD(gds)		(int4(gds+47) * 0.000001)   /**< Albers - Latitude where dx and dy are specified. */
#define GDS_Albers_Lov(gds)		(int4(gds+51) * 0.000001)       /**< Albers - Longitude of merididian parallel to y-axis. */
#define GDS_Albers_Latin1(gds)		(int4(gds+65) * 0.000001)   /**< Albers - First latitude from the pole at which the secant cone cuts the sphere. */
#define GDS_Albers_Latin2(gds)		(int4(gds+69) * 0.000001)   /**< Albers - Second latitude from the pole at which the secant cone cuts the sphere. */
#define GDS_Albers_LatSP(gds)		(int4(gds+73) * 0.000001)   /**< Albers - Latitude of Southern Pole. */
#define GDS_Albers_LonSP(gds)		(int4(gds+77) * 0.000001)   /**< Albers - Longitude of Southern Pole. */

#define GDS_Albers_NP(gds)		(((gds[63]) & 128) == 0)    /**< Albers - North Pole is on projection plane. */
#define GDS_Albers_nx(gds)		(uint4_missing(gds+30))     /**< Albers - Number of grid points in x-direction. */
#define GDS_Albers_ny(gds)		(uint4_missing(gds+34))     /**< Albers - Number of grid points in y-direction. */
#define GDS_Albers_dx(gds)		(int4(gds+55) * 0.001)      /**< Albers - Grid spacing in x-direction. */
#define GDS_Albers_dy(gds)		(int4(gds+59) * 0.001)      /**< Albers - Grid spacing in y-direction. */

#define GDS_LatLon_basic_ang(gds)	int4(gds+38)        /**< LatLon - Basic angle. */
#define GDS_LatLon_sub_ang(gds)		sub_angle(gds+42)   /**< LatLon - Sub angle. */
#define GDS_LatLon_lat1(gds)		int4(gds+46)        /**< LatLon - Latitude of first grid point. */
#define GDS_LatLon_lon1(gds)		uint4(gds+50)       /**< LatLon - Longitude of first grid point. */
#define GDS_LatLon_lat2(gds)		int4(gds+55)        /**< LatLon - Latitude of last grid point. */
#define GDS_LatLon_lon2(gds)		uint4(gds+59)       /**< LatLon - Longitude of last grid point. */
#define GDS_LatLon_dlon(gds)		int4(gds+63)        /**< LatLon - Longitude increment. */
#define GDS_LatLon_dlat(gds)		int4(gds+67)        /**< LatLon - Latitude increment. */
#define GDS_LatLon_nx(gds)		(uint4(gds+30))         /**< LatLon - Number of grid points in x-direction. */
#define GDS_LatLon_ny(gds)		(uint4(gds+34))         /**< LatLon - Number of grid points in y-direction. */

#define GDS_RotLatLon_sp_lat(gds)	(int4(gds+72))      /**< Rotated LatLon - Latitude of Southern Pole. */
#define GDS_RotLatLon_sp_lon(gds)	(uint4(gds+76))     /**< Rotated LatLon - Longitude of Southern Pole. */
#define GDS_RotLatLon_rotation(gds)	(int4(gds+80))      /**< Rotated LatLon - Rotation angle. */

#define GDS_NCEP_B_LatLon_nx(gds)           (uint4(gds+30))     /**< NCEP Rot LatLon Non-E - Number of grid points in x-direction. */
#define GDS_NCEP_B_LatLon_ny(gds)           (uint4(gds+34))     /**< NCEP Rot LatLon Non-E - Number of grid points in y-direction. */
#define GDS_NCEP_B_LatLon_basic_ang(gds)    int4(gds+38)        /**< NCEP Rot LatLon Non-E - Basic angle. */
#define GDS_NCEP_B_LatLon_sub_ang(gds)      sub_angle(gds+42)   /**< NCEP Rot LatLon Non-E - Sub angle. */
#define GDS_NCEP_B_LatLon_lat1(gds)         int4(gds+46)        /**< NCEP Rot LatLon Non-E - Latitude of first grid point. */
#define GDS_NCEP_B_LatLon_lon1(gds)         uint4(gds+50)       /**< NCEP Rot LatLon Non-E - Longitude of first grid point. */
#define GDS_NCEP_B_LatLon_tph0d(gds)        int4(gds+55)        /**< NCEP Rot LatLon Non-E - Tangent point height at 0 degrees. */
#define GDS_NCEP_B_LatLon_tlm0d(gds)        uint4(gds+59)       /**< NCEP Rot LatLon Non-E - Tangent line meridian at 0 degrees. */
#define GDS_NCEP_B_LatLon_dlon(gds)         int4(gds+63)        /**< NCEP Rot LatLon Non-E - Longitude increment. */
#define GDS_NCEP_B_LatLon_dlat(gds)         int4(gds+67)        /**< NCEP Rot LatLon Non-E - Latitude increment. */
#define GDS_NCEP_B_LatLon_lat2(gds)         (int4(gds+72))      /**< NCEP Rot LatLon Non-E - Latitude of last grid point. */
#define GDS_NCEP_B_LatLon_lon2(gds)         (uint4(gds+76))     /**< NCEP Rot LatLon Non-E - Longitude of last grid point. */

#define GDS_NCEP_E_LatLon_nx(gds)           (uint4(gds+30))     /**< NCEP Rot LatLon E - Number of grid points in x-direction. */
#define GDS_NCEP_E_LatLon_ny(gds)           (uint4(gds+34))     /**< NCEP Rot LatLon E - Number of grid points in y-direction. */
#define GDS_NCEP_E_LatLon_basic_ang(gds)    int4(gds+38)        /**< NCEP Rot LatLon E - Basic angle. */
#define GDS_NCEP_E_LatLon_sub_ang(gds)      sub_angle(gds+42)   /**< NCEP Rot LatLon E - Sub angle. */
#define GDS_NCEP_E_LatLon_lat1(gds)         int4(gds+46)        /**< NCEP Rot LatLon E - Latitude of first grid point. */
#define GDS_NCEP_E_LatLon_lon1(gds)         uint4(gds+50)       /**< NCEP Rot LatLon E - Longitude of first grid point. */
#define GDS_NCEP_E_LatLon_tph0d(gds)        int4(gds+55)        /**< NCEP Rot LatLon E - Tangent point height at 0 degrees. */
#define GDS_NCEP_E_LatLon_tlm0d(gds)        uint4(gds+59)       /**< NCEP Rot LatLon E - Tangent line meridian at 0 degrees. */
#define GDS_NCEP_E_LatLon_dlon(gds)         int4(gds+63)        /**< NCEP Rot LatLon E - Longitude increment. */
#define GDS_NCEP_E_LatLon_dlat(gds)         int4(gds+67)        /**< NCEP Rot LatLon E - Latitude increment. */


#define GDS_Mercator_nx(gds)		(uint4(gds+30))             /**< Mercator - Number of grid points in x-direction. */
#define GDS_Mercator_ny(gds)		(uint4(gds+34))             /**< Mercator - Number of grid points in y-direction. */
#define GDS_Mercator_dx(gds)		((uint4(gds+64))*0.001)     /**< Mercator - X-direction grid length. */
#define GDS_Mercator_dy(gds)		((uint4(gds+68))*0.001)     /**< Mercator - Y-direction grid length. */
#define GDS_Mercator_lat1(gds)		(int4(gds+38)*0.000001)     /**< Mercator - Latitude of first grid point. */
#define GDS_Mercator_lon1(gds)		(uint4(gds+42)*0.000001)    /**< Mercator - Longitude of first grid point. */
#define GDS_Mercator_lat2(gds)		(int4(gds+51)*0.000001)     /**< Mercator - Latitude of last grid point. */
#define GDS_Mercator_lon2(gds)		(uint4(gds+55)*0.000001)    /**< Mercator - Longitude of last grid point. */
#define GDS_Mercator_latD(gds)		(int4(gds+47)*0.000001)     /**< Mercator - Latitude  at which the Mercator projection intersects the Earth. */
#define GDS_Mercator_ori_angle(gds)	(uint4(gds+60)*0.000001)    /**< Mercator - Orientation angle. */

#define GDS_Polar_nx(gds)		(uint4_missing(gds+30))     /**< Polar - Number of grid points in x-direction. */
#define GDS_Polar_ny(gds)		(uint4_missing(gds+34))     /**< Polar - Number of grid points in y-direction. */
#define GDS_Polar_lat1(gds)		(int4(gds+38)*0.000001)     /**< Polar - Latitude of first grid point. */
#define GDS_Polar_lon1(gds)		(uint4(gds+42)*0.000001)    /**< Polar - Longitude of first grid point. */
#define GDS_Polar_lad(gds)		(int4(gds+47)*0.000001)     /**< Polar - Latitude where dx and dy are defined. */
#define GDS_Polar_lov(gds)		(uint4(gds+51)*0.000001)    /**< Polar - Orientation of the grid. */
#define GDS_Polar_dx(gds)		(uint4(gds+55)*0.001)       /**< Polar - X-direction grid length. */
#define GDS_Polar_dy(gds)		(uint4(gds+59)*0.001)       /**< Polar - Y-direction grid length. */
#define GDS_Polar_nps(gds)		((gds[63] & 128) == 0)      /**< Polar - North Pole is on projection plane. */
#define GDS_Polar_sps(gds)		((gds[63] & 128) == 128)    /**< Polar - South Pole is on projection plane. */

#define GDS_Gaussian_nx(gds)		(uint4_missing(gds+30))     /**< Gaussian - Number of grid points in x-direction. */
#define GDS_Gaussian_ny(gds)		(uint4(gds+34))             /**< Gaussian - Number of grid points in y-direction. */
#define GDS_Gaussian_nlat(gds)		(uint4(gds+67))             /**< Gaussian - Number of parallels between a pole and the equator. */
#define GDS_Gaussian_basic_ang(gds)	int4(gds+38)                /**< Gaussian - Basic angle. */
#define GDS_Gaussian_sub_ang(gds)	sub_angle(gds+42)           /**< Gaussian - Sub angle. */
#define GDS_Gaussian_lat1(gds)		int4(gds+46)                /**< Gaussian - Latitude of first grid point. */
#define GDS_Gaussian_lon1(gds)		uint4(gds+50)               /**< Gaussian - Longitude of first grid point. */
#define GDS_Gaussian_lat2(gds)		int4(gds+55)                /**< Gaussian - Latitude of last grid point. */
#define GDS_Gaussian_lon2(gds)		uint4(gds+59)               /**< Gaussian - Longitude of last grid point. */
#define GDS_Gaussian_dlon(gds)		int4(gds+63)                /**< Gaussian - Longitude increment. */

#define GDS_Harmonic_j(gds)		int4(gds+14)            /**< Harmonic - Pentagonal resolution parameter J. */
#define GDS_Harmonic_k(gds)		int4(gds+18)            /**< Harmonic - Pentagonal resolution parameter K. */
#define GDS_Harmonic_m(gds)		int4(gds+22)            /**< Harmonic - Pentagonal resolution parameter M. */
#define GDS_Harmonic_code_3_6(gds)	((int) gds[26])     /**< Harmonic - Spectral Data Rep. Type (Code Table 3.6). */
#define GDS_Harmonic_code_3_7(gds)	((int) gds[27])     /**< Harmonic - Spectral Data Rep. Mode (Code Table 3.7). */

#define GDS_Space_lap(gds)		(int4(gds+38)*1e-6)     /**< Space View - Latitude of sub-satellite point. */
#define GDS_Space_lop(gds)		(int4(gds+42)*1e-6)     /**< Space View - Longitude of sub-satellite point. */
#define GDS_Space_dx(gds)		uint4(gds+47)           /**< Space View - X-direction grid length. */
#define GDS_Space_dy(gds)		uint4(gds+51)           /**< Space View - Y-direction grid length. */
#define GDS_Space_xp(gds)		(int4(gds+55)*1e-3)     /**< Space View - X-coordinate of sub-satellite point. */
#define GDS_Space_yp(gds)		(int4(gds+59)*1e-3)     /**< Space View - Y-coordinate of sub-satellite point. */
#define GDS_Space_ori(gds)		(int4(gds+64)*1e-6)     /**< Space View - Orientation angle. */
// #define GDS_Space_altitude(gds)		(uint4_missing(gds+68) == -1 ? -1 : int4(gds+68)*1e-6)
#define GDS_Space_altitude(gds)		(uint4_missing(gds+68) == 0 ? -1 : int4(gds+68)*1e-6)   /**< Space View - Altitude. */
#define GDS_Space_x0(gds)		(int4(gds+72))          /**< Space View - X-coordinate of origin of sector image. */
#define GDS_Space_y0(gds)		(int4(gds+76))          /**< Space View - Y-coordinate of origin of sector image. */

#define GDS_AzRan_lat1(gds)		(int4(gds+22)*1e-6)         /**< Azimuth-Range Projection - Latitude of center point. */
#define GDS_AzRan_lon1(gds)		(uint4(gds+26)*1e-6)        /**< Azimuth-Range Projection - Longitude of center point. */
#define GDS_AzRan_dx(gds)		(uint4(gds+30)*1e-3)        /**< Azimuth-Range Projection - Spacing of bins along radials. */
#define GDS_AzRan_dstart(gds)		(uint4(gds+34)*1e-3)       /**< Azimuth-Range Projection - Offset from origin. */


#define GDS_Lambert_Az_La1(gds)         (int4(gds+38) * 0.000001)       /**< Lambert Azimuthal - Latitude of first grid point. */
// #define GDS_Lambert_Az_Lo1(gds)         (uint4(gds+42) * 0.000001)
#define GDS_Lambert_Az_Lo1(gds)         (int4(gds+42) * 0.000001)       /**< Lambert Azimuthal - Longitude of first grid point. */
#define GDS_Lambert_Az_Std_Par(gds)     (int4(gds+46) * 0.000001)       /**< Lambert Azimuthal - Standard parallel. */
#define GDS_Lambert_Az_Cen_Lon(gds)     (int4(gds+50) * 0.000001)       /**< Lambert Azimuthal - Central longitude. */
#define GDS_Lambert_Az_nx(gds)          (uint4_missing(gds+30))         /**< Lambert Azimuthal - Number of x grid points. */
#define GDS_Lambert_Az_ny(gds)          (uint4_missing(gds+34))         /**< Lambert Azimuthal - Number of y grid points. */
#define GDS_Lambert_Az_dx(gds)          (int4(gds+55) * 0.001)         /**< Lambert Azimuthal - X-direction grid length. */
#define GDS_Lambert_Az_dy(gds)          (int4(gds+59) * 0.001)         /**< Lambert Azimuthal - Y-direction grid length. */


#define GDS_CrossSec_basic_ang(gds)	int4(gds+34)        /**< Cross-Section - Basic angle. */
#define GDS_CrossSec_sub_ang(gds)	int4(gds+38)        /**< Cross-Section - Subdivisions of basic angle. */
#define GDS_CrossSec_lat1(gds)		int4(gds+42)        /**< Cross-Section - Latitude of first grid point. */
#define GDS_CrossSec_lon1(gds)		uint4(gds+46)       /**< Cross-Section - Longitude of first grid point. */
#define GDS_CrossSec_lat2(gds)		int4(gds+51)        /**< Cross-Section - Latitude of last grid point. */
#define GDS_CrossSec_lon2(gds)		uint4(gds+55)       /**< Cross-Section - Longitude of last grid point. */

/** Scanning Mode:  +ve x scanning */
#define GDS_Scan_x(scan)		((scan & 128) == 0)
/** Scanning Mode:  +ve y scanning */
#define GDS_Scan_y(scan)		((scan & 64) == 64)
/** Scanning Mode: fortran storage order */
#define GDS_Scan_fortran(scan)		((scan & 32) == 32)
/** Scanning Mode: row reversing order */
#define GDS_Scan_row_rev(scan)		((scan & 16) == 16)
/** Scanning Mode: test for staggered grid*/
#define GDS_Scan_staggered(scan)	(((scan) & 15) != 0)
/** Scanning Mode: test for grid size != nx*ny */
#define GDS_Scan_staggered_storage(scan)	(((scan) & (1)) != 0)

/* Section 4 */
#define GB2_Sec4_size(sec)		(sec[4] ? uint4(sec[4]+0) : 0)      /**< Size of Section 4 in bytes. */
#define GB2_Sec4_num(sec)		((int) (sec[4][4]))                 /**< Number of Section 4. */
#define GB2_ProdDefTemplateNo(sec)	(UINT2(sec[4][7],sec[4][8]))    /**< Product Definition Template Number. */

#define GB2_ParmCat(sec)		(sec[4][9])     /**< Parameter Category. */

#define GB2_ParmNum(sec)		(sec[4][10])    /**< Parameter Number. */

// #define GB2_ForecastTime(sec)		(UINT4(sec[4][18],sec[4][19],sec[4][20],sec[4][21]))
// replaced by forecast_time_in_units
// #define GB2_TimeUnit2(sec)		(sec[4][48])
// #define GB2_ForecastTime2(sec)		(UINT4(sec[4][49],sec[4][50],sec[4][51],sec[4][52]))
// #define GB2_StatProcess(sec)            UINT2(sec[4][45], sec[4][46])

/* Section 5 */
#define GB2_Sec5_size(sec)		(sec[5] ? uint4(sec[5]+0) : 0)      /**< Size of Section 5 in bytes. */
#define GB2_Sec5_nval(sec)		(sec[5] ? uint4(sec[5]+5) : 0)      /**< Number of defined data points. */

/* Section 6 */
#define GB2_Sec6_size(sec)		(sec[6] ? uint4(sec[6]+0) : 0)      /**< Size of Section 6 in bytes. */

/* Section 7 */
#define GB2_Sec7_size(sec)		(sec[7] ? uint4(sec[7]+0) : 0)      /**< Size of Section 7 in bytes. */

/* Section 8 */
#define	GB2_Sec8_size			4   /**< Size of Section 8 in bytes. */

/* some center codes */
#define NCEP 7              /**< NCEP Center Code */
#define ECMWF 98            /**< ECMWF Center Code */
#define JMA1 34             /**< JMA1 Center Code */
#define JMA2 35             /**< JMA2 Center Code */
#define KMA 40              /**< KMA Center Code */
#define DWD1 78             /**< DWD1 Center Code */
#define DWD2 79             /**< DWD2 Center Code */
#define USAF 57             /**< USAF Center Code */
