!> @file
!> @brief Module for the wgrib2 Fortran API to read and write grib2 files.
!>
!> ### Program History Log
!> Date | Programmer | Comments
!> -----|------------|---------
!> 12/2015 | W. Ebisuzaki | Initial
!> 01/2017 | W. Ebisuzaki | grb2_wrt, grb2_inq, added optional parameter debug
!> 01/2018 | W. Ebisuzaki | added real, parameter :: grb2_UNDEFINED
!>
!> @author Public Domain: Wesley Ebisuzaki @date 12/2015

! fortran api for reading and writing grib2
! 12/2015  Wesley Ebisuzaki   Public Domain
!
! fortran 2003 / fortran 95 + TR 15581  API to read and write grib2 files
!
!   requirements:
!        f2003 or f95 + TR 15581  (allowing subroutines to deallocate and allocate)
!        fort_wgrib2.c (wgrib2c wrapper)
!
! Provides:
!    wgrib2a(list of strings) :: same as $ wgrib2 [list of strings]
!    wgrib2c(argc, argv) :: same as $ wgrib2 [list of strings]
!
!    grb2_mk_inv :: makes inventory file
!        grb2_mk_inv('FILE.grb', 'FILE.inv')  same as wgrib2 FILE.grb -Match_inv >FILE.inv
!        The inventory file can be a temporary file if has the name @tmp:NAME
!        The inventory file can be a memory file if has the name @mem:N  N=0..29
!
!    grb2_free_file :: releases files handles after next call to wgrib2
!        grb2_free_file('FILE.grb')
!
!    grb2_inq :: grib2 inquire
!        grb2_inq('IN.grb', 'IN.inv', ...)
!         uses memory files @mem:19  .. always
!
!    grb2_wrt :: grib2 write
!       grb2_wrt('OUT.grb', 'TEMPLATE.grb', TEMPLATE_MSG_NO, ...)
!       TEMPLATE.grb is a grib file that is used as a template for the new grib file
!       TEMPLATE_MSG_NO = integer, message number of TEMPLATE.grb' to be used
!
!       meta = (string) .. sets metadata
!	order = 'raw' .. fastest data must be in same order as template
!	order = 'we:ns'  .. data must be in we:nw order
!	order = 'we:sn'  .. data must be in we:sn order
!

module wgrib2api
   use wgrib2lowapi

   !> Undefined value - if bitmap.
   real, parameter ::   grb2_UNDEFINED = 9.999e20

   integer, private :: next_mem
   private :: reserve_mem_buffer_name

contains

!       character (len=)  reserve_mem_buffer_name(buffer)
!       this function 1) reserves a memory file (@mem:XX)
!                     2) returns the name of the memory file
!                     3) returns the integer value of the memory file
!                     4)

   !> This function reserves a memory file (@mem:XX) and returns its name and integer value.
   !>
   !> @param[in] buffer Integer buffer to hold the memory file number.
   !>
   !> @return Character string containing the name of the memory file and its integer value.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   character(len=7) function reserve_mem_buffer_name(buffer)
      integer :: buffer
      buffer = next_mem
      if (next_mem <  5) then
         write(*,*) 'ran of memory files in grb2_inq()'
         write(*,*) ' use fewer options that use memory files'
         stop 19
      endif
      if (next_mem < 9) then
         write(reserve_mem_buffer_name,'(a,i1)') '@mem:', next_mem
      else
         write(reserve_mem_buffer_name,'(a,i2)') '@mem:', next_mem
      endif
      next_mem = next_mem - 1
   end function reserve_mem_buffer_name

   !> This function checks if a GRIB2 value is defined.
   !>
   !> @param[in] x Real value to check.
   !>
   !> @return Returns true if the value is defined, false otherwise.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   logical function grb2_DEFINED_VAL(x)
      real, intent(in) :: x
      grb2_DEFINED_VAL = (x < 9.9989e20) .or. (x > 9.9991e20)
      return
   end function grb2_DEFINED_VAL

   !> This function checks if a GRIB2 value is undefined.
   !>
   !> @param[in] x Real value to check.
   !>
   !> @return Returns true if the value is undefined, false otherwise.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   logical function grb2_UNDEFINED_VAL(x)
      real, intent(in) :: x
      grb2_UNDEFINED_VAL = (x >= 9.9989e20) .and. (x <= 9.9991e20)
      return
   end function grb2_UNDEFINED_VAL

   !> This function counts the number of variable arguments and populates the lines array.
   !>
   !> @param[in] lines Pointer to character array that holds the variable arguments.
   !> @param[in] istart Starting index for populating the lines array (initial value of n).
   !> @param[in] a1 First optional argument.
   !> @param[in] a2 Second optional argument.
   !> @param[in] a3 Third optional argument.
   !> @param[in] a4 Fourth optional argument.
   !> @param[in] a5 Fifth optional argument.
   !> @param[in] a6 Sixth optional argument.
   !> @param[in] a7 Seventh optional argument.
   !> @param[in] a8 Eighth optional argument.
   !> @param[in] a9 Ninth optional argument.
   !> @param[in] a10 Tenth optional argument.
   !> @param[in] a11 Eleventh optional argument.
   !> @param[in] a12 Twelfth optional argument.
   !> @param[in] a13 Thirteenth optional argument.
   !> @param[in] a14 Fourteenth optional argument.
   !> @param[in] a15 Fifteenth optional argument.
   !> @param[in] a16 Sixteenth optional argument.
   !> @param[in] a17 Seventeenth optional argument.
   !> @param[in] a18 Eighteenth optional argument.
   !> @param[in] a19 Nineteenth optional argument.
   !> @param[in] a20 Twentieth optional argument.
   !>
   !> @return Returns n, the number of arguments present in the lines array.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function grb2_var_args(lines, istart, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, &
         a12, a13, a14, a15, a16, a17, a18, a19, a20)

      character (len=*), optional, intent(in):: a1,a2,a3,a4,a5,a6,a7,a8,a9,a10
      character (len=*), optional, intent(in):: a11,a12,a13,a14,a15,a16,a17
      character (len=*), optional, intent(in):: a18,a19,a20
      integer, intent(in) :: istart
      character (len=300), intent(inout) :: lines(*)

      integer :: n

      n = istart

      if (.not. present(a1)) goto 100
         n=n+1
         lines(n) = a1

      if (.not. present(a2)) goto 100
         n=n+1
         lines(n) = a2

      if (.not. present(a3)) goto 100
         n=n+1
         lines(n) = a3

      if (.not. present(a4)) goto 100
         n=n+1
         lines(n) = a4

      if (.not. present(a5)) goto 100
         n=n+1
         lines(n) = a5

      if (.not. present(a6)) goto 100
         n=n+1
         lines(n) = a6

      if (.not. present(a7)) goto 100
         n=n+1
         lines(n) = a7

      if (.not. present(a8)) goto 100
         n=n+1
         lines(n) = a8

      if (.not. present(a9)) goto 100
         n=n+1
         lines(n) = a9

      if (.not. present(a10)) goto 100
         n=n+1
         lines(n) = a10

      if (.not. present(a11)) goto 100
         n=n+1
         lines(n) = a11

      if (.not. present(a12)) goto 100
         n=n+1
         lines(n) = a12

      if (.not. present(a13)) goto 100
         n=n+1
         lines(n) = a13

      if (.not. present(a14)) goto 100
         n=n+1
         lines(n) = a14

      if (.not. present(a15)) goto 100
         n=n+1
         lines(n) = a15

      if (.not. present(a16)) goto 100
         n=n+1
         lines(n) = a16

      if (.not. present(a17)) goto 100
         n=n+1
         lines(n) = a17

      if (.not. present(a18)) goto 100
         n=n+1
         lines(n) = a18

      if (.not. present(a19)) goto 100
         n=n+1
         lines(n) = a19

      if (.not. present(a20)) goto 100
         n=n+1
         lines(n) = a20

