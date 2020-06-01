#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * New_grid
 *
 *  interpolation using ipolates library
 *          input = winds are N/S or grid
 *          output = winds are N/S or grid  depending on wind_rotation
 *
 * to add new grids
 *
 *    input grids: add to mk_kdgs.c
 *    output grids: add to sec3_grids
 *                  add code to mode == -1 to parse grid specifications
 *
 * to add types to vector fields definition
 *             modify source code: vectors[]
 *
 * 6/2010: Public Domain Wesley Ebisuzaki
 * 5/2018: USE_IPOLATES == 0   disable
 *         USE_IPOLATES == 1   grib1 version of ipolates
 *         USE_IPOLATES == 3   grib2 version of ipolates (double)
 *          in ip2 code, use  dlon = mod( (angle + 3600 - 1), 360) + 1
 *              this makes dlon only xxx.xx precision whic is less
 *              than grib1 (xxx.xxx) and grib2 (xxx.xxxxxx) expectations
 *              should not use float version of ip2.
 */

const char **vectors;

const char *default_vectors[] = {"UGRD", "VGRD", "VUCSH", "VVCSH","UFLX", "VFLX",
	"UGUST","VGUST","USTM","VSTM","VDFUA", "VDFVA", "MAXUW", "MAXVW",
	"UOGRD","VOGRD", "UICE", "VICE", "U-GWD", "V-GWD", "USSD", "VSSD", NULL };

/* new_grid_vectors can be called submsg_uv, should not require ipolates to be installed */

static const char *no_vectors[] = { NULL };
static const char *UV_vectors[] = { "UGRD", "VGRD", NULL };

/*
 * HEADER:111:new_grid_vectors:misc:1:change fields to vector interpolate: X=none,default,UGRD:VGRD,(U:V list)
 */

int f_new_grid_vectors(ARG1) {

    int i, n;
    const char *from;
    char *to;

    struct local_struct {
	char *buff;
	const char **uv_vectors;
    };
    struct local_struct *save;

    if (mode == -1) {
	*local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
	if (save == NULL) fatal_error("new_grid_vectors: memory allocation","");
	save->buff = NULL;
	save->uv_vectors = NULL;

	if (strcmp(arg1,"none") == 0)  {
	    save->uv_vectors = vectors = (const char **) no_vectors;
	    return 0;
	}
	if (strcmp(arg1,"default") == 0)  {
	    save->uv_vectors = vectors = (const char **) default_vectors;
	    return 0;
	}
	if (strcmp(arg1,"UGRD:VGRD") == 0)  {
	    save->uv_vectors = (const char **) UV_vectors;
	    return 0;
	}

	from = arg1;
	n = 0;
	while (*from) {
	   if (*from++ == ':') n++;
	}
	if (n % 2 == 0) fatal_error("new_grid_vectors: bad definition: %s", arg1);

	i = strlen(arg1);
	save->buff = (char *) malloc(i + 1);
	if (save->buff == NULL) fatal_error("new_grid_vectors: memory allocation","");
	save->uv_vectors = (const char **) malloc((n+2) * sizeof(char *));
	if (save->uv_vectors == NULL) fatal_error("new_grid_vectors: memory allocation","");

	from = arg1;
	to = save->buff;
	
	for (i = 0; i <= n; i++) {
	    save->uv_vectors[i] = to;
	    while (*from != '\0' && *from != ':') {
		*to++ = *from++;
	    }
	    if (*from == ':') from++;
	    *to++ = '\0';
	}
	save->uv_vectors[n+1] = NULL;
        return 0;
    }
    save = *local;
    if (mode == -2) {
	if (save->buff != NULL) {
	    free(save->buff);
	    free(save->uv_vectors);
	}
	free(save);
	return 0;
    }
    vectors = save->uv_vectors;
    return 0;
}

#if USE_IPOLATES == 1

/* This is the grib1 version which is now frozen.
 * The grib2 version is similar but
 *  (1) uses the grib2 version of ipolates
 *  (2) double precision
 *  (3) spectral support was removed
 */

#ifdef G95
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
void g95_runtime_start(int ,char **);
void g95_runtime_stop(void);
static int g95_runstop = 0;
#endif

#ifdef GFORTRAN
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef OPENF95
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef IFORT
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef XLF
#define IPOLATES ipolates
#define IPOLATEV ipolatev
#endif

#ifdef CRAYCE
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef SOLARIS
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

void IPOLATES(int *interpol, int *ipopt, int *kgds, int *kgds_out, int *npnts, int *n_out0, 
		int *km, int *ibi, unsigned char *bitmap, float *data_in, int *n_out, 
		float *rlat, float *rlon, int *ibo, unsigned char *bitmap_out, 
		float *data_out, int *iret);

void IPOLATEV(int *interpol, int *ipopt, int *kgds, int *kgds_out, int *npnts, int *n_out0, 
		int *km, int *ibi, unsigned char *bitmap, float *u_in, float *v_in, 
		int *n_out, float *rlat, float *rlon, float *crot, float *srot, int *ibo,
		unsigned char *bitmap_out, float *u_out, float *v_out, int *iret);


extern unsigned int npnts,nx,ny;
extern double *lat, *lon;
extern int decode, latlon, file_append, flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;
extern enum output_order_type output_order;
extern int save_translation;
extern enum output_order_type output_order_wanted, output_order;

static int interpol_type = 0;
static int ipopt[20] = {-1,-1,0, 0,0,0, 0,0,0, 0};

/*
 * HEADER:111:new_grid_interpolation:misc:1:new_grid interpolation X=bilinear,bicubic,neighbor,budget
 */

