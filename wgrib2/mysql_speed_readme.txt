/******************************************************************************************
 Copyright (C) 2008 Niklas Sondell, Storm Weather Center
 MYSQL_speed is part of wgrib2 and could be distributed under terms of the GNU General Public License
/******************************************************************************************

MYSQL SPEED for wgrib2

This is a very fast version of the mysql insertion with wgrib2. It also results in a much 
better structure of the database table which also makes the usage afterwards much faster. 
It can be used only when the gribfile contains data with:

1. All fields for insertion with the same runtime (anatime).
2. All fields for insertion with the same validtime (verf time).
3. All fields with the same GDS, grid definition. All fields for insertion needs to have the same lat/lon grid definition. 

If these criterias are not fulfilled it will result in an error. 
Number of points in the grid are set to 2 000 000 as a maximum but can easily be changed - look 
for the number in the beginnig og the code!! You will get an error an a notice about this if the number is exceeded. 

NOTE! For UNDEFINED values!!
Are by wgrib2 default set to 9.999e+20, this is set to NULL in the database insert

/******************************************************************************************

USAGE Example:
wgrib2 [gribfile] -mysql_speed H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0|1] PV=[remove unlikely:0|1] LATLON=[LonW:LonE:LatS:LatN]
if remove_unlikely == 1 , values above zero that are smaller than 10e-8 are set to zero			
if western_lons == 1, longitudes above 180 are subtracted with -360
Use the undefine function to choose area and the mathc function to choose fields. 

Parameter name mapping is in database table 'wgrib2_parameter_mapping'. Paramaters not in this table are getting standard wgrib2 names for parameter names and level names, except for surface parameters that default are set to level '0' and pressure parameters that default are set to the pressure level in figures, like '925'. NB!! There are no unit conversion of parameters that are not mapped in this table. 

Matches are made like this for desired parameters and levels:
"(param_name1:(level1|level2|level3))|(param_name2:(level4|level5))....", where level notation is the first 1-3 words in the wgrib2 levelname description. Levelnames "mean sea level" and "2 m above..." can be shortened to "mean " and "2 " respectively (observe the blanks!). Of course this depends on the name convention in wgrib2. 

Parameter names in database is named 'our_paramname'_'our_levelname', for example TMP_0 or RH_925. 

/******************************************************************************************

CREATE WGRIB2_TO_MYSQL MAPPING TABLE

create table wgrib2_parameter_mapping (center_id int(5), wgrib_param_name varchar(30), wgrib_param_levelname varchar(100), our_name varchar(30), our_levelname varchar(100), conversion varchar(30), val_precision int(3));
alter table wgrib2_parameter_mapping add primary key (center_id, wgrib_param_name, wgrib_param_level);

Example of insert:
insert into wgrib2_parameter_mapping (center_id, wgrib_param_name,wgrib_param_levelname,our_name,our_levelname,conversion,val_precision) 
values (7,'TCDC', 'entire atmosphere (considered as a single layer)', 'TCDC', '0', 'PERC_PART', 2); 

Example of table with inserted values: 

+-----------+------------------+--------------------------------------------------+----------+---------------+------------+---------------+
| center_id | wgrib_param_name | wgrib_param_levelname                            | our_name | our_levelname | conversion | val_precision |
+-----------+------------------+--------------------------------------------------+----------+---------------+------------+---------------+
|         7 | TCDC             | entire atmosphere (considered as a single layer) | TCDC     | 0             | PERC_PART  |             2 |
|         7 | TCDC             | high cloud layer                                 | TCDC     | 500           | PERC_PART  |             2 |
|         7 | TCDC             | middle cloud layer                               | TCDC     | 700           | PERC_PART  |             2 |
|         7 | TCDC             | low cloud layer                                  | TCDC     | 925           | PERC_PART  |             2 |
|         7 | PRMSL            | mean sea level                                   | PRMSL    | 0             | PA_HPA     |             1 |
|         7 | TMP              | 850 mb                                           | TMP      | 850           | K_C        |             2 |
|         7 | TMP              | 925 mb                                           | TMP      | 925           | K_C        |             2 |
|         7 | TMP              | 700 mb                                           | TMP      | 700           | K_C        |             2 |
|         7 | TMP              | 500 mb                                           | TMP      | 500           | K_C        |             2 |
|         7 | TMP              | 1000 mb                                          | TMP      | 1000          | K_C        |             2 |
|         7 | TMP              | 300 mb                                           | TMP      | 300           | K_C        |             2 |
|         7 | TMP              | surface                                          | TMP      | 0             | K_C        |             2 |
|         7 | TMP              | 2 m above ground                                 | TMP      | 2             | K_C        |             2 |
|         7 | TMAX             | 2 m above ground                                 | TMAX     | 2             | K_C        |             2 |
|         7 | TMIN             | 2 m above ground                                 | TMIN     | 2             | K_C        |             2 |
|         7 | DPT              | 2 m above ground                                 | DPT      | 2             | K_C        |             2 |
|         7 | RH               | 850 mb                                           | RH       | 850           | None       |             1 |
|         7 | RH               | 925 mb                                           | RH       | 925           | None       |             1 |
|         7 | RH               | 700 mb                                           | RH       | 700           | None       |             1 |
|         7 | RH               | 500 mb                                           | RH       | 500           | None       |             1 |
|         7 | RH               | 1000 mb                                          | RH       | 1000          | None       |             1 |
|         7 | RH               | 300 mb                                           | RH       | 300           | None       |             1 |
|         7 | RH               | surface                                          | RH       | 0             | None       |             1 |
|         7 | RH               | 2 m above ground                                 | RH       | 2             | None       |             1 |
|         7 | APCP             | surface                                          | APCP     | 0             | None       |             2 |
|         7 | ACPCP            | surface                                          | ACPCP    | 0             | None       |             2 |
|         7 | UGRD             | 10 m above ground                                | UGRD     | 0             | None       |             2 |
|         7 | VGRD             | 10 m above ground                                | VGRD     | 0             | None       |             2 |
|         7 | HGT              | surface                                          | HGT      | 0             | None       |             1 |
|         7 | CSNOW            | surface                                          | CSNOW    | 0             | None       |             2 |
|         7 | DSWRF            | surface                                          | DSWRF    | 0             | None       |             1 |
|         7 | USWRF            | surface                                          | USWRF    | 0             | None       |             1 |
|         7 | LFTX             | surface                                          | LFTX     | 0             | None       |             1 |
|         7 | CAPE             | surface                                          | CAPE     | 0             | None       |             1 |
+-----------+------------------+--------------------------------------------------+----------+---------------+------------+---------------+

COLUMNS:
	center_id - center ID, for example 7 for NCEP, 98 for ECMWF
	wgrib_param_name - Parameter name in wgrib2
	wgrib_param_levelname - Levelname in wgrib2 
	our_name - The name we want in the column name
	our_levelname - The level prefix we want for the parameter column name
	conversion - type of conversion ("None" for no converisons)
	val_precision - decimal precision of output, currently not in use 



CONVERSION TYPES:
	GPH_M - Geopotential to meters, division by 9.82
	M_MM - Meter to millimeter, miltiplication by 1000
	PERC_PART - From percent to values between 0 and 1
	PA_HPA - pascal to hPa, division by 100
	PART_PERC Values between 0 and 1 multiplied by 100 
	K_C - kelvin to Celsius, subtraction by 273.16

/******************************************************************************************

CREATE DATA TABLE
Example for the GFS model based on the mapping table above (has to be changed according to chosen level and parameter names):

create table gfs (rt DATETIME NOT NULL,vt DATETIME NOT NULL,lat double NOT NULL,lon double NOT NULL,
HGT_850 double(7,1),TMP_850 double(6,2),UGRD_850 double(6,2),VGRD_850 double(6,2),
HGT_925 double(7,1),TMP_925 double(6,2),UGRD_925 double(6,2),VGRD_925 double(6,2),
HGT_0 double(7,1),TMP_2 double(6,2),RH_2 double(6,2), RH_925 double(6,2), 
RH_850 double(6,2),TMAX_2 double(6,2),TMIN_2 double(6,2),
UGRD_0 double(6,2),VGRD_0 double(6,2),APCP_0 double(6,2),ACPCP_0 double(6,2),
CSNOW_0 double(5,2),LFTX_0 double(4,1),CAPE_0 double(6,1),
TCDC_925 double(5,2),TCDC_700 double(5,2),TCDC_500 double(5,2),
TCDC_0 double(5,2),DSWRF_0 double(8,1),USWRF_0 double(11,1),PRMSL_0 double(6,1));
alter table gfs add primary key (rt,vt,lat,lon);

EXAMPLE of bulk insert from grib2 file

wgrib2 gfs.t00z.pgrb2f162 -undefine out-box 0:30 40:70 -mysql localhost user passwd database table 1 1 -match "(APCP:surface)|(TCDC:(middle|high|low|entire))|(RH:(2 |850|925))|(UGRD:(10 m above|925|850))|(VGRD:(10 m above|925|850))|(PRMSL:mean )|(HGT:(925|850|surface))|(CSNOW:surface)|(TMP:(2 |925|850))|(TMAX:2 )|(TMIN:2 )|((DSWRF|USWRF|:LFTX|CAPE|ACPCP):surface)"
