/* This file is part of wgrib2.
 * aec_pk.c
 * 6/2016  copyright DWD, under GNU GPL
 * 6/2016  cleanup, optimizations Wesley Ebisuzaki
 *
 * wgrib2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wgrib2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wgrib2.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_AEC

#include <libaec.h>


int aec_grib_out(unsigned char ** sec, float *data, unsigned int ndata, int use_scale,
		int dec_scale, int bin_scale, int wanted_bits, int max_bits, struct seq_file *out){

	int err, status;
	unsigned char *sec0, *sec1, *sec2 , *sec3, *sec4, *sec5, *sec6, *sec7;
	unsigned int i, j, n_defined, encodedLength, nbytes;

	uint32_t ccsds_flags = 0;
	uint32_t ccsds_block_size = 32; /* default value */
	uint32_t ccsds_rsi = 128;
        float min_val, max_val;
	double ref, fmin, frange, scale, dec_factor;
	int nbits, ii;

	struct aec_stream strm;
	unsigned char *inbuffer;
	/* printf("aec_grib_out: use_scale = %d dec_scale = %d bin_scale = %d wanted_bits = %d max_bits = %d " ,
	  use_scale, dec_scale, bin_scale, wanted_bits, max_bits ); */

	/* set default flags, TODO pass flags from template */
	ccsds_flags = AEC_DATA_MSB | AEC_DATA_PREPROCESS | AEC_DATA_3BYTE;

	inbuffer = NULL;

	/* required passed sections */
	sec0 = sec[0];
	sec1 = sec[1];
	sec2 = sec[2];
	sec3 = sec[3];
	sec4 = sec[4];

	/* make a sections 5-7 */
	n_defined = ndata;
	sec6 = mk_bms(data, &n_defined);   // make bitmap section, eliminate undefined grid values

	min_max_array_all_defined(data, n_defined, &min_val, &max_val);

        dec_factor = 1.0;
	if (use_scale == 0) {
		/* ecmwf style */
		fmin = min_val;
		frange = max_val - fmin;
		ref = fmin;
		dec_scale = 0;
		if (frange != 0.0) {
			frexp(frange, &ii);
			bin_scale = ii - wanted_bits;
			nbits = wanted_bits;
			scale = ldexp(1.0, -bin_scale);
			frange = floor((max_val-fmin)*scale + 0.5);
			frexp(frange, &ii);
			if (ii != nbits) bin_scale++;
		}
		else {
			bin_scale = nbits = 0;
			scale = 1;
		}
	}
	else {
		if (dec_scale) {
			dec_factor = Int_Power(10.0, -dec_scale);
			min_val *= dec_factor;
			max_val *= dec_factor;
#pragma omp parallel for private(j)
			for (j = 0; j < n_defined; j++) {
				data[j] *= dec_factor;
			}
		}
		ref = min_val;
		scale = ldexp(1.0, -bin_scale);
//		ii = (int) ( (max_val - ref)*scale + 0.5);
//		frange = (double) ii;
		frange = floor( (max_val - ref)*scale + 0.5);
		frexp(frange, &nbits);

		if (nbits > max_bits) {
			bin_scale += (nbits - max_bits);
			nbits = max_bits;
		}
	}

	if (bin_scale) {
		scale = ldexp(1.0, -bin_scale);
#pragma omp parallel for private(j)
		for (j = 0; j < n_defined; j++) {
			data[j] = (data[j] - ref)*scale;
		}
	}
	else {
#pragma omp parallel for private(j)
		for (j = 0; j < n_defined; j++) {
			data[j] = data[j] - ref;
		}
	}


	if (nbits > 0 && n_defined > 0) {
		nbytes = (nbits + 7) / 8;
		size_t inbuflen = nbytes * (size_t) n_defined;
		inbuffer = (unsigned char*) malloc(inbuflen);
		if (inbuffer == NULL) fatal_error("aes_pk: memory allocation","");

		unsigned long unsigned_val;

#pragma omp parallel for private(i, unsigned_val, j)
		for (i=0; i < n_defined; i++){
			unsigned_val = (unsigned long) floor(data[i]+0.5);
			unsigned_val = unsigned_val > 0 ? unsigned_val : 0;   // should not be necessary but ..
			for (j = 0;  j < nbytes; j++) {
				inbuffer[nbytes - 1 - j + i*nbytes] = (unsigned char) (unsigned_val & 255);
				unsigned_val = unsigned_val >> 8;
			}
		}

		size_t outbuflen = 10240 + nbytes * (size_t) n_defined;
		sec7 = (unsigned char *) malloc(outbuflen + 5);
		if (sec7 == NULL) fatal_error("aes_pk: memory allocation","");

		strm.flags = ccsds_flags;
		strm.bits_per_sample = nbits;
		strm.block_size = ccsds_block_size;
		strm.rsi = ccsds_rsi;

		strm.next_out = sec7 + 5;
		strm.avail_out = outbuflen;
		strm.next_in = inbuffer;
		strm.avail_in = inbuflen;

        /* printf("*** Packing with AEC flags: %d bits per sample: %d block size: %d rsi: %d \n", strm.flags, strm.bits_per_sample, strm.block_size, strm.rsi ); */

		if((err = aec_buffer_encode(&strm)) != AEC_OK) fatal_error_i("aec_buffer_encode %d", err);

		/* printf("*** after aec_buffer_encode() of %d bytes input, strm.avail_in: %d strm.total_in: %d\n" , inbuflen, strm.avail_in, strm.total_in); */

		encodedLength = strm.total_out;
	}
        else {
		ref = min_val / dec_factor;
		bin_scale = dec_scale = 0;
		encodedLength = 0;
		sec7 = (unsigned char *) malloc(5);
		if (sec7 == NULL) fatal_error("aes_pk: memory allocation","");
	}

	size_t sec5size = 25;
	sec5 = (unsigned char *) malloc(sec5size * sizeof(unsigned char));
	if (sec5 == NULL) fatal_error("aes_pk: memory allocation","");
	uint_char(sec5size * sizeof (unsigned char), sec5);
	sec5[4] = 5;				// section 5
	uint_char(n_defined, sec5+5);		// number of points
	uint2_char(42,sec5+9);          	// data template 42 - AEC
	flt2ieee(ref,sec5+11);			// reference value
	int2_char(bin_scale,sec5+15);		// binary scaling
	int2_char(-dec_scale,sec5+17);		// decimal scaling
	sec5[19] = nbits;
	sec5[20] = 0;				// 0 - float 1=int
	sec5[21] = ccsds_flags;
	sec5[22] = ccsds_block_size;
	uint2_char((unsigned int) ccsds_rsi, sec5+23);

	uint_char(encodedLength+5, sec7);
	sec7[4] = 7;                        // section 7

	status = wrt_sec(sec0, sec1, sec2, sec3, sec4, sec5, sec6, sec7, out);

	free(sec5);
	free(sec6);
	free(sec7);

	if(inbuffer) free(inbuffer);

	return status;
}

#endif
