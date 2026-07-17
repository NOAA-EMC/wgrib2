/** @file
 * @brief Routines for setting the Product Definition Template (PDT).
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 9/2008 | W. Ebisuzaki | Initial
 * 10/2013 | W. Ebisuzaki | use update_sec4()
 * 7/2014 | W. Ebisuzaki | added copy more metadata when making new pdt
 * 5/2017 | W. Ebisuzaki | update for better handling of statistical processing
 * 2/2022 | W. Ebisuzaki | fix misc problems
 * 3/2022 | W. Ebisuzaki | fix code, add misc_arg
 * @author Public Domain: Wesley Ebisuzaki @date 9/2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:set_pdt:misc:1:makes new pdt, X=(+)PDT_number or X=(+)PDT_number:size of PDT in octets, +=copy metadata
 */

/**
 * Set the Product Definition Template (PDT).
 * 
 * Section 4 of a grib message contains the product definition and the product is defined 
 * through the Product Definition Template (PDT). There are many different PDTs but only some 
 * are in common usage. The -set_pdt option changes the current PDT to another. For example, 
 * you want to add ensemble information to a forecast with no ensemble information. To do this, 
 * you have to change the template to one that has ensemble information and then fill in the 
 * various parts of the PDT. Another use of the -set_pdt option is when you have a PDT that is 
 * unsupported by a program such as GrADS. You can use this option to convert an unsupported PDT 
 * to a supported PDT. To retain metadata, prefix the PDT with a plus sign. The amount of metadata 
 * copied depends on the version of wgrib2. 
 * 
 * ## Usage
 * X = PDT number (ex: 8, not 4.8)
 * Y = byte size of PDT if variable-sized PDT (optional)
 * 
 * -set_pdt X           default size, PDT is cleared
 *
 * -set_pdt +X          copy metadata from current PDT, size may vary
 *
 * -set_pdt X:Y         PDT is cleared
 *
 * -set_pdt +X:Y        copy metadata (Note: using wrong value of Y can produce errors)
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * ## Example
 * @code{.sh}
 * $ wgrib2 png.grb -grib OUT.grb
 * 1:4:d=2009060500:RH:2 m above ground:330 hour fcst:std dev
 * $ wgrib2 png.grb -set_pdt 0 -grib OUT.grb
 * 1:4:d=2009060500:var discipline=0 center=7 local_table=1 parmcat=255 parm=255:no_level:-1 missing fcst:
 * $ wgrib2 png.grb -set_pdt +0 -grib OUT.grb
 * 1:4:d=2009060500:RH:2 m above ground:330 hour fcst:
 * @endcode
 *
 * Suppose a program has problems with pdt 60 and 61. Changing the pdt to 1 and 11 will solve the incompatibility 
 * with only a little loss of metadata. 
 * 
 * @code{.sh}
 * $ wgrib2 IN.grb -if ":pdt=60:" -set_pdt +1 -fi \
 *                -if ":pdt=61:" -set_pdt +11 -fi \
 *                -grib OUT.grb
 * @endcode
 * 
 * @author Wesley Ebisuzaki @date 9/2008
 */