100   continue

      grb2_var_args = n
      return
   end function grb2_var_args

   !> This function adds a line to the lines array and updates n.
   !>
   !> @param[in] lines Pointer to character array that holds the arguments.
   !> @param[in] n Current number of arguments.
   !> @param[in] line The line to add.
   !>
   !> @return 0 for success
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function add_line(lines,n,line)
      character (len=*), intent(inout):: lines(:)
      character (len=*), intent(in):: line
      integer, intent(inout) :: n

      n = n + 1
      lines(n) = line
      add_line = 0

      return
   end function add_line

   !> This function calls the wgrib2c command with the specified arguments. Same as 
   !> $ wgrib2 [list of strings].
   !>
   !> Not used by grb2_* routines.
   !>
   !> @param[in] a1 First optional argument.
   !> @param[in] a2 Second optional argument.
   !> @param[in] a3 Third optional argument.
   !> @param[in] a4 Fourth optional argument.
   !> @param[in] a5 Fifth optional argument.
   !> @param[in] a6 Sixth optional argument.
   !> @param[in] a7 Seventh optional argument.
   !> @param[in] a8 Eighth optional argument.
   !> @param[in] a9 Ninth optional argument.
   !> @param[in] a10 Tenth optional argument.
   !> @param[in] a11 Eleventh optional argument.
   !> @param[in] a12 Twelfth optional argument.
   !> @param[in] a13 Thirteenth optional argument.
   !> @param[in] a14 Fourteenth optional argument.
   !> @param[in] a15 Fifteenth optional argument.
   !> @param[in] a16 Sixteenth optional argument.
   !> @param[in] a17 Seventeenth optional argument.
   !> @param[in] a18 Eighteenth optional argument.
   !> @param[in] a19 Nineteenth optional argument.
   !> @param[in] a20 Twentieth optional argument.
   !>
   !> @return 0 for success, error code otherwise.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function wgrib2a(a1 ,a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, &
         a12, a13, a14, a15, a16, a17, a18, a19, a20)

      use iso_c_binding
      use wgrib2lowapi

      character (len=*), optional, intent(in):: a1,a2,a3,a4,a5,a6,a7,a8,a9,a10
      character (len=*), optional, intent(in):: a11,a12,a13,a14,a15,a16,a17
      character (len=*), optional, intent(in):: a18,a19,a20

      integer n
      character (len=300) :: lines(20)

      n = grb2_var_args(lines,0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11, &
                        a12,a13,a14,a15,a16,a17,a18,a19,a20)

      wgrib2a = wgrib2c(n,lines,300)
      return
   end function wgrib2a

   !> This function writes the match inventory to a file.
   !>
   !> ### Program History Log
   !> Date | Programmer | Comments
   !> -----|------------|---------
   !> 12/2015 | W. Ebisuzaki | Initial
   !> 12/2016 | W. Ebisuzaki | fixed -rewind_init grb, fixed -rewind_init-ignore error return
   !> 11/2019 | W. Ebisuzaki | added optional parameter  use_ncep_table=logical_value
   !>
   !> @param[in] grbfile Input GRIB file.
   !> @param[in] invfile Output inventory file.
   !> @param[in] use_ncep_table Logical flag to use NCEP table (optional).
   !>
   !> @return 0 for success, error code otherwise.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function grb2_mk_inv(grbfile, invfile, use_ncep_table)
      use wgrib2lowapi
      implicit none

      character (len=*), intent(in):: grbfile, invfile
      logical, optional, intent(in):: use_ncep_table
      character (len=300) :: cmd(10)
      integer :: n, i

      n = 0
      if (present(use_ncep_table)) then
         if (use_ncep_table) then
            i = add_line(cmd,n,'-set')
            i = add_line(cmd,n,'center')
            i = add_line(cmd,n,'7')
         endif
      endif
      i = add_line(cmd,n,grbfile)
      i = add_line(cmd,n,'-rewind_init')
      i = add_line(cmd,n,grbfile)
      i = add_line(cmd,n,'-inv')
      i = add_line(cmd,n,invfile)
      i = add_line(cmd,n,'-Match_inv')

      grb2_mk_inv = wgrib2c(n, cmd, len(cmd(1)))
      return
   end function grb2_mk_inv

   !> This function releases file's handle after next call to wgrib2.
   !>
   !> @param[in] file Name of the file to free from wgrib2's list of open files.
   !>
   !> @return 0 for success, error code otherwise.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function grb2_free_file(file)
      use wgrib2lowapi
      implicit none

      character (len=*), intent(in) :: file
      character (len=len_trim(file)+1,kind=C_CHAR) :: cfile
      !	convert from fortran string to C string
      cfile = trim(file) // C_NULL_CHAR
      grb2_free_file = wgrib2_free_file(cfile)
      return
   end function grb2_free_file

   !> This function writes a grib message to a specified file.
   !>
   !> Must specify either data1 or data2, but not both.
   !>
   !> ### Program History Log
   !> Date | Programmer | Comments
   !> -----|------------|---------
   !> 12/2015 | W. Ebisuzaki | Initial
   !> 10/2016 | W. Ebisuzaki | added append=integer, fixed data2=grid
   !> 01/2017 | W. Ebisuzaki | order code fixed, use we:ns, we:sn or raw
   !>
   !> @param[in] grbfile Name of the grib file to write.
   !> @param[in] template_file Name of the grib file that is used as a template for the new grib file.
   !> @param[in] template_id ID of the template to use.
   !> @param[in] data1 Optional 1D array of real numbers (grid data). Must be specified if data2 is not.
   !> @param[in] data2 Optional 2D array of real numbers (grid data). Must be specified if data1 is not.
   !> @param[in] packing Packing type (optional). Used with -set_grib_type option to control the packing
   !> method of the new grib message. If packing is not specified, the default packing method will be used (c2).
   !> @param[in] order Order of the data (optional). Used with -order option to specify the order of the
   !> data. The default configuration is for wgrib2 to convert all data into 'we:sn' order.
   !> If order = 'raw', fastest data must be in same order as the template.
   !> If order = 'we:ns', data must be in 'we:ns' order.
   !> If order = 'we:sn', data must be in 'we:sn' order.
   !> @param[in] var Variable name (optional). Used with -set_var option to change the variable name in the 
   !> grib message.
   !> @param[in] meta String to specify the metadata (optional). Used with -set_metadata_str option to apply 
   !> the metadata to the grib message.
   !> @param[in] append Flag to append to output files (optional). If non-zero, the -append option is applied 
   !> to append to a currently existing file.
   !> @param[in] level String representation of the level/layer of the field (optional). Used with -set_lev 
   !> option to change the level/layer 
   !> of the field in the grib message. 
   !> @param[in] date Reference date (optional). Used with -set_date option to change the reference date in the
   !> grib message.
   !> @param[in] timing String representation of ftime (optional). Used with -set_ftime2 option to change the ftime
   !> in the grib message.
   !> @param[in] fhour Forecast hour (optional). Used with -set_ftime option to change the ftime to [fhour] hour fcst
   !> in the grib message.
   !> @param[in] fminute Forecast minute (optional). Used with -set_ftime option to change the ftime to [fminute] minute
   !> fcst in the grib message.
   !> @param[in] fhour_ave1 Forecast hour average 1 (optional). Used with fhour_ave2 and -set_ave option to change the ftime
   !> in the grib message. This option was used with version 1 of ftime which will be removed in future releases.
   !> @param[in] fhour_ave2 Forecast hour average 2 (optional). Used with fhour_ave1 and -set_ave option to change the ftime
   !> in the grib message. This option was used with version 1 of ftime which will be removed in future releases.
   !> @param[in] fhour_acc1 Forecast hour accumulation 1 (optional). Used with fhour_acc2 and -set_ave option to change the ftime
   !> in the grib message. This option was used with version 1 of ftime which will be removed in future releases.
   !> @param[in] fhour_acc2 Forecast hour accumulation 2 (optional). Used with fhour_acc1 and -set_ave option to change the ftime
   !> in the grib message. This option was used with version 1 of ftime which will be removed in future releases.
   !> @param[in] center Center ID (optional). Used with -set center option to change the center ID in the grib message.
   !> @param[in] subcenter Subcenter ID (optional). Used with -set subcenter option to change the subcenter ID in the grib message.
   !> @param[in] mb Real value of level of the field (optional). Used with -set_lev option to change the level in the grib message.
   !> @param[in] debug Debug flag (optional).
   !> @param[in] encode_bits Number of bits(optional). Used with -set_bin_prec option to change number of bits to encode grid
   !> point data in the grib message.
   !>
   !> @return 0 for success, error code otherwise.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function grb2_wrt(grbfile, template_file,  template_id, &
         data1, data2, packing, order, var, meta, append, level, date, timing, &
         fhour, fminute, fhour_ave1, fhour_ave2, fhour_acc1, fhour_acc2, &
         center, subcenter, mb, debug, encode_bits)

      use iso_c_binding
      use wgrib2lowapi
      implicit none

      character (len=*), intent(in):: grbfile, template_file

      real, optional, intent(in) :: data1(:)
      real, optional, allocatable, intent(in) :: data2(:,:)
      integer, intent(in) :: template_id

      character (len=*), optional, intent(in):: var, meta, level, packing, order, timing
      integer (kind=8), optional, intent(in) :: date
      integer, optional, intent(in) :: fhour, fminute, fhour_ave1, fhour_ave2
      integer, optional, intent(in) :: fhour_acc1, fhour_acc2
      integer, optional, intent(in) :: center, subcenter, append, debug, encode_bits

      real, optional, intent(in) :: mb

      character (len=300) :: cmd(80)
      character (len=10) :: pack
      integer :: i,n
      logical :: present_data1, present_data2

      integer (C_INT) :: ierr
      integer (C_SIZE_T) :: ndata, nx, ny

      !        write(*,*) '>> grb2_wrt'
      present_data1 = present(data1)
      present_data2 = present(data2)

      if (present_data1 .and. present_data2) then
         write(*,*) '*** FATAL ERROR grb2_wrt:  must specify data1 or data2 but not both'
         stop 8
      endif
      if (.not.present_data1 .and. .not.present_data2) then
         write(*,*) '*** FATAL ERROR grb2_wrt:  neither data1 nor data2 specified'
         stop 8
      endif

      if (present_data1) then
         ndata = size(data1,1,C_SIZE_T)
         ierr = wgrib2_set_reg(data1, ndata, 9)
      endif

      if (present_data2) then
         nx = size(data2,1,C_SIZE_T)
         ny = size(data2,2,C_SIZE_T)
         ndata = nx*ny
         ierr = wgrib2_set_reg(data2, ndata, 9)
      endif

      n = 0

      i = add_line(cmd,n,template_file)
      i = add_line(cmd,n,'-rewind_init')
      i = add_line(cmd,n,template_file)

      if (present(order)) then
         if (order .eq. 'raw') then
            i = add_line(cmd,n,'-order')
            i = add_line(cmd,n,'raw')
         else if (order .eq. 'we:ns' .or. order .eq. 'ns') then
            i = add_line(cmd,n,'-order')
            i = add_line(cmd,n,'we:ns')
         else
            write(*,*) 'FATAL ERROR grb2_wrt: unknown order=',order
            stop 9
         endif
      endif

      i = add_line(cmd,n,'-d')
      n = n + 1
      write(cmd(n), '(i6.6)') template_id

      i = add_line(cmd,n,'-rpn_rcl')
      i = add_line(cmd,n,'9')

      if (present(meta)) then
         i = add_line(cmd,n,'-set_metadata_str')
         n = n + 1
         cmd(n) = '0:0:' // meta
      endif

      pack='c2'
      if (present(packing)) then
         pack=packing
      endif
      i = add_line(cmd,n,'-set_grib_type')
      i = add_line(cmd,n,pack)

      if (present(date)) then
         i = add_line(cmd,n,'-set_date')
         n = n + 1
         write(cmd(n),'(i14.10)') date
      endif

      if (present(var)) then
         i = add_line(cmd,n,'-set_var')
         n = n + 1
         cmd(n) = var
      endif

      if (present(level)) then
         i = add_line(cmd,n,'-set_lev')
         n = n + 1
         cmd(n) = level
      endif

      if (present(timing)) then
         i = add_line(cmd,n,'-set_ftime2')
         n = n + 1
         cmd(n) = timing
      endif

      if (present(mb)) then
         i = add_line(cmd,n,'-set_lev')
         n = n + 1
         write(cmd(n),*) mb, 'mb'
      endif

      if (present(fhour)) then
         i = add_line(cmd,n,'-set_ftime')
         n = n + 1
         write(cmd(n),*) fhour, 'hour fcst'
      endif

      if (present(fminute)) then
         i = add_line(cmd,n,'-set_ftime')
         n = n + 1
         write(cmd(n),*) fminute, 'min fcst'
      endif

      if (present(fhour_ave1).and.present(fhour_ave2)) then
         i = add_line(cmd,n,'-set_ave')
         n = n + 1
         write(cmd(n),'(i5,a1,i5,a)')  fhour_ave1, '-', fhour_ave2, ' hour ave fcst'
      endif

      if (present(fhour_acc1).and.present(fhour_acc2)) then
         i = add_line(cmd,n,'-set_ave')
         n = n + 1
         write(cmd(n),'(i5,a1,i5,a)')  fhour_acc1, '-', fhour_acc2, ' hour ave fcst'
      endif

      if (present(center)) then
         i = add_line(cmd,n,'-set')
         i = add_line(cmd,n,'center')
         n = n + 1
         write(cmd(n),*) center
      endif

      if (present(subcenter)) then
         i = add_line(cmd,n,'-set')
         i = add_line(cmd,n,'subcenter')
         n = n + 1
         write(cmd(n),*) subcenter
      endif

      if (present(append)) then
         if (append.ne.0) then
            i = add_line(cmd,n,'-append')
         endif
      endif

      !       encode_bits needs to be before -grib_out
      if (present(encode_bits)) then
         if (encode_bits .le. 0) then
            write(*,*) 'encode_bits is .le. 0 .. nothing done'
         else
            if (encode_bits.gt.16) then
               i = add_line(cmd,n,'-set_grib_max_bits')
               n = n + 1
               write(cmd(n),*) min(encode_bits,25)
            endif
            i = add_line(cmd,n,'-set_bin_prec')
            n = n + 1
            write(cmd(n),*) min(encode_bits,25)
         endif
      endif

      i = add_line(cmd,n,'-grib_out')
      i = add_line(cmd,n,grbfile)

      !	output from grb2_wrt
      if (present(debug)) then
         i = add_line(cmd,n,'-S')
      else
         i = add_line(cmd,n,'-inv')
         i = add_line(cmd,n,'/dev/null')
      endif

      if (present(debug)) then
         do i = 1, n
            write(*,*) 'grb2_wrt>>> i=',i,trim(cmd(i))
         enddo
      endif

      !        write(*,*) '>>> grb2_wrt >> wgrib2c'
      grb2_wrt = wgrib2c(n,cmd, len(cmd(1)))
      if (grb2_wrt .ne. 0) then
         write(*,*) 'ERROR: grb2_wrt err=', grb2_wrt, ' file=',trim(grbfile)
      endif
      !        write(*,*) '<<< grb2_wrt << wgrib2c'

      return
   end function grb2_wrt

