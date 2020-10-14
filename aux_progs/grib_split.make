p=grib_split

INC=-I../wgrib2
W=../wgrib2

$p:	$p.c
	${CC} ${CFLAGS} ${INC} -DSIMPLE_FATAL -o $p $p.c $W/rd_grib2_msg.c  $W/intpower.c $W/int8.c $W/seekgrib2.c $W/fatal_error.c -lm

clean:
	touch $p ; rm $p
