INC=-I../wgrib2 -I.

all:	grb2_mk_inv.o grb2_cmd.o grb2_inq.o

.c.o:
	$(CC) ${CPPFLAGS} ${CFLAGS} ${INC} -c $<

clean:
	touch junk.o; rm *.o