!
!        grb2_inq: grib2 inquire
!               give match strings
!               return number of matches
!               use optional arguments to get more info including grid data
!
!
!        use RPN register MAX    grid
!        use RPN register MAX-1  lat
!        use RPN register MAX-2  lon
!

   !> This function performs an inquiry of a grib message. Uses optional arguments to get more information including grid data.
   !>
   !> ### Program History Log
   !> Date | Programmer | Comments
   !> -----|------------|---------
   !> 12/2015 | W. Ebisuzaki | Initial
   !> 10/2016 | W. Ebisuzaki | added verf_date, verf_edate, start_date, start_edate, end_date, end_edate
   !> 01/2017 | W. Ebisuzaki | added copy=file
   !>
   !> @param[in] grbfile Input grib file.
   !> @param[in] invfile Inventory of input file.
   !> @param[in] a1 First optional argument.
   !> @param[in] a2 Second optional argument.
   !> @param[in] a3 Third optional argument.
   !> @param[in] a4 Fourth optional argument.
   !> @param[in] a5 Fifth optional argument.
   !> @param[in] a6 Sixth optional argument.
   !> @param[in] a7 Seventh optional argument.
   !> @param[in] a8 Eighth optional argument.
   !> @param[in] a9 Ninth optional argument.
   !> @param[in] a10 Tenth optional argument.
   !> @param[in] a11 Eleventh optional argument.
   !> @param[in] a12 Twelfth optional argument.
   !> @param[in] a13 Thirteenth optional argument.
   !> @param[in] a14 Fourteenth optional argument.
   !> @param[in] a15 Fifteenth optional argument.
   !> @param[in] a16 Sixteenth optional argument.
   !> @param[in] a17 Seventeenth optional argument.
   !> @param[in] a18 Eighteenth optional argument.
   !> @param[in] a19 Nineteenth optional argument.
   !> @param[in] a20 Twentieth optional argument.
   !> @param[in] npts Number of points, equivalent to nx*ny (optional). Ignored if NULL.
   !> @param[in] nx Number of grid points in the x-direction (optional). Ignored if NULL.
   !> @param[in] ny Number of grid points in the y-direction (optional). Ignored if NULL.
   !> @param[in] nmatch Number of matches found (optional). Ignored if NULL.
   !> @param[in] msgno Message number (optional). Ignored if NULL.
   !> @param[in] submsg Sub-message number (optional). Ignored if NULL.
   !> @param[in] data1 1D array of real numbers for grid data (optional). Ignored if NULL.
   !> @param[in] nread Number of data points read (optional). Ignored if NULL.
   !> @param[in] data2 2D array of real numbers for grid data (optional). Ignored if NULL.
   !> @param[in] lat 2D array of real numbers for latitude (optional). Ignored if NULL.
   !> @param[in] lon 2D array of real numbers for longitude (optional). Ignored if NULL.
   !> @param[in] ref_date Reference date (optional). Ignored if NULL.
   !> @param[in] ref_edate Reference date with minutes and seconds (optional). Ignored if NULL.
   !> @param[in] verf_date Verification date YYYYMMDD (optional). Ignored if NULL.
   !> @param[in] verf_edate Verification date YYYYMMDDmmss (optional). Ignored if NULL.
   !> @param[in] start_date Start date YYYYMMDDHH (optional). Ignored if NULL.
   !> @param[in] start_edate (YYYYMMDDHHmmss) Start date with minutes and seconds (optional). Ignored if NULL.
   !> @param[in] end_date End date YYYYMMDDHHmmss (optional). Ignored if NULL.
   !> @param[in] end_edate End date YYYYMMDDHHmmss (optional). Ignored if NULL.
   !> @param[in] order Order of the data (optional). Used with -order option to specify the order of the data.
   !> @param[in] lastuse Flag indicating if this is the last time file will be used (optional). If non-zero,
   !> the file will be closed after this call using -transient option.
   !> @param[in] close File to close after this call (optional). Used with -transient option.
   !> @param[in] desc Grid info description (optional). Populated with last inventory item of -S option. 
   !> Ignored if NULL.
   !> @param[in] grid_desc Grid description (optional). Populated with last inventory item of -grid option. 
   !> Ignored if NULL.
   !> @param[in] debug Debug flag (optional).
   !> @param[in] copy Name of file where GRIB record is written (optional). Used with -grib option. 
   !> @param[in] sequential Flag indicating whether or not to rewind inventory file (optional). If not null,
   !> the -end option is used, stopping the processing of the grib file after one line of the inventory has been 
   !> written. If non-zero, rewind_inv is set to false, meaning the inventory file will not be rewound after reading.
   !> If null, rewind_inv is set to true by default.
   !> @param[in] regex Flag indicating whether or not to use a POSIX extended regular expression. If non-zero,
   !> the -egrep option is used. If zero or null, the -fgrep option is used.
   !> @param[in] get_ref_edate If present, the reference date with minutes and seconds will be obtained and written
   !> to a new file. Ignored if NULL.
   !> @param[in] get_start_edate If present, the start date with minutes and seconds will be obtained and written
   !> to a new file. Ignored if NULL.
   !> @param[in] get_end_edate If present, the end date with minutes and seconds will be obtained and written
   !> to a new file. Ignored if NULL.
   !>
   !> @return Number of matches found or -1 if an error occurred.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function grb2_inq(grbfile, invfile, &
         a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, &
         npts, nx, ny, nmatch, msgno, submsg, data1, nread, data2, lat, lon, ref_date, ref_edate, &
         verf_date, verf_edate, start_date, start_edate, end_date, end_edate, order, lastuse, close, &
         desc, grid_desc, debug, copy, sequential, regex, get_ref_edate, get_start_edate, &
         get_end_edate)

      use wgrib2lowapi
      implicit none

      character (len=*), intent(in):: grbfile, invfile
      character (len=*), optional, intent(in):: a1,a2,a3,a4,a5,a6,a7,a8,a9,a10
      character (len=*), optional, intent(in):: a11,a12,a13,a14,a15,a16,a17
      character (len=*), optional, intent(in):: a18,a19,a20
      character (len=*), optional, intent(in):: copy

      integer, optional, intent(in) :: lastuse
      integer, optional, intent(in) :: debug, sequential, regex
      integer, optional, intent(out) :: npts, nx, ny, nmatch, nread, msgno
      integer, optional, intent(out) :: submsg
      integer (kind=8), optional, intent(in) :: ref_date, ref_edate
      integer (kind=8), optional, intent(in) :: verf_date, verf_edate
      integer (kind=8), optional, intent(in) :: start_date, start_edate
      integer (kind=8), optional, intent(in) :: end_date, end_edate
      integer (kind=8), optional, intent(out) :: get_ref_edate, get_start_edate, get_end_edate 
      integer (C_SIZE_T) :: buffer_size
      real, optional, intent(out) :: data1(:)
      real, optional, allocatable, intent(inout) :: data2(:,:), lat(:,:), lon(:,:)
      character (len=*), optional, intent(in):: order, close
      character (len=*), optional, intent(out):: desc, grid_desc
      integer, parameter :: string_size = 300

      character (len=string_size) :: lines(20), cmd(80), tmpline
      character (len=71) :: grid_id
      character (len=6) :: grep

      integer i, j, m, n, ierr, desc_file, grid_desc_file, grid_info_file
      integer get_ref_edate_file, get_start_edate_file, get_end_edate_file

      logical rewind_inv
      integer isubmsg
      integer (C_SIZE_T) :: ndata, nnx, nny, imsgno, one

      m = grb2_var_args(lines,0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11, &
         a12,a13,a14,a15,a16,a17,a18,a19,a20)

      one = 1
      n = 0
      grb2_inq = 0
      next_mem = 29				! next free mem file, work downwards
      rewind_inv = .true.
      grep = '-fgrep'

      if (present(regex)) then
         if (regex.ne.0) grep = '-egrep'
      endif

      if (present(sequential)) then
         if (sequential.ne.0) rewind_inv = .false.
      endif
      !
      i = add_line(cmd,n,grbfile)
      i = add_line(cmd,n,'-i_file')
      i = add_line(cmd,n,invfile)
      if (rewind_inv) then
         i = add_line(cmd,n,'-rewind_init')
         i = add_line(cmd,n,invfile)
      endif

      i = add_line(cmd,n,'-inv')
      !       ** this is linux/unix
      i = add_line(cmd,n,'/dev/null')
      !       ** this is for windows
      !       i = add_line(cmd,n,'NUL')

      if (present(lastuse)) then
         if (lastuse.ne.0) then
            i = add_line(cmd,n,'-transient')
            i = add_line(cmd,n,grbfile)
            i = add_line(cmd,n,'-transient')
            i = add_line(cmd,n,invfile)
         endif
      endif
      if (present(close)) then
         i = add_line(cmd,n,'-transient')
         i = add_line(cmd,n,trim(close))
      endif

      !	parse the match commands

      do j = 1, m
         i = add_line(cmd,n,grep)
         i = add_line(cmd,n,lines(j))        
      enddo

      !	ref_date: add to match command
      if (present(ref_date)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9000) ref_date
