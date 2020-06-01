#!/bin/sh

# Script to convert NCEP grib2 table information as found in
# http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml
# into usable form for wgrib2.
# As output, a file "gribtab" is produced, which contains
# a colon separated list of the following items:
#     column  1: Section 0 Discipline
#     column  2: Section 1 Master Tables Version Number
#     column  3: Section 1 Master Tables Minimum Version Number
#     column  4: Section 1 Master Tables Maximum Version Number
#     column  5: Section 1 originating centre, used for local tables
#     column  6: Section 1 Local Tables Version Number
#     column  7: Section 4 Template 4.0 Parameter category
#     column  8: Section 4 Template 4.0 Parameter number
#     column  9: Abbreviation
#     column 10: Description (parameter name)
#     column 11: Unit
# - Entries with parameter numbers smaller than 192 are printed
#   with Master Table Version equal 1.
# - Entries with parameter numbers greater than or equal 192 are declared
#   as NCEP-only with Master Table Version equal 0 and columns 5 and 6
#   set as "7" and "0".
# - Apostrophes of all sorts are removed, as they provoke problems in shells.
# - Units are converted to a more human readable format.
#
# (c) 2007 Manfred Schwarb <schwarb@meteodat.ch>
# Released under the General Public License Version 2 (GPLv2).

table_4_1=/tmp/grib2table_4_1.tmp
table_4_1_prep=/tmp/grib2table_4_1_prep.tmp
subtable=/tmp/grib2subtable.tmp
subtable_prep=/tmp/grib2subtable_prep.tmp

outfile="gribtab"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi
cat /dev/null > "$outfile"
unset POSIXLY_CORRECT

urlbase="http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc"

#---we use grib2_table4-1.shtml as starting point:
url="$urlbase/grib2_table4-1.shtml"

wget -q -O "$table_4_1" "$url"
if [ $? -ne 0 ]; then
  echo "Download of $url failed, exit."
  exit
fi

cat "$table_4_1" | tr -s "[:cntrl:]" "[ *]" | sed '{
    s/<\/t[dh]>\s*<t[dh][^<]*>/	/ig
    s/<t[dh][^<]*>/\
/ig
    s/<br>//ig
    s/&nbsp\;/ /ig
    s/<[/]*span[^<]*>//ig
    s/<\/center>/\
/ig
  }' > "$table_4_1_prep"
disc_lines=`grep -n "^ *Product Discipline " "$table_4_1_prep" | cut -d: -f1`

for line in $disc_lines; do
  echo ""
  endmatch='<\/table>'
  disc=`sed -n "$line,/$endmatch/p" "$table_4_1_prep" | grep \
      "^ *Product Discipline " | awk '{ print $3 }'`
  #---we provide character abbreviations for disciplines:
  case $disc in
     0) discstr="m";;
     1) discstr="h";;
     2) discstr="L";;
     3) discstr="s";;
    10) discstr="o";;
     *) discstr="x";;
  esac
  echo "Found discipline number: $disc (letter $discstr)"
  links=`sed -n "$line,/$endmatch/p" "$table_4_1_prep" | grep \
      -i "href=" | cut -d\" -f2`
  for link in $links; do
    pcat=`grep "$link" "$table_4_1_prep" | awk -F"\t" '{ print $1 }'`
    echo "Found product category number: $pcat"
    url="$urlbase/$link"
    wget -q -O "$subtable" "$url"
    if [ $? -ne 0 ]; then
      echo "Download of $url failed, exit."
      exit
    fi    
    cat "$subtable" | tr -s "[:cntrl:]" "[ *]" | sed '{
        s/Height of Convective Cloud Top[^<]*/&<\/td>/g 
        s/\(<\/t[dh]>\|<br>\)\s*<t[dh][^<]*>/	/ig
        s/<t[dh][^<]*>/\
