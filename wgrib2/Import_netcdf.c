/*
 * Import_netcdf.c  2017  Public Domain, Wesley Ebisuzaki
 *
 * This option opens a netcdf file (3 or 4 depending on netcdf libraries),
 * reads a hyperslab, and closes the netcdf file.
 *
 * The routine could be made faster by only opening the netcdf file once.
 * You could do this by adding a routine like ffopen and ffclose which
 * would keep a history of opened files and associated ncids.
 *
 * If I did a lot of netcdf -> grib conversions, I would do a before b.
 *  a) a netcdf <-> grib time conversions
 *  b) minimizing opens/closes of netcdf files
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#if defined USE_NETCDF3 || defined USE_NETCDF4

#include <netcdf.h>

extern int decode;
extern int use_scale;

/*
 * HEADER:100:import_netcdf:misc:3:alpha X=file Y=var Z=hyper-cube specification
 */

int f_import_netcdf(ARG3) {
    int status;
    int var_id, var_ndims, var_dimids[NC_MAX_VAR_DIMS], var_natts;
    nc_type var_type;
    size_t recs;
    size_t count[NC_MAX_VAR_DIMS], start[NC_MAX_VAR_DIMS];
    char name[NC_MAX_NAME+1];
    char attr[81];
    int i, j, k, m;
    unsigned int c0, s0, npnts0;
    int has_FillValue, has_missing_value;
    double FillValue, missing_value, limits;
    nc_type fv_type, nctype;
    size_t fv_len, lenp;
    double scale_factor, add_offset, *ddata;

    /* C89 variables for unix time to YYYYMMDDHH */
    time_t unix_time_code;
    struct tm *tmp_tm;

    struct local_struct {
        int ncid, ndims, nvars, ngatts, unlimdimid;
    };
    struct local_struct *save;

    if (mode == -1) {		// initialize
	decode = 1;
	*local = save = (struct local_struct *) malloc(sizeof(struct local_struct));
        if (save == NULL) fatal_error("import_netcdf memory allocation","");
	// open netcdf file
        status = nc_open(arg1, NC_NOWRITE, &(save->ncid));
        if (status != NC_NOERR) {
	    free(save);
	    fatal_error("import_netcdf: nc_open file failed %s", arg1);
	}
	// get number of variables etc
        status = nc_inq(save->ncid, &(save->ndims), &(save->nvars), &(save->ngatts), &(save->unlimdimid));
        if (status != NC_NOERR) fatal_error_i("import_netcdf: nc_inq %d error", status);
	return 0;
    }
    save = *local;
    if (mode == -2) {		// finalize
	/* close netcdf file */
        status = nc_close(save->ncid);
        if (status != NC_NOERR) fprintf(stderr, 
	    "*** ERROR import_netcdf end-of-processing: could not close netcdf file: %s\n", arg1);
	free(save);
	return 0;
    }
    if (mode < 0) return 0;

// fprintf(stderr,">>netcdf0: ndata=%d ncid=%d\n", (int) ndata, save->ncid);

    // get variable id
    status = nc_inq_varid(save->ncid, arg2, &var_id);
    if (status != NC_NOERR) {
	fprintf(stderr,"import_netcdf: %s is not valid variable, valid variables are\n", arg2);
	for (i = 0; i < save->nvars; i++) {
	    status = nc_inq_varname(save->ncid, i, name);
            if (status == NC_NOERR) fprintf(stderr,"variable %d: %s\n",i,name);
	}
	fatal_error_i("import_netcdf: nc_inq_varid %d error", status);
    }

    /* get particulars about variable */
    status = nc_inq_var(save->ncid, var_id, 0, &var_type, &var_ndims, var_dimids, &var_natts);
    if (status != NC_NOERR) fatal_error_i("import_netcdf: nc_inq_var %d error", status);
    fprintf(stderr,"ndims=%d var_type=%d #var_attributes %d\n", var_ndims, var_type, var_natts);

    for (i = 0; i < var_natts; i++) {
	status = nc_inq_attname(save->ncid, var_id, i, name);
        if (status != NC_NOERR) fatal_error("import_netcdf: nc_inq_attname","");
	status = nc_inq_att(save->ncid, var_id, name, &nctype, &lenp);
        if (status != NC_NOERR) fatal_error("import_netcdf: nc_inq_att","");

	if (nctype == NC_CHAR && lenp <= sizeof(attr)-1) {
	    status = nc_get_att_text(save->ncid,var_id,name,attr);
	    attr[lenp] = 0;
	    fprintf(stderr,"%s.%d attr=%s: %s\n", arg2, i, name, attr);
	}
	else {
	    fprintf(stderr,"%s.%d attr=%s type=%d len=%d\n", arg2, i, name, nctype, (int) lenp);
	}
    }

    // get the _FillValue
    has_FillValue = 0;
    FillValue = 0.0;
    status = nc_inq_att(save->ncid, var_id, "_FillValue", &fv_type, &fv_len);
    if (status == NC_NOERR && fv_len == 1) {
	status = nc_get_att_double(save->ncid, var_id, "_FillValue", &FillValue);
        if (status == NC_NOERR) has_FillValue = 1;
    }

    // get the missing_value (assume scalar, not vector)
    has_missing_value = 0;
    missing_value = 0.0;
    status = nc_inq_att(save->ncid, var_id, "missing_value", &fv_type, &fv_len);
    if (status == NC_NOERR && fv_len == 1) {
        status = nc_get_att_double(save->ncid, var_id, "missing_value", &missing_value);
        if (status == NC_NOERR) has_missing_value = 1;
    }
    fprintf(stderr,"_FillValue=%lf %d missing_value=%lf %d\n", FillValue,has_FillValue, missing_value,has_missing_value);

    // get scale_factor (if present)
    scale_factor = 1.0;
    status = nc_inq_att(save->ncid, var_id, "scale_factor", &fv_type, &fv_len);
    if (status == NC_NOERR && fv_len == 1) {
	status = nc_get_att_double(save->ncid, var_id, "scale_factor", &scale_factor);
        if (status != NC_NOERR) fatal_error("import_netcdf: nc_get_att_double scale_factor","");
    }

    // get add_offset (if present)
    add_offset = 0.0;
    status = nc_inq_att(save->ncid, var_id, "add_offset", &fv_type, &fv_len);
    if (status == NC_NOERR && fv_len == 1) {
	status = nc_get_att_double(save->ncid, var_id, "add_offset", &add_offset);
        if (status != NC_NOERR) fatal_error("import_netcdf: nc_get_att_double add_offset","");
    }
    fprintf(stderr,"import_netcdf scale_factor %lf, add_offset=%lf\n", scale_factor, add_offset);

    /* parse arg3, hypercube definition,  start1:count1:start2:count2:..:startN:countN */
    /* startN:countN can be replaced by * */
    npnts0 = 1;
    for (i = 0 ; i < var_ndims; i++) {
	/* get rid if colon */
	if (i) {
	    if (*arg3 != ':') fatal_error("import_netcdf: hypercube definition error","");
	    arg3++;
	}
	/* get size of dimension i */
	status = nc_inq_dim(save->ncid, var_dimids[i], name, &recs);
        if (status != NC_NOERR) fatal_error_i("import_netcdf:nc_inq_dim %d", status);

	if (*arg3 == '*') {
	    start[i] = 0;
	    count[i] = recs;
	    npnts0 *= recs;
	    arg3 += 1;
	}
	else {
            k = sscanf(arg3,"%u:%u%n", &s0, &c0, &m);
	    if (k != 2) break;
	    start[i] = s0;
	    count[i] = c0;
	    if (c0 < 1 || c0 > recs) fatal_error_ii("Import_netcdf: count must be between 1 and %d, not %d",(int) recs, c0);
	    npnts0 *= count[i];
	    arg3 += m;
	}
    }
    if (*arg3 == 0 && npnts0 != ndata) {
	fprintf(stderr, "import_netcdf: dimension mismatch hypercube size=%d grib grid size=%d\nhyper cube data=\n", 
		npnts0, ndata);
	/* read the data
	 *
	 * Non-time data should be read in as double precision floats.
	 *
	 * The unix time codes should be read has signed 64-bit ints
	 * however, that is a NetCDF-4 feature, so will read in as 64-bit float.
	 * That will be equivalent to a 57 bit integer.  Why 57 rather than 56?
	 * For ieee, the 1st bit of the mantissa is assumed to be one.
	 */
	ddata = (double *) malloc(sizeof(double) * (size_t) npnts0);
	if (ddata == NULL) fatal_error("import_netcdf: memory allocation","");
	status = nc_get_vara_double(save->ncid, var_id, start, count, &(ddata[0]));
	if (status != NC_NOERR) fatal_error_i("import_netcdf: nc_get_vara_double rc=%d",status);

	if (strcmp(arg2,"time") != 0) {
	    for (i = 0; i < npnts0; i++) fprintf(stderr,"%d: %f\n",i, ddata[i]);
	}
	else {
	    for (i = 0; i < npnts0; i++) {
		fprintf(stderr,"%d: %.0f",i, ddata[i]);
		/* convert unix time to standard time, and print */
		/* this is C89 code, not thread safe */
		unix_time_code = ddata[i];
		tmp_tm = gmtime(&unix_time_code);
		if (tmp_tm != NULL) 
	        fprintf(stderr, "   time = %4.4d%2.2d%2.2d%2.2d %2.2d%2.2d\n",
                    tmp_tm->tm_year+1900, tmp_tm->tm_mon+1,
                    tmp_tm->tm_mday, tmp_tm->tm_hour,
                    tmp_tm->tm_min, tmp_tm->tm_sec);
		else fprintf(stderr,"\n");
	    }
	}
	free(ddata);
    }
    if (*arg3 != 0 || npnts0 != ndata) {
        fprintf(stderr,"dimension mismatch %s, netcdf file has\n", arg3);
        for (j = 0; j < var_ndims; j++) {
	    status = nc_inq_dim(save->ncid, var_dimids[j], name, &recs);
	    fprintf(stderr,"   dim %d id=%d name=%s recs=%d\n", j, var_dimids[j], name, (int) (int) recs);
        }
	fatal_error("import_netcdf: bad dimension");
    }

    /* read the data */
    ddata = (double *) malloc(sizeof(double) * (size_t) ndata);
    if (ddata == NULL) fatal_error("import_netcdf: memory allocation","");

    status = nc_get_vara_double(save->ncid, var_id, start, count, &(ddata[0]));

    if (status != NC_NOERR) fatal_error_i("import_netcdf: nc_get_vara_double rc=%d",status);

#pragma omp parallel private(i)
    {
	if (has_FillValue) {
	    limits = 0.01 * fabs(FillValue);
#pragma omp for
	    for (i = 0; i < ndata; i++) {
		if (fabs(ddata[i] - FillValue) < limits) ddata[i] = UNDEFINED;
	    }
	}
	if (has_missing_value) {
	   limits = 0.01 * fabs(missing_value);
#pragma omp for
	    for (i = 0; i < ndata; i++) {
		if (fabs(ddata[i] - missing_value) < limits) ddata[i] = UNDEFINED;
	    }
	}
	if (add_offset != 0.0 || scale_factor != 1.0) {
#pragma omp for
	    for (i = 0; i < ndata; i++) {
	        if (DEFINED_VAL(ddata[i])) {
		    data[i] = (float) ddata[i]*scale_factor + add_offset;
	        }
		else {
		    data[i] = UNDEFINED;
		}
	    }
	}
	else {
#pragma omp for
	    for (i = 0; i < ndata; i++) {
	        if (DEFINED_VAL(ddata[i])) {
		    data[i] = (float) ddata[i];
	        }
		else {
		    data[i] = UNDEFINED;
		}
	    }
	}
    }

    free(ddata);
    use_scale = 0;
    return 0;
}


#else
int f_import_netcdf(ARG3) {
    fatal_error("import_netcdf: not installed","");
    return 1;
}
#endif
