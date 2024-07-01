#!/bin/sh

urlbase="https://github.com/wmo-im/CCT"

outfile="BUFRTable_0_02_019.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---Common Code Table 08: Satellite Instruments
wget -nv "$urlbase/raw/master/C08.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$1; agency=$2; instype=$3; shortname=$4; longname=$5
    if (num !~ "-" && num !~ "Code") {
      printf "case %d:\n",num
      printf "      agency=\"%s\";\n",agency
      printf "      instype=\"%s\";\n",instype
      printf "      shortname=\"%s\";\n",shortname
      printf "      longname=\"%s\";\n",longname
      printf "      break;\n"
    }
  }
  END {
      printf "default:\n"
      printf "      agency=\"%s\";\n","Unknown"
      printf "      instype=\"%s\";\n","Unknown"
      printf "      shortname=\"%s\";\n","Unknown"
      printf "      longname=\"%s\";\n","Unknown"
      printf "      break;\n"
  }' > "$outfile"

exit
