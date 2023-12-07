/*
 * Modified for use in NCEPLIBS-g2c and later for wgrib2
 */

/*
 * Copyright 2005-2019 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
 * virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
 */
#include "wgrib2.h"
#include "config.h"

#ifdef USE_OPENJPEG

#include "openjpeg.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void openjpeg_warning(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stderr,"openjpeg: %s",msg);
}

static void openjpeg_error(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stderr,"openjpeg: %s",msg);
}

static void openjpeg_info(const char *msg, void *client_data)
{
    (void)msg;
    (void)client_data;
    /* fprintf(stderr,"openjpeg: %s",msg); */
}

/* opj_* Helper code from
 * https://groups.google.com/forum/#!topic/openjpeg/8cebr0u7JgY
 */
/* These routines are added to use memory instead of a file for input and output */
/* struct need to treat memory as a stream */
typedef struct
{
    OPJ_UINT8* pData;       /* our data */
    OPJ_SIZE_T dataSize;    /* how big is our data */
    OPJ_SIZE_T offset;      /* where we are currently in our data */
} opj_memory_stream;

/* This will read from our memory to the buffer */
static OPJ_SIZE_T opj_memory_stream_read(void *buffer, OPJ_SIZE_T nb_bytes, void * p_user_data)
{
    opj_memory_stream* mstream = (opj_memory_stream*) p_user_data; /* Our data */
    OPJ_SIZE_T nb_bytes_read = nb_bytes; /* Amount to move to buffer */

    /* Check if the current offset is outside our data buffer */
    if (mstream->offset >= mstream->dataSize)
        return (OPJ_SIZE_T) -1;

    /* Check if we are reading more than we have */
    if (nb_bytes > (mstream->dataSize - mstream->offset))
        nb_bytes_read = mstream->dataSize - mstream->offset;

    memcpy(buffer, &(mstream->pData[mstream->offset]), nb_bytes_read);
    mstream->offset += nb_bytes_read; /* Update the pointer to the new location */
    return nb_bytes_read;
}

/* Write from the buffer to our memory */
static OPJ_SIZE_T opj_memory_stream_write(void *buffer, OPJ_SIZE_T nb_bytes, void *user_data)
{
    opj_memory_stream* mstream = (opj_memory_stream*) user_data; /* our data */
    OPJ_SIZE_T nb_bytes_write = nb_bytes; /* Amount to move to buffer */

    /* Check if the current offset is outside our data buffer */
    if (mstream->offset >= mstream->dataSize)
        return (OPJ_SIZE_T)-1;

    /* Check if we are writing more than we have space for */
    if (nb_bytes > (mstream->dataSize - mstream->offset))
        nb_bytes_write = mstream->dataSize - mstream->offset;

    /* Copy the data from the internal buffer */
    memcpy(&(mstream->pData[mstream->offset]), buffer, nb_bytes_write);
    mstream->offset += nb_bytes_write; /* Update the pointer to the new location */
    return nb_bytes_write;
}

/* Moves the pointer forward, but never more than we have */
static OPJ_OFF_T opj_memory_stream_skip(OPJ_OFF_T nb_bytes, void *user_data)
{
    opj_memory_stream* mstream = (opj_memory_stream*) user_data;
    OPJ_SIZE_T l_nb_bytes;

    if (nb_bytes < 0)
        return -1; /* No skipping backwards */
    l_nb_bytes = (OPJ_SIZE_T) nb_bytes; /* Allowed because it is positive */
    /* Do not allow jumping past the end */
    if (l_nb_bytes > mstream->dataSize - mstream->offset)
        l_nb_bytes = mstream->dataSize - mstream->offset;
    mstream->offset += l_nb_bytes;
    return (OPJ_OFF_T)l_nb_bytes; /* Return how far we jumped */
}

