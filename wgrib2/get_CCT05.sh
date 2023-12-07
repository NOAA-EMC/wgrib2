#!/bin/sh

urlbase="https://github.com/wmo-im/CCT"

outfile="BUFRTable_0_01_007.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---Common Code Table 05: Satellite identifier
wget -nv "$urlbase/raw/master/C05.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=gensub("^\\s*","",1,$4)
    if (num !="" && num !~ "-" && num !~ "CodeFigure") {
      printf "case %5d: satellite=\"%s\"; break;\n",num,name
    }
  }
  END {
    print "default:    satellite=\"Unknown\"; break;"
  }' > "$outfile"

exit
