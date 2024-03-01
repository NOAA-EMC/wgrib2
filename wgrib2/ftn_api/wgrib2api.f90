! fortran api for reading and writing grib2
! 12/2015  Wesley Ebisuzaki   Public Domain
!
! fortran 2003 / fortran 95 + TR 15581  API to read and write grib2 files
!
!   requirements:
!        f2003 or f95 + TR 15581  (allowing subroutines to deallocate and allocate)
!        callable wgrib2
!        fort_wgrib2.c (wgrib2c wrapper for callable wgrib2)
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
!    grb2_filter :: filters a grib file by 
!        grb2_filter('IN.grb', 'OUT.grb', ...)
!
!    grb2_free_file :: releases files handles after next call to wgrib2
!        grb2_free_file('FILE.grb')
!
!    grb2_inq :: grib2 inquire
!        grb2_inq('IN.grb', 'IN.inv', ...)
!         uses memory files @mem:19  .. always
!
!    grb2_wrt :: grib2 write
!       grb2_wrt('OUT.grb', 'TEMPLATE.grb', 'ID', ...)
!       TEMPLATE.grb is a grib file that is used as a template for the new grib file
!
!       meta = (string) .. sets metadata
!	order = 'raw' .. fastest data must be in same order as template
!	order = 'we:ns'  .. data must be in we:nw order
!	order = 'we:sn'  .. data must be in we:sn order
!

module wgrib2api
use wgrib2lowapi

	real, parameter ::   grb2_UNDEFINED = 9.999e20
        integer, private :: next_mem
        private :: reserve_mem_buffer_name, all_line
contains

!       character (len=)  reserve_mem_buffer_name(buffer)
!       this function 1) reserves a memory file (@mem:XX)
!                     2) returns the name of the memory file
!                     3) returns the integer value of the memory file
!                     4)

        character (len=7) function reserve_mem_buffer_name(buffer)
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

	logical function grb2_DEFINED_VAL(x)
	real, intent(in) :: x
	grb2_DEFINED_VAL = (x < 9.9989e20) .or. (x > 9.9991e20)
	return
	end function grb2_DEFINED_VAL

	logical function grb2_UNDEFINED_VAL(x)
	real, intent(in) :: x
	grb2_UNDEFINED_VAL = (x >= 9.9989e20) .and. (x <= 9.9991e20)
	return
	end function grb2_UNDEFINED_VAL

        integer function grb2_var_args(lines,istart,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11, &
          a12,a13,a14,a15,a16,a17,a18,a19,a20)

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

100        continue

        grb2_var_args = n
        return
        end function

        integer function add_line(lines,n,line)

        character (len=*), intent(inout):: lines(:)
        character (len=*), intent(in):: line
        integer, intent(inout) :: n

        n = n + 1
        lines(n) = line
        add_line = 0

        return
        end function add_line

        integer function wgrib2a(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11, &
          a12,a13,a14,a15,a16,a17,a18,a19,a20)

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

!
!        grb2_mk_inv
!        writes the match inventory to a file
!
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

!        integer function grb2_filter(ingrbfile, outgrbfile, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,&
!              a11,a12,a13,a14,a15,a16,a17,a18,a19,a20)
!	use wgrib2lowapi
!        implicit none
!        character (len=*), intent(in):: ingrbfile, outgrbfile
!        character (len=*), optional, intent(in):: a1,a2,a3,a4,a5,a6,a7,a8,a9,a10
!        character (len=*), optional, intent(in):: a11,a12,a13,a14,a15,a16,a17
!        character (len=*), optional, intent(in):: a18,a19,a20
!
!        character (len=300) :: lines(20), cmd(80)
!	integer :: n, m, i, j
!
!        m = grb2_var_args(lines,0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11, &
!          a12,a13,a14,a15,a16,a17,a18,a19,a20)
!
!	n = 0
!        i = add_line(cmd,n,ingrbfile)
!        i = add_line(cmd,n,'-rewind_init')
!        i = add_line(cmd,n,ingrbfile)
!        i = add_line(cmd,n,'-transient')
!        i = add_line(cmd,n,ingrbfile)
!        i = add_line(cmd,n,'-grib')
!        i = add_line(cmd,n,outgrbfile)
!        i = add_line(cmd,n,'-inv')
!       ** this is linux/unix
!        i = add_line(cmd,n,'/dev/null')
!       ** this is for windows
!       i = add_line(cmd,n,'NUL')
!
!        do j = 1, m
!            i = add_line(cmd,n,'-match')
!            i = add_line(cmd,n,lines(j))        
!        enddo
!
!        do i = 1, n
!           write(*,*) 'grb2_filter>>> i=',i,trim(cmd(i))
!        enddo
!
!        grb2_filter = wgrib2c(n,cmd, len(cmd(1)))
!
!        return
!        end function grb2_filter
!
!       grb2_free_file(name)
!       frees file from wgrib2's list of open files

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

