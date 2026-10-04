#!/bin/sh

## @file
## @brief This script updates wgrib2 with WMO code info.
## @author Public Domain: Manfred Schwarb <schwarb@meteodat.ch>
## @date 10/2024

## @cond

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.15.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi
outfile2="CodeTable_4.15_short.dat"
if [ -f "$outfile2" ]; then mv "$outfile2" "$outfile2.old"; fi

#---GRIB2 Code Table 4.15: Type of spatial processing used to arrive at given data value from the source data
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_15_CodeTable_en.csv" -O- | sed '{
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

#---Same content as CodeTable_4.15.dat but with shorter strings:
awk -F"[:;]" '
  {
    split($1,arr," ")
    num=arr[2]
    string=gensub(" *string=","",1,$2)
    string=gensub("\"","","g",string)
    str=tolower(string)
    switch (str) {
      case /.*no interpolation.*/: str="no interpolation"              ; break
      case /.*bilinear.*/:         str="bilinear interpolation"        ; break
      case /.*bicubic.*/:          str="bicubic interpolation"         ; break
      case /.*nearest.*/:          str="nearest neighbor interpolation"; break
      case /.*neighbor-budget.*/:  str="neighbor-budget interpolation" ; break
      case /.*budget.*/:           str="budget interpolation"          ; break
      case /.*spectral.*/:         str="spectral interpolation"        ; break
      case /.*missing.*/:          str="missing interpolation"         ; break
      default:                     str=string
    }
    printf "case %5d: string=\"%s\"; break;\n",num,str
  }' "$outfile" > "$outfile2"

exit

## @endcond