/* Sets the pointer to anywhere in the memory */
static OPJ_BOOL opj_memory_stream_seek(OPJ_OFF_T nb_bytes, void * user_data)
{
    opj_memory_stream* mstream = (opj_memory_stream*) user_data;

    if (nb_bytes < 0)
        return OPJ_FALSE; /* Not before the buffer */
    if (nb_bytes >(OPJ_OFF_T) mstream->dataSize)
        return OPJ_FALSE; /* Not after the buffer */
    mstream->offset = (OPJ_SIZE_T) nb_bytes; /* Move to new position */
    return OPJ_TRUE;
}

static void opj_memory_stream_do_nothing(void * p_user_data)
{
    OPJ_ARG_NOT_USED(p_user_data);
}

/* Create a stream to use memory as the input or output */
static opj_stream_t* opj_stream_create_default_memory_stream(opj_memory_stream* memoryStream, OPJ_BOOL is_read_stream)
{
	opj_stream_t* stream;

	if (!(stream = opj_stream_default_create(is_read_stream)))
		return (NULL);
    /* Set how to work with the frame buffer */
	if (is_read_stream)
		opj_stream_set_read_function(stream, opj_memory_stream_read);
	else
		opj_stream_set_write_function(stream, opj_memory_stream_write);

	opj_stream_set_seek_function(stream, opj_memory_stream_seek);
	opj_stream_set_skip_function(stream, opj_memory_stream_skip);
	opj_stream_set_user_data(stream, memoryStream, opj_memory_stream_do_nothing);
	opj_stream_set_user_data_length(stream, memoryStream->dataSize);
	return stream;
}

int dec_jpeg2000_clone(char *injpc, int bufsize, int *outfld)
/*$$$  SUBPROGRAM DOCUMENTATION BLOCK
*                .      .    .                                       .
* SUBPROGRAM:    dec_jpeg2000      Decodes JPEG2000 code stream
*   PRGMMR: Jovic            ORG: W/NP11     DATE: 2020-06-08
*
* ABSTRACT: This Function decodes a JPEG2000 code stream specified in the
*   JPEG2000 Part-1 standard (i.e., ISO/IEC 15444-1) using OpenJPEG
*
* PROGRAM HISTORY LOG:
* 2002-12-02  Gilbert
* 2016-06-08  Jovic
*
* USAGE:     int dec_jpeg2000_clone(char *injpc, int bufsize, int *outfld)
*
*   INPUT ARGUMENTS:
*      injpc - Input JPEG2000 code stream.
*    bufsize - Length (in bytes) of the input JPEG2000 code stream.
*
*   OUTPUT ARGUMENTS:
*     outfld - Output matrix of grayscale image values.
*
*   RETURN VALUES :
*          0 = Successful decode
*         -3 = Error decode jpeg2000 code stream.
*         -5 = decoded image had multiple color components.
*              Only grayscale is expected.
*
* REMARKS:
*
*      Requires OpenJPEG Version 2
*
* ATTRIBUTES:
*   LANGUAGE: C
*   MACHINE:  Linux
*
*$$$*/
{
    int iret = 0;
    unsigned int i;
    OPJ_INT32 mask;

    opj_stream_t *stream = NULL;
    opj_image_t *image = NULL;
    opj_codec_t *codec = NULL;

    /* set decoding parameters to default values */
    opj_dparameters_t parameters = {0,};	/* decompression parameters */
    opj_set_default_decoder_parameters(&parameters);
    parameters.decod_format = 1; /* JP2_FMT */

    /* get a decoder handle */
    codec = opj_create_decompress(OPJ_CODEC_J2K);

    /* catch events using our callbacks */
    opj_set_info_handler(codec, openjpeg_info, NULL);
    opj_set_warning_handler(codec, openjpeg_warning, NULL);
    opj_set_error_handler(codec, openjpeg_error,NULL);

    /* initialize our memory stream */
    opj_memory_stream mstream;
    mstream.pData = (OPJ_UINT8 *)injpc;
    mstream.dataSize = (OPJ_SIZE_T)bufsize;
    mstream.offset = 0;
    /* open a byte stream from memory stream */
    stream = opj_stream_create_default_memory_stream( &mstream, OPJ_STREAM_READ);

    /* setup the decoder decoding parameters using user parameters */
    if (!opj_setup_decoder(codec, &parameters)) {
        fprintf(stderr,"openjpeg: failed to setup decoder");
        iret = -3;
        goto cleanup;
    }
    if  (!opj_read_header(stream, codec, &image)) {
        fprintf(stderr,"openjpeg: failed to read the header");
        iret = -3;
        goto cleanup;
    }
    if (!opj_decode(codec, stream, image)) {
        fprintf(stderr,"openjpeg: failed to decode");
        iret = -3;
        goto cleanup;
    }

    if ( (image->numcomps != 1) || (image->x1 * image->y1)==0 ) {
        iret = -3;
        goto cleanup;
    }

    assert(image->comps[0].sgnd == 0);
    assert(image->comps[0].prec < sizeof(mask)*8-1);

    mask = (1 << image->comps[0].prec) - 1;

    for (i = 0; i < image->comps[0].w * image->comps[0].h ; i++)
        outfld[i] = (int) (image->comps[0].data[i] & mask);

    if (!opj_end_decompress(codec, stream)) {
        fprintf(stderr,"openjpeg: failed in opj_end_decompress");
        iret = -3;
    }

cleanup:
    /* close the byte stream */
    if (codec)  opj_destroy_codec(codec);
    if (stream) opj_stream_destroy(stream);
    if (image)  opj_image_destroy(image);

    return iret;
}

