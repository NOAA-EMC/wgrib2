# REQUIRES GMAKE!!!!
#
# wgrib2 uses components of varying copyrights and licences.  See wgrib2/LICENSE-wgrib2
#
# makefile for grib2/lib/wgrib2api.lib
# 
SHELL=/bin/sh
# SHELL=/bin/ksh
.SUFFIXES: .x

# list of all libraries
libs:=$(wildcard *.a)

# remove libwgrib2.a
libs:=$(libs:libwgrib2.a=)

# .a -> .x
libs:=$(libs:.a=.x)
libsdir:=$(libs:.x=)

all:	${libs}
	$(AR) cr libwgrib2.a */*.o
	rm */*.o *.x
	rm -r ${libsdir}

clean:
	touch junk.la ; rm *.la
	touch *.a ; rm *.a
	touch junk.x ; rm  *.x
	touch junk.mod ; rm *.mod
	touch junk.h ; rm *.h
	mkdir -p pkgconfig && rm -r pkgconfig

.a.x:   $(*).a
	mkdir -p $(*) && touch $(*)/junk && rm -r $(*)/*
	cd $(*) && $(AR) x ../$(*).a
	touch $*.x
