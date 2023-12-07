#!/bin/sh

urlbase="https://github.com/wmo-im/CCT"

outfile="code_table0.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---Common Code Table 11: Originating/generating centres
wget -nv "$urlbase/raw/master/C11.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$2+0; name=$3
    if (num>0) {
      if (name==")") {
        if (lastname!="") name="Reserved for " lastname
      } else {
        lastname=name
      }
      printf "case %5d: string=\"%s\"; break;\n",num,name
    }
  }
  END {
    print "default:    sprintf(tmp,\"%d\", ctr); string = tmp; break;"
  }' > "$outfile"

exit
