8-2021 wgrib2 v3.0.3
https://opendata.dwd.de/weather/lib/grib/
download latest version  eccodes_definitions.edzw-2.22.1-1.tar.bz2
extract shortName.def
run ./dwd2tab.pl which creates dwd_gribtable
run  ../../make_gribtable.sh dwd_gribtable which makes dwd_gribtable.dat

wgrib2 v3.0.1 : removed CDCT .. defined cloud type and cloud top
		SM (soil moisture) defined twice, set_var should default to new/good version
		TSEC defined twice
new variable:  "PR_GSP", "Large-scale precipitation rate" 

note: many DWD variables have no units.. not in the shortName.def file

-----------------------------
Source of gribtable

   reference: http://www.cosmo-model.org/content/consortium/generalMeetings/general2019/wg6/GRIB2-for-DUMMIES.pdf
   download file: https://opendata.dwd.de/weather/lib/grib/definitions.edzw-2.19.0-2

In tarball locate
   definitions.edzw-2.19.0-2/grib2/shortName.def

cd to the directory with shortName.def, and run
    dwd2tab.pl

This script will read shortName.def and make dwd_gribtable

Convert dwd_gribtable to C code by make_gribtable.sh
put results in grib2/wgrib2/gribtables/dwd/dwd_gribtable.dat

check for duplicates
   ../../check_dup_gribtable.sh dwd_gribtable
wgrib2 v3.0.1 : removed CDCT .. defined cloud type and cloud top
		SM (soil moisture) defined twice, set_var should default to new/good version
		TSEC defined twice