int f_new_grid_interpolation(ARG1) {

   if (strcmp(arg1,"bilinear") == 0) { interpol_type = 0; ipopt[0] = -1; }
   else if (strcmp(arg1,"bicubic") == 0) { interpol_type = 1; ipopt[0] = 0; }
   else if (strcmp(arg1,"neighbor") == 0) { interpol_type = 2; ipopt[0] = 1; }
   else if (strcmp(arg1,"budget") == 0) { interpol_type = 3; ipopt[0] = -1; }
//  turned off spectral -- new library rarely used interpolation option
//   else if (strcmp(arg1,"spectral") == 0) { interpol_type = 4; ipopt[0] = 0; ipopt[1] = -1; }
//  turned off neighbor-budget - save space for rarely used interpolation option
//   else if (strcmp(arg1,"neighbor-budget") == 0) { interpol_type = 6; ipopt[0] = -1; }
   else fatal_error("new_grid_interpolation: unknown type %s", arg1);

   return 0;
}

/*
 * HEADER:111:new_grid_ipopt:misc:1:new_grid ipopt values X=i1:i2..:iN N <= 20
 */
int f_new_grid_ipopt(ARG1) {
    int i, k, val, m;

    i = 0;
    k = sscanf(arg1, "%d%n", &val, &m);
    while (k == 1) {
        if (i > 19) fatal_error("new_grid_ipopt: too many ipopt values, 20 max","");
        ipopt[i++] = val;
        arg1 += m;
        k = sscanf(arg1, ":%d%n", &val, &m);
    }
    return 0;
}


/*
 * HEADER:111:new_grid_winds:misc:1:new_grid wind orientation: X = grid, earth (no default)
 */

static enum {grid, earth, undefined} wind_rotation  = undefined;

int f_new_grid_winds(ARG1) {
    int *save;
    if (mode == -2) {
	free(*local);
	return 0;
    }
    if (mode == -1) {
	if ((*local = save = (int *) malloc(sizeof(int))) == NULL) fatal_error("new_grid_winds: malloc","");
	if (strcmp(arg1,"grid") == 0) *save = 0;
	else if (strcmp(arg1,"earth") == 0) *save = 1;
        else fatal_error("new_grid_winds: bad arg %s", arg1);
    }
    save = (int *) *local;
    wind_rotation = (*save) ? earth : grid;
    return 0;
}

struct local_struct {
	// U data
        float *u_val;
        int has_u, nx, ny;
        unsigned char *clone_sec[9];
        char name[NAMELEN];

	// interpolation
        int npnts_out;		// must be integer .. fortran call requires int
        float *rlat, *rlon, *crot, *srot;
        unsigned char *sec3;
	int kgds_out[200];
	double radius_major, radius_minor;

	// output file
        struct seq_file out;
};

unsigned char blank_sec1[21] = { 0,0,0,21,1,
		255,255,255,255,	// center subcenter
		2,1,255,		// grib master table, local table, sig ref time
		255, 255, 		// year
		255, 255, 255, 255, 255, // month .. second
		255, 255};

/*
 * HEADER:111:new_grid:output:4:bilinear interpolate: X=projection Y=x0:nx:dx Z=y0:ny:dy A=grib_file alpha
 */

