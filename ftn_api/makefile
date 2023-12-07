#
# v1.1 7.2020  WNE: support make -j N makefile, simplify clean
#

ARFLAGS=-ruv

all:	wgrib2lowapi.o wgrib2api.o fort_wgrib2.o libwgrib2_api.a

.c.o:
	$(CC) ${CPPFLAGS} ${CFLAGS} -c $<

wgrib2api.o:	wgrib2lowapi.mod wgrib2api.f90
	$(FC) ${FFLAGS} -c wgrib2api.f90

wgrib2lowapi.o:	wgrib2lowapi.f90
	$(FC) ${FFLAGS} -c wgrib2lowapi.f90

wgrib2lowapi.mod:	wgrib2lowapi.f90 wgrib2lowapi.o
	$(FC) ${FFLAGS} -c wgrib2lowapi.f90

libwgrib2_api.a:	wgrib2lowapi.o wgrib2api.o fort_wgrib2.o
	ar ${ARFLAGS} libwgrib2_api.a *.o

clean:
	touch junk.o junk.mod junk.a ; rm *.o *.mod *.a
