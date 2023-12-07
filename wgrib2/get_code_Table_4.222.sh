#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.222.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.222: Categorical result
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_222_CodeTable_en.csv"  -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      printf "case %5d: string=\"%s\"; break;\n",num,name
      if (num==1) {  # append custom NCEP entries after case 1
        print "case     4: string=\"Low\"; break;"
        print "case     5: string=\"Reserved\"; break;"
        print "case     6: string=\"Medium\"; break;"
        print "case     7: string=\"Reserved\"; break;"
        print "case     8: string=\"High\"; break;"
      }

    }
  }' > "$outfile"

exit
