#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_3.2.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 3.2: Shape of the reference system
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_3_2_CodeTable_en.csv" -O- | sed '{
    s/695,990,000/695 990 000/g
    s/; /# /g
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
    s/\([0-9]\) \([0-9]\)/\1\2/g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      printf "case %5d: string=\"%s\"; break;\n",num,name
    }
  }' > "$outfile"

exit
