program test_fortran_api
   use wgrib2api
   implicit none

   character(len=*), parameter :: GRB2_FILE = 'data/gdaswave.t00z.wcoast.0p16.f000.grib2'
   character(len=*), parameter :: GRB2_INV = 'junk_ftn_api.inv'
   character(len=*), parameter :: EXP_GRB2_INV = 'data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv'
   integer nx, ny
   integer (kind=8) get_ref_edate, get_start_edate, get_end_edate;


   logical :: use_ncep_table = .false.

   logical :: ret
   integer :: iret

   print *, "Testing Fortran API..."

   print *, "Testing grb2_DEFINED_VAL() and grb2_UNDEFINED_VAL()..."

   ret = grb2_DEFINED_VAL(9.998e20)
   if (.not. ret) stop 2

   ret = grb2_DEFINED_VAL(9.99911e20)
   if (.not. ret) stop 3

   ret = grb2_DEFINED_VAL(grb2_UNDEFINED)
   if (ret) stop 4

   ret = grb2_UNDEFINED_VAL(9.998e20)
   if (ret) stop 5

   ret = grb2_UNDEFINED_VAL(9.99911e20)
   if (ret) stop 6

   ret = grb2_UNDEFINED_VAL(grb2_UNDEFINED)
   if (.not. ret) stop 7

   print *, "Testing grb2_mk_inv()..."
   iret = grb2_mk_inv(GRB2_FILE, GRB2_INV, use_ncep_table)
   if (iret .ne. 0) stop 11
   
   print *, "Testing grb2_inq()..."
   iret = grb2_inq(GRB2_FILE, GRB2_INV, ':WVHGT:',nx=nx,ny=ny,get_ref_edate=get_ref_edate)
   if (iret.ne. 1) stop 12
   if (nx.ne.241 .or. ny.ne.151) stop 13
   if (get_ref_edate.ne.20211130000000_8) stop 14

   iret = grb2_inq(GRB2_FILE, GRB2_INV, ':WVHGT:',get_start_edate=get_start_edate,get_end_edate=get_end_edate)
   if (get_start_edate.ne.20211130000000_8) stop 15
   if (get_end_edate.ne.20211130000000_8) stop 16

   print *, 'SUCCESS!'
end program test_fortran_api
