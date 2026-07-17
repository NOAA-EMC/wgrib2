/* This is a test of the functions in grb2_inq.c, which is part of the C API for wgrib2.
 * 
 * Alyson Stahl
*/

#include "c_wgrib2api.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define NDATA 100

int wgrib2_set_reg(float *data, size_t size, int reg);

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
        wgrib2_set_reg(data, ndata * sizeof(float), 19);

        /* Give intial values to avoid undefined behavior. */
        last_options = 0;
        good = 0;
        npnts = 0;

        grb2_inq_set_state(last_options, good, npnts);

        /* Last find failed (good = 0), so should return 3. */
        ret = grb2_get_data(data, ndata);
        if (ret != 3) {
            printf("ERROR: grb2_get_data() returned %d, expected 3.\n", ret);
            return 2;
        }

        /* Wrong size data (ndata != npnts). Should return 2. */
        good = 1;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_data(data, ndata);
        if (ret != 2) {
            printf("ERROR: grb2_get_data() returned %d, expected 2.\n", ret);
            return 3;
        }

        /* Invalid options (reading data not requested). Should return 4. */
        good = 1;
        npnts = ndata;
        grb2_inq_set_state(last_options, good, npnts);
        ret = grb2_get_data(data, ndata);
        if (ret != 4) {
            printf("ERROR: grb2_get_data() returned %d, expected 4.\n", ret);
            return 4;
        }

        /* Valid Case. Should return 0. */
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
    printf("SUCCESS!\n");
    return 0;
}