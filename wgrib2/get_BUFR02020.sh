#!/bin/sh

urlbase="https://github.com/wmo-im/BUFR4"

outfile="BUFRTable_0_02_020.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---BUFR Code Table 02 020: Satellite classification
wget -nv "$urlbase/raw/master/BUFRCREX_CodeFlag_en_02.csv" -O- | grep "^002020" | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
    s|\o342\o200\o260|o/oo|g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$4
    if (num !="" && num !~ "-") {
      printf "case %5d: classification=\"%s\"; break;\n",num,name
    }
  }
  END {
    print "default:    classification=\"Unknown\"; break;"
  }' > "$outfile"

exit
