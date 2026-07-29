/* This is a test of the functions in grb2_inq.c, which is part of the C API for wgrib2.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define NDATA 100
#define GRB_FILE "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
#define GRB_INV "data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv"

int wgrib2_set_reg(float *data, size_t size, int reg);
int wgrib2_set_mem_buffer(const unsigned char *my_buffer, size_t size, int n);

int
main()
{
    printf("Testing grb2_get_data()...\n");
    {
        int ret;
        int last_options, good, npnts;
        int ndata = NDATA;
        float data[NDATA] = { 0. };

        /* Set register 19 with data. */
        wgrib2_set_reg(data, (size_t)ndata, 19);

        /* Give intial values to avoid undefined behavior. */
        last_options = 0;
        good = 0;
        npnts = 0;

        grb2_inq_set_state(last_options, good, npnts);

        printf("Last find failed (good = 0), so should return 10.\n");
        ret = grb2_get_data(data, ndata);
        if (ret != 10) {
            printf("ERROR: grb2_get_data() returned %d, expected 10.\n", ret);
            return 2;
        }

        printf("Wrong size data (ndata != npnts). Should return 11.\n");
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_data(data, ndata);
        if (ret != 11) {
            printf("ERROR: grb2_get_data() returned %d, expected 11.\n", ret);
            return 3;
        }

        printf("Invalid options (reading data not requested). Should return 12.\n");
        good = 1;
        npnts = ndata;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_data(data, ndata);
        if (ret != 12) {
            printf("ERROR: grb2_get_data() returned %d, expected 12.\n", ret);
            return 4;
        }

        printf("Valid Case. Should return 0.\n");
        last_options = DATA;
        good = 1;
        npnts = ndata;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_data(data, ndata);
        if (ret != 0) {
            printf("ERROR: grb2_get_data() returned %d, expected 0.\n", ret);
            return 5;
        }
    }
    printf("Testing grb2_get_lonlat()...\n");
    {
        int ret;
        int last_options, good, npnts;
        int ndata = NDATA;
        float lon[NDATA] = { 0. };
        float lat[NDATA] = { 0. };

        /* Set registers 17 and 18 with longitude and latitude data. */
        wgrib2_set_reg(lon, (size_t)ndata, 17);
        wgrib2_set_reg(lat, (size_t)ndata, 18);

        /* Reset all from last tests. */
        last_options = 0;
        good = 0;
        npnts = 0;

        grb2_inq_set_state(last_options, good, npnts);

        printf("Last find failed (good = 0), so should return 10.\n");
        ret = grb2_get_lonlat(lon, lat, ndata);
        if (ret != 10) {
            printf("ERROR: grb2_get_lonlat() returned %d, expected 10.\n", ret);
            return 6;
        }

        printf("Wrong size data (ndata != npnts). Should return 11.\n");
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_lonlat(lon, lat, ndata);
        if (ret != 11) {
            printf("ERROR: grb2_get_lonlat() returned %d, expected 11.\n", ret);
            return 7;
        }

        printf("Invalid options (reading lonlat not requested). Should return 12.\n");
        good = 1;
        npnts = ndata;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_lonlat(lon, lat, ndata);
        if (ret != 12) {
            printf("ERROR: grb2_get_lonlat() returned %d, expected 12.\n", ret);
            return 8;
        }

        printf("Valid Case. Should return 0.\n");
        last_options = LONLAT;
        good = 1;
        npnts = ndata;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_lonlat(lon, lat, ndata);
        if (ret != 0) {
            printf("ERROR: grb2_get_lonlat() returned %d, expected 0.\n", ret);
            return 9;
        }
    }
    printf("Testing grb2_size_meta()...\n");
    {
        int ret;
        int last_options, good, npnts;
        size_t ndata = NDATA;
        unsigned char data[NDATA] = { 0 };

        /* Set register 18 with metadata. */
        wgrib2_set_mem_buffer(data, ndata, 18);

        /* Reset all from last tests. */
        last_options = 0;
        good = 0;
        npnts = 0;

        grb2_inq_set_state(last_options, good, npnts);

        printf("Last find failed (good = 0), so should return -1.\n");
        ret = grb2_size_meta();
        if (ret != -1) {
            printf("ERROR: grb2_size_meta() returned %d, expected -1.\n", ret);
            return 10;
        }

        printf("Invalid options (reading metadata not requested). Should return -2.\n");
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_size_meta();
        if (ret != -2) {
            printf("ERROR: grb2_size_meta() returned %d, expected -2.\n", ret);
            return 11;
        }

        printf("Valid Case. Should return buffer size + 1.\n");
        last_options = META;
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_size_meta();
        if (ret != (int)(ndata + 1)) {
            printf("ERROR: grb2_size_meta() returned %d, expected %d.\n", ret, (int)(ndata + 1));
            return 12;
        }
    }
    printf("Testing grb2_get_meta()...\n");
    {
        int ret;
        int last_options, good, npnts;
        size_t ndata = NDATA;
        unsigned char data[NDATA] = { 0 };

        /* Reset all from last tests. */
        last_options = 0;
        good = 0;
        npnts = 0;
        grb2_inq_set_state(last_options, good, npnts);

        printf("Last find failed (good = 0), so should return 10.\n");
        ret = grb2_get_meta(data, ndata);
        if (ret != 10) {
            printf("ERROR: grb2_get_meta() returned %d, expected 10.\n", ret);
            return 13;
        }

        printf("Invalid options (reading metadata not requested). Should return 11.\n");
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);

        ret = grb2_get_meta(data, ndata);
        if (ret != 11) {
            printf("ERROR: grb2_get_meta() returned %d, expected 11.\n", ret);
            return 14;
        }

        last_options = META;
        grb2_inq_set_state(last_options, good, npnts);

        /* Set register 18 with bad size. */
        wgrib2_set_mem_buffer(data, 0, 18);

        printf("Size of metadata is 0, so should return 12 (grib format error).\n");
        ret = grb2_get_meta(data, ndata);
        if (ret != 12) {
            printf("ERROR: grb2_get_meta() returned %d, expected 12.\n", ret);
            return 15;
        }

        /* Set register 18 with metadata of correct size. */
        wgrib2_set_mem_buffer(data, ndata, 18);

        printf("Size mismatch. Should return 13.\n");
        ret = grb2_get_meta(data, ndata-1);
        if (ret != 13) {
            printf("ERROR: grb2_get_meta() returned %d, expected 13.\n", ret);
            return 16;
        }

        printf("Valid Case. Should return 0.\n");
        ret = grb2_get_meta(data, ndata+1);
        if (ret != 0) {
            printf("ERROR: grb2_get_meta() returned %d, expected 0.\n", ret);
            return 17;
        }
    }
    printf("Testing grb2_size_gridmeta()...\n");
    {
        int ret;
        int last_options, good, npnts;
        size_t ndata = NDATA;
        unsigned char data[NDATA] = { 0 };

        /* Reset all from last tests. */
        last_options = 0;
        good = 0;
        npnts = 0;
        grb2_inq_set_state(last_options, good, npnts);

        /* Set register 17 with metadata. */
        wgrib2_set_mem_buffer(data, ndata, 17);

        printf("Last find failed (good = 0), so should return -1.\n");
        ret = grb2_size_gridmeta();
        if (ret != -1) {
            printf("ERROR: grb2_size_gridmeta() returned %d, expected -1.\n", ret);
            return 18;
        }

        printf("Invalid options (reading grid metadata not requested). Should return -2.\n");
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_size_gridmeta();
        if (ret != -2) {
            printf("ERROR: grb2_size_gridmeta() returned %d, expected -2.\n", ret);
            return 19;
        }

        printf("Valid Case. Should return buffer size + 1.\n");
        last_options = GRIDMETA;
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_size_gridmeta();
        if (ret != (int)(ndata + 1)) {
            printf("ERROR: grb2_size_gridmeta() returned %d, expected %d.\n", ret, (int)(ndata + 1));
            return 20;
        }
    }
    printf("Testing grb2_get_gridmeta()...\n");
    {
        int ret;
        int last_options, good, npnts;
        size_t ndata = NDATA;
        unsigned char data[NDATA] = { 0 };

        /* Reset all from last tests. */
        last_options = 0;
        good = 0;
        npnts = 0;
        grb2_inq_set_state(last_options, good, npnts);

        printf("Last find failed (good = 0), so should return 10.\n");
        ret = grb2_get_gridmeta(data, ndata);
        if (ret != 10) {
            printf("ERROR: grb2_get_gridmeta() returned %d, expected 10.\n", ret);
            return 21;
        }

        printf("Invalid options (reading grid metadata not requested). Should return 11.\n");
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);

        ret = grb2_get_gridmeta(data, ndata);
        if (ret != 11) {
            printf("ERROR: grb2_get_gridmeta() returned %d, expected 11.\n", ret);
            return 22;
        }

        last_options = GRIDMETA;
        grb2_inq_set_state(last_options, good, npnts);

        /* Set register 17 with bad size. */
        wgrib2_set_mem_buffer(data, 0, 17);

        printf("Size of grid metadata is 0, so should return 12 (grid problem).\n");
        ret = grb2_get_gridmeta(data, ndata);
        if (ret != 12) {
            printf("ERROR: grb2_get_gridmeta() returned %d, expected 12.\n", ret);
            return 23;
        }

        /* Set register 17 with grid metadata of correct size. */
        wgrib2_set_mem_buffer(data, ndata, 17);

        printf("Size mismatch. Should return 13.\n");
        ret = grb2_get_gridmeta(data, ndata-1);
        if (ret != 13) {
            printf("ERROR: grb2_get_gridmeta() returned %d, expected 13.\n", ret);
            return 24;
        }

        printf("Valid Case. Should return 0.\n");
        ret = grb2_get_gridmeta(data, ndata+1);
        if (ret != 0) {
            printf("ERROR: grb2_get_gridmeta() returned %d, expected 0.\n", ret);
            return 25;
        }
    }
    // GRIB file has JPEG packing type.
