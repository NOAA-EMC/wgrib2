#include <stdio.h>
#include <stdlib.h>
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
 * HEADER:100:import_netcdf:misc:3:TESTING X=file Y=var Z=hyper-cube specification
 */

int f_import_netcdf(ARG3) {
    int status, ncid, ndims, nvars, ngatts, unlimdimid;
    int var_id, var_ndims, var_dimids[NC_MAX_VAR_DIMS], var_natts;
    nc_type var_type;
    size_t recs;
    size_t count[NC_MAX_VAR_DIMS], start[NC_MAX_VAR_DIMS];
    char name[NC_MAX_NAME+1];
    char attr[81];
    int i, j, k, m;
    unsigned int c0, s0, npnts0;
    int has_FillValue, has_missing_value;
    float FillValue, missing_value, limits;
    nc_type fv_type, nctype;
    size_t fv_len, lenp;
    float scale_factor, add_offset;

    if (mode == -1) {
	decode = 1;
    }
    if (mode >= 0) {
	// open netcdf file
        status = nc_open(arg1, NC_NOWRITE, &ncid);
        if (status != NC_NOERR) fatal_error("import_netcdf: nc_open file %s", arg1);

	// get number of variables etc
        status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
        if (status != NC_NOERR) fatal_error_i("import_netcdf: nc_inq %d error", status);
//        fprintf(stderr,"ndimsp=%d nvarsp=%d nattsp=%d, unlim=%d\n", ndims, nvars, ngatts, unlimdimid);

	// get variable id
        status = nc_inq_varid(ncid, arg2, &var_id);
        if (status != NC_NOERR) {
	    fprintf(stderr,"import_netcdf: %s is not valid variable, valid variables are\n", arg2);
	    for (i = 0; i < nvars; i++) {
		status = nc_inq_varname(ncid, i, name);
                if (status == NC_NOERR) fprintf(stderr,"variable %d: %s\n",i,name);
	    }
	    fatal_error_i("import_netcdf: nc_inq_varid %d error", status);
	}

	/* get particulars about variable */
	status = nc_inq_var(ncid, var_id, 0, &var_type, &var_ndims, var_dimids, &var_natts);
        if (status != NC_NOERR) fatal_error_i("import_netcdf: nc_inq_var %d error", status);
        fprintf(stderr,"ndims=%d var_type=%d #var_attributes %d\n", var_ndims, var_type, var_natts);

	for (i = 0; i < var_natts; i++) {
	    status = nc_inq_attname(ncid, var_id, i, name);
            if (status != NC_NOERR) fatal_error("import_netcdf: nc_inq_attname","");
	    status = nc_inq_att(ncid, var_id, name, &nctype, &lenp);
            if (status != NC_NOERR) fatal_error("import_netcdf: nc_inq_att","");

	    if (nctype == NC_CHAR && lenp <= sizeof(attr)-1) {
		status = nc_get_att_text(ncid,var_id,name,attr);
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
	status = nc_inq_att(ncid, var_id, "_FillValue", &fv_type, &fv_len);
	if (status == NC_NOERR && fv_type == NC_FLOAT && fv_len == 1) {
	    status = nc_get_att_float(ncid, var_id, "_FillValue", &FillValue);
            if (status == NC_NOERR) has_FillValue = 1;
	}
	// get the missing_value (assume scalar, not vector)
	has_missing_value = 0;
	missing_value = 0.0;
	status = nc_inq_att(ncid, var_id, "missing_value", &fv_type, &fv_len);
	if (status == NC_NOERR && fv_type == NC_FLOAT && fv_len == 1) {
	    status = nc_get_att_float(ncid, var_id, "missing_value", &missing_value);
            if (status == NC_NOERR) has_missing_value = 1;
	}
	fprintf(stderr,"_FillValue=%lf %d missing_value=%lf %d\n", FillValue,has_FillValue, missing_value,has_missing_value);

	// get scale_factor (if present)
	scale_factor = 1.0;
	status = nc_inq_att(ncid, var_id, "scale_factor", &fv_type, &fv_len);
	if (status == NC_NOERR && fv_len == 1) {
	    status = nc_get_att_float(ncid, var_id, "scale_factor", &missing_value);
            if (status != NC_NOERR) fatal_error("import_netcdf: nc_get_att_float scale_factor","");
	}
	// get add_offset (if present)
	add_offset = 0.0;
	status = nc_inq_att(ncid, var_id, "add_offset", &fv_type, &fv_len);
	if (status == NC_NOERR && fv_len == 1) {
	    status = nc_get_att_float(ncid, var_id, "add_offset", &missing_value);
            if (status != NC_NOERR) fatal_error("import_netcdf: nc_get_att_float add_offset","");
	}

	/* parse arg3, hypercube definition,  start1:count1:start2:count2:..:startN:countN */
	i = 0;
	k = sscanf(arg3,"%u:%u%n", &s0, &c0, &m);
	while (k == 2) {
	    start[i] = s0;
	    count[i] = c0;
	    i++;
	    arg3 += m + 1;
	    k = sscanf(arg3,"%u:%u%n", &s0, &c0, &m);
	}

	if (i != var_ndims) {
	    fprintf(stderr,"dimension mismatch %s, netcdf file has\n", arg3);
	    for (i = 0; i < var_ndims; i++) {
	        status = nc_inq_dim(ncid, var_dimids[i], name, &recs);
	        fprintf(stderr,"   dim %d id=%d name=%s recs=%d\n", i, var_dimids[i], name, (int) (int) recs);
	    }
	    fatal_error("import_netcdf: dimensions do not match","");
	}

        npnts0 = 1;
	for (j = 0; j < var_ndims; j++) {
	    status = nc_inq_dim(ncid, var_dimids[j], name, &recs);
            if (status != NC_NOERR) fatal_error_i("import_netcdf:nc_inq_dim %d", status);
	    if (count[j] != 1 && count[j] != recs) {
	        fatal_error_ii("import_netcdf: size dimension %d size=%d",j, (int) recs);
	    }
	    npnts0 *= count[j];
	}
	if (npnts0 > ndata) fatal_error_ii("import_netcdf: size mismatch grib:%u netcdf:%u",ndata,npnts0);
	if (npnts0 < ndata) fprintf(stderr,"WARNING: import_netcdf: size mismatch grib:%u netcdf:%u\ndata is padded\n", ndata, npnts0);

	/* read the data */
	status = nc_get_vara_float(ncid, var_id, start, count, &(data[0]));
        if (status != NC_NOERR) fatal_error_i("import_netcdf: nc_get_vara_float rc=d",status);

	if (has_FillValue) {
	    limits = 0.01 * fabs(FillValue);
	    for (i = 0; i < ndata; i++) {
		if (fabs(data[i] - FillValue) < limits) data[i] = UNDEFINED;
	    }
	}
	if (has_missing_value) {
	    limits = 0.01 * fabs(missing_value);
	    for (i = 0; i < ndata; i++) {
		if (fabs(data[i] - missing_value) < limits) data[i] = UNDEFINED;
	    }
	}
	if (add_offset != 0.0 && scale_factor != 1.0) {
	    fprintf(stderr,"import_netcdf scale_factor %lf, add_offset=%lf\n", scale_factor, add_offset);
	    for (i = 0; i < ndata; i++) {
	        if (DEFINED_VAL(data[i])) {
		    data[i] = data[i]*scale_factor + add_offset;
	        }
	    }
	}
	use_scale = 0;
        status = nc_close(ncid);
    }
    return 0;
}


#else
int f_import_netcdf(ARG3) {
    fatal_error("import_netcdf: not installed","");
    return 1;
}
#endif
