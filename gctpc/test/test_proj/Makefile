#
# File to make is the testproj executable
#
exe:	testproj	

saber:
	#load $(SOURCE) \
		$(OPTS) \
		$(OBJLIB) \
		$(TRLIB) -lm \
	#link
#
# Set up build options--the debug switch (debug or optimize), 
# and directories to search for include files
#
OPTS = $(CFLAGS)     \
	-I$(SRCGCTP)

#
# Include files needed for testproj.  The source code depends on
# these files.  Therefore, if one of the include files are
# modified, the module will be recompiled.
#
CINCFIL = \
	$(SRCGCTP)/cproj.h   \
	$(SRCGCTP)/proj.h

OBJLIB = \
	$(LIBGCTP)/geolib.a

#
# Object libraries required to load the program
#

#
# directives for the release procedure.  This identifies 
# which files are to be treated as source files, executable 
# files, and misc. files which are in both the 
# release directory and source directory
#
SOURCE = testproj.c Makefile
EXECS  = testproj 

# qz

# Make the executable file for program "testproj".
testproj: 	testproj.c $(OBJLIB) $(CINCFIL)
		cc $(OPTS) \
		-o testproj testproj.c  \
		$(OBJLIB) \
		$(TRLIB) -lm
#
#
# qz