!
!        grb2_wrt: grib2 wrt
!               write grib message
!
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
        integer :: i,n, yrev
        logical :: present_data1, present_data2

	integer (C_INT) :: ierr
	integer (C_SIZE_T) :: ndata, nx, ny
	real (C_FLOAT), allocatable :: wgrib2_data(:)

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
        end function


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
        integer function grb2_inq(grbfile, invfile, &
            a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11, &
            a12,a13,a14,a15,a16,a17,a18,a19,a20, &
            npts,nx,ny,nmatch,msgno,submsg,data1,nread,data2,lat,lon,ref_date,ref_edate, &
            verf_date, verf_edate,start_date,start_edate,end_date,end_edate,order,lastuse,close,&
            desc,grid_desc,debug,copy,sequential,regex,get_ref_edate, get_start_edate, &
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
	integer (C_SIZE_T) :: ndata, nnx, nny, imsgno, one, iret

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
9000	    format(':d=',i10.10)
	    i = add_line(cmd,n,tmpline)
	endif

!	ref_edate: add to match command
	if (present(ref_edate)) then
            i = add_line(cmd,n,'-egrep')
	    write(tmpline,9010) ref_edate
9010	    format(':D=',i14.14)
	    i = add_line(cmd,n,tmpline)
	endif

!	verf_date: add to match command
	if (present(verf_date).or.present(start_date)) then
            i = add_line(cmd,n,'-egrep')
	    write(tmpline,9020) verf_date
9020	    format(':end_FT=',i10.10)
	    i = add_line(cmd,n,tmpline)
	endif

!	verf_edate: add to match command
	if (present(verf_edate).or.present(start_edate)) then
            i = add_line(cmd,n,'-egrep')
	    write(tmpline,9030) verf_date
9030	    format(':end_FT=',i14.14)
	    i = add_line(cmd,n,tmpline)
	endif

!	start_date: add to match command
	if (present(start_date)) then
            i = add_line(cmd,n,'-egrep')
	    write(tmpline,9040) verf_date
9040	    format(':start_FT=',i10.10)
	    i = add_line(cmd,n,tmpline)
	endif

!	start_edate: add to match command
	if (present(start_edate)) then
            i = add_line(cmd,n,'-egrep')
	    write(tmpline,9050) verf_date
9050	    format(':start_FT=',i14.14)
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
100     continue
        write(*,*) 'FATAL ERROR: grb2_inq reading grid info'
        grb2_inq = -1
        return
110     continue
!	write(*,*) 'ndata=',ndata, nnx, nny

        if (present(npts)) then
           npts = ndata
        endif
        if (present(nx)) then
           nx = nnx
        endif
        if (present(ny)) then
           ny = nny
        endif
        if (present(nmatch)) then
            nmatch = grb2_inq
        endif
        if (present(msgno)) then
            msgno = imsgno
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
                nread = ndata
            endif
        endif
!
!       does not work for staggered grids
!
        if (present(data2)) then
            if (max0(nnx,one)*max0(nny,one) .ne. ndata) then
                write(*,*) '*** FATAL ERROR: ndata != nx*ny ', ndata, nnx, nny
                grb2_inq = -1
                return
            endif
            if (.not.allocated(data2)) then
                allocate(data2(max0(nnx,one),max0(nny,one)))
            else
                if (size(data2,1).ne.max0(nnx,one) .or. size(data2,2).ne.max0(nny,one)) then
                    deallocate(data2)
                    allocate(data2(max0(nnx,one),max0(nny,one)))
                endif
            endif
            i = wgrib2_get_reg_data(data2, ndata, 19)
            if (i.ne.0) then
                write(*,*) 'FATAL ERROR: get_reg 19 ',i
                grb2_inq = -1
                return
            endif
            if (present(nread)) then
                nread = ndata
            endif
        endif

	if (present(lon)) then
            if (max0(nnx,one)*max0(nny,one) .ne. ndata) then
                write(*,*) '*** FATAL ERROR: ndata != nx*ny ', ndata, nnx, nny
                grb2_inq = -1
                return
            endif
            if (.not.allocated(lon)) then
                allocate(lon(max0(nnx,one),max0(nny,one)))
            else
                if (size(lon,1).ne.max0(nnx,one) .or. size(lon,2).ne.max0(nny,one)) then
                    deallocate(lon)
                    allocate(lon(max0(nnx,one),max0(nny,one)))
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
            if (max0(nnx,one)*max0(nny,one) .ne. ndata) then
                write(*,*) '*** FATAL ERROR: ndata != nx*ny ', ndata, nnx, nny
                grb2_inq = -1
                return
            endif
            if (.not.allocated(lat)) then
                allocate(lat(max0(nnx,one),max0(nny,one)))
            else
                if (size(lat,1).ne.max0(nnx,one) .or. size(lat,2).ne.max0(nny,one)) then
                    deallocate(lat)
                    allocate(lat(max0(nnx,one),max0(nny,one)))
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
        end function

!	grb2_rewind(file)
!	     rewinds internal wgrib2 file  .. grid is from N to S, default is from S to N
!
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

!	grb2_set_substring(string,substring,position)
!               returns 0 if ok
!               returns 8 if error

	integer function grb2_set_substring(string, substring, position)

	character (len=*), intent(inout) :: string
	character (len=*), intent(in) :: substring
	integer, intent(in) :: position
	integer :: i, k, ilen, start

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

	character (len=200) function grb2_get_substring(string, position)

        character (len=*), intent(in) :: string
        integer, intent(in) :: position
        integer :: i, ilen, k, start

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
