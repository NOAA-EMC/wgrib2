enum projection_type {p_unknown, p_latlon, p_lambert_conic, p_polar_stereographic, 
   p_mercator, p_rotated_latlon, p_stretched_latlon, p_rotated_streched_latlon, p_lambert_azimuthal};

enum grid_arrangement_type {rectangular, thinned_rectangular_regional, thinned_rectangular_global, gaussian, thinned_gaussian, staggered_B, list};

#define N_proj_args 20

struct grid_type { 
    double x0; double y0; int x0_y0_in_m;
    int nx; int ny; int n;
    int valid_dx_dy; double dx; double dy; int dx_dy_in_m;
    int valid_xn_yn; double xn; double yn; int xn_yn_in_m;
    enum grid_arrangement_type grid_arrangement;
    int valid_xy_list; double *xlist; double *ylist; int xy_list_in_m;
};


#define GRID_ID_ARGS unsigned char **sec, double *r_major, double *r_minor, enum projection_type *proj_id, double *proj_args, int n_proj_args, struct grid_type *grid_defn



int grid_id(GRID_ID_ARGS);


