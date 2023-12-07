C-----------------------------------------------------------------------
      FUNCTION NCPUS()
C$$$  SUBPROGRAM DOCUMENTATION BLOCK
C
C SUBPROGRAM: NCPUS          SET NUMBER OF CPUS
C   PRGMMR: IREDELL          ORG: W/NMC23     DATE: 94-08-19
C
C ABSTRACT: SET NUMBER OF CPUS 
C   DESIGNATING THE NUMBER OF PROCESSORS OVER WHICH TO PARALLELIZE.
C
C PROGRAM HISTORY LOG:
C   94-08-19  IREDELL
C   98-11-09  VUONG     ADD DOC BLOCK AND REMOVE CRAY REFERENCES
C 1998-12-18  IREDELL   IBM SMP VERSION
C 2010-11-16  SLOVACEK  LINUX MUST HAVE DIFFERENT CALL
C 2012-11-01  MIRVIS    Multi- threading on LINUX-IBM/TIDE
C 2019-02-01  EBISUZAKI replaced code by call to omp_get_max_threads
C                       assumes set_num_threads() is not used .. checked 4/2020
C 2020-04029  EBISUZAKI works if openmp not used
C
C USAGE:    NC=NCPUS()
C   OUTPUT ARGUMENTS:
C     NCPUS        INTEGER NUMBER OF CPUS
C
C ATTRIBUTES:
C   LANGUAGE: FORTRAN
C
C$$$
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _OPENMP
      INTEGER omp_get_max_threads
      NCPUS = omp_get_max_threads()
#else
      NCPUS=1
#endif
      RETURN
      END
