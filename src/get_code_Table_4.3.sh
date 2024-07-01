#!/bin/sh

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.3.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.3: Type of generating process
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_3_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num !="" && num !~ "-" && num !~ "Code") {
      switch (num) {
        case   0: wgrib2name="anl"; break
        case   1: wgrib2name="init"; break
        case   2: wgrib2name="fcst"; break
        case   3: wgrib2name="bias-corr fcst"; break
        case   4: wgrib2name="ens fcst"; break
        case   5: wgrib2name="prob fcst"; break
        case   6: wgrib2name="fcst err"; break
        case   7: wgrib2name="anl err"; break
        case   8: wgrib2name="obs"; break
        case   9: wgrib2name="clim"; break
        case  10: wgrib2name="prob wt fcst"; break
        case  11: wgrib2name="bias-corr ens fcst"; break
        case  12: wgrib2name="post-proc anl"; break
        case  13: wgrib2name="post-proc fcst"; break
        case  14: wgrib2name="nowcast"; break
        case  15: wgrib2name="hindcast"; break
        case  16: wgrib2name="physical retrieval"; break
        case  17: wgrib2name="regression analysis"; break
        case  18: wgrib2name="difference between two forecasts"; break
        case  19: wgrib2name="first guess"; break
        case  20: wgrib2name="analysis increment"; break  # analysis minus first guess
        case  21: wgrib2name="initialization increment for analysis"; break  # initialized analysis minus analysis
        case 255: wgrib2name="missing"; break
        default: { print "ERROR: missing switch statement for",num > "/dev/stderr"; exit 1 }
      }
      printf "case %5d: string=\"%s\"; break;  // %s\n",num,wgrib2name,name
      if (num==21) {  # append custom entries after case 21
        print "case   192: if (center == NCEP) string=\"fcst confidence\"; else string=\"undefined\"; break;"
        print "case   193: if (center == NCEP) string=\"probability-matched mean\"; else string=\"undefined\"; break;"
        print "case   194: if (center == NCEP) string=\"neighborhood probability\"; else string=\"undefined\"; break;"
        print "case   195: if (center == NCEP) string=\"bias-corrected and downscaled ensemble forecast\"; else string=\"undefined\"; break;"
        print "case   196: if (center == NCEP) string=\"perturbed analysis for ensemble initialization\"; else string=\"undefined\"; break;"
        print "case   197: if (center == NCEP) string=\"ensemble agreement scale probability\"; else string=\"undefined\"; break;"
        print "case   198: if (center == NCEP) string=\"post-processed deterministic-expert-weighted forecast\"; else string=\"undefined\"; break;"
        print "case   199: if (center == NCEP) string=\"ens fcst based on counting\"; else string=\"undefined\"; break;"
      }
    }
  }' > "$outfile"

exit