int f_new_grid(ARG4) {
    struct local_struct *save;

    unsigned int i;
    int is_u, is_v, ftn_npnts, ftn_nout;
    int kgds[200], km;
    float *data_in, *data_out;
    double x0, y0, dx, dy, xn, yn;
    double lov, lad, latin1, latin2;
    int proj;					// projection: for LC 0 = NP, 128 = SP
    char name[NAMELEN];
    int j, ibi, ibo, iret, nnx, nny, n_out;
    unsigned char *new_sec[8], *s, *bitmap, *bitmap_out, *p;

    /* for lambertc */
    double r_maj, r_min, ref_lon, ref_lat;

    if (mode == -1) {			// initialization
        decode = 1;
        output_order_wanted = raw;	// in raw order


#ifdef G95
	// initialize g95 runtime library
	if (g95_runstop == 0) { g95_runtime_start(0,NULL); g95_runstop = 1; }
#endif

//        if ( (sizeof(vectors) / sizeof (vectors[0])) % 2 == 1) fatal_error("new_grid: program error in vectors[]","");

	// allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("new_grid: memory allocation error","");

        if (fopen_file(&(save->out), arg4, file_append ? "ab" : "wb") != 0) {
	    fatal_error("-new_grid: could not open file %s", arg4);
	}

	save->has_u = 0;
	save->radius_major = save->radius_minor = 0.0;
	init_sec(save->clone_sec);
	s = NULL;

	// parse NCEP grids */
	ncep_grids(&arg1, &arg2, &arg3);

	// for each output grid
        if (strcmp(arg1,"latlon") == 0) {
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

	    if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;
            if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");

            // make a new section 3
            s = sec3_lola(nnx, x0, dx, nny, y0, dy, sec);
	}
        else if (strcmp(arg1,"rot-latlon") == 0) {
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;
            if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");

            // make a new section 3
            s = sec3_lola(nnx, x0, dx, nny, y0, dy, sec);
        }


        else if (strncmp(arg1,"mercator:",9) == 0) {
            if (sscanf(arg1,"mercator:%lf",  &lad) != 1) 
                fatal_error("new_grid: LaD (latitude interesection) not specified","");
            if (sscanf(arg2,"%lf:%d:%lf:%lf", &x0, &nnx, &dx, &xn) != 4)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf:%lf", &y0, &nny, &dy, &yn) != 4)

	    if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;
            if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");

            // make a new section 3
            s = sec3_mercator(lad, nnx, x0, dx, xn, nny, y0, dy, yn, sec);
	}
        else if (strcmp(arg1,"gaussian") == 0) {
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d", &y0, &nny) != 2)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

	    if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;
            if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");
            // make a new section 3
            s = sec3_gaussian(nnx, x0, dx, nny, y0, sec);
	}
        else if (strncmp(arg1,"lambert:",8) == 0) {
            i = sscanf(arg1,"lambert:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
            if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            if (lov < 0.0)  lov += 360.0;
            if (i < 3) latin2 = latin1;
            if (i < 4) lad = latin2;
            proj = 0;
            if (latin2 < 0.0) proj = 128;

            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

	    if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;
            if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");

            // make a new section 3
            s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);
        }

	/* for lambertc, input is the lon-lat of center point */
	/* can not calc grid until radius is given, so do lambert code to check args */

        else if (strncmp(arg1,"lambertc:",9) == 0) {
            i = sscanf(arg1,"lambertc:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
            if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            if (lov < 0.0)  lov += 360.0;
            if (i < 3) latin2 = latin1;
            if (i < 4) lad = latin2;
            proj = 0;
            if (latin2 < 0.0) proj = 128;

            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;
            if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");

            // make a new section 3
            s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);
        }

        else if (strncmp(arg1,"nps:",4) == 0 || strncmp(arg1,"sps:",4) == 0)  {
            if (sscanf(arg1,"%*[ns]ps:%lf:%lf", &lov, &lad) != 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
	    if (lad != 60.0) fatal_error("New_grid: only LatD = 60 is supported","");
            proj = 0;
	    if (arg1[0] == 's') proj = 128;
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);
	    if (lov < 0.0)  lov += 360.0;

	    if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;
            if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");

            // make a new section 3
	    s = sec3_polar_stereo(lov, lad, proj, nnx, x0, dx, nny, y0, dy, sec);
	}
        else fatal_error("new_grid: unsupported output grid %s", arg1);

	new_sec[1] = blank_sec1;	// add center info
	// save new section 3
        i = (int) uint4(s);         // size of section 3
        new_sec[3] = save->sec3 = (unsigned char *) malloc(i * sizeof(unsigned char));
        for (j = 0; j < i; j++) save->sec3[j] = s[j];

	// apply wind rotation .. change flag 3.3
	if (wind_rotation == undefined) { fprintf(stderr,"Warning: -new_grid wind orientation undefined, "
		"use \"-new_grid_winds (grid|earth)\", earth used (N=North Pole)\n");
	}
	if ((p = flag_table_3_3_location(new_sec)) != NULL) {
	    if (wind_rotation == grid) *p = *p | 8;
	    else *p = *p & (255 - 8);
	}

        if (mk_kgds(new_sec, save->kgds_out)) fatal_error("new_grid: encoding output kgds","");

	/* some vectors need by interpolation routines */
        if ((save->rlat = (float *) malloc( sizeof(float) * (size_t) n_out)) == NULL)
                fatal_error("new_grid memory allocation","");
        if ((save->rlon = (float *) malloc(sizeof(float) * (size_t) n_out)) == NULL)
                fatal_error("new_grid memory allocation","");
        if ((save->crot = (float *) malloc(sizeof(float) * (size_t) n_out)) == NULL)
                fatal_error("new_grid memory allocation","");
        if ((save->srot = (float *) malloc(sizeof(float) * (size_t) n_out)) == NULL)
                fatal_error("new_grid memory allocation","");

	return 0;
    }

    save = (struct local_struct *) *local;

    if (mode == -2) {			// cleanup
#ifdef G95
	if (g95_runstop == 1) { g95_runtime_stop(); g95_runstop = 0; }
#endif
	if (save->has_u > 0) {
	    fprintf(stderr,"-new_grid: last field %s was not interpolated (missing V)\n", save->name);
	    free(save->u_val);
	    free_sec(save->clone_sec);
	}
	free(save->rlon);
	free(save->rlat);
	free(save->crot);
	free(save->srot);
	free(save->sec3);
	fclose_file(&(save->out));
	free(save);

	return 0;
    }

    if (mode >= 0) {			// processing

	/* The kgds of some output grids will change depending on input grid */
	/* for example, radius of earth is not known grib file is read, */
	/*   and mass vs wind fields */
	/* right nowm, only affects lambertc */

        if (strncmp(arg1,"lambertc:",8) == 0) {

	    // lambertc depends on the radius of the earth which is
            // set by the input grib file

	    /* read earth radius */
	    i = axes_earth(sec, &r_maj, &r_min);
	    if (i) fatal_error_i("axes_earth: error code %d", i);

	    if (save->radius_major != r_maj || save->radius_minor != r_min) {

		// update sec3 and kgds

                i = sscanf(arg1,"lambertc:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
                if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
                if (lov < 0.0)  lov += 360.0;
                if (i < 3) latin2 = latin1;
                if (i < 4) lad = latin2;
                proj = 0;
                if (latin2 < 0.0) proj = 128;

                if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                    fatal_error("new_grid: XDEF wrong:%s",arg2);
                if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                    fatal_error("new_grid: YDEF wrong:%s",arg3);

                if (x0 < 0.0) x0 += 360.0;
                save->nx = nnx;
                save->ny = nny;
                save->npnts_out = n_out = nnx*nny;
                if (n_out <= 0) fatal_error("new_grid: bad nx, ny","");

	        ref_lon = x0;
	        ref_lat = y0;
 
	        i = new_grid_lambertc(nnx, nny, ref_lon, ref_lat, latin1, latin2, lov, lad, r_maj, r_min, dx, dy, &x0, &y0);
	        if (i) fatal_error_i("new_grid_lambertc: error code %d", i);

                // make a new section 3
                s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);

	        // save new section 3
                i = (int) uint4(s);         // size of section 3
                for (j = 0; j < i; j++) save->sec3[j] = s[j];

	        // make kgds
		new_sec[1] = blank_sec1;	// add center info
	        new_sec[3] = save->sec3;
                if (mk_kgds(new_sec, save->kgds_out)) fatal_error("new_grid: encoding output kgds","");

	        // save radius of earth, to show sec3 and kgds has been done
	        save->radius_major = r_maj;
	        save->radius_minor = r_min;
	    }
        }

	if (output_order != raw) fatal_error("new_grid: must be in raw output order","");
        i = getName(sec, mode, NULL, name, NULL, NULL);
	is_u = is_v = 0;
//	for (j = 0 ; j < sizeof(vectors) / sizeof(vectors[0]); j++) {
	for (j = 0; vectors[j] != NULL; j++) {
	    if (strcmp(name,vectors[j]) == 0) {
		if (j % 2 == 0) is_u = 1;
		else is_v = 1;
		break;
	    }
	}

// fprintf(stderr, " %s isu %d isv %d has_u %d\n", name, is_u, is_v, save->has_u);
//  for (i = 0; i < 12; i++) { printf("kgds_out[%d] = %d ",i,save->kgds_out[i]); }

	// check if V matches expectation

	if (is_v && (save->has_u == 0  || (same_sec0(sec,save->clone_sec) != 1 ||
            same_sec1(sec,save->clone_sec) != 1 ||
            same_sec3(sec,save->clone_sec) != 1 ||
            same_sec4(sec,save->clone_sec) != 1) )) {
	    fprintf(stderr,"-new_grid: %s doesn't pair with previous vector field, field ignored\n", name);
	    return 0;
	}

	// if U field - save

        if (is_u) {
            if (save->has_u > 0) {
                fprintf(stderr,"-new_grid: missing V, %s not interpolated\n",save->name);
                free(save->u_val);
                free_sec(save->clone_sec);
            }
            copy_sec(sec, save->clone_sec);
            copy_data(data,ndata,&(save->u_val));
            GB2_ParmNum(save->clone_sec) = GB2_ParmNum(sec) + 1;
            save->has_u = 1;
	    strncpy(save->name, name,NAMELEN-1);
	    save->name[NAMELEN-2]=0;
            return 0;
        }

	// at this point will call polates with either a scalar or vector

	n_out = save->npnts_out;
	nnx = save->nx;
	nny = save->ny;
	km = 1;			// only one field

	if (mk_kgds(sec, kgds)) fatal_error("new_grid: encoding input kgds","");

	data_in = (float *) malloc((1 + (is_v != 0)) * sizeof(float) * (size_t) npnts);
        bitmap = (unsigned char *) malloc(sizeof(unsigned char) * (size_t) npnts);
        bitmap_out = (unsigned char *) malloc(sizeof(unsigned char) * (size_t) n_out);
	data_out = (float *) malloc((1 + (is_v != 0)) * sizeof(float) * (size_t) n_out);

	if (data_in == NULL || data_out == NULL || bitmap == NULL || bitmap_out == NULL) 
	    fatal_error("new_grid: memory allocation problem","");

	if (is_v) {
#pragma omp parallel for private(i)
	    for (i = 0; i < npnts; i++) {
                if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->u_val[i])) {
                    data_in[i] = save->u_val[i];
                    data_in[i+npnts] = data[i];
                    bitmap[i] = 1;
		}
		else {
                    data_in[i] = data_in[i + npnts] = 0.0;
                    bitmap[i] = 0;
		}
	    }
	    if (mode == 98) fprintf(stderr," UV interpolation %s , %s\n", save->name, name);
	}
	else {
#pragma omp parallel for private(i)
	    for (i = 0; i < npnts; i++) {
                if (DEFINED_VAL(data[i])) {
                    data_in[i] = data[i];
                    bitmap[i] = 1;
		}
		else {
                    data_in[i] = 0.0;
                    bitmap[i] = 0;
		}
	    }
	}
	// check if bitmap is used
	ibi = 0;                        // input bitmap is not used
	for (i = 0; i < npnts; i++) {
	    if (bitmap[i] == 0) {
		ibi = 1;
		break;
	    }
	}
	// interpolate

