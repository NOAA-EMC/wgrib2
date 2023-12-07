SHELL=/bin/sh

#  If you want to enable support for PNG or JPEG2000 encoding/decoding,
#  you must specify -DUSE_PNG and/or -DUSE_JPEG2000 in the DEFS variable
#  for the C pre-processor
#  -DUSE_PNG requires libpng.a and libz.a
#                     ( and png.h pngconf.h zconf.h zlib.h include files).
#  -DUSE_JPEG2000 requires libjasper.a
#                     ( and all the jasper/*.h include files).
#

DEFS=-DUSE_JPEG2000 -DUSE_PNG

#
#   This "C" source code contains many uses of the C++
#   comment style "//".  Please make sure you include the
#   appropriate compiler flag to allow the use of "//" comment indicators.
#
# CFLAGS= ${CPPFLAGS} $(DEFS)

LIB=libgrib2c.a
ARFLAGS=

all: $(LIB)

$(LIB)(gridtemplates.o): gridtemplates.h

$(LIB)(pdstemplates.o): pdstemplates.h

$(LIB)(drstemplates.o): drstemplates.h

$(LIB):	$(LIB)(gridtemplates.o) \
	$(LIB)(drstemplates.o) \
	$(LIB)(pdstemplates.o) \
	$(LIB)(gbits.o) \
	$(LIB)(g2_unpack1.o) \
	$(LIB)(g2_unpack2.o) \
	$(LIB)(g2_unpack3.o) \
	$(LIB)(g2_unpack4.o) \
	$(LIB)(g2_unpack5.o) \
	$(LIB)(g2_unpack6.o) \
	$(LIB)(g2_unpack7.o) \
	$(LIB)(g2_free.o) \
	$(LIB)(g2_info.o) \
	$(LIB)(g2_getfld.o) \
	$(LIB)(simunpack.o) \
	$(LIB)(comunpack.o) \
        $(LIB)(pack_gp.o) \
        $(LIB)(reduce.o) \
	$(LIB)(specpack.o) \
	$(LIB)(specunpack.o) \
	$(LIB)(rdieee.o) \
	$(LIB)(mkieee.o) \
	$(LIB)(int_power.o) \
	$(LIB)(simpack.o) \
	$(LIB)(compack.o) \
        $(LIB)(cmplxpack.o) \
	$(LIB)(misspack.o) \
	$(LIB)(jpcpack.o) \
	$(LIB)(jpcunpack.o) \
	$(LIB)(pngpack.o) \
	$(LIB)(pngunpack.o) \
	$(LIB)(dec_jpeg2000.o) \
	$(LIB)(enc_jpeg2000.o) \
	$(LIB)(dec_png.o) \
	$(LIB)(enc_png.o) \
	$(LIB)(g2_create.o) \
	$(LIB)(g2_addlocal.o) \
	$(LIB)(g2_addgrid.o) \
	$(LIB)(g2_addfield.o) \
	$(LIB)(g2_gribend.o) \
	$(LIB)(getdim.o) \
	$(LIB)(g2_miss.o) \
	$(LIB)(getpoly.o) \
	$(LIB)(seekgb.o)
#	$(LIB)(getfield.o) \
#	$(LIB)(gettemplates.o) \
#	$(LIB)(getlocal.o) \
#	$(LIB)(gribinfo.o) \
#	$(LIB)(ixgb2.o) \
#	$(LIB)(getg2i.o) \
#	$(LIB)(getg2ir.o) \
#	$(LIB)(getgb2s.o) \
#	$(LIB)(getgb2r.o) \
#	$(LIB)(getgb2l.o) \
#	$(LIB)(getgb2.o) \
#	$(LIB)(putgb2.o) \
#	$(LIB)(params.o)

.c.a:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) ${DEFS} $<
	ar $(ARFLAGS) -ruv $@ $*.o
	rm -f $*.o

clean:
	touch junk.o ; rm *.o
	touch junk.a ; rm *.a
