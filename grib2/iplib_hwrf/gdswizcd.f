      SUBROUTINE GDSWIZCD(KGDS,IOPT,NPTS,FILL,XPTS,YPTS,RLON,RLAT,NRET,
     &                    LROT,CROT,SROT)
C$$$  SUBPROGRAM DOCUMENTATION BLOCK
C
C SUBPROGRAM:  GDSWIZCD   GDS WIZARD FOR ROTATED EQUIDISTANT CYLINDRICAL
C   PRGMMR: GAYNO       ORG: W/NMC23       DATE: 2007-NOV-15
C
C ABSTRACT: THIS SUBPROGRAM DECODES THE GRIB GRID DESCRIPTION SECTION
C           (PASSED IN INTEGER FORM AS DECODED BY SUBPROGRAM W3FI63)
C           AND RETURNS ONE OF THE FOLLOWING:
C             (IOPT=+1) EARTH COORDINATES OF SELECTED GRID COORDINATES
C             (IOPT=-1) GRID COORDINATES OF SELECTED EARTH COORDINATES
C           FOR NON-"E" STAGGERED ROTATED EQUIDISTANT CYLINDRICAL PROJECTIONS.
C           (MASS OR VELOCITY POINTS.)
C           IF THE SELECTED COORDINATES ARE MORE THAN ONE GRIDPOINT
C           BEYOND THE THE EDGES OF THE GRID DOMAIN, THEN THE RELEVANT
C           OUTPUT ELEMENTS ARE SET TO FILL VALUES.
C           THE ACTUAL NUMBER OF VALID POINTS COMPUTED IS RETURNED TOO.
C
C PROGRAM HISTORY LOG:
C 2010-JAN-15  GAYNO     BASED ON ROUTINES GDSWIZCB AND GDSWIZCA
C
C USAGE:    CALL GDSWIZCD(KGDS,IOPT,NPTS,FILL,XPTS,YPTS,RLON,RLAT,NRET,
C     &                   LROT,CROT,SROT)
C
C   INPUT ARGUMENT LIST:
C     KGDS     - INTEGER (200) GDS PARAMETERS AS DECODED BY W3FI63
C     IOPT     - INTEGER OPTION FLAG
C                (+1 TO COMPUTE EARTH COORDS OF SELECTED GRID COORDS)
C                (-1 TO COMPUTE GRID COORDS OF SELECTED EARTH COORDS)
C     NPTS     - INTEGER MAXIMUM NUMBER OF COORDINATES
C     FILL     - REAL FILL VALUE TO SET INVALID OUTPUT DATA
C                (MUST BE IMPOSSIBLE VALUE; SUGGESTED VALUE: -9999.)
C     XPTS     - REAL (NPTS) GRID X POINT COORDINATES IF IOPT>0
C     YPTS     - REAL (NPTS) GRID Y POINT COORDINATES IF IOPT>0
C     RLON     - REAL (NPTS) EARTH LONGITUDES IN DEGREES E IF IOPT<0
C                (ACCEPTABLE RANGE: -360. TO 360.)
C     RLAT     - REAL (NPTS) EARTH LATITUDES IN DEGREES N IF IOPT<0
C                (ACCEPTABLE RANGE: -90. TO 90.)
C     LROT     - INTEGER FLAG TO RETURN VECTOR ROTATIONS IF 1
C
C   OUTPUT ARGUMENT LIST:
C     XPTS     - REAL (NPTS) GRID X POINT COORDINATES IF IOPT<0
C     YPTS     - REAL (NPTS) GRID Y POINT COORDINATES IF IOPT<0
C     RLON     - REAL (NPTS) EARTH LONGITUDES IN DEGREES E IF IOPT>0
C     RLAT     - REAL (NPTS) EARTH LATITUDES IN DEGREES N IF IOPT>0
C     NRET     - INTEGER NUMBER OF VALID POINTS COMPUTED
C     CROT     - REAL (NPTS) CLOCKWISE VECTOR ROTATION COSINES IF LROT=1
C     SROT     - REAL (NPTS) CLOCKWISE VECTOR ROTATION SINES IF LROT=1
C                (UGRID=CROT*UEARTH-SROT*VEARTH;
C                 VGRID=SROT*UEARTH+CROT*VEARTH)
C
C ATTRIBUTES:
C   LANGUAGE: FORTRAN 77
C
C$$$
      IMPLICIT NONE
      INTEGER,INTENT(IN):: IOPT,KGDS(200),NPTS,LROT
      INTEGER,INTENT(OUT):: NRET
      REAL,INTENT(IN):: FILL
      REAL,INTENT(INOUT):: XPTS(NPTS),YPTS(NPTS),RLON(NPTS),RLAT(NPTS)
      REAL,INTENT(OUT):: CROT(NPTS),SROT(NPTS)
      INTEGER IROT,IM,JM,ISCAN,JSCAN,N
      REAL XMIN,XMAX,YMIN,YMAX
      INTEGER,PARAMETER:: KD=SELECTED_REAL_KIND(15,45)
      REAL(KIND=KD):: HI,HJ,HS,HS2,PI,DPR
      REAL(KIND=KD):: RLAT1,RLON1,RLAT0,RLON0,RLAT2,RLON2
      REAL(KIND=KD):: SLAT1,CLAT1,SLAT0,CLAT0,SLAT2,CLAT2,CLON2
      REAL(KIND=KD):: CLON1,SLATR,CLATR,CLONR,SLON
      REAL(KIND=KD):: RLATR,RLONR,DLATS,DLONS
      REAL(KIND=KD):: SLAT,CLAT,CLON,WBD,SBD,NBD,EBD
      PARAMETER(PI=3.14159265358979_KD,DPR=180._KD/PI)
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      IF(KGDS(1).EQ.205) THEN
        RLAT1=KGDS(4)*1.E-3_KD
        RLON1=KGDS(5)*1.E-3_KD
        RLAT0=KGDS(7)*1.E-3_KD
        RLON0=KGDS(8)*1.E-3_KD
        RLAT2=KGDS(12)*1.E-3_KD
        RLON2=KGDS(13)*1.E-3_KD
        IROT=MOD(KGDS(6)/8,2)
        IM=KGDS(2)
        JM=KGDS(3)
        ISCAN=MOD(KGDS(11)/128,2)
        JSCAN=MOD(KGDS(11)/64,2)
        HI=(-1.)**ISCAN
        HJ=(-1.)**(1-JSCAN)
        SLAT1=SIN(RLAT1/DPR)
        CLAT1=COS(RLAT1/DPR)
        SLAT0=SIN(RLAT0/DPR)
        CLAT0=COS(RLAT0/DPR)
        HS=SIGN(1._KD,MOD(RLON1-RLON0+180+3600,360._KD)-180)
        CLON1=COS((RLON1-RLON0)/DPR)
        SLATR=CLAT0*SLAT1-SLAT0*CLAT1*CLON1
        CLATR=SQRT(1-SLATR**2)
        CLONR=(CLAT0*CLAT1*CLON1+SLAT0*SLAT1)/CLATR
        RLATR=DPR*ASIN(SLATR)
        RLONR=HS*DPR*ACOS(CLONR)
        WBD=RLONR
        SBD=RLATR
        SLAT2=SIN(RLAT2/DPR)
        CLAT2=COS(RLAT2/DPR)
        HS2=SIGN(1._KD,MOD(RLON2-RLON0+180+3600,360._KD)-180)
        CLON2=COS((RLON2-RLON0)/DPR)
        SLATR=CLAT0*SLAT2-SLAT0*CLAT2*CLON2
        CLATR=SQRT(1-SLATR**2)
        CLONR=(CLAT0*CLAT2*CLON2+SLAT0*SLAT2)/CLATR
        NBD=DPR*ASIN(SLATR)
        EBD=HS2*DPR*ACOS(CLONR)
        DLATS=(NBD-SBD)/FLOAT(JM-1)
        DLONS=(EBD-WBD)/FLOAT(IM-1)
        XMIN=0
        XMAX=IM+1
        YMIN=0
        YMAX=JM+1
        NRET=0
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C  TRANSLATE GRID COORDINATES TO EARTH COORDINATES
        IF(IOPT.EQ.0.OR.IOPT.EQ.1) THEN
          DO N=1,NPTS
            IF(XPTS(N).GE.XMIN.AND.XPTS(N).LE.XMAX.AND.
     &         YPTS(N).GE.YMIN.AND.YPTS(N).LE.YMAX) THEN
              RLONR=WBD+(XPTS(N)-1._KD)*DLONS
              RLATR=SBD+(YPTS(N)-1._KD)*DLATS
              IF(RLONR <= 0._KD) THEN
                HS=-HI
              ELSE
                HS=HI
              ENDIF
              CLONR=COS(RLONR/DPR)
              SLATR=SIN(RLATR/DPR)
              CLATR=COS(RLATR/DPR)
              SLAT=CLAT0*SLATR+SLAT0*CLATR*CLONR
              IF(SLAT.LE.-1) THEN
                CLAT=0.
                CLON=COS(RLON0/DPR)
                RLON(N)=0
                RLAT(N)=-90
              ELSEIF(SLAT.GE.1) THEN
                CLAT=0.
                CLON=COS(RLON0/DPR)
                RLON(N)=0
                RLAT(N)=90
              ELSE
                CLAT=SQRT(1-SLAT**2)
                CLON=(CLAT0*CLATR*CLONR-SLAT0*SLATR)/CLAT
                CLON=MIN(MAX(CLON,-1._KD),1._KD)
                RLON(N)=MOD(RLON0+HS*DPR*ACOS(CLON)+3600,360._KD)
                RLAT(N)=DPR*ASIN(SLAT)
              ENDIF
              NRET=NRET+1
              IF(LROT.EQ.1) THEN
                IF(IROT.EQ.1) THEN
                  IF(CLATR.LE.0) THEN
                    CROT(N)=-SIGN(1._KD,SLATR*SLAT0)
                    SROT(N)=0
                  ELSE
                    SLON=SIN((RLON(N)-RLON0)/DPR)
                    CROT(N)=(CLAT0*CLAT+SLAT0*SLAT*CLON)/CLATR
                    SROT(N)=SLAT0*SLON/CLATR
                  ENDIF
                ELSE
                  CROT(N)=1
                  SROT(N)=0
                ENDIF
              ENDIF
            ELSE
              RLON(N)=FILL
              RLAT(N)=FILL
            ENDIF
          ENDDO
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C  TRANSLATE EARTH COORDINATES TO GRID COORDINATES
        ELSEIF(IOPT.EQ.-1) THEN
          DO N=1,NPTS
            IF(ABS(RLON(N)).LE.360.AND.ABS(RLAT(N)).LE.90) THEN
              HS=SIGN(1._KD,MOD(RLON(N)-RLON0+180+3600,360._KD)-180)
              CLON=COS((RLON(N)-RLON0)/DPR)
              SLAT=SIN(RLAT(N)/DPR)
              CLAT=COS(RLAT(N)/DPR)
              SLATR=CLAT0*SLAT-SLAT0*CLAT*CLON
              IF(SLATR.LE.-1) THEN
                CLATR=0.
                RLONR=0
                RLATR=-90
              ELSEIF(SLATR.GE.1) THEN
                CLATR=0.
                RLONR=0
                RLATR=90
              ELSE
                CLATR=SQRT(1-SLATR**2)
                CLONR=(CLAT0*CLAT*CLON+SLAT0*SLAT)/CLATR
                CLONR=MIN(MAX(CLONR,-1._KD),1._KD)
                RLONR=HS*DPR*ACOS(CLONR)
                RLATR=DPR*ASIN(SLATR)
              ENDIF
              XPTS(N)=(RLONR-WBD)/DLONS+1._KD
              YPTS(N)=(RLATR-SBD)/DLATS+1._KD
              IF(XPTS(N).GE.XMIN.AND.XPTS(N).LE.XMAX.AND.
     &           YPTS(N).GE.YMIN.AND.YPTS(N).LE.YMAX) THEN
                NRET=NRET+1
                IF(LROT.EQ.1) THEN
                  IF(IROT.EQ.1) THEN
                    IF(CLATR.LE.0) THEN
                      CROT(N)=-SIGN(1._KD,SLATR*SLAT0)
                      SROT(N)=0
                    ELSE
                      SLON=SIN((RLON(N)-RLON0)/DPR)
                      CROT(N)=(CLAT0*CLAT+SLAT0*SLAT*CLON)/CLATR
                      SROT(N)=SLAT0*SLON/CLATR
                    ENDIF
                  ELSE
                    CROT(N)=1
                    SROT(N)=0
                  ENDIF
                ENDIF
              ELSE
                XPTS(N)=FILL
                YPTS(N)=FILL
              ENDIF
            ELSE
              XPTS(N)=FILL
              YPTS(N)=FILL
            ENDIF
          ENDDO
        ENDIF
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C  PROJECTION UNRECOGNIZED
      ELSE
        IF(IOPT.GE.0) THEN
          DO N=1,NPTS
            RLON(N)=FILL
            RLAT(N)=FILL
          ENDDO
        ENDIF
        IF(IOPT.LE.0) THEN
          DO N=1,NPTS
            XPTS(N)=FILL
            YPTS(N)=FILL
          ENDDO
        ENDIF
      ENDIF
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      END
