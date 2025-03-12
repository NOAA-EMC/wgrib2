#!/bin/sh
# 03/2025 Public Domain   Manfred Schwarb <schwarb@meteodat.ch>
# This script updates wgrib2 with WMO code info.

# Table 4.235 is no official WMO table, we fetch it from NCEP therefore

urlbase="http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc"

outfile="CodeTable_4.235.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.235: Wind-Generated Wave Spectral Description
wget -nv "$urlbase/grib2_table4-235.shtml"  -O- | tr -s "[:cntrl:]" "[ *]" | sed '{
        s/\(<\/t[dh]>\|<br>\)\s*<t[dh][^<]*>/\t/ig
        s/<t[dh][^<]*>/\
/ig
        s/<br>//ig
        s/&nbsp\;/ /ig
        s/<[/]*span[^<]*>//ig
        s/<\/center>/\
/ig
        s/<[^<]*>//ig
  }' | grep -v "Reserved" | grep "^[0-9]" | awk -F"\t" '
  {
    printf "case %5d: string=\"%s\"; break;\n",$1,gensub(" *$","",1,$2)
  }' > "$outfile"

exit
