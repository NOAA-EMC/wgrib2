SHELL=/bin/sh

# splib is called by iplib.  For grib2, the iplib should be compiled in
# double precision, so that means splib should also be in double precision.
# (You could save time by rewritting iplib to call the single precision 
# version of splib.)
#
# To compile splib in double precision ${FTN_REAL8} with the options to
# set the compiler to use real*8 variables
#   gfortran:      FTN_REAL8="-fdefault-real-8 -fdefault-double-8"
#   ifort:         FTN_REAL8="-r8"
#
# problem with .mod can be on the default suffixes and invokes the modula2 compiler
.SUFFIXES:
.SUFFIXES: .F .f .f90 .F90 .o .a

LIB=libsp_v2.0.2_d.a
ARFLAGS=-ruv

OBJ=speps.o  spgradq.o  splegend.o sptezmd.o sptgpm.o  sptgptd.o  sptrand.o  \
sptranfv.o sptrungv.o sptrunsv.o spgradx.o  sppad.o  \
sptezm.o  sptgpmv.o sptgpt.o  sptran.o  sptranv.o  sptrunl.o  sptrunv.o \
ncpus.o spffte.o  spgrady.o  spsynth.o  sptezmv.o sptgpsd.o sptgptsd.o sptranf0.o \
sptrund.o  sptrunm.o  spuv2dz.o spanaly.o splaplac.o sptezd.o  sptezv.o \
sptgps.o  sptgptvd.o sptranf1.o sptrun.o  sptrunmv.o spvar.o spdz2uv.o spfftpt.o \
splat.o   sptez.o  sptgpmd.o sptgpsv.o sptgptv.o  sptranf.o  sptrung.o  sptruns.o \
spwget.o fftpack.o decomp.o solve.o 

# The targets
all:	${LIB}

${LIB}:	$(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)

clean:
	touch junk.o
	-rm -f *.o *.mod *.a

fftpack.o:	fftpack.f
	$(FC) $(FFLAGS) ${FTN_REAL8} ${FTN_LEGACY} $<

.F.o:
	$(FC) $(FFLAGS) ${FTN_REAL8} $(FPPFLAGS) $<

.f.o:
	$(FC) $(FFLAGS) ${FTN_REAL8} $<

