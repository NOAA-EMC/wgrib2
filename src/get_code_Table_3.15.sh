#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_3.15.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 3.15: Physical meaning of vertical coordinate
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_3_15_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5; unit=$8
    if (num !="" && num !~ "-" && num !~ "Code") {
      if (unit=="K m-2 kg-1 s-1" || unit=="K m2 kg-1 s-1") { unit="K*m^2/kg/s" }
      if (unit!="") { name=name " [" unit "]" }
      printf "case %5d: string=\"%s\"; break;\n",num,name
    }
  }' > "$outfile"

exit
