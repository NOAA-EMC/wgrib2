#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_3.1.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 3.1: Grid definition template number
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_3_1_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      printf "case %5d: string=\"%s\"; break;\n",num,name
      if (num==1200) {  # append custom entries after case 1200
        printf "case %5d: string=\"%s\"; break;\n",\
          32768,"Rotated Latitude/Longitude (Arakawa Staggered E-Grid)"
      }
    }
  }' > "$outfile"

exit