int f_set_pdt(ARG1) {

    unsigned char new_sec4[SET_PDT_SIZE];
    int i, j, pdt,len,copy_metadata;
    const char *p;
    int n;
    char misc_arg[STRING_SIZE];

    if (mode < 0) return 0;

    /* parse arguments */

    p = arg1;
    if (p == NULL) fatal_error("set_pdt: missing arg1","");

    /* get copy_metadata */
    copy_metadata = arg1[0] == '+';

    /* get pdt */
    i = sscanf(p,"%d%n", &pdt, &n);
    if (i == 0 || i == EOF || pdt < 0) fatal_error("set_pdt: X=PDT_number[:PDT_SIZE]","");

    /* get optional len */
    p += n;
    if (*p == ':') p++;

    i = sscanf(p,"%d%n", &len, &n);
    if (i == 0 || i == EOF || n == 0) {
        len = -1;
    }
    else p += n;
    if (*p == ':') p++;

    /* get free form: n=X:nc=Y */

    j = strlen(p);
    if (j+2 >= STRING_SIZE) fatal_error("set_pdt, arg (%s) too long", arg1);

    if (j == 0) {
        misc_arg[0] = 0;
    }
    else {
        for (i = 0; i < j; i++) {
            misc_arg[i] = p[i] == ':' ? '\n' : p[i];
        }
        if (p[j-1] != ':') misc_arg[i++] = '\n';
        misc_arg[i] = '\0';
    }

    i = new_pdt(sec, new_sec4, pdt, len, copy_metadata, misc_arg);
    if (i == 0) update_sec4(sec, new_sec4);
    return i;
}

/**
 * Makes a new Section 4 (Product Definition Section).
 *
 * Some metadata affects the size of the new pdt. 
 * 
 * Specify the various parameters that affect the length of the new pdt:
 *      - default (minimum size)
 *      - parameter from initial sec if copy_metadata == 1
 *      - from misc_arg
 *
 * Once these parameters are specified, the length of the pdt can be determined by:
 *    1. specified length (old way), needed for pdt not in the code
 *    2. let code calculate the length of the pdt
 *
 * ## Copying Metadata
 *
 * The parameters that affect the pdt size are always set (assuming in code). If 
 * copy_metadata == 1, much of the metadata is copied from 1st sec[][].
 *
 * @param sec Pointer to the current GRIB section 4 data.
 * @param new_sec4 Pointer to the new Section 4 data.
 * @param pdt New Product Definition Template number.
 * @param len Length of the new Section 4 data or -1 for default size.
 * @param copy_metadata Flag indicating whether to copy metadata (1 = yes, 0 = no).
 * @param misc_arg String with new parameters for size of pdt.
 * 
 * @return 0 on success. Throws fatal_error() on failure.
 * 
 * @author Wesley Ebisuzaki @date 9/2008
 */
