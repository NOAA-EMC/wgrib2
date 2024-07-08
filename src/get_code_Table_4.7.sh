#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.7.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.7: Derived forecast
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_7_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      printf "case %5d: string=\"%s\"; break;\n",num,name
      if (num==9) {  # append custom NCEP entries after case 9
        print "case   192: if (center == NCEP) string=\"Unweighted Mode of All Members\"; break;"
        print "case   193: if (center == NCEP) string=\"Percentile value (10%) of All Members\"; break;"
        print "case   194: if (center == NCEP) string=\"Percentile value (50%) of All Members\"; break;"
        print "case   195: if (center == NCEP) string=\"Percentile value (90%) of All Members\"; break;"
        print "case   196: if (center == NCEP) string=\"Statistically decided weights for each ensemble member\"; break;"
        print "case   197: if (center == NCEP) string=\"Climate Percentile (percentile values from climate distribution)\"; break;"
      }
    }
  }' > "$outfile"

exit
