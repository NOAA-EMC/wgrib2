SHELL=/bin/sh

# problem with .mod can be on the default suffixes and invokes the modula2 compiler
.SUFFIXES:
.SUFFIXES: .f .f90 .o .a

LIB=libipolate.a
ARFLAGS=-ruv

# FFLAGS=-O2
# FC=gfortran

# mods=gdswzd00_mod.mod gdswzd01_mod.mod gdswzd03_mod.mod gdswzd04_mod.mod gdswzd05_mod.mod \
# gdswzdcb_mod.mod gdswzdcd_mod.mod gdswzd_mod.mod

objs=gdswzd00_mod.o gdswzd01_mod.o gdswzd03_mod.o gdswzd04_mod.o \
gdswzd05_mod.o gdswzdcb_mod.o gdswzdcd_mod.o gdswzd_mod.o ijkgds0.o ijkgds1.o ipolates.o \
ipolatev.o ipxwafs2.o ipxwafs3.o ipxwafs.o makgds.o movect.o polates0.o polates1.o polates2.o \
polates3.o polates4.o polates6.o polatev0.o polatev1.o polatev2.o polatev3.o polatev4.o \
polatev6.o polfixs.o polfixv.o splat.o

# $(LIB):	$(mods) $(objs)

$(LIB):	$(objs)
	ar $(ARFLAGS) ${LIB} $(objs)

.f90.mod:
	${FC} ${FFLAGS} -c $<

.f90.o:
	${FC} ${FFLAGS} -c $<

.f.o:
	${FC} ${FFLAGS} -c $<

clean:
	touch junk.o junk.mod libipolate.a
	rm *.o *.mod libipolate.a
