#!/bin/sh

## @file
## @brief This script updates wgrib2 with WMO code info.
## @author Public Domain: Manfred Schwarb <schwarb@meteodat.ch>
## @date 10/2024

## @cond

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.4.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi
outfile2="CodeTable4_4.h"
if [ -f "$outfile2" ]; then mv "$outfile2" "$outfile2.old"; fi

#---GRIB2 Code Table 4.4: Indicator of unit of time range
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_4_CodeTable_en.csv" -O- | sed '{
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

#---Same content as CodeTable_4.4.dat provided as macro values:
awk -F"[:;]" '
  BEGIN { print "/** @file\n\
 * @brief Code Table 4.4: Indicator of unit of time range\n\
 * @author Public Domain: Wesley Ebisuzaki @date 2005\n\
 */\n\
"
  }
  {
    split($1,arr," ")
    num=arr[2]
    string=gensub(" *string=","",1,$2)
    string=gensub("\"","","g",string)
    str=toupper(string)
    if(str=="MISSING")  { next }
    if(str=="3 HOURS")  { str="HOUR3" }
    if(str=="6 HOURS")  { str="HOUR6" }
    if(str=="12 HOURS") { str="HOUR12" }
    str=gensub(" .*$","",1,str)
    printf "%s %-8s  %3s %s%s%s\n","#define",str,num,"  /**< Code Table 4.4: ",string," */"
  }' "$outfile" > "$outfile2"

exit

## @endcond
