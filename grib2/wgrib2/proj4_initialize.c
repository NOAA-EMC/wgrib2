#include <stdio.h>
#include <stdlib.h>

#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_PROJ4

#include "proj_api.h"
#include "proj4_wgrib2.h"

int proj4_initialize(unsigned char **sec, struct proj4_struct *projection) {

    unsigned int gdt;
    unsigned char *gds;
    int has_np, center;
    double r_maj, r_min, c_lon, c_lat, x_0, y_0, lat1, lon1;
    double latsp1, latsp2;
    char proj4_def[1000];

    gdt = code_table_3_1(sec);
    gds = sec[3];
    center = GB2_Center(sec);

    axes_earth(sec, &r_maj, &r_min);
    projection->radius_minor = r_min;
    projection->radius_minor = r_min;

    if (gdt == 0) {
	projection->proj_is_nop = 1;
	projection->x_0 = projection->y_0 = projection->lat_0 = projection->lon_0 = 0.0;
	return 0;
    }

    projection->proj_is_nop = 0;

    sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
    if ( (projection->pj_latlon = pj_init_plus(proj4_def)) == NULL) 
         fatal_error("proj4_initialize: pj_init_plus %s failed", proj4_def);

    if (gdt == 10 && (GDS_Mercator_ori_angle(gds) == 0.0) ) {            // mercator no rotation

	/* central point */
	c_lon = 0.0;
        c_lat = GDS_Mercator_latD(gds);

        sprintf(proj4_def,"+proj=merc +lat_ts=%lf +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +a=%lf +b=%lf",
            c_lat, r_maj, r_min);
        if ((projection->pj_grid = pj_init_plus(proj4_def)) == NULL) 
		fatal_error("proj4_initialize: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        projection->lat_0 = lat1 = GDS_Mercator_lat1(gds);
        projection->lon_0 = lon1 = GDS_Mercator_lon1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(projection->pj_latlon, projection->pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
		fatal_error("proj4_initialize: Proj4 transform to lat-lon","");

	projection->x_0 = x_0;
	projection->y_0 = y_0;
    }
    else if (gdt == 20) {            // polar stereographic

        /* central point */
        c_lon = GDS_Polar_lov(gds);
        c_lat = GDS_Polar_lad(gds);

        /* strange but np/sp flag is used by proj4 but not gctpc */
        has_np = ((flag_table_3_5(sec) & 128) == 0);

        sprintf(proj4_def,"+proj=stere +lat_ts=%lf +lat_0=%s +lon_0=%lf +k_0=1 +x_0=0 +y_0=0 +a=%lf +b=%lf",
                c_lat, has_np ? "90" : "-90", c_lon, r_maj,r_min);
        if ((projection->pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        projection->lon_0 = lon1 = GDS_Polar_lon1(gds);
        projection->lat_0 = lat1 = GDS_Polar_lat1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(projection->pj_latlon, projection->pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
		fatal_error("proj4_init: Proj4 transform to lat-lon","");

        projection->x_0 = x_0;
        projection->y_0 = y_0;
    }
    else if (gdt == 30) {            // lambert conformal conic

        /* latitudes of tangent/intersection */
        latsp1 = GDS_Lambert_Latin1(gds);
        latsp2 = GDS_Lambert_Latin2(gds);

        /* central point */
        c_lon = GDS_Lambert_Lov(gds);
        c_lat = GDS_Lambert_LatD(gds);

        sprintf(proj4_def,"+proj=lcc +lon_0=%lf +lat_0=%lf +lat_1=%lf +lat_2=%lf +a=%lf +b=%lf",c_lon,
                   c_lat,latsp1,latsp2,r_maj,r_min);
        if ((projection->pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        projection->lon_0 = lon1 = GDS_Lambert_Lo1(gds);
        projection->lat_0 = lat1 = GDS_Lambert_La1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(projection->pj_latlon, projection->pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
		fatal_error("proj4_init: Proj4 transform to lat-lon","");
        projection->x_0 = x_0;
        projection->y_0 = y_0;
    }

    else if (gdt == 140) {            // lambert azimuthal equal area
        /* central point */
        c_lon = GDS_Lambert_Az_Cen_Lon(gds);
        c_lat = GDS_Lambert_Az_Std_Par(gds);

        sprintf(proj4_def,"+proj=laea +lon_0=%lf +lat_0=%lf +a=%lf +b=%lf",c_lon,c_lat,r_maj,r_min);
        if ((projection->pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        projection->lon_0 = lon1 = GDS_Lambert_Lo1(gds);
        projection->lat_0 = lat1 = GDS_Lambert_La1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(projection->pj_latlon, projection->pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 )
                fatal_error("proj4_init: Proj4 transform to lat-lon","");
        projection->x_0 = x_0;
        projection->y_0 = y_0;
    }

    else if (center == NCEP && gdt == 32769) {         // ncep rotated latlon Non-E
        /* central point */
        c_lon = GDS_NCEP_B_LatLon_tlm0d(gds) * 0.000001;
        c_lat = GDS_NCEP_B_LatLon_tph0d(gds) * 0.000001;

        sprintf(proj4_def,"+proj=ob_tran +o_proj=latlon +o_lon_p=%f +o_lat_p=%f",c_lon,90.0+c_lat);
        if ((projection->pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4: pj_init_plus %s failed", proj4_def);

	/* longitude and latitude of first grid point */
        lon1 = GDS_NCEP_B_LatLon_lon1(gds) * 0.000001;
        lat1 = GDS_NCEP_B_LatLon_lat1(gds) * 0.000001;

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(projection->pj_latlon, projection->pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 )
                fatal_error("proj4_init: Proj4 transform to lat-lon","");
        projection->x_0 = x_0;
        projection->y_0 = y_0;
    }
    else {
	return 1;
    }
    return 0;
}

#endif
