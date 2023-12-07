#include "wgrib2.h"
#include "config.h"

/* dec_png_clone()
 * based on dec_png() from g2clib (public domain)  by Steve Gilbert NCO/NCEP/NWS
 *
 * enc_png() from g2clib is limited to bit depth of 8, 16, 24, 32
 *   and automnatically coverts bit_depth = ((int) (bit_depth + 7) / 8 ) * 8
 * dec_png() assumes a bit depth of 8, 16, 24, 32, and nbytes=bit_depth/8
 *
 * the WMO grib2 specifications mention bit depth of 1, 2, 4, 8, 24, 32.
 *    so that is why dep_png() will fail for bit_depth of 1, 2 and 4
 *
 * v1.0:  copied dec_png() from g2clib, called it dep_png_clone() to avoid name
 *        conflict if also use g2clib
 *
 * 4/2021: v1.1  modification W. Ebisuzaki
 *        need to check if grib2 definition of bit_depth is the same as from
 *          decoding the png stream, otherwise silent bad decode
 *        if differs, then delayed error
 *	  changed char *cout to unsigned char *cout to be consistent with wgrib2
 *        Now handles bit_depth of 1, 2 and 4 as well as 8, 16, 24 and 32.
 *
 */

#ifdef USE_PNG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

extern unsigned int last_message;

struct png_stream {
   unsigned char *stream_ptr;     /*  location to write PNG stream  */
   int stream_len;               /*  number of bytes written       */
};
typedef struct png_stream png_stream;

void user_read_data_clone(png_structp , png_bytep , png_size_t );

void user_read_data_clone(png_structp png_ptr,png_bytep data, png_size_t length)
/*
        Custom read function used so that libpng will read a PNG stream
        from memory instead of a file on disk.
*/
{
     char *ptr;
     int offset;
     png_stream *mem;

     mem=(png_stream *)png_get_io_ptr(png_ptr);
     ptr=(void *)mem->stream_ptr;
     offset=mem->stream_len;
     memcpy(data,ptr+offset,length);
     mem->stream_len += length;
}



int dec_png_clone(unsigned char *pngbuf,int *width,int *height, unsigned char *cout, int *grib2_bit_depth, unsigned int ndata)
{
    int interlace,color,compres,filter,bit_depth;
    int j,k, rowlen, tmp;
    long int rowlen_bits;
    png_structp png_ptr;
    png_infop info_ptr,end_info;
    png_bytepp row_pointers;
    png_stream read_io_ptr;
    png_uint_32 h32, w32;

/*  check if stream is a valid PNG format   */

    if ( png_sig_cmp(pngbuf,0,8) != 0) 
       return (-3);

/* create and initialize png_structs  */

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, 
                                      NULL, NULL);
    if (!png_ptr)
       return (-1);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
       png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
       return (-2);
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
       png_destroy_read_struct(&png_ptr,(png_infopp)info_ptr,(png_infopp)NULL);
       return (-2);
    }

/*     Set Error callback   */

    if (setjmp(png_jmpbuf(png_ptr)))
    {
       png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
       return (-3);
    }

/*    Initialize info for reading PNG stream from memory   */

    read_io_ptr.stream_ptr=(png_voidp)pngbuf;
    read_io_ptr.stream_len=0;

/*    Set new custom read function    */

    png_set_read_fn(png_ptr,(png_voidp)&read_io_ptr,(png_rw_ptr)user_read_data_clone);
/*     png_init_io(png_ptr, fptr);   */

/*     Read and decode PNG stream   */

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

/*     Get pointer to each row of image data   */

    row_pointers = png_get_rows(png_ptr, info_ptr);

/*     Get image info, such as size, depth, colortype, etc...   */

    /*printf("SAGT:png %d %d %d\n",info_ptr->width,info_ptr->height,info_ptr->bit_depth);*/
    // (void)png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)width, (png_uint_32 *)height,
    (void)png_get_IHDR(png_ptr, info_ptr, &w32, &h32,
               &bit_depth, &color, &interlace, &compres, &filter);

    *height = h32;
    *width = w32;
    if ((unsigned int) h32 * (unsigned int) w32 > ndata) fatal_error("png decode: size of png grid too large","");

/*     Check if image was grayscale      */

/*
    if (color != PNG_COLOR_TYPE_GRAY ) {
       fprintf(stderr,"dec_png: Grayscale image was expected. \n");
    }
*/
    if ( color == PNG_COLOR_TYPE_RGB ) {
       bit_depth=24;
    }
    else if ( color == PNG_COLOR_TYPE_RGB_ALPHA ) {
       bit_depth=32;
    }

    if (bit_depth != *grib2_bit_depth) {
	fprintf(stderr, "** DELAYED ERROR: png bit depth error: Sec 5 octet 20 is %d, png lib value (%d) is used **\n",
	    *grib2_bit_depth, bit_depth);
	fprintf(stderr, "** file will be read incorrectly by some grib libraries such as NCEPlib\n");
	fprintf(stderr, "** add -reset_delayed_error option to continue processing\n");
	*grib2_bit_depth = bit_depth;
	last_message |= DELAYED_MISC;
    }

/*     Copy image data to output string   */

    /* get number of bytes per row used to store packed numbers */
    rowlen =  png_get_rowbytes(png_ptr, info_ptr);

    rowlen_bits = w32 * bit_depth;

    // to test bitstream code:  if (bit_depth == 32) {
    // if (rowlen_bits % 8 == 0) {
    // if (bit_depth == 32) {
    if (rowlen_bits % 8 == 0) {
#pragma omp parallel for private(j,k) schedule(static)
        for (j = 0; j < h32; j++) {
            for (k = 0; k < rowlen; k++) {
                cout[j*rowlen+k]=*(row_pointers[j]+k);
            }
        }
    }
    else {
	/* bitstream is set to *cout */
	init_bitstream(cout);
	for (j = 0; j < h32; j++) {
	    rowlen_bits = w32 * bit_depth;
	    k = 0;
	    while (rowlen_bits > 0) {
		tmp = (int) *(row_pointers[j] + k++);
		add_bitstream(tmp, rowlen_bits > 8 ? 8: rowlen_bits);
		rowlen_bits -= 8;
	    }
	}
	finish_bitstream();
    }

/*      Clean up   */

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    return 0;

}
#endif   /* USE_PNG */