// for (i = 0; i < 12; i++) { printf("\nkgds_in[%d] = %d  out=%d ",i,kgds[i],save->kgds_out[i]); }
	ftn_npnts = (int) npnts;
	ftn_nout = (int) n_out;
	if (is_v) {
	    IPOLATEV(&interpol_type, ipopt,kgds,save->kgds_out, 
		&ftn_npnts, &n_out, &km, &ibi, bitmap, data_in, data_in+npnts, 
		&ftn_nout,save->rlat,save->rlon, save->crot, save->srot,
                &ibo, bitmap_out, data_out, data_out + n_out, &iret);
	}
	else {
	    IPOLATES(&interpol_type, ipopt,kgds,save->kgds_out, 
		&ftn_npnts, &n_out, &km, &ibi, bitmap, data_in, &ftn_nout,
		save->rlat,save->rlon, &ibo, bitmap_out, data_out, &iret);
	}
	if (iret != 0) {
	    for (i = 0; i < 12; i++) {
		fprintf(stderr," IPOLATES error: kgds[%d] input %d output %d\n", i+1,kgds[i],save->kgds_out[i]);
	    }
	    if (iret == 2) fatal_error("IPOLATES failed, unrecognized input grid or no grid point of output grid is in input grid","");
	    if (iret == 3) fatal_error("IPOLATES failed, unrecognized output grid","");
	    fatal_error_i("IPOLATES failed, error %d",iret);
	   
	}
	n_out = (unsigned int) ftn_nout;

        /* use bitmap to set UNDEFINED values */
        if (ibo == 1) {         // has a bitmap
	    if (is_v) {
                for (i = 0; i < n_out; i++) {
		    if (bitmap_out[i] == 0) data_out[i] = data_out[i+n_out] = UNDEFINED;
		}
	    }
	    else {
                for (i = 0; i < n_out; i++) {
		    if (bitmap_out[i] == 0) data_out[i] = UNDEFINED;
		}
            }
	} 

	// now to write out the grib file

	for (i = 0; i < 8; i++) new_sec[i] = sec[i];
	new_sec[3] = save->sec3;

	if (is_v != 0) {
            GB2_ParmNum(new_sec) = GB2_ParmNum(new_sec) - 1;
            grib_wrt(new_sec, data_out, n_out, nnx, nny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));
            GB2_ParmNum(new_sec) = GB2_ParmNum(new_sec) + 1;
            grib_wrt(new_sec, data_out+n_out, n_out, nnx, nny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));
	}
	else {
            grib_wrt(new_sec, data_out, n_out, nnx, nny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));
	}
        if (flush_mode) fflush_file(&(save->out));
        free(data_in);
        free(bitmap);
        free(bitmap_out);
	free(data_out);
	if (is_v != 0) {
	    save->has_u = 0;
            free(save->u_val);
            free_sec(save->clone_sec);
	}
    }
    return 0;
}
#endif

