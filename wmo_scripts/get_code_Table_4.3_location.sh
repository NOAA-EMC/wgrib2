#!/bin/sh

## @file
## @brief Script to extract section 4 index offsets of the parameter
## "Type of generating process" out of the Product Definition Templates (PDT).
## @note The original index formula is noted as comment ("// OctetNo:")
## in the generated code (note the shift by +1).
## Manual inspection of the conversion of the formulas to the
## resulting code is needed.
## @author Public Domain: Manfred Schwarb <schwarb@meteodat.ch>

homedir=`dirname -- $0`
homedir=`realpath "$homedir"`

name="CodeTable_4.3_location"
grepstr="Type of generating process"  # Code Table 4.3

if [ ! -d GRIB2-master ]; then
  urlbase="https://github.com/wmo-im/GRIB2"
  wget -nv "$urlbase/archive/refs/heads/master.tar.gz"
  tar xf GRIB2-master.tar.gz
fi
cd GRIB2-master || exit 1

outfile="$homedir/$name.dat"
if [ -f "$outfile" ]; then cp -a "$outfile" "$outfile.old"; fi

files=`grep -ia "$grepstr" GRIB2_Template_4_* | cut -d: -f1`

{
for fil in $files; do
  pdt=`echo "$fil" | cut -d_ -f4`
  env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT "$fil" \
    | sed '{
      s/, /# /g
      s/,/;/g
      s/# /, /g
      s/"//g
    }' | grep -ia "$grepstr" | awk -F";" -v f="$fil" -v pdt="$pdt" '
  {
    octet=tolower(gensub("[)]$","","g",gensub("^[(]","","g",$2)))
    octet1=substr(octet,1,2)-1
    octet2=substr(octet,3)
    switch (pdt) {
      case /^5[34]$/:    prefix="np=sec[4][12]; if (np==255) np=0; "
                         octet2=gensub("np","*np",1,octet2)
                         break
      case /^[56][78]$/: prefix="np=sec[4][19]; if (np==255) np=0; "
                         octet2=gensub("np","*np",1,octet2)
                         break
      case /^11[3-6]$/:  prefix="nutaftac=sec[4][16]; if (nutaftac==255) nutaftac=0; "
                         octet1="36+nutaftac-1"
                         octet2=""
                         break
      default:           prefix=""
    }
    octet=prefix "#sec[4]+" octet1 octet2
    printf "%s %4g: %s\n","        case",pdt,octet ";  // OctetNo: " $2
  }'
done
} | LC_ALL=en_US sort -t"#" -r -k2,2 -k1.13nr,1.16nr | awk -F":" '
  {
    str=$2 ":" $3
    if (str!=laststr) { print } else { print $1 ":" }
    laststr=str
  }' | sed 's/#/return /' | tac > "$outfile"

dup=`awk '{ print $2 }' "$outfile" | tr -d ":" | sort -n | uniq -D`
if [ "$dup" ]; then
  echo "ERROR: duplicate entries in table: "$dup
fi


#---Extract offset indications of the variables np and nutaftac
#---to manually cross-check above switch entries (note offset by one):
if true; then
for fil in $files; do
  tmp=`env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT "$fil" \
    | sed '{
      s/, /# /g
      s/,/;/g
      s/# /, /g
      s/"//g
    }'`
  echo "$tmp" | grep -m1 -Fi "(np)"       | awk -v f="$fil" '{ print f ":" $0 }' | sed 's/:"\?[^";]*"\?;/:/'
  echo "$tmp" | grep -m1 -Fi "(nutaftac)" | awk -v f="$fil" '{ print f ":" $0 }' | sed 's/:"\?[^";]*"\?;/:/'
done
fi

exit