9000     format(':d=',i10.10)
         i = add_line(cmd,n,tmpline)
      endif

      !	ref_edate: add to match command
      if (present(ref_edate)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9010) ref_edate
9010     format(':D=',i14.14)
         i = add_line(cmd,n,tmpline)
      endif

      !	verf_date: add to match command
      if (present(verf_date)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9020) verf_date
9020     format(':end_FT=',i10.10)
         i = add_line(cmd,n,tmpline)
      endif

      !	end_date: add to match command
      if (present(end_date)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9020) end_date
         i = add_line(cmd,n,tmpline)
      endif

      !	verf_edate: add to match command
      if (present(verf_edate)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9030) verf_edate
9030     format(':end_FT=',i14.14)
         i = add_line(cmd,n,tmpline)
      endif

      !	end_edate: add to match command
      if (present(end_edate)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9030) end_edate
         i = add_line(cmd,n,tmpline)
      endif

      !	start_date: add to match command
      if (present(start_date)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9040) start_date
9040     format(':start_FT=',i10.10)
         i = add_line(cmd,n,tmpline)
      endif

      !	start_edate: add to match command
      if (present(start_edate)) then
         i = add_line(cmd,n,'-egrep')
         write(tmpline,9050) start_edate
9050     format(':start_FT=',i14.14)
         i = add_line(cmd,n,tmpline)
      endif

      !       these options get values
      !       use memory files 0..29

      !	add call to get grid info

      i = add_line(cmd,n,'-ftn_api_fn0')
      i = add_line(cmd,n,'-last0')
      i = add_line(cmd,n,reserve_mem_buffer_name(grid_info_file))

      !	grid_info desc
      if (present(desc)) then
         i = add_line(cmd,n,'-S')
         i = add_line(cmd,n,'-last0')
         i = add_line(cmd,n,reserve_mem_buffer_name(desc_file))
      endif

      !	grid_desc
      if (present(grid_desc)) then
         i = add_line(cmd,n,'-grid')
         i = add_line(cmd,n,'-last0')
         i = add_line(cmd,n,reserve_mem_buffer_name(grid_desc_file))
      endif

      !	get_ref_edate
      if (present(get_ref_edate)) then
         i = add_line(cmd,n,'-T')
         i = add_line(cmd,n,'-last0')
         i = add_line(cmd,n,reserve_mem_buffer_name(get_ref_edate_file))
      endif

      !	get_start_edate
      if (present(get_start_edate)) then
         i = add_line(cmd,n,'-start_FT')
         i = add_line(cmd,n,'-last0')
         i = add_line(cmd,n,reserve_mem_buffer_name(get_start_edate_file))
      endif

      !	get_end_edate
      if (present(get_end_edate)) then
         i = add_line(cmd,n,'-end_FT')
         i = add_line(cmd,n,'-last0')
         i = add_line(cmd,n,reserve_mem_buffer_name(get_end_edate_file))
      endif

      if (present(order)) then
         if (order == 'we:ns') then
                  i = add_line(cmd,n,'-order')
                  i = add_line(cmd,n,'we:ns')
         else if (order == 'raw') then
                  i = add_line(cmd,n,'-order')
                  i = add_line(cmd,n,'raw')
         else if (order /= 'we:sn') then
            write(*,*) '*** FATAL ERROR: order should be we:ns, we:sn or raw. not', order
            stop 8
         endif
      endif

      if (present(data1).or.present(data2)) then
         i = add_line(cmd,n,'-rpn_sto')
         i = add_line(cmd,n,'19')
      endif

      if (present(lon)) then
         i = add_line(cmd,n,'-rpn')
         i = add_line(cmd,n,'rcl_lon:sto_17')
      endif

      if (present(lat)) then
         i = add_line(cmd,n,'-rpn')
         i = add_line(cmd,n,'rcl_lat:sto_18')
      endif

      if (present(copy)) then
         i = add_line(cmd,n,'-grib')
         i = add_line(cmd,n,copy)
      endif

      if (present(sequential)) then
         i = add_line(cmd,n,'-end')
      endif

      if (present(debug)) then
         do i = 1, n
               write(*,*) 'grb2_inq>>> ',trim(cmd(i))
         enddo
      endif

      ierr = wgrib2c(n,cmd, len(cmd(1)))
      if (ierr .ne. 0) then
         grb2_inq = -1
         return
      endif

      !	get grid_info
      buffer_size = 71
      ierr = wgrib2_get_mem_buffer(grid_id, buffer_size, grid_info_file)
      if (ierr.ne.0) then
      !	   no grid info .. no match
         grb2_inq = 0
         return
      endif
      !	write(*,*) 'grid_id=',grid_id

      read(grid_id,'(i11,5(1x,i11))',end=100,err=100) grb2_inq, ndata, nnx, nny, imsgno, isubmsg
      goto 110

