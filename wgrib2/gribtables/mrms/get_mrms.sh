#!/bin/sh
# 03/2025 Public Domain   Manfred Schwarb <schwarb@meteodat.ch>
# This script fetches MRMS table information as found in 
# https://github.com/NOAA-National-Severe-Storms-Laboratory/mrms-support
# into usable form for wgrib2.
# As output, a file "MRMS_gribtable" is produced, which contains
# a colon separated list of the following items:
#     column  1: Section 0 Discipline
#     column  2: Section 1 Master Tables Version Number
#     column  3: Section 1 Master Tables Minimum Version Number
#     column  4: Section 1 Master Tables Maximum Version Number
#     column  5: Section 1 originating centre, used for local tables
#     column  6: Section 1 Local Tables Version Number
#     column  7: Section 4 Template 4.0 Parameter category
#     column  8: Section 4 Template 4.0 Parameter number
#     column  9: Abbreviation
#     column 10: Description (parameter name)
#     column 11: Unit
# - Entries are printed with Master Table Version equal 10 and
#   column 6 set to 1 (local table used).
# - Centre is set to 161 (US NOAA Office of Oceanic and Atmospheric Research).

urlbase="https://raw.githubusercontent.com/NOAA-National-Severe-Storms-Laboratory"

outfile="MRMS_gribtable"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

wget -nv "$urlbase/mrms-support/refs/heads/main/GRIB2_TABLES/UserTable_MRMS_v12.2.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  BEGIN { OFS=":" }
  {
    disc=$1; master=10; centre=161; local=1; pcat=$2; pnum=$3
    abbr=$4; name=$10; unit=$6
    abbr=gensub("_","","g",abbr); abbr=gensub("-","M","g",abbr)
    if (disc+0>0) {
      print disc,master,0,255,centre,local,pcat,pnum,abbr,name,unit
    }
  }' > "$outfile"

exit
