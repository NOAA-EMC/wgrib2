#!/bin/sh

urlbase="https://github.com/wmo-im/CCT"

outfile="codetable_4_230.c"  # resp. codetable_4_233.c
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---Common Code Table 14: Atmospheric chemical or physical constituent type
#---This corresponds to GRIB2 Table 4.230 (Atmospheric chemical constituent type)
#---and GRIB2 Table 4.233 (Aerosol type)
wget -nv "$urlbase/raw/master/C14.csv" -O- | sed '{
    /"/ s/, /# /g
    /"/ s/\([1-9]\),\([1-9]\)-di/\1#\2-di/g
    /"/ s/\([1-9]\),\([1-9]\),\([1-9]\)-tri/\1#\2#\3-tri/g
    /"/ s/\([1-9]\),\([1-9]\),\([1-9]\),\([1-9]\)-tetra/\1#\2#\3#\4-tetra/g
    /"/ s/\([1-9]\),\([1-9]\),\([1-9]\),\([1-9]\),\([1-9]\)-penta/\1#\2#\3#\4#\5-penta/g
    /"/ s/\([1-9]\),\([1-9]\),\([1-9]\),\([1-9]\),\([1-9]\),\([1-9]\)-hexa/\1#\2#\3#\4#\5#\6-hexa/g
        s/,/;/g
    /"/ s/#/,/g
        s/"//g
        s/\o316\o261/alpha/g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  BEGIN {
    print "#include \"wgrib2.h\""
    print "struct codetable_4_230 codetable_4_230_table[] = {"
  }
  {
    num=$1; name=$2
    if (num !~ "-" && num != "CodeFigure") {
      printf "{%s, \"%s\"},\n",num,name
    }
  }
  END {
    print "};"
  }' > "$outfile"

exit
