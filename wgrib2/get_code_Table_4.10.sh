#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.10.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.10: Type of statistical processing
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_10_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      switch (num) {
        case   0: wgrib2name="ave"; break
        case   1: wgrib2name="acc"; break
        case   2: wgrib2name="max"; break
        case   3: wgrib2name="min"; break
        case   4: wgrib2name="last-first"; break
        case   5: wgrib2name="RMS"; break
        case   6: wgrib2name="StdDev"; break
        case   7: wgrib2name="covar"; break
        case   8: wgrib2name="first-last"; break
        case   9: wgrib2name="ratio"; break
        case  10: wgrib2name="standardized anomaly"; break
        case  11: wgrib2name="summation"; break
        case  12: wgrib2name="return period"; break
        case 100: wgrib2name="severity"; break
        case 101: wgrib2name="mode"; break
        case 255: wgrib2name="missing"; break
        default: { print "ERROR: missing switch statement for",num > "/dev/stderr"; exit 1 }
      }
      printf "case %5d: string=\"%s\"; break;  // %s\n",num,wgrib2name,name
    }
  }' > "$outfile"

exit
