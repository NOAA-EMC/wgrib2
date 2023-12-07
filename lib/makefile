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
# remove libpng.a - duplicated libpng12
libs:=$(libs:libpng.a=)

# .a -> .x
libs:=$(libs:.a=.x)
libsdir:=$(libs:.x=)

all:	${libs}
	touch libpng.a ; rm libpng.*
	$(AR) cr libwgrib2.a */*.o
ifeq ($(MAKE_SHARED_LIB),gnu_linux)
	${CC} -shared ${CPPFLAGS} -o libwgrib2.so  */*.o ${LDFLAGS} 
else ifeq ($(MAKE_SHARED_LIB),clang_linux)
	${CC} -shared ${CPPFLAGS} -o libwgrib2.so */*.o ${LDFLAGS} 
else ifeq ($(MAKE_SHARED_LIB),nvidia_linux)
	${CC} -shared ${CPPFLAGS} -o libwgrib2.so */*.o ${LDFLAGS} 
else ifeq ($(MAKE_SHARED_LIB),oneapi_linux)
	${CC} -shared ${CPPFLAGS} -o libwgrib2.so */*.o ${LDFLAGS} 
else ifeq ($(MAKE_SHARED_LIB),gnu_mac)
	${CC} -dynamiclib ${CPPFLAGS} -o libwgrib2.dylib  */*.o ${LDFLAGS} 
else ifeq ($(MAKE_SHARED_LIB),cygwin_win)
	${CC} -shared ${CPPFLAGS} -o libwgrib2.so */*.o ${LDFLAGS} 
endif
	rm */*.o *.x
	rm -r ${libsdir}

clean:
	touch junk.x junk.so junk.dynlib junk.mod junk.la junk.a
	rm  *.x *.so *.dynlib *.mod *.la *.a
	mkdir -p pkgconfig && rm -r pkgconfig
	touch libjunk ; rm -r lib*

.a.x:   $(*).a
	mkdir -p $(*) && touch $(*)/junk && rm -r $(*)/*
	cd $(*) && $(AR) x ../$(*).a
	touch $*.x
