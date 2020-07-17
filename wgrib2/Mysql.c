/******************************************************************************************
 Copyright (C) 2008 Niklas Sondell, Storm Weather Center
 This file is part of wgrib2 and could be distributed under terms of the GNU General Public License


 Example of mysql table structure for use with this script:
 create table wgrib2 (rt datetime, vt datetime, lat double, lon double, param varchar(80), level varchar(30), value double); 

 v1.0 5/2008
      10/2009 Jerry Stueve, tmpnam -> mkstemp, "LOAD DATA <<LOCAL>>" added
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <unistd.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_MYSQL

#include <mysql.h>

extern int decode;
extern int latlon;
extern double *lat, *lon;
//old: extern float *lat, *lon;


/*
 * HEADER:100:mysql:output:5:H=[host] U=[user] P=[password] D=[db] T=[table]
 */

int f_mysql(ARG5) {

    char sql[300];
    char server[100];
    char user[100];
    char password[100]; 
    char database[100];
    char table[100];

    char name[100], desc[100], unit[100], level_buf[100];
    int year, month, day, hour, minute, second;
	
    char vt[20],rt[20];
//    unsigned char *p;
	
    int j;
    double longitude,latitude;
	
    struct local_struct {
        MYSQL *conn;
        char filename[200];
    };
    struct local_struct *save;

    FILE *file;
    int tempfile;

    strcpy(server,arg1);
    strcpy(user,arg2);
    strcpy(password,arg3);
    strcpy(database,arg4);
    strcpy(table,arg5);


    /* initialization phase */

    if (mode == -1) {
        decode = latlon = 1;
            
	*local = save = (struct local_struct *) malloc(sizeof(struct local_struct));
	if (save == NULL) fatal_error("mysql memory allocation ","");

	save->conn = mysql_init(NULL);

	/* Connect to database */
	if (!mysql_real_connect(save->conn, server, user, password, database, 0, NULL, 0))
	    fatal_error("mysql error %s", mysql_error(save->conn));
    }

    /* cleanup phase */
    else if (mode == -2) {
    	save = (struct local_struct *) *local;
    	mysql_close(save->conn);
	free(save);
    }

    /* processing phase */
    else if (mode >= 0) {
    	save = (struct local_struct *) *local;

        /*Collect runtime and validtime into vt and rt*/
        reftime(sec, &year, &month, &day, &hour, &minute, &second);
//	p = sec[1];
//	sprintf(rt, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", (p[12]<<8)+p[13], p[14],p[15],p[16],p[17],p[18]);
        sprintf(rt, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year,month,day,hour,minute,second);

        if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            sprintf(vt,"%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year,month,day,hour,minute,second);
        }

	/* Get levels, parameter name, description and unit*/
        // f_lev(mode, sec, data, ndata, level_buf, local);
        f_lev(call_ARG0(level_buf, NULL));

   
        if (strcmp(level_buf, "reserved") == 0) return 0;

        strcpy(save->filename, "/tmp/wgrib2_mysqlXXXXXX");
        if ((tempfile = mkstemp(save->filename)) == -1) fatal_error("mysql filename setup","");

        fprintf(stdout, "\nUsing temporary pathname %s\n", save->filename);
        if ((file = fdopen(tempfile, "w")) == NULL ) fatal_error("mysql: error opening temporary filename: %s", save->filename);

        getName(sec, mode, NULL, name, desc, unit);
	fprintf(stderr,"Start processing of %s at %s\n", name, level_buf);
	fprintf(stderr,"Gridpoints in data: %d\n", ndata);
	fprintf(stderr,"Description: %s, Unit %s\n", desc,unit);
	
	printf("Inserting into database, table=%s, runtime=%s, validtime=%s, parameter=%s, level=%s \n",table,rt,vt,name,level_buf);
	
	for (j = 0; j < ndata; j++) {
	    if (!UNDEFINED_VAL(data[j])) {
		longitude = lon[j];
		latitude = lat[j];
		if (longitude > 180) longitude -= 360;
		fprintf(file,"\"%s\",\"%s\",%g,%g,\"%s\",\"%s\",%lg\n",rt,vt,latitude,longitude,name,level_buf,data[j]);
	    }
	}
	fclose(file);
	sprintf(sql,"LOAD DATA LOCAL INFILE '%s' INTO TABLE %s FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES TERMINATED BY '\\n'",save->filename,table);
	printf("%s \n",sql);
	/* send SQL query */
	if (mysql_query(save->conn, sql)) fatal_error("mysql: %s", mysql_error(save->conn));

//      unlink = POSIX, remove = ANSI
//        unlink(save->filename);
        remove(save->filename);
    }
    return 0;
}

#else

int f_mysql(ARG5) {
    if (mode == -1) { fprintf(stderr,"mysql package not installed\n"); return 1; }
    return 0;
}

#endif