int enc_jpeg2000_clone_float(float *data, int width, int height, int nbits,
                 int ltype, int ratio, int retry, char *outjpc,
                 int jpclen)
/*$$$  SUBPROGRAM DOCUMENTATION BLOCK
*                .      .    .                                       .
* SUBPROGRAM:    enc_jpeg2000      Encodes JPEG2000 code stream
*   PRGMMR: Jovic            ORG: W/NP11     DATE: 2020-06-08
*
* ABSTRACT: This Function encodes a grayscale image into a JPEG2000 code stream
*   specified in the JPEG2000 Part-1 standard (i.e., ISO/IEC 15444-1)
*   using OpenJPEG library
*
* PROGRAM HISTORY LOG:
* 2002-12-02  Gilbert
* 2016-06-08  Jovic
* 2020-11-18  Ebisuzaki modified from enc_jpeg2000 .. use float *data
*
* USAGE:    int enc_jpeg2000_clone_float(float *data,int width,int height,
*                            int nbits, int ltype, int ratio,
*                            int retry, char *outjpc, int jpclen)
*
*   INPUT ARGUMENTS:
*      data   - float matrix of Grayscale image values to encode.
*     width  - width of image
*     height - height of image
*     nbits  - depth (in bits) of image.  i.e number of bits
*              used to hold each data value
*    ltype   - indicator of lossless or lossy compression
*              = 1, for lossy compression
*              != 1, for lossless compression
*    ratio   - target compression ratio.  (ratio:1)
*              Used only when ltype == 1.
*    retry   - Pointer to option type.
*              1 = try increasing number of guard bits
*              otherwise, no additional options
*    jpclen  - Number of bytes allocated for new JPEG2000 code stream in
*              outjpc.
*
*   INPUT ARGUMENTS:
*     outjpc - Output encoded JPEG2000 code stream
*
*   RETURN VALUES :
*        > 0 = Length in bytes of encoded JPEG2000 code stream
*         -3 = Error decode jpeg2000 code stream.
*         -5 = decoded image had multiple color components.
*              Only grayscale is expected.
*
* REMARKS:
*
*      Requires OpenJPEG Version 2.
*
* ATTRIBUTES:
*   LANGUAGE: C
*   MACHINE:  Linux
*
*$$$*/
{
    (void) retry;
    int iret = 0;
    unsigned int i;
    const int numcomps = 1;

    opj_codec_t *codec = NULL;
    opj_image_t *image = NULL;
    opj_stream_t *stream = NULL;

    /* set encoding parameters to default values */
    opj_cparameters_t parameters = {0,};	/* compression parameters */
    opj_set_default_encoder_parameters(&parameters);

    parameters.tcp_numlayers  = 1;
    parameters.cp_disto_alloc = 1;
    if (ltype == 1) {
        assert(ratio != 255);
        parameters.tcp_rates[0] = 1.0f/(float)ratio;
    }

    /* By default numresolution = 6 (must be between 1 and 32)
     * This may be too large for some of our datasets, eg. 1xn, so adjust ...
     */
    parameters.numresolution = 6;
    while ( (width < (1 << (parameters.numresolution - 1)) ) ||
            (height < (1 << (parameters.numresolution - 1)) ))
    {
        parameters.numresolution--;
    }

    /* initialize image component */
    opj_image_cmptparm_t cmptparm = {0,};
    cmptparm.prec = (OPJ_UINT32)nbits;
    /* cmptparm.bpp  = (OPJ_UINT32)nbits; */
    cmptparm.sgnd = 0;
    cmptparm.dx   = 1;
    cmptparm.dy   = 1;
    cmptparm.w    = (OPJ_UINT32)width;
    cmptparm.h    = (OPJ_UINT32)height;

    /* create the image */
    image = opj_image_create(numcomps, &cmptparm, OPJ_CLRSPC_GRAY);
    if(!image) {
        iret = -3;
        goto cleanup;
    }
    image->x0 = 0;
    image->y0 = 0;
    image->x1 = (OPJ_UINT32)width;
    image->y1 = (OPJ_UINT32)height;

    assert(cmptparm.prec <= sizeof(image->comps[0].data[0])*8 - 1); /* BR: -1 because I don't know what happens if the sign bit is set */

    for (i=0; i< width*height; i++){
        image->comps[0].data[i] = floor(data[i]+0.5);
    }

    /* get a J2K compressor handle */
    codec = opj_create_compress(OPJ_CODEC_J2K);

    opj_set_info_handler(codec, openjpeg_info, NULL);
    opj_set_warning_handler(codec, openjpeg_warning, NULL);
    opj_set_error_handler(codec, openjpeg_error,NULL);

    /* setup the encoder parameters using the current image and user parameters */
    if (!opj_setup_encoder(codec, &parameters, image)) {
        fprintf(stderr,"openjpeg: failed to setup encoder");
        iret = -3;
        goto cleanup;
    }

    /* open a byte stream for writing */
    opj_memory_stream mstream;
    mstream.pData = (OPJ_UINT8*) outjpc;
    mstream.offset = 0;
    mstream.dataSize = (OPJ_SIZE_T)jpclen;
    stream = opj_stream_create_default_memory_stream(&mstream, OPJ_STREAM_WRITE);
    if (stream == NULL) {
        fprintf(stderr,"openjpeg: failed create default memory stream");
        iret = -3;
        goto cleanup;
    }

    if (!opj_start_compress(codec, image, stream)) {
        fprintf(stderr,"openjpeg: failed to setup encoder");
        iret = -3;
        goto cleanup;
    }

    /* encode image */
    if (!opj_encode(codec, stream)) {
        fprintf(stderr,"openjpeg: opj_encode failed");
        iret = -3;
        goto cleanup;
    }

    if (!opj_end_compress(codec, stream)) {
        fprintf(stderr,"openjpeg: opj_end_compress failed");
        iret = -3;
        goto cleanup;
    }
    iret = (int)mstream.offset;

cleanup:
    if (codec)  opj_destroy_codec(codec);
    if (stream) opj_stream_destroy(stream);
    if (image)  opj_image_destroy(image);

    return iret;
}

#endif
