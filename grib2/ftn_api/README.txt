               Callable wgrib2 for writing and reading grib2  v2

The FTN_API uses "callable wgrib2" which is a 2015 change to wgrib2 which 
allows C and fortran programs to call wgrib2 as a subroutine.  This change 
allows programs to read and write grib2 files using the features built 
into wgrib2.  Thanks to John Howard for his idea and code submission.

Level 0:

The callable wgrib2 allows a C program to call wgrib2 using the same 
arguments as a main C program; i.e., 

        return_code = wgrib2(int argc, const char **argv);

This is equivalent to the shell command (note: argv[0] is ignored)

	wgrib2 argv[1] argv[2] .. argv[n-1]
	return_code=$?

The equivalent fortran call is

	integer function wgrib2(n, argv)
	integer, intent(in) :: n
	character len(*), intent(in) :: argv(n)

This is equivalent to the shell command

	wgrib2 argv(1) argv(2) .. argv(n)
	return_code=$?

Since it is extra work to set up the array argv, it is convenient to call wgrib2
with a variable list of arguments.  For C, the first argument is the number of
strings that follow.

        return_code = wgrib2a(int N, const char *arg1, const char *arg2,..,const char *argN)

This is equivalent to the shell command

	wgrib2 arg1 arg2 .. argN
	return_code=$?


The equivalent fortran call is

	!      integer function wgrib2a(arg1,arg2,..,argN)
        return_code = wgrib2a(arg1,arg2,..,argN)

This is equivalent to the shell command

	wgrib2 arg1 arg2 .. argN
	return_code=$?


With the script version of wgrib2, all the variables are initialized before
execution.  With the callable wgrib2, all the variables are initialized
on the first call to wgrib2.  Flags, registers, memory files and temporary
files are retained as well the reading/writing locations.

              New File Types: temporary files

Some operations need a temporary file. For example, to read a
file, you may want a temporary inventory.  To write a file, you may
want to create a temporary template file.  Temporary files are
useful because they are automatically deleted and it use a system
determined filesystem.  To use a temporary file, you use a time 
like @tmp:XYZ where XYZ is an alpha-numeric string.

              New File Types: memory files

Memory files are blocks of memory that can be read or written by wgrib2.
They can be used to replace temporary files.  They can also be used to
convert a random access reads to a serial read.  Memory files can be
read and written by the calling program.  So you you want to decode
a grib message, you can write the grib message to a memory file
and then call wgrib2 to decode the memory file and write the
decoded message to another memory file.



00000000000000000000000000000000000000000000000000000000000000000000000000000000
Some options had to be added to allow the transfer of data between the calling and 
called routines.  Of course, we had to worry about memory leaks and freeing unused 
file handles.


Level 0
	integer function wgrib2c(n, args)
	character len(*) :: args(n)

        This is similar to the basic C interface and looks like the C main program.

Level 0.1
	integer function wgrib2([list of strings])
        equivalent to the command line:  wgrib2 [list of strings]

