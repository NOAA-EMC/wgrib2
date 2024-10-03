#!/bin/sh
# 10/2024 Public Domain   Manfred Schwarb <schwarb@meteodat.ch>
# This script updates wgrib2 with WMO code info.

urlbase="https://github.com/wmo-im/GRIB2"

outfile="../wgrib2/CodeTable_4.16.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.16: Quality value associated with parameter
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_16_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      printf "case %5d: string=\"%s\"; break;\n",num,name
    }
  }' > "$outfile"

exit
