/******************************************************************************************
 Copyright (C) 2008 Niklas Sondell, Storm Weather Center
 This file is part of wgrib2 and could be distributed under terms of the GNU General Public License

  v1.1 10/2009 Jerry Stueve: tmpnam -> mkstemp
        4/2011 WNE change char temp_pathname[L_tmpnam] to char temp_pathname[STRING_SIZE];
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_MYSQL

#include <mysql.h>


extern int decode, GDS_change_no, latlon;
extern double *lat, *lon;

//maximum number of rows to insert (similiar to number of gridpoints)
#define MAX_NXNY   2000000


static double convunit(double val, char convstring[50]) {
		
	if (strcmp(convstring,"GPH_M")==0) val/=9.82;
	else if (strcmp(convstring,"M_MM")==0) val*=1000;
	else if (strcmp(convstring,"PERC_PART")==0 || strcmp(convstring,"PA_HPA")==0) val/=100;
	else if (strcmp(convstring,"PART_PERC")==0) val*=100; 
	else if (strcmp(convstring,"K_C")==0) val-=273.16;
	return val;
}

/*
 * HEADER:100:mysql_dump:output:7:H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0|1] PV=[remove unlikely:0|1]
 */

int f_mysql_dump(ARG7) {

    char temp_pathname[STRING_SIZE];
    char sql[3000];
    char server[100];
    char user[100];
    char password[100]; 
    char database[100];
    char table[100];
    MYSQL_RES *res;
    MYSQL_ROW row;

    char name[100], desc[100], unit[100], level_buf[100];
    int year, month, day, hour, minute, second;
	
    char vt[20],rt[20];
//    unsigned char *p;
   		
    unsigned int i,j;
    double longitude,latitude,value;

    char new_level[50];
    char new_name[50];
    char conv[50];
    char precision[10];
    char last;
    char param[50];
    int ctr;
    int level;
    unsigned int load_local_infile;

    struct local_struct {
        MYSQL *conn;
        FILE *temp_fileptr;
        unsigned int npts;
        char *rows[MAX_NXNY];
        char *params;
	char *columns;
        unsigned int isset;
        unsigned int wlon;
        unsigned int remove_unlikely;
	// unsigned int points[MAX_NXNY];
        char runtime[20], validtime[20];
	int last_GDS_change_no;
    };
    struct local_struct *save;

    strcpy(server,arg1);
    strcpy(user,arg2);
    strcpy(password,arg3);
    strcpy(database,arg4);
    strcpy(table,arg5);

    /* initialization phase */

    if (mode == -1) {
        decode = latlon = 1;
        
	*local = save = (struct local_struct *) malloc( sizeof(struct local_struct));
	if (save == NULL) fatal_error("mysql_speed memory allocation ","");

        for (i = 0; i < MAX_NXNY; i++) save->rows[i] = NULL;

	save->conn = mysql_init(NULL);
	save->temp_fileptr=0;
	save->params = (char *)  malloc(1500*sizeof(char));
	save->columns = (char *)  malloc(2500*sizeof(char));
	sprintf(save->columns,"%s","");
	sprintf(save->params,"%s","");
	save->isset = 0;
	save->runtime[0] = 0;
	save->validtime[0] = 0;
	load_local_infile = 1;		// 1 = LOAD LOCAL INFILE,  0 = do not LOAD LOCAL INFILE

	if (sscanf(arg6,"%d", &save->wlon) != 1) {
            fatal_error("Argument 6, use western longitudes, has to be 0 or 1, error parsing %s", arg6);
        }
        if (sscanf(arg7,"%d", &save->remove_unlikely) != 1) {
            fatal_error("Argument 7, remove unlikely values, has to be 0 or 1, error parsing %s", arg7);
        }
	
	/* Set options for database */
	mysql_options(save->conn,MYSQL_OPT_LOCAL_INFILE, (char *) &load_local_infile);

	/* Connect to database */
	if (!mysql_real_connect(save->conn, server, user, password, database, 0, NULL, 0)) {
	   fatal_error("f_mysql_speed: could not connect to %s", mysql_error(save->conn));
	} 
	save->last_GDS_change_no = 0;
	return 0;
    }

    /* cleanup phase */

    if (mode == -2) {
    	save = (struct local_struct *) *local;
    	
    	strcpy(temp_pathname, "/tmp/wgrib2_mysqlXXXXXX");
    	if ( -1 == (load_local_infile = mkstemp(temp_pathname))) {
	    fatal_error("f_mysql_speed: error making temporary filename","");
	}
	if ( !(save->temp_fileptr = fdopen(load_local_infile, "w")) ) {
	    fatal_error("f_mysql_speed: error making temporary filename","");
	}
	
	//Create table, if not exists...
	sprintf(sql,"create table if not exists %s (rt DATETIME NOT NULL,vt DATETIME NOT NULL,lat double NOT NULL,lon double NOT NULL%s, primary key (rt,vt,lat,lon))",table,save->columns);
	printf("Table create statement, if not exists:\n %s\n",sql);
	if (mysql_query(save->conn, sql)) {	
	    fatal_error("f_mysql_speed: connection error %s", mysql_error(save->conn));
	}
	//End create table

    	fprintf(stdout, "Columns to insert: rt,vt,lat,lon%s\n", save->params);
    	fprintf(save->temp_fileptr,"rt,vt,lat,lon%s\n",save->params);
    	for (j = 0; j < save->npts; j++) {
	    fprintf(save->temp_fileptr,"%s\n",save->rows[j]);
	}
	fflush(save->temp_fileptr);
	sprintf(sql,"LOAD DATA LOCAL INFILE '%s' INTO TABLE %s FIELDS TERMINATED BY ',' ENCLOSED BY '\"' LINES TERMINATED BY '\\n' IGNORE 1 LINES (rt,vt,lat,lon%s)",temp_pathname,table,save->params);
	printf("Inserting into database, table=%s \n",table);
	printf("%s\n",sql);
	/* send SQL query */
	if (mysql_query(save->conn, sql)) {	
	    fatal_error("f_mysql_speed: connection error %s", mysql_error(save->conn));
	}
	fclose(save->temp_fileptr);
//      unlink = POSIX, remove = ANSI
//	unlink(temp_pathname);
        remove(temp_pathname);
    	mysql_close(save->conn);
    	return 0;
    }
    /* processing phase */

    save = (struct local_struct *) *local;

//     if (new_GDS && save->isset == 0) { 
       if (save->last_GDS_change_no != GDS_change_no && save->isset == 0) {
  	save->npts = GB2_Sec3_npts(sec);
  	if (save->npts > MAX_NXNY) {
	    fatal_error_i("f_mysql_speed: MAX_NXNY exceeded %d", MAX_NXNY);
  	}

	for (i = 0; i < save->npts; i++) {
	    if (save->rows[i] == NULL) {
		save->rows[i] = (char *)  malloc(1500*sizeof(char));
		if (save->rows[i] == NULL) fatal_error("f_mysql_speed: memory allocation problem","");
	    }
	    save->rows[i][0] = '\0';
	}

	save->isset = 1;
//   } else if (new_GDS) {
  } else if (save->last_GDS_change_no != GDS_change_no) {
	fatal_error("f_mysql_speed, grid definition has to be the same for all fields","");
  }
  save->last_GDS_change_no = GDS_change_no;
  
    /*Collect runtime and validtime into vt and rt*/
    reftime(sec, &year, &month, &day, &hour, &minute, &second);
//  p = sec[1];
//  sprintf(rt, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", (p[12]<<8)+p[13], p[14],p[15],p[16],p[17],p[18]);
    sprintf(rt, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year,month,day,hour,minute,second);

    if (verftime(sec, &year, &month, &day, &hour, &minute, &second) == 0) {
            sprintf(vt,"%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year,month,day,hour,minute,second);
    }
	/*Check that runtimes are equal and validtimes are equal*/
    if (save->runtime[0] != 0 && strcmp(save->runtime,rt) != 0) {
	fprintf(stderr, "Error, runtime has to be equal for all fields\n");
    }
    strcpy(save->runtime,rt);

    if (save->validtime[0] != 0 && strcmp(save->validtime,vt) != 0) {
	fprintf(stderr, "Error, validtime has to be equal for all fields\n");
    }
    strcpy(save->validtime,vt);
	
    /*Get levels, parameter name, description and unit*/
    // f_lev(mode, sec, data, ndata, level_buf, local);
    f_lev(call_ARG0(level_buf,NULL));
    if (ndata != save->npts && save->npts>0) fprintf(stderr,"ERROR: fields do not contain equally many gridpoints, %u , %u \n",save->npts,ndata);
 	   
    if (strcmp(level_buf, "reserved")==0) return(0);
    getName(sec, mode, NULL, name, desc, unit);
    fprintf(stderr,"Start processing of %s at %s, runtime=%s, validtime=%s \n", name, level_buf,rt,vt);

    ctr = GB2_Center(sec);
	
	sprintf(sql,"select * from wgrib2_parameter_mapping where center_id=%d and wgrib_param_name='%s' and wgrib_param_levelname='%s'", ctr, name, level_buf);
	fprintf(stderr,"SQL: %s\n", sql);
	if (mysql_query(save->conn, sql)) {	
		fatal_error("f_mysql_speed: mysql error %s", mysql_error(save->conn));
	}
	sprintf(new_level,"%s", "None");
	sprintf(conv,"%s", "None");
	sprintf(precision, "%s", "2");
	printf("\nCenter_id: %d \n", ctr);
	res = mysql_use_result(save->conn);
	while ( (row = mysql_fetch_row(res) ) != NULL) {
		printf("our_name: %s \n", row[3]);
		sprintf(new_name,"%s", row[3]);
		sprintf(new_level,"%s", row[4]);
		sprintf(conv,"%s", row[5]);
		sprintf(precision, "%s", row[6]);
	}

	printf("val_precision: %s \n", precision);
	printf("conversion: %s \n", conv);
	printf("our_levelname: %s \n", new_level);
	
	if (strcmp(new_level,"None") != 0) {
		fprintf(stderr,"Sets level %s as level %s\n", level_buf, new_level);
		sprintf(param, "%s_%s", new_name, new_level);
	} else {
		if (strcmp(level_buf,"surface") == 0) {
			sprintf(param, "%s_0", name);
		} else {		
			sscanf(level_buf,"%d %s", &level, &last);
			sprintf(param, "%s_%d", name, level);
		}
	}
	
	mysql_free_result(res);

	strcat(save->params,",");
	strcat(save->params,param);
	strcat(save->columns,",");
	strcat(save->columns,param);
	strcat(save->columns," double");
	
	fprintf(stderr,"Gridpoints in data: %u\n", ndata);
	fprintf(stderr, "Gridpoints for insert: %u\n", save->npts);

	fprintf(stderr, "Remove unlikely: %d\n", save->remove_unlikely);
	fprintf(stderr, "Western longitudes: %d\n", save->wlon);

	for (j = 0; j < ndata; j++) {
		longitude = lon[j];
		latitude = lat[j];
		value = data[j];
		if (save->remove_unlikely == 1 && value>0 && value<10e-8) value=0; 			
		if (ctr == 98 && save->remove_unlikely == 1 && (strcmp(name,"APCP")==0 || strcmp(name,"ASNOW")==0 || strcmp(name,"ACPCP")==0 ) && value==1) value=0;
		if (longitude > 180 && (save->wlon==1)) longitude-=360;
		value = convunit(value, conv);
		if (strlen(save->rows[j]) < 2) 	{
			if (!UNDEFINED_VAL(data[j])) {
				sprintf(save->rows[j],"\"%s\",\"%s\",%g,%g,%lg",rt,vt,latitude,longitude,value);
			} else {
				sprintf(save->rows[j],"\"%s\",\"%s\",%g,%g,NULL",rt,vt,latitude,longitude);
			}
		} else {
			if (!UNDEFINED_VAL(data[j])) {
				sprintf(sql,",%lg",value);
			} else {
				sprintf(sql,",NULL");
			}
			strcat(save->rows[j],sql);
		}

	}
    return 0;
}

#else

int f_mysql_dump(ARG7) {
    if (mode == -1) { fprintf(stderr,"mysql package not installed\n"); return 1; }
    return 0;
}

#endif