#if G2_JPEG2000_ENABLED == 1
    printf("Testing grb2_inqVA()...\n");
    {
        long long int ret;
        long long int size = 36391;
        unsigned int options;

        /* Reset to avoid unexpected behavior. */
        grb2_inq_set_state(0, 0, 0);

        printf("WENS & LATLON conflict test. Should return -1.\n");
        options = WENS | LATLON;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, NULL);
        if (ret != -1) {
            printf("ERROR: grb2_inqVA() returned %lld, expected -1.\n", ret);
            return 26;
        }

        printf("WENS & RAW_ORDER conflict test. Should return -1.\n");
        options = WENS | RAW_ORDER;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, NULL);
        if (ret != -1) {
            printf("ERROR: grb2_inqVA() returned %lld, expected -1.\n", ret);
            return 27;
        }

        printf("LATLON & RAW_ORDER conflict test. Should return -1.\n");
        options = LATLON | RAW_ORDER;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, NULL);
        if (ret != -1) {
            printf("ERROR: grb2_inqVA() returned %lld, expected -1.\n", ret);
            return 28;
        }

        printf("Invalid file name. Should return -2.\n");
        options = SEQUENTIAL;
        ret = grb2_inqVA("invalid.grib2", GRB_INV, options, NULL);
        if (ret != -2) {
            printf("ERROR: grb2_inqVA() returned %lld, expected -2.\n", ret);
            return 29;
        }

        printf("Invalid argument. Should return -3.\n");
        options = SEQUENTIAL;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, "-invalid_arg", NULL);
        if (ret != -3) {
            printf("ERROR: grb2_inqVA() returned %lld, expected -3.\n", ret);
            return 30;
        }

        printf("Non-sequential option with empty argument. Should return -5.\n");
        options = 0;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, "", NULL);
        if (ret != -5) {
            printf("ERROR: grb2_inqVA() returned %lld, expected -5.\n", ret);
            return 32;
        }

        printf("Test with non-conflicting options.\n");
        options = DATA|LATLON|META|GRIDMETA;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, "UGRD", NULL);
        if (ret != size) {
            printf("ERROR: grb2_inqVA() returned %lld, expected %lld.\n", ret, size);
            return 34;
        }

        printf("Test with WENS.\n");
        options = WENS;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, "UGRD", NULL);
        if (ret != size) {
            printf("ERROR: grb2_inqVA() returned %lld, expected %lld.\n", ret, size);
            return 35;
        }

        printf("Test with RAW_ORDER.\n");
        options = RAW_ORDER;
        ret = grb2_inqVA(GRB_FILE, GRB_INV, options, "UGRD", NULL);
        if (ret != size) {
            printf("ERROR: grb2_inqVA() returned %lld, expected %lld.\n", ret, size);
            return 36;
        }
    }
#endif
    printf("SUCCESS!\n");
    return 0;
}