ARFLAGS=-ruv

all:	wgrib2lowapi.o wgrib2api.o fort_wgrib2.o libwgrib2_api.a

.c.o:
	$(CC) ${CPPFLAGS} ${CFLAGS} -c $<

wgrib2api.o:	wgrib2lowapi.o wgrib2lowapi.mod wgrib2api.f90
	$(FC) ${FFLAGS} -c wgrib2api.f90

wgrib2lowapi.o:	wgrib2lowapi.f90
	$(FC) ${FFLAGS} -c wgrib2lowapi.f90

wgrib2lowapi.mod:	wgrib2lowapi.f90
	$(FC) ${FFLAGS} -c wgrib2lowapi.f90

libwgrib2_api.a:	wgrib2lowapi.o wgrib2api.o fort_wgrib2.o
	ar ${ARFLAGS} libwgrib2_api.a *.o

clean:
	touch fort_wgrib2.o ; rm fort_wgrib2.o
	touch wgrib2api.o ; rm wgrib2api.o
	touch wgrib2api.mod ; rm wgrib2api.mod
	touch wgrib2lowapi.o ; rm wgrib2lowapi.o
	touch wgrib2lowapi.mod ; rm wgrib2lowapi.mod

