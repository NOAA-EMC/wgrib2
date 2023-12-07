#!/bin/bash

# Script to create a "gribtable" file for wgrib2 out of
# information pulled from https://apps.ecmwf.int/codes/grib/param-db
# This gribtable information uses the ECMWF short-name nomenclature
# which differs from NCEP nomenclature.
#
# The "gribtable" file contains a colon separated list as follows:
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
#
# Besides standard unix commands, this script needs the
# program "jq" (https://stedolan.github.io/jq)
#
# (c) 2020 Manfred Schwarb <schwarb@meteodat.ch>
# Released under the General Public License Version 2 (GPLv2).


set +o posix
unset POSIXLY_CORRECT
set -o pipefail

#---functions
convert_units ()
{
  sed '{
    s|\*\*|^|g
    s|^Degree \?[EN]$|deg|i
    s|^Degree true$|deg|i
    s|^degrees C$|deg|i
    s|^Degrees\?|deg|i
    s|deg \?C|deg|
    s| per time step$|/timestep|
    s| \?per \?day$|/day|
    s|Bites per day per person|bites/day/person|
    s|Code|code|g
    s|[(]\(code table [0-9.]*\)[)]|\1|
    s|[(]\([0-9.]*\)[)]|\1|
    s| \?radian\^-1|/radian|
    s| \?sr\^-1|/sr|
    s| \?kg\^-1|/kg|
    s| \?kg\^-2|/kg^2|
    s| \?day\^-1|/day|
    s| \?K\^-1|/K|
    s| \?m\^-1|/m|
    s| \?m\^-2|/m^2|
    s| \?m-2|/m^2|
    s| \?m\^-3|/m^3|
    s| \?m\^-4|/m^4|
    s| \?m\^-5|/m^5|
    s| \?s\^-1|/s|
    s| \?s-1|/s|
    s| \?s\^-2|/s^2|
    s| \?s\^-3|/s^3|
    s| \?s\^-4|/s^4|
    s| \?W\^-2|/W^2|
    s| \?\([(].*[)]\)\^-1|/\1|
    s|Dobson|DU|
    s|Fraction|fraction|
    s|Index|index|
    s|Integer|integer|
    s|Millimetres|mm|
    s|Number|number|
    s|Numeric|numeric|
    s|Person|person|
    s|Proportion|proportion|
    s|Various|various|
    s|~|-|
    s|^/|1/|
    s|^ \+||
    s| \+$||
  }'
}
#------------

urlbase1="https://apps.ecmwf.int/codes/grib/param-db"
urlbase2="https://apps.ecmwf.int/codes/grib/json/"
url2="${urlbase2}?discipline=All&category=All&filter=grib2"

WGET="wget -e robots=off --no-check-certificate -nv"

jsonfile="param-db.json"
if [ ! -s "$jsonfile" ]; then
  $WGET "$url2" -O "$jsonfile"
fi

#---all occurring units:
##allunits=`jq -Mc '.parameters[]|{units_name:.units_name}' "$jsonfile" \
##  | tr -d '}{"' | cut -d: -f2 | sort -u`
##allunits2=`echo "$allunits" | convert_units | sort -u`
##echo "$allunits"  > allunits.txt
##echo "$allunits2" > allunits2.txt

gribtable="ECMWF_gribtable"
if [ -f "$gribtable" ]; then mv "$gribtable" "$gribtable.old"; fi
touch local_gribtable && rm local_gribtable
exec 3>"$gribtable"


