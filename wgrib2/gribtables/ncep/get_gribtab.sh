#!/bin/sh

# Script to convert NCEP grib2 table information as found in
# http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/
# into usable form for wgrib2.
# As output, a file "gribtable" is produced, which contains
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
# - Entries with parameter categories smaller than 192 are printed
#   with Master Table Version equal 1 (operational table)
#   and columns 5 and 6 set as "0" (WMO) and "0" (no local table used).
# - Entries with parameter categories greater than or equal 192 are declared
#   as NCEP-only with Master Table Version equal 0 (experimental)
#   and columns 5 and 6 set as "7" (NCEP) and "1" (local table).
# - Apostrophes of all sorts are removed, as they provoke problems in shells.
# - Units are converted to a more human readable format.
#
# (c) 2007 Manfred Schwarb <schwarb@meteodat.ch>
# Released under the General Public License Version 2 (GPLv2).

table_4_1=/tmp/grib2table_4_1.tmp
table_4_1_prep=/tmp/grib2table_4_1_prep.tmp
subtable=/tmp/grib2subtable.tmp
subtable_prep=/tmp/grib2subtable_prep.tmp

outfile="gribtable"
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
      s/<\/tr>/\n/ig
      s/<tr>/\n/ig
      s/<td[^<]*>/\t/ig
      s/<\/sup>\?//g
      s/&nbsp;\?/ /ig
      s/&#176;\?/deg/g    # Degree Sign
      s/&#181;\?/u/g      # Micro Sign
      s/&#928;\?/Pi/g     # Greek Capital Letter Pi
      s/&#937;\?/Omega/g  # Greek Capital Letter Omega
      s/&#952;\?/theta/g  # Greek Small Letter Theta
      s/&#956;\?/u/g      # Greek Small Letter Mu
      s/&#8217;\?//g      # Right Single Quotation Mark
      s/&#8747;\?/int/g   # Integral
      s/\o042//g
      s/\o047//g
      s/\o140//g
      s/&deg;\?/deg/g
      s/&micro;g/10-6g/ig
    }' | sed '{
      s/<[^<]*>//g
      s/^ *\t//
      s/ *$//
      s/ *\t/\t/g
    }' | grep "^[0-9]" | grep -vi "reserved" > "$subtable_prep"

    if { grep -qi "use another" "$subtable_prep" ;}; then
      echo "ATTENTION: Deprecated parameters without abbrev in $url." \
           "Maybe need of manual inspection."
    fi
    ##if { grep -q "Eastward turbulent surface stress" "$subtable" ;}; then echo $url; fi
    cat "$subtable_prep" | awk -F"\t" -v d="$disc" -v c="$pcat" \
      -v ds="$discstr" '{ \
        abbr=gensub(" ","","g",$4); \
        if (abbr=="" || abbr=="-") abbr=sprintf("var%d%s%d",c,ds,$1); \
        descr=gensub("^ *| *$","","g",$2); \
        descr=gensub(" *[*]*$","","g",descr); \
        unit=gensub(" ","","g",$3); \
        if (unit~/^[sS]ee|^[cC]ode/) unit="-"; \
        if (d<192 && c<192 && $1<192) { for (mtab=1; mtab<2; ++mtab) \
           { printf "%g:%g:%g:%g:%g:%g:%g:%g:%s:%s:%s\n", \
                    d,mtab,0,255,0,0,c,$1,abbr,descr,unit } } \
        else if ($1<255) \
           { printf "%g:%g:%g:%g:%g:%g:%g:%g:%s:%s:%s\n", \
                    d,   0,0,255,7,1,c,$1,abbr,descr,unit } \
      }' | tr -s " " | sed '{
          s/::/:-:/g
          s/:$/:-/
          s|:(kgm-3)(ms-1)$|:kg/m^2/s|
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
          s|:Bqs-1$|:Bq/s|
          s|:day-1$|:1/day|
          s|:DegreeTrue$|:deg|i
          s|:degreeperday$|:K/day|
          s|:degtrue$|:deg|
          s|deg[EN]|deg|
          s|degc|degC|
          s|:Deg|:deg|
          s|:&deg|:deg|
          s|:Jkg-1$|:J/kg|
          s|:Jm-2$|:J/m^2|
          s|:Jm-2K$|:J/m^2*K|
          s|:Jm-2s-1$|:J/m^2/s|
          s|:K[*]ms-1$|:K*m/s|
          s|:Km2kg-1s-1$|:K*m^2/kg/s|
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
          s|:Wm-1$|:W/m|
          s|:Wm-1sr-1$|:W/m/sr|
          s|:Wm-2$|:W/m^2|
          s|:Wm-2Hz-1$|:W/m^2/Hz|
          s|:Wm-3sr-1$|:W/m^3/sr|
          s|:Wm-2nm-1$|:W/m^2/m^-9|
          s|:Wsr-1m-2$|:W/sr/m^2|
          s|:integer$|:Integer|
          s|:d$|:day|
          s|:h-1$|:1/h|
          s|:gkgm-2s-1$|:g*kg/m^2/s|
          s|:gkg-1m-2s-1$|:g/kg/m^2/s|
          s|:gkg-1m$|:g/kg*m|
          s|:gkg-1s-1$|:g/kg/s|
          s|:kg-1$|:1/kg|
          s|:kg-2s-1$|:1/kg^2/s|
          s|:kgday-1$|:kg/day|
          s|:kgkg-1$|:kg/kg|
          s|:kgkg-1s-1$|:kg/kg/s|
          s|:kgkg-1ms-1$|:kg/kg*m/s|
          s|:kgkg-1m$|:kg/kg*m|
          s|:kgm-1$|:kg/m|
          s|:kgm-1s-1$|:kg/m/s|
          s|:[Kk]gm-2$|:kg/m^2|
          s|:kgm-2s-1$|:kg/m^2/s|
          s|:kgm-3$|:kg/m^3|
          s|:kgm-3s-1$|:kg/m^3/s|
          s|:km-2day-1$|:1/km^2/day|
          s|:log10(10-6gm-3)$|:log10(10^-6g/m^3)|
          s|:log10(10-6g/m3)$|:log10(10^-6g/m^3)|
          s|:log10(kgm-3)$|:log10(kg/m^3)|
          s|:m-1$|:1/m|
          s|:m-2$|:1/m^2|
          s|:m-3$|:1/m^3|
          s|:m-1sr-1$|:1/m/sr|
          s|:m-2s-1$|:1/m^2/s|
          s|:m-2srad-1$|:1/m^2*s/rad|
          s|:m2$|:m^2|
          s|:m2/3 s-1|:m^(2/3)/s|
          s|:m2srad-1|:m^2*s/rad|
          s|:m2m-2$|:m^2/m^2|
          s|:m2/s2$|:m^2/s^2|
          s|:m2kg-1s-1$|:m^2/kg/s|
          s|:m2s-1$|:m^2/s|
          s|:m2s-2$|:m^2/s^2|
          s|:m3$|:m^3|
          s|:m3m-2$|:m^3/m^2|
          s|:m3m-3$|:m^3/m^3|
          s|:m3m-2s-1$|:m^3/m^2/s|
          s|:m2/3s-1$|:m^(2/3)/s|
          s|:m3s-1$|:m^3/s|
          s|:m3s-1m-1$|:m^3/s/m|
          s|:mm6/m3$|:mm^6/m^3|
          s|:mm6m-3$|:mm^6/m^3|
          s|:mols-1$|:mol/s|
          s|:molm-2s-1$|:mol/m^2/s|
          s|:molm-3$|:mol/m^3|
          s|:molm-3s-1$|:mol/m^3/s|
          s|:molmol-1$|:mol/mol|
          s|:ms-1$|:m/s|
          s|:ms-2$|:m/s^2|
          s|:nSvh-1|:nSv/h|
          s|:s-1$|:1/s|
          s|:s-2$|:1/s^2|
          s|:sm-1$|:s/m|
          s|:[Pp]am$|:Pa*m|
          s|Moisture(non-frozen)|Moisture (non-frozen)|
          s|Cease(Soil Moisture)|Cease (Soil Moisture)|
          s|Onset(Soil Moisture)|Onset (Soil Moisture)|
          s|see note [^:]*:|:|
          s|:categorical$|:Categorical|
          s|:psuperday$|:psu/day|
          s|:proportion$|:Proportion|
          s|:numeric$|:Numeric|
          s|:Numeric.*$|:Numeric|
          s|:Numberm-2|:Number/m^2|
          s|:Personm-2|:Number/m^2|
          s|:Bitesperdayperperson|:bites/day/person|
          s| \* - Parameter deprecated:|:|
          s| *(Parameter Deprecated, see Note [1-9])||i
          s|[ ,]*(See Note [1-9])||i
          s| *See Note [1-9]||i
          s| *(see Note)||i
          s| See Note||i
          s| *(see Local Use Note A)||i
          s| (Defined In Section 1)||i
        }' | sed '{
          s|[.,]:|:|g
          s|<br:|:|
          s|Pblackominant|Predominant|
          s|Coveblack|Covered|
          s|0:0:0:255:7:1:1:197:MCONV:|0:0:0:255:7:1:1:197:MDIV:|
          s|0:1:0:255:0:0:1:22:CLWMR:|0:1:0:255:0:0:1:22:CLMR:|
          s|0:1:0:255:0:0:1:87:SPRATE:|0:1:0:255:0:0:1:87:STPRATE:|
          s|0:1:0:255:0:0:6:17:TCONDO:|0:1:0:255:0:0:6:17:TCONDold:|
          s|0:1:0:255:0:0:6:18:TCOLWO:|0:1:0:255:0:0:6:18:TCOLWold:|
          s|0:1:0:255:0:0:6:19:TCOLIO:|0:1:0:255:0:0:6:19:TCOLIold:|
          s|2:1:0:255:0:0:0:3:.*|2:1:0:255:0:0:0:3:SOILM:Soil Moisture Content:kg/m^2|
          s|2:1:0:255:0:0:0:22:.*|2:1:0:255:0:0:0:22:SOIL_M:Soil Moisture:kg/m^3|
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


duplicates1=`awk -F: '{ if ($8 < 192) print $9 }' "$outfile" | LC_ALL=C sort | uniq -d`
if [ "$duplicates1" ]; then
  echo ""
  echo "ATTENTION: Duplication of \"official\" abbreviations:"
  echo "$duplicates1"
fi

tmpstr=`cut -d: -f9,11 "$outfile"`
grepstr=`echo "$tmpstr" | LC_ALL=C sort -u | cut -d: -f1 | uniq -d | tr -s "\n" " " \
  | sed 's/ $//' | sed 's/ /\\\|/'g`
duplicates2=`echo "$tmpstr" | grep "^\($grepstr\):" | LC_ALL=C sort`
if [ "$duplicates2" ]; then
  echo ""
  echo "ATTENTION: Duplicate abbreviations with diverging units:"
  echo "$duplicates2"
fi

rm "$table_4_1" "$table_4_1_prep" "$subtable" "$subtable_prep"

exit