100   continue
      write(*,*) 'FATAL ERROR: grb2_inq reading grid info'
      grb2_inq = -1
      return

110   continue
      !	write(*,*) 'ndata=',ndata, nnx, nny

      if (present(npts)) then
         npts = int(ndata, KIND=kind(npts))
      endif
      if (present(nx)) then
         nx = int(nnx, KIND=kind(nx))
      endif
      if (present(ny)) then
         ny = int(nny, KIND=kind(ny))
      endif
      if (present(nmatch)) then
         nmatch = grb2_inq
      endif
      if (present(msgno)) then
         msgno = int(imsgno, KIND=kind(msgno))
      endif
      if (present(submsg)) then
         submsg = isubmsg
      endif
      if (present(desc)) then
         buffer_size = wgrib2_get_mem_buffer_size(desc_file)
         if (len(desc).ge.buffer_size) then
            desc = ' '
            i = wgrib2_get_mem_buffer(desc, buffer_size, desc_file)
            if (i.ne.0) desc='****'
         else
            desc = '****'
         endif
      endif
      if (present(grid_desc)) then
         buffer_size = wgrib2_get_mem_buffer_size(grid_desc_file)
         if (len(grid_desc).ge.buffer_size) then
            grid_desc = ' '
            i = wgrib2_get_mem_buffer(grid_desc, buffer_size, grid_desc_file)
            if (i.ne.0) grid_desc='****'
         else
            grid_desc = '****'
         endif
      endif
      if (present(get_ref_edate)) then
         buffer_size = wgrib2_get_mem_buffer_size(get_ref_edate_file)
         get_ref_edate=-1
         if (string_size .ge. buffer_size) then
            tmpline = ' '
            i = wgrib2_get_mem_buffer(tmpline, buffer_size, get_ref_edate_file)
            if (i==0) then
      !                   ignore T=
               read(tmpline(3:buffer_size),*) get_ref_edate
            endif
         endif
      endif
      if (present(get_start_edate)) then
         buffer_size = wgrib2_get_mem_buffer_size(get_start_edate_file)
         get_start_edate=-1
         if (string_size .ge. buffer_size) then
            tmpline = ' '
            i = wgrib2_get_mem_buffer(tmpline, buffer_size, get_start_edate_file)
            if (i==0) then
      !                   ignore start_FT=
               read(tmpline(10:buffer_size),*) get_start_edate
            endif
         endif
      endif
      if (present(get_end_edate)) then
         buffer_size = wgrib2_get_mem_buffer_size(get_end_edate_file)
         get_end_edate=-1
         if (string_size .ge. buffer_size) then
            tmpline = ' '
            i = wgrib2_get_mem_buffer(tmpline, buffer_size, get_end_edate_file)
            if (i==0) then
      !                   ignore end_FT=
               read(tmpline(8:buffer_size),*) get_end_edate
            endif
         endif
      endif

      if (present(data1)) then
         if (ndata.le.size(data1)) then
               data1 = -1
               i = wgrib2_get_reg_data(data1, ndata, 19)
               if (i.ne.0) then
                  write(*,*) 'FATAL ERROR: get_reg 19 ',i
                  grb2_inq = -1
                  return
               endif
         endif
         if (present(nread)) then
            nread = int(ndata, KIND=kind(nread))
         endif
      endif
      !
      !       does not work for staggered grids
      !
      if (present(data2)) then
         if (max(nnx,one)*max(nny,one) .ne. ndata) then
            write(*,*) '*** FATAL ERROR: ndata != nx*ny ', ndata, nnx, nny
            grb2_inq = -1
            return
         endif
         if (.not.allocated(data2)) then
            allocate(data2(max(nnx,one),max(nny,one)))
         else
            if (size(data2,1).ne.max(nnx,one) .or. size(data2,2).ne.max(nny,one)) then
               deallocate(data2)
               allocate(data2(max(nnx,one),max(nny,one)))
            endif
         endif
         i = wgrib2_get_reg_data(data2, ndata, 19)
         if (i.ne.0) then
            write(*,*) 'FATAL ERROR: get_reg 19 ',i
            grb2_inq = -1
            return
         endif
         if (present(nread)) then
            nread = int(ndata, KIND=kind(nread))
         endif
      endif

      if (present(lon)) then
         if (max(nnx,one)*max(nny,one) .ne. ndata) then
            write(*,*) '*** FATAL ERROR: ndata != nx*ny ', ndata, nnx, nny
            grb2_inq = -1
            return
         endif
         if (.not.allocated(lon)) then
            allocate(lon(max(nnx,one),max(nny,one)))
         else
            if (size(lon,1).ne.max(nnx,one) .or. size(lon,2).ne.max(nny,one)) then
               deallocate(lon)
               allocate(lon(max(nnx,one),max(nny,one)))
            endif
         endif
         i = wgrib2_get_reg_data(lon, ndata, 17)
         if (i.ne.0) then
            write(*,*) 'FATAL ERROR: get_reg 17 ',i
            grb2_inq = -1
            return
         endif
      endif

      if (present(lat)) then
         if (max(nnx,one)*max(nny,one) .ne. ndata) then
            write(*,*) '*** FATAL ERROR: ndata != nx*ny ', ndata, nnx, nny
            grb2_inq = -1
            return
         endif
         if (.not.allocated(lat)) then
            allocate(lat(max(nnx,one),max(nny,one)))
         else
            if (size(lat,1).ne.max(nnx,one) .or. size(lat,2).ne.max(nny,one)) then
               deallocate(lat)
               allocate(lat(max(nnx,one),max(nny,one)))
            endif
         endif
         i = wgrib2_get_reg_data(lat, ndata, 18)
         if (i.ne.0) then
            write(*,*) 'FATAL ERROR: get_reg 18 ',i
            grb2_inq = -1
            return
         endif
      endif

      return
   end function grb2_inq

