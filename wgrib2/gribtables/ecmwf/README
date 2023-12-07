v3.0.3 7/29/2021

./create_ecmwf_gribtable.sh   need to run at home because jq is not found
  makes ECMWF_gribtable and local_gribtable
  note: local_gribtable includes definitions for other centers, filter them out

./filter_local_gribtable.sh
   takes local_gribtable and makes local_gribtable_ecmwf
   as of current,  no local ECMWF definitions end up in this file



Note:  ../../check_dup_gribtable.sh ECMWF_gribtable
  shows that there are duplicate gribnames, nothing done

../../make_gribtable.sh ECMWF_gribtable ECMWF_gribtable
   makes ECMWF_gribtable.dat

../../make_gribtable.sh local_gribtable_ecmwf
   makes local_gribtable_ecmwf.dat

v3.0.1 : duplicate check 1/29/2021

../../check_dup_gribtable.sh ECMWF_gribtable
  many variables are doublely defined like

192:0:0:255:98:0:150:139:uv:U*V product:m/s^2
192:0:0:255:98:0:151:140:uv:UV product:m^2/s^2

The units don't make sense for the first definition.  To make sure
this is nota problem with the download code, I found the web page

https://apps.ecmwf.int/codes/grib/param-db?id=150139

One typo is understandable but this is more wide spread. So
until the situation is resolved, uv will be consider to
be the same name used on different variables.

Names which have two different meanings.  There was a case
of a flux which had a missing units which I considered them
to be the same meaning.
al
apt
asn
cdct
hcc
lcc
lspa
mcc
par
ra
sr
sro
ssro
uu
uv
uvi
vv

../../check_gribtab.sh
  rm "dist"  leave "h"

Once ECMWF_gribtable is finalized, make ECMWF_gribtable.dat

../../make_gribtable.sh ECMWF_gribtable

