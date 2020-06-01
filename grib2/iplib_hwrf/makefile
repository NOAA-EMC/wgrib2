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

LIB=libipolate.a
ARFLAGS=-ruv

# FFLAGS=-O2
FC=gfortran

objs=gausslat.o gcdist.o gdsawt.o gdswiz00.o gdswiz01.o gdswiz03.o gdswiz04.o gdswiz05.o \
gdswizc9.o gdswizca.o gdswizcb.o gdswiz.o gdswzd00.o gdswzd01.o gdswzd03.o gdswzd04.o \
gdswzd05.o gdswzdc9.o gdswzdca.o gdswzdcb.o gdswzd.o ijkgds0.o ijkgds1.o ijkgds.o ipmerge2.o \
ipolates.o ipolatev.o ipsector.o ipspaste.o ipxetas.o ipxwafs2.o ipxwafs.o makgds.o movect.o \
polateg0.o polateg1.o polateg4.o polates0.o polates1.o polates2.o polates3.o polates4.o \
polates6.o polatev0.o polatev1.o polatev2.o polatev3.o polatev4.o polatev6.o polfixs.o polfixv.o splat.o \
gdswizcd.o  gdswzdcd.o

$(LIB):	$(objs)
	ar $(ARFLAGS) ${LIB} $(objs)

.f.o:
	$(FC) ${FFLAGS} -c $<

clean:
	rm junk.o
	rm *.o
