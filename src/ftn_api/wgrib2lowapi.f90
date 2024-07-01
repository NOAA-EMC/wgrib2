! fortran lowapi for reading and writing grib2
! 10/2015  Wesley Ebisuzaki   Public Domain
!
!   requirements:
!        f2003 or f95 + TR 15581  (allowing subroutine to deallocate and allocate)
!        callable wgrib2
!        fort_wgrib2.c (wgrib2c wrapper for callable wgrib2)
!	 f2003 or f95 with iso_c_binding support
!
! Provides:
!    wgrib2a(list of strings) :: same as $ wgrib2 [list of strings]
!        not used by grb2_* routines
!    wgrib2c(argc, argv) :: same as $ wgrib2 [list of strings]
!
!    integer wgrib2_get_reg_size(integer reg)
!    integer wgrib2_get_reg_data(real data(ndata), integer ndata, integer reg)
!    integer wgrib2_set_reg_data(real data(ndata), integer ndata, integer reg)
!    integer wgrib2_get_mem_buffer_size(integer n)
!    integer wgrib2_get_mem_buffer(buffer, size_buffer, n)
!    integer wgrib2_set_mem_buffer(buffer, size_buffer, n)
!
!   requirements:
!        f2003 or f95 + TR 15581  (allowing subroutine to deallocate and allocate)
!        callable wgrib2
!        fort_wgrib2.c (wgrib2c wrapper for callable wgrib2)
!	 f2003 or f95 with iso_c_binding support
!
! Provides:
!    wgrib2a(list of strings) :: same as $ wgrib2 [list of strings]
!        not used by grb2_* routines
!    wgrib2c(argc, argv) :: same as $ wgrib2 [list of strings]
!
!    integer wgrib2_get_reg_size(integer reg)
!    integer wgrib2_get_reg_data(real data(ndata), integer ndata, integer reg)
!    integer wgrib2_set_reg_data(real data(ndata), integer ndata, integer reg)
!    integer wgrib2_get_mem_buffer_size(integer n)
!    integer wgrib2_get_mem_buffer(buffer, size_buffer, n)
!    integer wgrib2_mk_file_transient(string)
!


module wgrib2lowapi
    USE ISO_C_BINDING
    interface
        integer (C_SIZE_T) function wgrib2_get_reg_size(reg) bind(C)
            USE ISO_C_BINDING
            integer (C_INT), value :: reg
        end function wgrib2_get_reg_size

        integer (C_INT) function wgrib2_get_reg_data(data, ndata, reg) bind(C)
            USE ISO_C_BINDING
            integer (C_SIZE_T), value :: ndata
            integer (C_INT), value :: reg
            real (C_FLOAT) :: data(ndata)
        end function wgrib2_get_reg_data

        integer (C_INT) function wgrib2_set_reg(data, ndata, reg) bind(C)
            USE ISO_C_BINDING
            integer (C_SIZE_T), value :: ndata
            integer (C_INT), value :: reg
            real (C_FLOAT) :: data(ndata)
        end function wgrib2_set_reg

        integer (C_SIZE_T) function wgrib2_get_mem_buffer_size(n) bind(C)
            USE ISO_C_BINDING
            integer (C_INT), value :: n
        end function wgrib2_get_mem_buffer_size

	integer (C_INT) function wgrib2_get_mem_buffer(buffer, size_buffer, n) bind(C)
            USE ISO_C_BINDING
            integer (C_INT), value :: n
            integer (C_SIZE_T), value :: size_buffer
	    character (kind=c_signed_char) :: buffer(*)
        end function wgrib2_get_mem_buffer

	integer (C_INT) function wgrib2_set_mem_buffer(buffer, size_buffer, n) bind(C)
            USE ISO_C_BINDING
            integer (C_INT), value :: n
            integer (C_SIZE_T), value :: size_buffer
	    character (kind=c_signed_char) :: buffer(*)
        end function wgrib2_set_mem_buffer

        integer (C_INT) function wgrib2_free_file(string) bind(C)
            USE ISO_C_BINDING
            character (kind=c_char) :: string(*)
        end function wgrib2_free_file

	integer (C_INT) function wgrib2c(n, buffer, len) bind(C)
	    USE ISO_C_BINDING
            integer (C_INT), value :: n, len
	    character (kind=c_char) :: buffer(*)
	end function wgrib2c

    end interface
end module