/ig
        s/<br>//ig
        s/&nbsp\;/ /ig
        s/&#8217\;//g
        s/&#176\;/deg/g
        s/&#176/deg/g
        s/&#181/u/g
        s/&#956/u/g
        s/&#937/Omega/g
        s/&#952\;/theta/g
        s/&#8747\;/int/g
        s/\o042//g
        s/\o047//g
        s/\o140//g
        s/&micro\;g/10-6g/g
        s/<[/]*span[^<]*>//ig
        s/<\/center>/\
/ig
        s/<[^<]*>//ig
        s/<\/sup//g
      }' | grep "^[0-9]" | grep -vi "reserved" > "$subtable_prep"
    cat "$subtable_prep" | awk -F"\t" -v d="$disc" -v c="$pcat" \
      -v ds="$discstr" '{ \
        abbr=gensub(" ","","g",$4); \
        if (abbr=="" || abbr=="-") abbr=sprintf("var%d%s%d",c,ds,$1); \
        descr=gensub("^ *| *$","","g",$2); \
        descr=gensub(" *[*]*$","","g",descr); \
        unit=gensub(" ","","g",$3); \
        if (unit~/^[sS]ee|^[cC]ode/) unit="-"; \
        if ($1<192 && c < 192) { for (mtab=1; mtab<2; ++mtab) \
           { printf "%g:%g:%g:%g:%g:%g:%g:%g:%s:%s:%s\n", \
                    d,mtab,0,255,0,0,c,$1,abbr,descr,unit } } \
        else if ($1<255) \
           { printf "%g:%g:%g:%g:%g:%g:%g:%g:%s:%s:%s\n", \
                    d,   0,0,255,7,1,c,$1,abbr,descr,unit } \
      }' | tr -s " " | sed '{
          s/::/:-:/g
          s/:$/:-/
          s|:(kgm-3)(ms-1)$|:(kg/m^3)(m/s)|
          s|:(m2ssr)-1$|:1/(m^2s*sr)|
          s|:(m2ssreV)-1$|:1/(m^2s*sr*eV)|
          s|:(m2ssreV/nuc)-1$|:1/(m^2s*sr*eV/nuc)|
          s|:10-6gm-3$|:10^-6g/m^3|
          s|:ugm-3$|:10^-6g/m^3|
          s|:10-6g/m3$|:10^-6g/m^3|
          s|:Bqkg-1$|:Bq/kg|
          s|:Bqm-2$|:Bq/m^2|
          s|:Bqm-3$|:Bq/m^3|
          s|:Bqsm-3$|:Bqs/m^3|
          s|:DegreeTrue$|:deg|i
          s|:degreeperday$|:K/day|
          s|:degtrue$|:deg|
          s|:Jkg-1$|:J/kg|
          s|:Jm-2$|:J/m^2|
          s|:Jm-2K$|:J/m^2*K|
          s|:K[*]ms-1$|:K*m/s|
          s|:Km2kg-1s-1$|:Km^2/kg/s|
          s|:Km-1$|:K/m|
          s|:Ks-1$|:K/s|
          s|:Nm-1$|:N/m|
          s|:Nm-2$|:N/m^2|
          s|:Nm-2s$|:N/m^2*s|
          s|:Non-Dim$|:non-dim|
          s|:PPB$|:ppb|
          s|:Pas-1$|:Pa/s|
          s|:Pa2/s2$|:Pa^2/s^2|
          s|:Sm-1$|:S/m|
          s|:Vm-1$|:V/m|
          s|:Wm-1sr-1$|:W/m/sr|
          s|:Wm-2$|:W/m^2|
          s|:Wm-2Hz-1$|:W/m^2/Hz|
          s|:Wm-3sr-1$|:W/m^3/sr|
          s|:Wm-2nm-1$|:W/m^2/m^-9|
          s|:Wsr-1m-2$|:W/sr/m^2|
          s|:integer$|:Integer|
          s|:d$|:day|
          s|:h-1$|:1/h|
          s|:kg-1$|:1/kg|
          s|:kg-2s-1$|:1/kg^2/s|
          s|:kgday-1$|:kg/day|
          s|:kgkg-1$|:kg/kg|
          s|:kgkg-1s-1$|:kg/kg/s|
          s|:kgkg-1ms-1$|:kg/kg*m/s|
          s|:kgm-1$|:kg/m|
          s|:[Kk]gm-2$|:kg/m^2|
          s|:kgm-2s-1$|:kg/m^2/s|
          s|:kgm-3$|:kg/m^3|
          s|:kgm-3s-1$|:kg/m^3/s|
          s|:log10(10-6gm-3)$|:log10(10^-6g/m^3)|
          s|:log10(10-6g/m3)$|:log10(10^-6g/m^3)|
          s|:log10(kgm-3)$|:log10(kg/m^3)|
          s|:m-1$|:1/m|
          s|:m-2$|:1/m^2|
          s|:m-3$|:1/m^3|
          s|:m-1sr-1$|:1/m/sr|
          s|:m-2s-1$|:1/m^2/s|
          s|:m-2srad-1$|:1/m^2*s/rad|
          s|:m2/s2$|:m^2/s^2|
          s|:m2kg-1s-1$|:m^2/kg/s|
          s|:m2s-1$|:m^2/s|
          s|:m2s-2$|:m^2/s^2|
          s|:m3s-1$|:m^3/s|
          s|:m3m-3$|:m^3/m^3|
          s|:mm6/m3$|:mm^6/m^3|
          s|:mm6m-3$|:mm^6/m^3|
          s|:mols-1$|:mol/s|
          s|:molm-2s-1$|:mol/m^2/s|
          s|:molm-3$|:mol/m^3|
          s|:molm-3s-1$|:mol/m^3/s|
          s|:molmol-1$|:mol/mol|
          s|:ms-1$|:m/s|
          s|:ms-2$|:m/s^2|
          s|:s-1$|:1/s|
          s|:sm-1$|:s/m|
          s|:[Pp]am$|:Pa*m|
          s|see note [^:]*:|:|
          s|:categorical$|:Categorical|
          s|:psuperday$|:psu/day|
          s|:proportion$|:Proportion|
          s|:numeric$|:Numeric|
          s|:Numeric.*$|:Numeric|
          s| *(See Note [1-9])||i
          s|deg[EN]|deg|
          s|degc|degC|
          s|:Deg|:deg|
        }' | sed '{
          s|<br:|:|
          s|Pblackominant|Predominant|
          s|0:1:0:255:0:0:1:22:CLWMR:|0:1:0:255:0:0:1:22:CLMR:|
          s|2:1:0:255:0:0:0:22:SOILM:|2:1:0:255:0:0:0:22:SOIL_M:|
          s|0:1:0:255:0:0:1:87:SPRATE:|0:1:0:255:0:0:1:87:STPRATE:|
          s|0:0:0:255:7:1:1:197:MCONV:|0:0:0:255:7:1:1:197:MDIV:|
          s|0:1:0:255:0:0:6:17:TCOND:|0:1:0:255:0:0:6:17:TCONDold:|
          s|0:1:0:255:0:0:6:18:TCOLW:|0:1:0:255:0:0:6:18:TCOLWold:|
          s|0:1:0:255:0:0:6:19:TCOLI:|0:1:0:255:0:0:6:19:TCOLIold:|
          s|2:1:0:255:0:0:0:26:WILT:|2:1:0:255:0:0:0:26:WILTPT:|
          s|10:1:0:255:0:0:0:17:FRICV:|10:1:0:255:0:0:0:17:FRICVW:|
          s|10:1:0:255:0:0:191:3:DSLOBS:|10:1:0:255:0:0:191:3:DSLOBSO:|
        }' >> "$outfile"
  done
done

#---Add a special entry: 
echo "255:0:0:255:7:1:255:255:IMGD:Image data:-" >> "$outfile"

echo ""
echo "Output file: $outfile"

rm "$table_4_1" "$table_4_1_prep" "$subtable" "$subtable_prep"

exit