#if USE_IPOLATES == 3

/*
 * this is the double precision float and 4-byte integer version of grib2 version of ipolates
 * this version will not handle 2G+ grids (4 byte integers)
 * some optimizations relative to grib1 version and the single precision grib2 versions
 * of the wrappers
 *
 * spectral restored (removed from grib1 version because I didn't want to support it)
 */

#ifdef G95
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
void g95_runtime_start(int ,char **);
void g95_runtime_stop(void);
static int g95_runstop = 0;
#endif

#ifdef GFORTRAN
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef OPENF95
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef IFORT
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef XLF
#define IPOLATES ipolates
#define IPOLATEV ipolatev
#endif

#ifdef CRAYCE
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

#ifdef SOLARIS
#define IPOLATES ipolates_
#define IPOLATEV ipolatev_
#endif

void IPOLATES(int *interpol, int *ipopt, int *gdt_in, int *gdttmpl_in, int *gdttmpl_size_in,
  int *gdt_out, int *gdttmpl_out, int *gdttmpl_size_out, int *mi, int *mo, int *km,
  int *ibi, unsigned char *bitmap, double *data_in, int *n_out, double *rlat, double *rlon,
   int *ibo, unsigned char *bitmap_out, double *data_out, int *iret);

void IPOLATEV(int *interpol, int *ipopt, int *gdt_in, int *gdttmpl_in, int *gdttmpl_size_in,
  int *gdt_out, int *gdttmpl_out, int *gdttmpl_size_out, int *mi, int *mo, int *km,
  int *ibi, unsigned char *bitmap, double *u_in, double *v_in, int *n_out, double *rlat, double *rlon,
   double *crot, double *srot, int *ibo, unsigned char *bitmap_out,
   double *u_out, double *v_out, int *iret);


extern int decode, latlon, file_append, flush_mode;
extern int use_scale, dec_scale, bin_scale, wanted_bits, max_bits;
extern enum output_grib_type grib_type;
extern enum output_order_type output_order;
extern int save_translation;
extern enum output_order_type output_order_wanted, output_order;
static enum {grid, earth, undefined} wind_rotation  = undefined;

int f_new_grid_winds(ARG1) {
    int *save;
    if (mode == -2) {
        free(*local);
        return 0;
    }
    if (mode == -1) {
        if ((*local = save = (int *) malloc(sizeof(int))) == NULL) fatal_error("new_grid_winds: malloc","");
        if (strcmp(arg1,"grid") == 0) *save = 0;
        else if (strcmp(arg1,"earth") == 0) *save = 1;
        else fatal_error("new_grid_winds: bad arg %s", arg1);
    }
    save = (int *) *local;
    wind_rotation = (*save) ? earth : grid;
    return 0;
}

static int interpol_type = 0;
static int ipopt[20] = {-1,-1,0, 0,0,0, 0,0,0, 0};

int f_new_grid_interpolation(ARG1) {
   
#ifdef USE_SPECTRAL
   char type;
   int max_wave;
#endif

   if (mode >= -1) {
       if (strcmp(arg1,"bilinear") == 0) { interpol_type = 0; ipopt[0] = -1; }
       else if (strcmp(arg1,"bicubic") == 0) { interpol_type = 1; ipopt[0] = 0; }
       else if (strcmp(arg1,"neighbor") == 0) { interpol_type = 2; ipopt[0] = 1; }
       else if (strcmp(arg1,"budget") == 0) { interpol_type = 3; ipopt[0] = -1; }
#ifdef USE_SPECTRAL
       else if (sscanf(arg1,"spectral-%c%d", &type, &max_wave) == 2) {
            if (type == 't' || type == 'T') ipopt[0] = 0;
            else if (type == 'r' || type == 'R') ipopt[0] = 1;
	    else fatal_error("new_grid_interpolation: spectral-(T|R)NUM not %s", arg1);
	    if (max_wave <= 1) fatal_error("new_grid_interpolation: spectral-(T|R)NUM, NUM > 1", "");
	    ipopt[1] = max_wave;
	    interpol_type = 4;
       }
#endif
//     turned off neighbor-budget - poorly explained in documentation
//     else if (strcmp(arg1,"neighbor-budget") == 0) { interpol_type = 6; ipopt[0] = -1; }
      else fatal_error("new_grid_interpolation: unknown type %s", arg1);
   }
   return 0;
}

int f_new_grid_ipopt(ARG1) {
    int i, k, val, m;

    if (mode >= -1) {
        i = 0;
        k = sscanf(arg1, "%d%n", &val, &m);
        while (k == 1) {
            if (i > 19) fatal_error("new_grid_ipopt: too many ipopt values, 20 max","");
            ipopt[i++] = val;
            arg1 += m;
            k = sscanf(arg1, ":%d%n", &val, &m);
        }
    }
    return 0;
}

/* ipolates is using a fortran library with integers up to 2GB
   need to check that the grid size is < 2GB
 */

static void check_grid_size(int nx, int ny);

static void check_grid_size(int nx, int ny) {
   int err;

   err=0;
   if (nx <= 0) err = 1;
   if (ny <= 0) err = 1;
   if ((double) nx * (double) ny > INT_MAX) err = 1;
   if (err == 1) fatal_error_i("new_grid: grid size exceeds %d (INT_MAX)", INT_MAX);
}

struct local_struct {
        // U data
        float *u_val;
        int has_u, nx, ny;
        unsigned char *clone_sec[9];
        char name[NAMELEN];

        // interpolation
        int npnts_out;          // must be integer .. fortran call requires int
        double *rlat, *rlon, *crot, *srot, *data_out;
	float *data_wrt;
        unsigned char *bitmap_out;
        unsigned char *sec3;
        double radius_major, radius_minor;
        int gdtnum_out, gdt_out[200], gdt_out_size;

