C-----------------------------------------------------------------------
      SUBROUTINE GDSWZDCB(KGDS,IOPT,NPTS,FILL,XPTS,YPTS,RLON,RLAT,NRET,
     &                    LROT,CROT,SROT,LMAP,XLON,XLAT,YLON,YLAT,AREA)
C$$$  SUBPROGRAM DOCUMENTATION BLOCK
C
C SUBPROGRAM:  GDSWZDCB   GDS WIZARD FOR ROTATED EQUIDISTANT CYLINDRICAL
C   PRGMMR: IREDELL       ORG: W/NMC23       DATE: 96-04-10
C
C ABSTRACT: THIS SUBPROGRAM DECODES THE GRIB GRID DESCRIPTION SECTION
C           (PASSED IN INTEGER FORM AS DECODED BY SUBPROGRAM W3FI63)
C           AND RETURNS ONE OF THE FOLLOWING:
C             (IOPT=+1) EARTH COORDINATES OF SELECTED GRID COORDINATES
C             (IOPT=-1) GRID COORDINATES OF SELECTED EARTH COORDINATES
C           FOR STAGGERED ROTATED EQUIDISTANT CYLINDRICAL PROJECTIONS.
C           (SEE UNDER THE DESCRIPTION OF KGDS TO DETERMINE WHETHER
C           TO COMPUTE A STAGGERED WIND GRID OR A STAGGERED MASS GRID.)
C           IF THE SELECTED COORDINATES ARE MORE THAN ONE GRIDPOINT
C           BEYOND THE THE EDGES OF THE GRID DOMAIN, THEN THE RELEVANT
C           OUTPUT ELEMENTS ARE SET TO FILL VALUES.
C           THE ACTUAL NUMBER OF VALID POINTS COMPUTED IS RETURNED TOO.
C           OPTIONALLY, THE VECTOR ROTATIONS AND THE MAP JACOBIANS
C           FOR THIS GRID MAY BE RETURNED AS WELL.
C
C PROGRAM HISTORY LOG:
C   96-04-10  IREDELL
C   97-10-20  IREDELL  INCLUDE MAP OPTIONS
C   98-08-19  BALDWIN  MODIFY GDSWZDC9 FOR TYPE 203 ETA GRIDS
C 2003-06-11  IREDELL    INCREASE PRECISION
C
C USAGE:    CALL GDSWZDCB(KGDS,IOPT,NPTS,FILL,XPTS,YPTS,RLON,RLAT,NRET,
C    &                    LROT,CROT,SROT,LMAP,XLON,XLAT,YLON,YLAT,AREA)
C
C   INPUT ARGUMENT LIST:
C     KGDS     - INTEGER (200) GDS PARAMETERS AS DECODED BY W3FI63
C                IMPORTANT NOTE: IF THE 9TH BIT (FROM RIGHT) OF KGDS(11)
C                                (SCANNING MODE FLAG) IS 1, THEN THIS
C                                THE GRID IS COMPUTED FOR A WIND FIELD;
C                                OTHERWISE IT IS FOR A MASS FIELD.  THUS
C                                MOD(KGDS(11)/256,2)=0 FOR MASS GRID.
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
C     LMAP     - INTEGER FLAG TO RETURN MAP JACOBIANS IF 1
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
C     XLON     - REAL (NPTS) DX/DLON IN 1/DEGREES IF LMAP=1
C     XLAT     - REAL (NPTS) DX/DLAT IN 1/DEGREES IF LMAP=1
C     YLON     - REAL (NPTS) DY/DLON IN 1/DEGREES IF LMAP=1
C     YLAT     - REAL (NPTS) DY/DLAT IN 1/DEGREES IF LMAP=1
C     AREA     - REAL (NPTS) AREA WEIGHTS IN M**2 IF LMAP=1
C
C ATTRIBUTES:
C   LANGUAGE: FORTRAN 77
C
C$$$
      INTEGER KGDS(200)
      REAL XPTS(NPTS),YPTS(NPTS),RLON(NPTS),RLAT(NPTS)
      REAL CROT(NPTS),SROT(NPTS)
      REAL XLON(NPTS),XLAT(NPTS),YLON(NPTS),YLAT(NPTS),AREA(NPTS)
      INTEGER,PARAMETER:: KD=SELECTED_REAL_KIND(15,45)
      REAL(KIND=KD):: RERTH,PI,DPR
      REAL(KIND=KD):: RLAT1,RLON1,RLAT0,RLON0
      REAL(KIND=KD):: SLAT1,CLAT1,SLAT0,CLAT0
      REAL(KIND=KD):: CLON1,SLATR,CLATR,CLONR
      REAL(KIND=KD):: RLATR,RLONR,DLATS,DLONS,XPTFC,YPTFC,SLON
      REAL(KIND=KD):: SLAT,CLAT,CLON,RLATCR,RLONCR,DMIDS,RLATLR,RLONLR
      REAL(KIND=KD):: TERM1,TERM2
      REAL(KIND=KD):: XLONF,XLATF,YLONF,YLATF
      PARAMETER(RERTH=6.3712E6_KD)
      PARAMETER(PI=3.14159265358979_KD,DPR=180._KD/PI)
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      IF(KGDS(1).EQ.203) THEN
        RLAT1=KGDS(4)*1.E-3_KD
        RLON1=KGDS(5)*1.E-3_KD
        RLAT0=KGDS(7)*1.E-3_KD
        RLON0=KGDS(8)*1.E-3_KD
        DLONS=KGDS(9)*1.E-3_KD
        DLATS=KGDS(10)*1.E-3_KD
        DMIDS=SQRT(DLONS*DLONS+DLATS*DLATS)
        IROT=MOD(KGDS(6)/8,2)
        IM=KGDS(2)*2-1
        JM=KGDS(3)
        KSCAN=MOD(KGDS(11)/256,2)
        ISCAN=MOD(KGDS(11)/128,2)
        JSCAN=MOD(KGDS(11)/64,2)
        NSCAN=MOD(KGDS(11)/32,2)
        HI=(-1.)**ISCAN
        HJ=(-1.)**(1-JSCAN)
        SLAT1=SIN(RLAT1/DPR)
        CLAT1=COS(RLAT1/DPR)
        SLAT0=SIN(RLAT0/DPR)
        CLAT0=COS(RLAT0/DPR)
        HS0=SIGN(1._KD,MOD(RLON1-RLON0+180+3600,360._KD)-180)
        CLON1=COS((RLON1-RLON0)/DPR)
        SLATR=CLAT0*SLAT1-SLAT0*CLAT1*CLON1
        CLATR=SQRT(1-SLATR**2)
        CLONR=(CLAT0*CLAT1*CLON1+SLAT0*SLAT1)/CLATR
        RLATLR=DPR*ASIN(MAX(MIN(SLATR,1.0_KD),-1.0_KD))
        RLATCR=RLATLR+(JM-1)/2.*DLATS
        RLONLR=HS0*DPR*ACOS(MAX(MIN(CLONR,1.0_KD),-1.0_KD))
        RLONCR=RLONLR+(IM-1)/2.*DLONS