Level 1
	Reading:
	    Step 1: make a inventory

		grb2_mk_inv('FILE.grb', 'FILE.inv')

                equivalent to wgrib2 FILE.grb -match_inv >FILE.inv
                Note that the The inventory file can be a temporary file.

                temporary file:   @tmp:NAME

	    Step 2: Inquire the file
                i = grb2_inq('IN.grb','IN.inv',list of matches,list of optional arguments)

                With wgrib2, you have match strings to specify te desired grib message.
		  For example, you may want the 12-hour-fcst HGT for 2015010200 at 500 mb.
                  The wgrib2 match strings are "12 hour fcst", "HGT", "d=2015010200" and
                  "500 mb".   The function will return the number of matches in "i".  If i is zero,
                  no fields matched and the return values have undefined values.  If i is greater 
                  than zero, then the returned values are for the last match.  

                The optional arguments allow you to set values or return values.

                1. Getting the gridded values in WE:SN order

                   real, allocatable :: grid(:,:)

                   (assume only one z500 field in the file)
                   i = grb2_inq('IN.grb', 'IN.inv', 'HGT', '500 mb', grid=grid)

		   if (i.eq.1) write(*,*) 'nx=',size(grid,1),' ny=',size(grid,2)
		   if (i.eq.1) write(*,*) 'grid(1,1)=', grid(1,1)

                2. Getting the lat, lon and gridded values in WE:SN order
                  
                   real, allocatable :: grid(:,:), lon(:,:), lat(:,:)

                   (assume only one z500 field in the file)
                   i = grb2_inq('IN.grb','IN.inv','HGT','500 mb',grid=grid,lon=lon,lat=lat)
		
		   if (i.eq.1) write(*,*) 'nx=',size(grid,1),' ny=',size(grid,2)
		   if (i.eq.1) write(*,*) 'grid(1,1)=', grid(1,1)
		   if (i.eq.1) write(*,*) 'lat/lon=', lat(1,1), lon(1,1)

                Note: wgrib2's default is to convert data to WE:SN order.  Having the data in WE:NS
                order is possible but with reduced functionality.

	Writing
	    Step 1: make a template file
                A template is a sample grib2 message with the appropriate grid and metadata
                like center, subcenter, process id, etc.  For any given conversion, you
                usually only need one grib2 template and for speed, you should set the grid
                values to zero by

                     wgrib2 template.slow -rpn 0 -grib_out template.fast

                Most of the time, it is easy to make a template.
                   - if a grib2 file exists, you can use wgrib2's -new_grid get the right grid
                   - if a grib1 file exists, you can use cnvgrb to make the grib2 template
                   - if a grib2 with the right template (PDT) exists, modify metadata with with wgrib2
                   - creating a grib2 message with the right PDT by using wgrib2's set_byte option.

	    Step 2: erase output file if necessary

	    Step 3a: write grib message using metadata string

                real, allocatable :: grid(:,:)
                allocate(grid(nx,ny))
                .. 
                i = grb2_wrt('OUT.grb','TMPLATE.grb','1',data2=grid, &
                       meta='0:1:d=2001020304:TMP:200 mb:4 hour fcst:')
                write(*,*) 'ierr=',i

                '1' is the grib message number of the template file
                meta is a wgrib2 metadata string (-set_metadata)

	    Step 3b: write grib message using optional parameters

                integer (kind=8) :: date
                real, allocatable :: grid(:,:)
                allocate(grid(nx,ny))
                .. 
                date=2001010218
                i = grb2_wrt('OUT.grb','TMPLATE.grb','1',data2=grid,date=date,var='UGRD',mb=0.01,fhour=234)
                .. 
                i = grb2_wrt('OUT.grb','TMPLATE.grb','1',data2=grid,date=date,var='ULWRF',level='surface'.&
                  fhour_ave1=0,fhour_ave2=6,center=91,subcenter=1)

            Note: the order of the data can be: raw, we:sn or we:ns.


                date=2001010218
                i = grb2_wrt('OUT.grb','TMPLATE.grb','1',data2=grid,date=date,var='UGRD',mb=0.01,fhour=234)

                i = grb2_wrt('OUT.grb','TMPLATE.grb','1',data2=grid,date=date,var='ULWRF',level='surface'.&
                  fhour_ave1=0,fhour_ave2=6,center=91,subcenter=1)

            Note: the order of the data can be: raw, we:sn or we:ns.

Comments: 1) grids need to be allocatable arrays.  grb2_inq will allocate/change the dimension if necessary.
             The calling program can deallocate the arrays after usage.

          2) The grb2_inq and grb2_wrt are very simple codes and are not frozen.


          Memory files:

		Make a memory file:  grib2_set_mem_buffer(n, size_buffer, buffer)
			n:  memory file (0..19)
			size_buffer:  number of bytes in the memory file
			buffer: buffer[0..size_buffer-1] is copied to the memory file

		To clear a memory file:
			grib2_set_mem_buffer(n, 0, NULL)

		To get the size of a memory file n:
			i  =  wgrib2_get_mem_buffer_size(n)
