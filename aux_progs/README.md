1/2023           aux_programs

Some programs for multitasking wgrib2

gmerge, smallest_grib2, smallest_4 assume that there is no
junk between the grib messages.


gmerge:  combines two or more streams of grib files
         the output takes one grib message from the 1st file/pipe
            one grib message from the 2nd file/pipe
            and so on until you reach the last file/pipe.  Then
            the process starts again from the 1st file/pipe.

         The program is to split the workload over several copies of wgrib2.

            mkfifo pipe1 pipe2
            wgrib2 IN.grb -for 1::2 -set_grib_type j -grib_out pipe1 &
            wgrib2 IN.grb -for 2::2 -set_grib_type j -grib_out pipe2 &
            gmerge OUT.grb pipe1 pipe2
            rm pipe1 pipe2

        OUT.grb contains the jpeg compressed version of IN.grb.  The compression
        was run as two tasks.

smallest_grib2:  takes 3 input files/pipes.
         smallest_grib2 reads 1 grib message from all 3 files/pipes
            and writes the shortest (byte length) to the output file/pipe.
         This process continues until the file 1 runs out of data.

         This program is used for compression.

            mkfifo pipe1 pipe2 pipe3
            wgrib2 IN.grb -set_grib_type c1 -grib_out pipe1 \
                          -set_grib_type c2 -grib_out pipe2 \
                          -set_grib_type c3 -grib_out pipe3 &
            smallest_grib2 OUT.grb pipe1 pipe2 pipe3
            rm pipe1 pipe2 pipe3

         OUT.grb contains the same data as IN.grb but using the
         best complex packing (c1, c2 or c3).


smallest_4:  like small_grib2 but takes 4 input files/pipes.

            mkfifo pipe1 pipe2 pipe3 pipe4
            wgrib2 IN.grb -set_grib_type c1 -grib_out pipe1 \
                          -set_grib_type c2 -grib_out pipe2 \
                          -set_grib_type c3 -grib_out pipe3 \
                          -set_grib_type j  -grib_out pipe4 &
            smallest_grib2 OUT.grb pipe1 pipe2 pipe3 pipe4
            rm pipe1 pipe2 pipe3 pipe4

         OUT.grb contains the same data as IN.grb but compressed using
         the best of jpeg and complex packing.

grib_split:  
	1/2023 .. C code fails to compile because of changes in wgrib2.
	Fixing the code is involved because the code needs to assume 
	that there may be junk between the grib messages.  The best 
	way is to replace the program by wgrib2.
         
	grib_split.sh takes a grib2 file and splits it into N pieces.
	The first grib message goes to out1, 2nd grib message goes to
	out2, and round robin the rest.

	input and output files can be pipes
	grib_split IN OUT-1 OUT-2 .. OUT-N

	mkfifo pipe1a pipe2a pipe1b pipe2b
	grib_split.sh IN.grb pipe1a pipe2a &
	wgrib2 pipe1a -set_grib_type j -grib_out pipe1b &
	wgrib2 pipe2a -set_grib_type j -grib_out pipe2b &
	gmerge OUT.grb pipe1a pipe2a

	grib_split and -for/-for_n are two solutions to the same problem
	noticed that on an linux/NFS system, the system time was getting
        when 12 processes were reading the same file.

	grib_split.sh replaces grib_split.

	Note: I rarely use grib_split because it is easier to just use wgrib2.

-----------------------------------------------

               To compile

1. make -f [program].make

or

2 cmake -S . -B .
  make

Both methods use the default C compiler.  The later compiles all the
codes.  To use a different compiler, set CC with the compiler name.
