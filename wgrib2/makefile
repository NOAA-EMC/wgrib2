#
# this makefile is for gnu-make on a linux box
#
#  REQUIRES GNU make
# mod 1/07 M. Schwarb
# mod 2/07 W. Ebisuzaki changes for *.dat files
# mod 8/07 W. Ebisuzaki cleanup
# mod 4/09 W. Ebisuzaki config.h, netcdf4
# mod 3/12 W. Ebisuzake openmp, gctpc

SHELL=/bin/sh


all:=$(patsubst %.c,%.o,$(wildcard *.c))
code:=$(filter-out fnlist.o,$(all))
gribtables=$(wildcard gribtables/*/*gribtable.dat)
o=$(wildcard *.o)
h:=grb2.h  wgrib2.h config.h
options=$(wildcard [A-Z]*.c)
CODE_TABLE_DAT=$(wildcard CodeTable_[0-9].[0-9]*.dat)
FLAGS:=${CPPFLAGS} ${CFLAGS}
ARFLAGS=-ruv

wgrib2: $h fnlist.c fnlist.h ${all}
	${CC} -o wgrib2 ${FLAGS} ${all} ${LDFLAGS}
	rm Config.o

fast:	${code}
	touch fnlist.o fnlist.c
	${CC} -o wgrib2 ${FLAGS} ${all} ${LDFLAGS}
	rm Config.o

lib:	$h fnlist.c fnlist.h ${all}
	touch libwgrib2.a ; rm libwgrib2.a
	${CC} -c ${FLAGS} -DCALLABLE_WGRIB2 wgrib2.c
	${CC} -c ${FLAGS} -DCALLABLE_WGRIB2 fatal_error.c
	${CC} -c ${FLAGS} -DCALLABLE_WGRIB2 wgrib2_api.c
	ar ${ARFLAGS} libwgrib2.a *.o
	rm wgrib2.o fatal_error.o wgrib2_api.o

fnlist.c fnlist.h:	${options}
	./function.sh 

Help.o:	Help.c wgrib2.h
	${CC} -c ${FLAGS} Help.c

CodeTable.o:	CodeTable.c ${CODE_TABLE_DAT}
	${CC} -c ${FLAGS} CodeTable.c

cname.o:	cname.c gribtab.c
	${CC} -c ${FLAGS} cname.c

Sec1.o:	Sec1.c code_table0.dat ncep_tableC.dat
	${CC} -c ${FLAGS} Sec1.c

gribtab.o:	gribtab.c ${gribtables}
	${CC} -c ${FLAGS} gribtab.c

.c.o:	$(*).c $h fnlist.c
	$(CC) -c ${FLAGS}  $*.c

clean:
	touch wgrib2
	rm ${o} wgrib2