int new_pdt(unsigned char **sec, unsigned char *new_sec4, int pdt, int len, int copy_metadata, char *misc_arg) {

    int i, k, len_old, pdt_old;
    unsigned char *new_sec[9];
    unsigned char *p_old, *p_new;
    char *p;
//    int mode = 99;

    /* X_old: X from sec, X: desired X, X_new: X for new_sec */
    int n, n_old, n_new;
    int vert_coor, vert_coor_old, vert_coor_new;
    int nb, nb_old, nb_new;
    // int nc, nc_old, nc_new;
    int ncat, ncat_old, ncat_new;			// number of categories
    int nfcluster, nfcluster_old, nfcluster_new;	// number of forecasts in cluster
    int np, np_old, np_new;
    int np_dp, np_dp_old, np_dp_new;

//fprintf(stderr,"::: new_pdt1: pdt=%d\n", pdt);

    /* get parameters from current pdt, calculate desired parameter */

    len_old = GB2_Sec4_size(sec);
    pdt_old = GB2_ProdDefTemplateNo(sec);

    // make **new_sec, with different sec[4]
    for (i = 0; i < 9; i++) new_sec[i] = sec[i];
    new_sec[4] = new_sec4;

    vert_coor_old = number_of_coordinate_values_after_template(sec);
    vert_coor = copy_metadata ? vert_coor_old : 0;

    nb_old = number_of_contributing_spectral_bands(sec);
    if (nb_old < 0) nb_old = 0;
    nb = copy_metadata ? nb_old : 0;

    ncat_old = number_of_categories(sec);
    if (ncat_old < 0) ncat_old = 0;
    ncat = copy_metadata ? ncat_old : 0;

    nfcluster_old = number_of_forecasts_in_the_cluster(sec);
    if (nfcluster_old < 0) nfcluster_old = 0;
    nfcluster = copy_metadata ? nfcluster_old : 0;

    np_old = number_of_partitions(sec);
    if (np_old < 0) np_old = 0;
    np = copy_metadata ? np_old : 0;

    np_dp_old = number_of_following_distribution_parameters_np(sec);
    if (np_dp_old < 0) np_dp_old = 0;
    np_dp = copy_metadata ? np_dp_old : 0;

    k = stat_proc_n_time_ranges_index(sec);
    n_old = k >= 0 ? sec[4][k] : 0;
    n = copy_metadata && n_old > 1 ? n_old : 1;

// fprintf(stderr,"::: new_pdt2: k=%d n_old %d n=%d nb=%d\n",k, n_old, n, nb);

    /* read misc_arg, modify desired parameters */

    /* default misc parameters are set, can override from misc_arg */
    p = misc_arg == NULL ? "" : misc_arg;

// fprintf(stderr, "prior n=%d nb=%d ncat=%d np=%d np_dp=%d vert_corr=%d\n", n, nb, ncat, np, np_dp, vert_coor);
    while (*p) {
        i = sscanf(p, "n=%d", &n);
        if (i != 1) i = sscanf(p, "nb=%d", &nb);
        if (i != 1) i = sscanf(p, "ncat=%d", &ncat);
        if (i != 1) i = sscanf(p, "nfcluster=%d", &nfcluster);
        if (i != 1) i = sscanf(p, "np=%d", &np);
        if (i != 1) i = sscanf(p, "np_dp=%d", &np_dp);
        if (i != 1) i = sscanf(p, "vert_coor=%d", &vert_coor);
        if (i != 1) fatal_error("new_pdt: misc_arg unknown variable: %s\n", p);

        /* skip to next line */
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
// fprintf(stderr, "post n=%d nb=%d ncat=%d np=%d np_dp=%d vert_coor=%d\n", n, nb, ncat, np, np_dp, vert_coor);

    /* modify desired parameters depending on new pdt */


    vert_coor_new = vert_coor;	/*all pdts have vert coordinates */
    nb_new = number_of_contributing_spectral_bands(new_sec) ? nb : 0;
    ncat_new = number_of_categories_location(new_sec) ? ncat : 0;
    nfcluster_new = number_of_forecasts_in_the_cluster(new_sec) ? nfcluster : 0;
    np_new = number_of_partitions_location(new_sec) ? np : 0;
    np_dp_new = number_of_following_distribution_parameters_np_location(new_sec) ? np_dp : 0;

    if (n > 1 && stat_proc_n_time_ranges_index(new_sec) >= 0 ) n_new = n;
    else n_new = 1;

// fprintf(stderr, "adjusted new n=%d nb=%d ncat=%d np=%d np_dp=%d vert_coor=%d\n", 
// n_new, nb_new, ncat_new, np_new, np_dp_new, vert_coor_new);
   
    /* if len is not specified then determine the len of the new pdt */

    if (len <= 0) {
        len = pdt_len(NULL, pdt);
// fprintf(stderr,"calc size of pdt befor additions %d\n",len);
        if (len <= 0) fatal_error_i("pdt_len: pdt=%d is not supported", pdt);

        // increase len for additional time ranges
        len += (n_new - 1) * 12;

        // increase len for vertical coordinte values
        len += 4*vert_coor_new;

        len += 11*nb_new;
        len += 12*ncat_new;
        len += nfcluster_new;
        len += 2*np_new;
        len += 5*np_dp_new;
    }

// fprintf(stderr,"set_pdt: n_new %d vert_coor_new %d\n", n_new, vert_coor_new);
// fprintf(stderr,"set_pdt: final len %d\n",len);

    // define new_sec4

    if (len > SET_PDT_SIZE) fatal_error_ii("set_pdt: maximum pdt len is %d wanted %d", SET_PDT_SIZE, len);

    // set everything in new_sec4 to missing
    for (i = 9; i < len; i++) new_sec4[i] = 255;
// if (mode == 99) fprintf(stderr,"set_pdt: a\n");

    uint_char(len, new_sec4);			// size of sec[4];
    new_sec4[4] = 4;				// section number
    uint2_char(vert_coor_new, new_sec4 + 5);	// number of vert coordinate parameters
    uint2_char(pdt, new_sec4+7);		// pdt
   
    p_new = number_of_contributing_spectral_bands_location(new_sec);
    if (p_new) *p_new = (unsigned char) nb_new;

    p_new = number_of_categories_location(new_sec);
    if (p_new) *p_new = (unsigned char) ncat_new;

    p_new = number_of_forecasts_in_the_cluster_location(new_sec);
    if (p_new) *p_new = (unsigned char) nfcluster_new;

    p_new = number_of_partitions_location(new_sec);
    if (p_new) *p_new = (unsigned char) np_new;

    p_new = number_of_following_distribution_parameters_np_location(new_sec);
    if (p_new) *p_new = (unsigned char) np_dp_new;

    k = stat_proc_n_time_ranges_index(new_sec);
    if (k >= 0) new_sec4[k] = n_new;
//fprintf(stderr, "potential update n_new=%d, k=%d nb_new=%d\n", n_new, k, nb_new);

// fprintf(stderr,"size of new pdt %d\n", GB2_Sec4_size(new_sec));

    if (copy_metadata == 0) return 0;		// finished

    /* copy statistical processing terms */
    if (n_new >= 1 && n_new == n_old) {
        p_old = stat_proc_verf_time_location(sec);
        p_new = stat_proc_verf_time_location(new_sec);
        if (p_old != NULL && p_new != NULL) {
            k = (58-34) + 12*(n_new-1);
            for (i = 0; i < k; i++) p_new[i] = p_old[i];
        }
    }
// fprintf(stderr,"new_pdtA: n_old %d, n %d, n_new %d\n", n_old, n, n_new);

    /* copy vertical coordinates */
    if (vert_coor_new == vert_coor_old && vert_coor_new > 0 ) {
        // copy 4*vert_coor bytes from end of 
        for (i = 0; i < 4*vert_coor; i++) 
            new_sec4[i + len-4*vert_coor] = sec[4][i + len_old-4*vert_coor];
    }

    /* copy satellite spectral band info */
    if (nb_new == nb_old && nb_new > 0) {
        p_old = number_of_contributing_spectral_bands_location(sec);
        p_new = number_of_contributing_spectral_bands_location(new_sec);
        /* assumes spectral data follows nb */
        for (i = 1; i <= 11*nb_new; i++) p_new[i] = p_old[i];
    }
// fprintf(stderr,"new_pdtA: 2\n");

    /* copy ncat . number of categories */
    if (ncat_old == ncat_new && ncat_new > 0) {
        p_old = number_of_categories_location(sec);
        p_new = number_of_categories_location(new_sec);
        for (i = 1; i <= 12*ncat_new; i++) p_new[i] = p_old[i];
    }

    /* number of forecasts in cluster */
    if (nfcluster_old == nfcluster_new && nfcluster_new > 0) {
        p_old = list_of_nc_ensemble_forecast_numbers_location(sec);
        p_new = list_of_nc_ensemble_forecast_numbers_location(new_sec);
        for (i = 1; i <= nfcluster_new; i++) p_new[i] = p_old[i];
    }
// fprintf(stderr,"new_pdtA: 3\n");

    /* copy np . number of partitions */
    if (np_old == np_new && np_new > 0) {
        p_old = number_of_partitions_location(sec);
        p_new = number_of_partitions_location(new_sec);
        for (i = 1; i <= 2*np_new; i++) p_new[i] = p_old[i];
    }

    /* copy number_of_following_distribution_parameters_np */
    if (np_dp_old == np_dp_new && np_dp_new > 0) {
        p_old = number_of_following_distribution_parameters_np_location(sec);
        p_new = number_of_following_distribution_parameters_np_location(new_sec);
        for (i = 1; i <= 5*np_new; i++) p_new[i] = p_old[i];
    }
// fprintf(stderr,"new_pdtA: 5\n");

// if (mode == 99) fprintf(stderr,"set_pdt: c\n");
    new_sec4[9] = sec[4][9];		// parameter category 4.1
    new_sec4[10] = sec[4][10];		// parameter number   4.2

    p_old = code_table_4_3_location(sec);
    p_new = code_table_4_3_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = code_table_4_4_location(sec);
    p_new = code_table_4_4_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        for (i = 0; i < 5; i++) p_new[i] = p_old[i];
    }

    p_old = code_table_4_5a_location(sec);
    p_new = code_table_4_5a_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        for (i = 0; i < 6; i++) p_new[i] = p_old[i];
    }

    p_old = code_table_4_5b_location(sec);
    p_new = code_table_4_5b_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        for (i = 0; i < 6; i++) p_new[i] = p_old[i];
    }

    p_old = code_table_4_6_location(sec);
    p_new = code_table_4_6_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = code_table_4_7_location(sec);
    p_new = code_table_4_7_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = code_table_4_8_location(sec);
    p_new = code_table_4_8_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    /* probabilty info */
    p_old = code_table_4_9_location(sec);
    p_new = code_table_4_9_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        for (i = -2; i < 11; i++) p_new[i] = p_old[i];
    }

    p_old = code_table_4_10_location(sec);
    p_new = code_table_4_10_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = code_table_4_15_location(sec);
    p_new = code_table_4_15_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = code_table_4_16_location(sec);
    p_new = code_table_4_16_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    /* chemical type */
    p_old = code_table_4_230_location(sec);
    p_new = code_table_4_230_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        *p_new++ = *p_old++;
        *p_new = *p_old;
    }

    /* aerosol type */
    p_old = code_table_4_233_location(sec);
    p_new = code_table_4_233_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        *p_new++ = *p_old++;
        *p_new = *p_old;
    }

    if (pdt_old == 20 && pdt == 20) {			// radar products
        for (i = 9; i < 42; i++) new_sec[4][i] = sec[4][i];
    }

    p_old = perturbation_number_location(sec);
    p_new = perturbation_number_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = number_of_forecasts_in_the_ensemble_location(sec);
    p_new = number_of_forecasts_in_the_ensemble_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = background_generating_process_identifier_location(sec);
    p_new = background_generating_process_identifier_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = analysis_or_forecast_generating_process_identifier_location(sec);
    p_new = analysis_or_forecast_generating_process_identifier_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    p_old = hours_of_observational_data_cutoff_after_reference_time_location(sec);
    p_new = hours_of_observational_data_cutoff_after_reference_time_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        *p_new++ = *p_old++;
        *p_new = *p_old;
    }

    p_old = observation_generating_process_identifier_location(sec);
    p_new = observation_generating_process_identifier_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    /* year of model version */
    p_old = year_of_model_version_date_location(sec);
    p_new = year_of_model_version_date_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        for (i = 0; i < 7; i++) p_new[i] = p_old[i];
    }

    /* percentile values */
    p_old = percentile_value_location(sec);
    p_new = percentile_value_location(new_sec);
    if (p_old != NULL && p_new != NULL) *p_new = *p_old;

    /* aerosol type */
    p_old = code_table_4_233_location(sec);
    p_new = code_table_4_233_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        *p_new++ = *p_old++;
        *p_new = *p_old;
    }

    /* 1st aerosol size */
    p_old = code_table_4_91_location(sec);
    p_new = code_table_4_91_location(new_sec);
    if (p_old != NULL && p_new != NULL) {
        for (i = 0; i < 11; i++) p_new[i] = p_old[i];
    }
    return 0;
}
