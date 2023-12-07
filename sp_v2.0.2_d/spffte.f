C-----------------------------------------------------------------------
      SUBROUTINE SPFFTE(IMAX,INCW,INCG,KMAX,W,G,IDIR,AFFT)
C$$$  SUBPROGRAM DOCUMENTATION BLOCK
C
C SUBPROGRAM:  SPFFTE     PERFORM MULTIPLE FAST FOURIER TRANSFORMS
C   PRGMMR: IREDELL       ORG: W/NMC23       DATE: 96-02-20
C
C ABSTRACT: THIS SUBPROGRAM PERFORMS MULTIPLE FAST FOURIER TRANSFORMS
C           BETWEEN COMPLEX AMPLITUDES IN FOURIER SPACE AND REAL VALUES
C           IN CYCLIC PHYSICAL SPACE.
C           SUBPROGRAM SPFFTE MUST BE INVOKED FIRST WITH IDIR=0
C           TO INITIALIZE TRIGONEMETRIC DATA.  USE SUBPROGRAM SPFFT1
C           TO PERFORM AN FFT WITHOUT PREVIOUS INITIALIZATION.
C           THIS VERSION INVOKES THE IBM ESSL FFT. (now fftpack)
C
C PROGRAM HISTORY LOG:
C 1998-12-18  IREDELL
C 2012-11-12  MIRVIS -fixing hard-wired types problem on Intel/Linux 
C 2020-09-18  EBISUZAKI, changed to directly call fftpack
C
C USAGE:    CALL SPFFTE(IMAX,INCW,INCG,KMAX,W,G,IDIR,AFFT)
C
C   INPUT ARGUMENT LIST:
C     IMAX     - INTEGER NUMBER OF VALUES IN THE CYCLIC PHYSICAL SPACE
C                (SEE LIMITATIONS ON IMAX IN REMARKS BELOW.)
C     INCW     - INTEGER FIRST DIMENSION OF THE COMPLEX AMPLITUDE ARRAY
C                (INCW >= IMAX/2+1)
C     INCG     - INTEGER FIRST DIMENSION OF THE REAL VALUE ARRAY
C                (INCG >= IMAX)
C     KMAX     - INTEGER NUMBER OF TRANSFORMS TO PERFORM
C     W        - COMPLEX(INCW,KMAX) COMPLEX AMPLITUDES IF IDIR>0
C     G        - REAL(INCG,KMAX) REAL VALUES IF IDIR<0
C     IDIR     - INTEGER DIRECTION FLAG
C                IDIR=0 TO INITIALIZE TRIGONOMETRIC DATA
C                IDIR>0 TO TRANSFORM FROM FOURIER TO PHYSICAL SPACE
C                IDIR<0 TO TRANSFORM FROM PHYSICAL TO FOURIER SPACE
C     AFFT       REAL(8) (50000+4*IMAX) AUXILIARY ARRAY IF IDIR<>0
C                note: should be real afft(15+2*imax)
C
C   OUTPUT ARGUMENT LIST:
C     W        - COMPLEX(INCW,KMAX) COMPLEX AMPLITUDES IF IDIR<0
C     G        - REAL(INCG,KMAX) REAL VALUES IF IDIR>0
C     AFFT       REAL(8) (50000+4*IMAX) AUXILIARY ARRAY IF IDIR=0
C
C SUBPROGRAMS CALLED:
C   rffti        fftpack initialization
C   rfftb        fftpack
C   rfftf        fftpack
C
C ATTRIBUTES:
C   LANGUAGE: FORTRAN 90
C
C REMARKS:
C   THE RESTRICTIONS ON IMAX ARE THAT IT MUST BE A MULTIPLE
C   OF 1 TO 25 FACTORS OF TWO, UP TO 2 FACTORS OF THREE,
C   AND UP TO 1 FACTOR OF FIVE, SEVEN AND ELEVEN.
C
C   IF IDIR=0, THEN W AND G NEED NOT CONTAIN ANY VALID DATA.
C   THE OTHER PARAMETERS MUST BE SUPPLIED AND CANNOT CHANGE
C   IN SUCCEEDING CALLS UNTIL THE NEXT TIME IT IS CALLED WITH IDIR=0.
C
C   THIS SUBPROGRAM IS THREAD-SAFE.
C
C$$$
        IMPLICIT NONE
        INTEGER,INTENT(IN):: IMAX,INCW,INCG,KMAX,IDIR
        REAL,INTENT(INOUT):: W(2*INCW,KMAX)
        REAL,INTENT(INOUT):: G(INCG,KMAX)
        REAL(8),INTENT(INOUT):: AFFT(50000+4*IMAX)
        INTEGER:: i, j
        REAL:: SCALE
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C  INITIALIZATION.
C  FILL AUXILIARY ARRAYS WITH TRIGONOMETRIC DATA
        SELECT CASE(IDIR)
          CASE(0)

C           size: AFFT(2*imax+15)
            CALL rffti(imax,AFFT)
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C  FOURIER TO PHYSICAL TRANSFORM.
          CASE(1:)
            do j = 1, kmax
               g(1,j) = w(1,j)
               do i = 2, imax
                  g(i,j) = w(i+1,j)
               enddo
               CALL rfftb(imax,g(1,j),AFFT)
            enddo

C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C  PHYSICAL TO FOURIER TRANSFORM.
          CASE(:-1)
            SCALE=1./IMAX
            do j = 1, kmax
               do i = 1, imax
                  w(i,j) = g(i,j)
               enddo
               CALL rfftf(imax,w(1,j),AFFT)
               do i = 1, imax
                  w(i,j) = w(i,j)*scale
               enddo
               do i = imax, 2, -1
                  w(i+1,j) = w(i,j)
               enddo
               w(2,j) = 0.0
               w(imax+2,j) = 0.0
            enddo
        END SELECT
      END SUBROUTINE
