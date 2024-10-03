/* wgrib2 interface to proj4.c */
/* proj4.h  10/2024    Public Domain     Wesley Ebisuzaki */


/* projection parameters  and scaling for x, y (to get in grid lengths) */

struct proj4_struct {
    double lon_0, lat_0, x_0, y_0;
    double radius_major, radius_minor;
    int proj_is_nop;
    projPJ pj_grid, pj_latlon;
};

int proj4_init(unsigned char **sec, double *grid_lon, double *grid_lat);
int proj4_initialize(unsigned char **sec, struct proj4_struct *projection);
int proj4_ll2xy(struct proj4_struct *projection, int n, double *lon, double *lat, double *x, double *y);
int Proj4_ll2xy(int n, double *lon, double *lat, double *x, double *y);

int Proj4_ll2i(int n, double *lon, double *lat, unsigned int *ipnt);
int Proj4_ij2ll(unsigned char **sec, int n, double *x, double *y, double *lon, double *lat);
int proj4_xy2ll(struct proj4_struct *projection, int n, double *x, double *y, double *lon, double *lat);
int Proj4_xy2ll(int n, double *x, double *y, double *lon, double *lat);
