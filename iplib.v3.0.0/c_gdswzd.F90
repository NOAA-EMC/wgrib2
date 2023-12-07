 subroutine c_gdswzd(kgds,iopt,npts,fill,xpts,ypts,rlon,rlat,nret, &
                     crot,srot,xlon,xlat,ylon,ylat,area) bind(c, name='gdswzd')

 use, intrinsic :: iso_c_binding

 use gdswzd_mod

 implicit none
 
#if (LSIZE==8)
 integer(kind=c_long), intent(in) :: kgds(200)
 integer(kind=c_long), value, intent(in) :: iopt
 integer(kind=c_long), value, intent(in) :: npts
 integer(kind=c_long), intent(out) :: nret
#else
 integer(kind=c_int), intent(in) :: kgds(200)
 integer(kind=c_int), value, intent(in) :: iopt
 integer(kind=c_int), value, intent(in) :: npts
 integer(kind=c_int), intent(out) :: nret
#endif

#if (LSIZE==8 || LSIZE==d)
 real(kind=c_double), value, intent(in) :: fill
 real(kind=c_double), intent(inout) :: xpts(npts),ypts(npts),rlon(npts),rlat(npts)
 real(kind=c_double), intent(out) :: crot(npts),srot(npts),xlon(npts),xlat(npts),ylon(npts), &
                                     ylat(npts),area(npts)
#else
 real(kind=c_float), value, intent(in) :: fill
 real(kind=c_float), intent(inout) :: xpts(npts),ypts(npts),rlon(npts),rlat(npts)
 real(kind=c_float), intent(out) :: crot(npts),srot(npts),xlon(npts),xlat(npts),ylon(npts), &
                                    ylat(npts),area(npts)
#endif

 call gdswzd(kgds,iopt,npts,fill,xpts,ypts,rlon,rlat,nret, &
             crot,srot,xlon,xlat,ylon,ylat,area)

 end subroutine c_gdswzd