C        DLATS=RLATR/(-(JM-1)/2)
C        DLONS=RLONR/(-(IM-1)/2)

        write(0,*) 'in gdswzdcb.f'
        write(0,*) 'rlatcr=',rlatcr,' rloncr=',rloncr
        write(0,*) 'dlons=',dlons,' dlats=',dlats
        IF(KSCAN.EQ.0) THEN
          IS1=(JM+1)/2
        ELSE
          IS1=JM/2
        ENDIF
        XMIN=0
        XMAX=IM+1
        IF(IM.EQ.NINT(360/ABS(DLONS))) XMAX=IM+2
        YMIN=0
        YMAX=JM+1
        NRET=0
C - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C  TRANSLATE GRID COORDINATES TO EARTH COORDINATES
        IF(IOPT.EQ.0.OR.IOPT.EQ.1) THEN
C          Calculate grid center point in rotated coordinates:
           XPTFC=(JM-1)/2. + ( (IM-1)/2. - IS1 )
           YPTFC=(JM-1)/2. - ( (IM-1)/2. - IS1 ) + KSCAN

          DO N=1,NPTS
            XPTF=YPTS(N)+(XPTS(N)-IS1)
            YPTF=YPTS(N)-(XPTS(N)-IS1)+KSCAN
            IF(XPTF.GE.XMIN.AND.XPTF.LE.XMAX.AND.
     &         YPTF.GE.YMIN.AND.YPTF.LE.YMAX) THEN
