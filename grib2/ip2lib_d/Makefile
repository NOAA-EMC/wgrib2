# Compiler substitution variables
# FC       = gfortran
# FCFLAGS  = -g -O2 -c
FPPFLAGS  = -cpp -DLSIZE=8
# AR       = ar
# ARFLAGS  = crvs

# The library name
LIBRARY = libip2_d.a

# The file definitions. This include must occur before targets.
include make.filelist

# The targets
all:	${LIBRARY}

${LIBRARY}:	$(OBJ_FILES)
	$(AR) $(ARFLAGS) $@ $(OBJ_FILES)
        
clean:
	-rm -f *.i90 *.i *.o *.mod *.a

# Suffix rules
.SUFFIXES:
.SUFFIXES: .F90 .f90 .o
.F90.o:
	$(FC) $(FFLAGS) ${FTN_REAL8} $(FPPFLAGS) $<

.f90.o:
	$(FC) $(FFLAGS) ${FTN_REAL8} $<