!	grb2_rewind(file)
!	     rewinds internal wgrib2 file  .. grid is from N to S, default is from S to N
!

   !> This function rewinds the internal wgrib2 file by calling the -rewind_init [infile] option.
   !>
   !> @param[in] infile The name of the input file to rewind.
   !>
   !> @return 0 for success, error code otherwise.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   integer function grb2_rewind(infile)
      use wgrib2lowapi

      character (len=*), optional, intent(in):: infile
      character (len=300) :: lines(2)

      lines(1) = '-rewind_init'
      lines(2) = infile
      grb2_rewind = wgrib2c(2,lines,100)
      return
   end function grb2_rewind
!
!        grb2_ncep_uv: run wgrib2 infile -ncep_uv outfile
!               combines u and v together
!

   !> This function sets a substring in a string at a specified position.
   !>
   !> @param[in] string The original string to modify.
   !> @param[in] substring The substring to insert.
   !> @param[in] position The position to insert the substring.
   !>
   !> @return 0 for success, 8 for error.
   integer function grb2_set_substring(string, substring, position)

      character (len=*), intent(inout) :: string
      character (len=*), intent(in) :: substring
      integer, intent(in) :: position
      integer :: i, k, ilen
      integer :: start = 0   ! silence compiler

      grb2_set_substring = 8
      if (position.lt.1) return
      if (position.eq.1) then
         i = index(string,':')
         if (i.eq.0) return
         string = trim(substring) // string(i:)
         grb2_set_substring = 0
         return 
      endif

      ilen = len(string)
      k = 0
      do i = 1, ilen
         if (ichar(string(i:i)) .eq. ichar(':')) then
            k = k + 1
            if (k.eq.position-1) start = i
            if (k.eq.position) then
               string = string(1:start) // trim(substring) // string(i:)
               grb2_set_substring = 0
               return
            endif
         endif
      enddo
      return
   end function grb2_set_substring

!	grb2_get_substring(string,position)

   !> This function retrieves a substring from a string at a specified position.
   !>
   !> @param[in] string The original string from which to retrieve the substring.
   !> @param[in] position The position to retrieve the substring.
   !>
   !> @return The extracted substring or an empty string if not found.
   !>
   !> @author Wesley Ebisuzaki @date 12/2015
   character (len=200) function grb2_get_substring(string, position)

      character (len=*), intent(in) :: string
      integer, intent(in) :: position
      integer :: i, ilen, k
      integer :: start = 0   ! silence compiler

      if (position.lt.1) then
         grb2_get_substring = ''
         return
      endif
      if (position .eq. 1) then
         i = index(string,':')
         grb2_get_substring = string(1:i-1)
         return
      endif
      ilen = len(string)
      k = 0
      do i = 1, ilen
         if (ichar(string(i:i)) .eq. ichar(':')) then
            k = k + 1
            if (k.eq.position-1) start = i
            if (k.eq.position) then
               grb2_get_substring = string(start+1:i-1)
               return
            endif
         endif
      enddo
      grb2_get_substring = ''
      return
   end function grb2_get_substring

end module