while read -r jsonitem; do
  #---parse parameter info coming from the top-level json file:
  jsonparams=`echo "$jsonitem" | tr -d '}{"' | tr "," "\n" | sed 's/  \+/ /g'`
  id=`echo "$jsonparams" | grep param_id | cut -d: -f2 | tr -d " "`

  #---fetch individual html page for $id:
  url3="${urlbase1}?id=${id}"
  htmlfile="param-${id}.html"
  if [ ! -s "$htmlfile" ]; then
    $WGET "$url3" -O "$htmlfile"
  fi
  #---select only grib2 section:
  grib2section=`tr -d "\r\n" < "$htmlfile" | tr -s " " | sed 's/<[a-zA-Z]/\n&/g' \
    | sed -n '/<div id=.grib2./,/<\/div>/p' \
    | sed -e :a -e '$!N;s|<\/td> *\n *<td|<\/td><td|;ta' -e 'P;D'`

  #---there might exist several variants for different centres:
  centres=`echo "$grib2section" | grep centre | sed 's/<[^<]*>//g' | tr -d " "`

  for centre in $centres; do
    centre2=`echo "$centre" | tr "[:lower:]" "[:upper:]"`
    case $centre2 in
      CNMC)  centre_num=80 ;;
      ECMWF) centre_num=98 ;;
      KWBC)  centre_num=7  ;;
      LFPW)  centre_num=84 ;;
      WMO)   centre_num=0  ;;
      *) echo "Unknown centre! Please update code with numerical value for $centre!"
         exit 1 ;;
    esac

    #---we extract the centre section:
    centresection=`echo "$grib2section" | sed -n "/centre.*$centre/,/centre\|table/p"`

    #---there might be multiple "Key" sections which are not further identified,
    #---we simply iterate over the keys:
    lines=( `echo "$centresection" | grep -n "class=.*>Key<" | cut -d: -f1` '$' )
    cnt=${#lines[*]}
    if [ $cnt -eq 1 ]; then
      lines=( 1 '$' )
      cnt=2
    fi
    for ((no=0; no<cnt-1; ++no)); do
      keysection=`echo "$centresection" | sed -n "${lines[$no]},${lines[$((no+1))]}p"`
      params=`echo "$keysection" | grep "class=.value." | sed '{
        s/<[^<]*>/ /g
        s/^ \+//
        s/ \+$//
        s/ \+/:/g
      }'` 
      discipline=`echo "$params" | grep discipline | cut -d: -f2`
      master=0
      local=0
      parameterCategory=`echo "$params" | grep parameterCategory | cut -d: -f2 | tr -d " "`
      parameterNumber=`echo "$params" | grep parameterNumber | cut -d: -f2 | tr -d " "`
      param_shortName=`echo "$jsonparams" | grep param_shortName | cut -d: -f2 | tr -d " "`
      # WNE
      typeOfFirstFixedSurface=`echo "$params" | grep typeOfFirstFixedSurface | cut -d: -f2 | tr -d " "`
      typeOfStatisticalProcessing=`echo "$params" | grep typeOfStatisticalProcessing | cut -d: -f2 | tr -d " "`
      gridDefinitionTemplateNumber=`echo "$params" | grep gridDefinitionTemplateNumber | cut -d: -f2 | tr -d " "`
      instrumentType=`echo "$params" | grep instrumentType | cut -d: -f2 | tr -d " "`
      aerosolType=`echo "$params" | grep aerosolType | cut -d: -f2 | tr -d " "`

      if [ -z "$param_shortName" ] || [ "$param_shortName" == "~" ]; then
        param_shortName="$id"
      fi
      param_name=`echo "$jsonparams" | grep param_name | cut -d: -f2 \
        | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | sed 's/ \+$//'`
      if [ "$param_name" == "Experimental product" ]; then
        continue
      fi
      if [ -z "$param_name" ]; then
        param_name="-"
      fi
      units_name=`echo "$jsonparams" | grep units_name | cut -d: -f2 | convert_units`
      if [ -z "$units_name" ]; then
        units_name="-"
      fi
      # WNE 
      if [ "$typeOfFirstFixedSurface" == '' -a "$typeOfStatisticalProcessing" == '' -a "$gridDefinitionTemplateNumber" == '' \
	       -a "$instrumentType" == '' -a "$aerosolType" == '' ] ; then
          part1="$discipline:$master:0:255:$centre_num:$local:$parameterCategory:$parameterNumber"
          part2="$param_shortName:$param_name:$units_name"
          ##echo "X-${id}-${centre}-${no}Y:$part1:$part2:Z"
	  if [ "$centre_num" -eq 0 -o "$centre_num" -eq 98 ] ; then
            echo "$part1:$part2" >&3
          else
            echo "$part1:$part2" >>local_gribtable
	  fi
      fi
    done  # for no (key number)
  done  # for centre
done < <( jq -Mc '.parameters[]' "$jsonfile" )

exec 3>&-   # close descriptor 3

#---there are duplicate lines due to multiple keys, we sort and make them unique:
sort -u -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n -k9,11 \
  "$gribtable" > "$gribtable.$$" && mv "$gribtable.$$" "$gribtable"

exit
