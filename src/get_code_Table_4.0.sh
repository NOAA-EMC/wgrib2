#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.0.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.0: Product definition template number
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_0_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | sed 's/?/o/g' | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      printf "case %5d: string=\"%s\"; break;\n",num,name
    }
  }' > "$outfile"

exit