        // output file
        struct seq_file out;
};

unsigned char blank_sec1[21] = { 0,0,0,21,1,
                255,255,255,255,        // center subcenter
                2,1,255,                // grib master table, local table, sig ref time
                255, 255,               // year
                255, 255, 255, 255, 255, // month .. second
                255, 255};

int f_new_grid(ARG4) {
    struct local_struct *save;

    unsigned int i;
    int is_u, is_v, ftn_npnts, ftn_nout;
    int km;
    double *data_in;
    int gdtnum_in, gdt_in[200], gdt_in_size;
    double x0, y0, dx, dy, xn, yn;
    double lov, lad, latin1, latin2;
    int proj;                                   // projection: for LC 0 = NP, 128 = SP
    double sp_lat, sp_lon, sp_rot;
    char name[NAMELEN];
    int j, ibi, ibo, iret, nnx, nny, n_out, tmp_interpol_type;
    unsigned char *new_sec[8], *s, *bitmap, *p;

    /* for lambertc */
    double r_maj, r_min, ref_lon, ref_lat;

    if (mode == -1) {                   // initialization
        decode = 1;
        output_order_wanted = raw;      // in raw order

#ifdef G95
        // initialize g95 runtime library
        if (g95_runstop == 0) { g95_runtime_start(0,NULL); g95_runstop = 1; }
#endif

        // allocate static variables

        *local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
        if (save == NULL) fatal_error("new_grid: memory allocation error","");

        if (fopen_file(&(save->out), arg4, file_append ? "ab" : "wb") != 0) {
            fatal_error("-new_grid: could not open file %s", arg4);
        }

        save->has_u = 0;
        save->radius_major = save->radius_minor = 0.0;
        init_sec(save->clone_sec);
        s = NULL;

        // parse NCEP grids -- if ncep_grid, replace arg1, arg2, arg3
        ncep_grids(&arg1, &arg2, &arg3);

        // for each output grid
        if (strcmp(arg1,"latlon") == 0) {
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_lola(nnx, x0, dx, nny, y0, dy, sec);
        }
	else if (strncmp(arg1,"rot-ll:",7) == 0) {
            if (sscanf(arg1,"rot-ll:%lf:%lf:%lf",  &sp_lon, &sp_lat, &sp_rot) != 3)
                fatal_error("new_grid: bad %s",arg1);
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);
            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;
	    s = sec3_rot_ll(nnx, x0, dx, nny, y0, dy, sp_lon, sp_lat, sp_rot, sec);
	}
        else if (strncmp(arg1,"mercator:",9) == 0) {
            if (sscanf(arg1,"mercator:%lf",  &lad) != 1)
                fatal_error("new_grid: LaD (latitude interesection) not specified","");
            if (sscanf(arg2,"%lf:%d:%lf:%lf", &x0, &nnx, &dx, &xn) != 4)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf:%lf", &y0, &nny, &dy, &yn) != 4)

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_mercator(lad, nnx, x0, dx, xn, nny, y0, dy, yn, sec);
        }
        else if (strcmp(arg1,"gaussian") == 0) {
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d", &y0, &nny) != 2)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_gaussian(nnx, x0, dx, nny, y0, sec);
        }
        else if (strncmp(arg1,"lambert:",8) == 0) {
            i = sscanf(arg1,"lambert:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
            if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            if (lov < 0.0)  lov += 360.0;
            if (i < 3) latin2 = latin1;
            if (i < 4) lad = latin2;
            proj = 0;
            if (latin2 < 0.0) proj = 128;

            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            check_grid_size(nnx, nny);
            save->nx = nnx;
            save->ny = nny;
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);
        }

        /* for lambertc, input is the lon-lat of center point */
        /* can not calc grid until radius is given, so do lambert code to check args */

        else if (strncmp(arg1,"lambertc:",9) == 0) {
            i = sscanf(arg1,"lambertc:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
            if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            if (lov < 0.0)  lov += 360.0;
            if (i < 3) latin2 = latin1;
            if (i < 4) lad = latin2;
            proj = 0;
            if (latin2 < 0.0) proj = 128;

            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);
        }

        else if (strncmp(arg1,"nps:",4) == 0 || strncmp(arg1,"sps:",4) == 0)  {
            if (sscanf(arg1,"%*[ns]ps:%lf:%lf", &lov, &lad) != 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
            // if (lad != 60.0) fatal_error("New_grid: only LatD = 60 is supported","");
            proj = 0;
            if (arg1[0] == 's') proj = 128;
            if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                fatal_error("new_grid: XDEF wrong:%s",arg2);
            if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                fatal_error("new_grid: YDEF wrong:%s",arg3);
            if (lov < 0.0)  lov += 360.0;

            if (x0 < 0.0) x0 += 360.0;
            save->nx = nnx;
            save->ny = nny;
            check_grid_size(nnx, nny);
            save->npnts_out = n_out = nnx*nny;

            // make a new section 3
            s = sec3_polar_stereo(lov, lad, proj, nnx, x0, dx, nny, y0, dy, sec);
        }
        else fatal_error("new_grid: unsupported output grid %s", arg1);

        new_sec[1] = blank_sec1;        // add center info
        // save new section 3
        i = (int) uint4(s);         // size of section 3
        new_sec[3] = save->sec3 = (unsigned char *) malloc(i * sizeof(unsigned char));
        for (j = 0; j < i; j++) save->sec3[j] = s[j];

        // apply wind rotation .. change flag 3.3
        if (wind_rotation == undefined) { fprintf(stderr,"Warning: -new_grid wind orientation undefined, "
                "use \"-new_grid_winds (grid|earth)\", earth used (N=North Pole)\n");
        }
	if ((p = flag_table_3_3_location(new_sec)) != NULL) {
	    if (wind_rotation == grid) *p = *p | 8;
	    else *p = *p & (255 - 8);
	}

        save->gdt_out_size = sizeof(save->gdt_out) / sizeof(save->gdt_out[0]);
        if (mk_gdt(new_sec, &(save->gdtnum_out), &(save->gdt_out[0]), &(save->gdt_out_size) ))
                fatal_error("new_grid: encoding output gdt","");

        /* some vectors need by interpolation routines */
        save->rlat = (double *) malloc(sizeof(double) * (size_t) n_out);
        save->rlon = (double *) malloc(sizeof(double) * (size_t) n_out);
        save->crot = (double *) malloc(sizeof(double) * (size_t) n_out);
        save->srot = (double *) malloc(sizeof(double) * (size_t) n_out);
        save->data_out = (double *) malloc(2 * sizeof(double) * (size_t) n_out);
        save->data_wrt = (float *) malloc(sizeof(double) * (size_t) n_out);
        save->bitmap_out = (unsigned char *) malloc(sizeof(char) * (size_t) n_out);
	if (save->rlat == NULL || save->rlon == NULL || save->crot == NULL || save->srot == NULL || 
		save->data_out == NULL || save->data_wrt == NULL || save->bitmap_out == NULL) 
                fatal_error("new_grid memory allocation","");

        return 0;
    }

    save = (struct local_struct *) *local;

    if (mode == -2) {                   // cleanup
#ifdef G95
        if (g95_runstop == 1) { g95_runtime_stop(); g95_runstop = 0; }
#endif
        if (save->has_u > 0) {
            fprintf(stderr,"-new_grid: last field %s was not interpolated (missing V)\n", save->name);
            free(save->u_val);
            free_sec(save->clone_sec);
        }
        free(save->rlon);
        free(save->rlat);
        free(save->crot);
        free(save->srot);
	free(save->data_out);
	free(save->data_wrt);
        free(save->bitmap_out);
        free(save->sec3);
        fclose_file(&(save->out));
        free(save);

        return 0;
    }
    if (mode >= 0) { /* processing grid */

        /* The kgds of some output grids will change depending on input grid */
        /* for example, radius of earth is not known until grib file is read, */
        /*   and mass vs wind fields */
        /* right nowm, only affects lambertc */

        if (strncmp(arg1,"lambertc:",8) == 0) {

            // lambertc depends on the radius of the earth which is
            // set by the input grib file

            /* read earth radius */
            i = axes_earth(sec, &r_maj, &r_min);
            if (i) fatal_error_i("axes_earth: error code %d", i);

            if (save->radius_major != r_maj || save->radius_minor != r_min) {

                // update sec3 and kgds

                i = sscanf(arg1,"lambertc:%lf:%lf:%lf:%lf", &lov, &latin1, &latin2, &lad);
                if (i < 2) fatal_error("new_grid: arg1 wrong:%s",arg1);
                if (lov < 0.0)  lov += 360.0;
                if (i < 3) latin2 = latin1;
                if (i < 4) lad = latin2;
                proj = 0;
                if (latin2 < 0.0) proj = 128;

                if (sscanf(arg2,"%lf:%d:%lf", &x0, &nnx, &dx) != 3)
                    fatal_error("new_grid: XDEF wrong:%s",arg2);
                if (sscanf(arg3,"%lf:%d:%lf", &y0, &nny, &dy) != 3)
                    fatal_error("new_grid: YDEF wrong:%s",arg3);

                if (x0 < 0.0) x0 += 360.0;
                save->nx = nnx;
                save->ny = nny;
                check_grid_size(nnx, nny);
                save->npnts_out = n_out = nnx*nny;

                ref_lon = x0;
                ref_lat = y0;

                i = new_grid_lambertc(nnx, nny, ref_lon, ref_lat, latin1, latin2, lov, lad, r_maj, r_min, dx, dy, &x0, &y0);
                if (i) fatal_error_i("new_grid_lambertc: error code %d", i);

                // make a new section 3
                s = sec3_lc(lov, lad, latin1, latin2, proj, nnx, x0, dx, nny, y0, dy, sec);

                // save new section 3
                i = (int) uint4(s);         // size of section 3
                for (j = 0; j < i; j++) save->sec3[j] = s[j];

                // make gdt
                new_sec[1] = blank_sec1;        // add center info
                new_sec[3] = save->sec3;

                if (mk_gdt(new_sec, &(save->gdtnum_out), &(save->gdt_out[0]), &(save->gdt_out_size) ))
                  fatal_error("new_grid: encoding output gdt","");

                // save radius of earth, to show sec3 and kgds has been done
                save->radius_major = r_maj;
                save->radius_minor = r_min;
            }
        }

        if (output_order != raw) fatal_error("new_grid: must be in raw output order","");
        i = getName(sec, mode, NULL, name, NULL, NULL);

        is_u = is_v = 0;
        for (j = 0; vectors[j] != NULL; j++) {
            if (strcmp(name,vectors[j]) == 0) {
                if (j % 2 == 0) is_u = 1;
                else is_v = 1;
                break;
            }
        }

        // check if V matches expectation

        if (is_v && (save->has_u == 0  || (same_sec0(sec,save->clone_sec) != 1 ||
            same_sec1(sec,save->clone_sec) != 1 ||
            same_sec3(sec,save->clone_sec) != 1 ||
            same_sec4(sec,save->clone_sec) != 1) )) {
            fprintf(stderr,"-new_grid: %s doesn't pair with previous vector field, field ignored\n", name);
            return 0;
        }

        // if U field - to sec 

        if (is_u) {
            if (save->has_u > 0) {
                fprintf(stderr,"-new_grid: missing V, %s not interpolated\n",save->name);
                free(save->u_val);
                free_sec(save->clone_sec);
            }
            copy_sec(sec, save->clone_sec);
            copy_data(data,ndata,&(save->u_val));
            GB2_ParmNum(save->clone_sec) = GB2_ParmNum(sec) + 1;
            save->has_u = 1;
            strncpy(save->name, name,NAMELEN-1);
            save->name[NAMELEN-2]=0;
            return 0;
        }

        // at this point will call polates with either a scalar or vector

        n_out = save->npnts_out;
        nnx = save->nx;
        nny = save->ny;
        km = 1;                 // only one scalar or vector field

        gdt_in_size = sizeof(gdt_in) / sizeof(gdt_in[0]);
        if (mk_gdt(sec, &gdtnum_in, &(gdt_in[0]), &gdt_in_size ))
                fatal_error("new_grid: encoding input gdt","");

        data_in = (double *) malloc((1 + (is_v != 0)) * sizeof(double) * (size_t) ndata);
        bitmap = (unsigned char *) malloc(sizeof(unsigned char) * (size_t) ndata);

        if (data_in == NULL || bitmap == NULL)
            fatal_error("new_grid: memory allocation problem","");

	/* data format for ipolates, calling double precision ip2lib
           double precision data_in[0..npts-1] for scalar
           double precision data_in[0..2*npts-1] for vector 
           ibi = 0 if bitmap[] is not used (all defined)
                 1 if bitmap[] is used 
         */

	ibi = 0;
        if (is_v) {
#pragma omp parallel for private(i) reduction(|:ibi)
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(data[i]) && DEFINED_VAL(save->u_val[i])) {
                    data_in[i] = save->u_val[i];
                    data_in[i+ndata] = data[i];
                    bitmap[i] = 1;
                }
                else {
                    data_in[i] = data_in[i + ndata] = 0.0;
		    ibi = ibi | 1;
                    bitmap[i] = 0;
                }
            }
            if (mode == 98) fprintf(stderr," UV interpolation %s , %s\n", save->name, name);
        }
        else {
#pragma omp parallel for private(i) reduction(|:ibi)
            for (i = 0; i < ndata; i++) {
                if (DEFINED_VAL(data[i])) {
                    data_in[i] = data[i];
                    bitmap[i] = 1;
                }
                else {
                    data_in[i] = 0.0;
		    ibi = ibi | 1;
                    bitmap[i] = 0;
                }
            }
        }

        // interpolate

        tmp_interpol_type = interpol_type;
	if (interpol_type == 4 && ibi == 1) {
	    fprintf(stderr,"new_grid: undef values, spectral interpolation changed to binlinear\n");
	    tmp_interpol_type = 0;
	}

        ftn_npnts = (int) ndata;
        ftn_nout = (int) n_out;
        if (is_v) {
            IPOLATEV(&tmp_interpol_type, ipopt, &gdtnum_in, &(gdt_in[0]), &gdt_in_size,
                &(save->gdtnum_out), &(save->gdt_out[0]), &(save->gdt_out_size),
                &ftn_npnts, &ftn_nout, &km, &ibi, bitmap, data_in, data_in+ndata, &n_out,
                save->rlat,save->rlon,save->crot,save->srot, &ibo, save->bitmap_out, save->data_out, 
                save->data_out + n_out, &iret);
        }
        else {
            IPOLATES(&tmp_interpol_type, ipopt, &gdtnum_in, &(gdt_in[0]), &gdt_in_size,
                &(save->gdtnum_out), &(save->gdt_out[0]), &(save->gdt_out_size),
                &ftn_npnts, &ftn_nout, &km, &ibi, bitmap, data_in, &n_out,
                save->rlat,save->rlon, &ibo, save->bitmap_out, save->data_out, &iret);
        }

        if (iret != 0) {
            if (iret == 2) fatal_error("IPOLATES failed, unrecognized input grid or no grid point of output grid is in input grid","");
            if (iret == 3) fatal_error("IPOLATES failed, unrecognized output grid","");
	    if (iret == 41 && tmp_interpol_type == 4) fatal_error("IPOLATES failed, non-global grid for spectral interpolation","");

            fatal_error_i("IPOLATES failed, error %d",iret);
        }

        // now to write out the grib file

        for (i = 0; i < 8; i++) new_sec[i] = sec[i];
        new_sec[3] = save->sec3;

	// write scalar or U

        if (is_v == 1) {		// change V -> U
            GB2_ParmNum(new_sec) = GB2_ParmNum(new_sec) - 1;
	}
	if (ibo == 1) {		// has a bitmap
#pragma omp parallel for private(i)
            for (i = 0; i < n_out; i++) {
		save->data_wrt[i] = save->bitmap_out[i] == 0 ? UNDEFINED : save->data_out[i];
	    }
	}
	else {
#pragma omp parallel for private(i)
            for (i = 0; i < n_out; i++) {
	        save->data_wrt[i] = save->data_out[i];
	    }
	}
        grib_wrt(new_sec, save->data_wrt, n_out, nnx, nny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));

	// write V if necessary

        if (is_v == 1) {		// vector
            GB2_ParmNum(new_sec) = GB2_ParmNum(new_sec) + 1;
	    if (ibo == 1) {		// has a bitmap
#pragma omp parallel for private(i)
                for (i = 0; i < n_out; i++) {
		    save->data_wrt[i] = save->bitmap_out[i] == 0 ? UNDEFINED : save->data_out[i+n_out];
		}
	    }
	    else {
#pragma omp parallel for private(i)
                for (i = 0; i < n_out; i++) {
	            save->data_wrt[i] = save->data_out[i+n_out];
	        }
	    }
            grib_wrt(new_sec, save->data_wrt, n_out, nnx, nny, use_scale, dec_scale, bin_scale,
                wanted_bits, max_bits, grib_type, &(save->out));
        }

        if (flush_mode) fflush_file(&(save->out));
        free(data_in);
        free(bitmap);
        if (is_v != 0) {
            save->has_u = 0;
            free(save->u_val);
            free_sec(save->clone_sec);
        }
    }
    return 0;
}

#endif

#if USE_IPOLATES == 0
int f_new_grid_interpolation(ARG1) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid_ipopt(ARG1) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid(ARG4) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
int f_new_grid_winds(ARG1) {
    fprintf(stderr,"IPOLATES package is not installed\n");
    return 1;
}
#endif