c$$$              HS=HI*SIGN(1.,XPTF-(IM+1)/2)
c$$$              RLONR=(XPTF-(IM+1)/2)*DLONS
c$$$              RLATR=(YPTF-(JM+1)/2)*DLATS
              HS=HI*SIGN(1._KD,XPTFC+XPTF-1)
              RLONR=(XPTF-(IM+1)/2)*DLONS+RLONCR
              RLATR=(YPTF-(JM+1)/2)*DLATS+RLATCR
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
                RLON(N)=MOD(RLON0+HS*DPR*ACOS(MAX(MIN(CLON,1.0_KD),
     &               -1.0_KD))+3600,360._KD)
                RLAT(N)=DPR*ASIN(MAX(MIN(SLAT,1.0_KD),-1.0_KD))
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
              IF(LMAP.EQ.1) THEN
                IF(CLATR.LE.0) THEN
                  XLON(N)=FILL
                  XLAT(N)=FILL
                  YLON(N)=FILL
                  YLAT(N)=FILL
                  AREA(N)=FILL
                ELSE
                  SLON=SIN((RLON(N)-RLON0)/DPR)
                  TERM1=(CLAT0*CLAT+SLAT0*SLAT*CLON)/CLATR
                  TERM2=SLAT0*SLON/CLATR
                  XLONF=TERM1*CLAT/(DLONS*CLATR)
                  XLATF=-TERM2/(DLONS*CLATR)
                  YLONF=TERM2*CLAT/DLATS
                  YLATF=TERM1/DLATS
                  XLON(N)=XLONF-YLONF
                  XLAT(N)=XLATF-YLATF
                  YLON(N)=XLONF+YLONF
                  YLAT(N)=XLATF+YLATF
                  AREA(N)=RERTH**2*CLATR*DLATS*DLONS*2/DPR**2
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
                RLONR=HS*DPR*ACOS(MAX(MIN(CLONR,1.0_KD),-1.0_KD))
                RLATR=DPR*ASIN(MAX(MIN(SLATR,1.0_KD),-1.0_KD))
              ENDIF
c$$$              XPTF=(IM+1)/2+RLONR/DLONS
c$$$              YPTF=(JM+1)/2+RLATR/DLATS
              XPTF=(RLONR-RLONLR)/DLONS+1
              YPTF=(RLATR-RLATLR)/DLATS+1
c$$$              XPTF=(IM+1)/2+(RLONR-RLONCR)/DLONS
c$$$              YPTF=(JM+1)/2+(RLATR-RLATCR)/DLATS
              IF(XPTF.GE.XMIN.AND.XPTF.LE.XMAX.AND.
     &           YPTF.GE.YMIN.AND.YPTF.LE.YMAX) THEN
                XPTS(N)=IS1+(XPTF-(YPTF-KSCAN))/2
                YPTS(N)=(XPTF+(YPTF-KSCAN))/2
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
                IF(LMAP.EQ.1) THEN
                  IF(CLATR.LE.0) THEN
                    XLON(N)=FILL
                    XLAT(N)=FILL
                    YLON(N)=FILL
                    YLAT(N)=FILL
                    AREA(N)=FILL
                  ELSE
                    SLON=SIN((RLON(N)-RLON0)/DPR)
                    TERM1=(CLAT0*CLAT+SLAT0*SLAT*CLON)/CLATR
                    TERM2=SLAT0*SLON/CLATR
                    XLONF=TERM1*CLAT/(DLONS*CLATR)
                    XLATF=-TERM2/(DLONS*CLATR)
                    YLONF=TERM2*CLAT/DLATS
                    YLATF=TERM1/DLATS
                    XLON(N)=XLONF-YLONF
                    XLAT(N)=XLATF-YLATF
                    YLON(N)=XLONF+YLONF
                    YLAT(N)=XLATF+YLATF
                    AREA(N)=RERTH**2*CLATR*DLATS*DLONS*2/DPR**2
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
        IRET=-1
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
