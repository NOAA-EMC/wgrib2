#!/bin/bash

# Script to create a "gribtable" file for wgrib2 out of
# information pulled from https://codes.ecmwf.int/grib/param-db
# and https://codes.ecmwf.int/parameter-database/api/v1/param/.
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
# (c) 2020-2025 Manfred Schwarb <schwarb@meteodat.ch>
# Released under the General Public License Version 3 (GPLv3).


set +o posix
unset POSIXLY_CORRECT
set -o pipefail

WGET="wget -e robots=off --no-check-certificate -nv"

tempdir="create_ecmwf_gribtable.tmp"
mkdir -p "$tempdir"

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
    s| \?k/m^2|km^2|
    s| \?m\^-3|/m^3|g
    s| \?m\^-4|/m^4|
    s| \?m\^-5|/m^5|
    s| \?s\^-1|/s|
    s| \?s-1|/s|
    s| \?s\^-2|/s^2|
    s| \?s\^-3|/s^3|
    s| \?s\^-4|/s^4|
    s| \?W\^-2|/W^2|
    s| \?\([(].*[)]\)\^-1|/\1|
    s|mol mol\^-1|mol/mol|
    s| -1000|-1000|
    s| cm|*cm|
    s| C|*C|
    s| deg|*deg|
    s| g|*g|
    s|6g|6*g|
    s| hPa|*hPa|g
    s| K|*K|g
    s| kg|*kg|g
    s| m|*m|g
    s| Pa|*Pa|g
    s| s|*s|g
    s| W|*W|g
    s|[*]stations| stations|g
    s|%[*]|%|
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

#---URL for accessing entries via web interface, not used in this script:
##weburl="https://codes.ecmwf.int/grib/param-db/?encoding=grib2&discipline=&category=&origin="

urlbase="https://codes.ecmwf.int/parameter-database/api/v1/"
urlbase1="${urlbase}param/"
url1="${urlbase1}?encoding=grib2&format=json&regex=false&all=true"
url2="${urlbase}unit/"
url3="${urlbase}origin/"
url4="${urlbase}encoding/"

#---As of February 2024, https://apps.ecmwf.int/codes/grib/param-db redirects the
#---requests to https://codes.ecmwf.int/parameter-database/api/v1/param/, which
#---has a formalized API for retrieving information.
jsonfile1="$tempdir/param.json"
if [ -s "$jsonfile1" ]; then
  echo "File $jsonfile1 exists, not downloading again."
else
  $WGET "$url1" -O "$jsonfile1"
fi
jsonfile2="$tempdir/unit.json"
if [ -s "$jsonfile2" ]; then
  echo "File $jsonfile2 exists, not downloading again."
else
  $WGET "$url2" -O "$jsonfile2"
fi
jsonfile3="$tempdir/origin.json"
if [ -s "$jsonfile3" ]; then
  echo "File $jsonfile3 exists, not downloading again."
else
  $WGET "$url3" -O "$jsonfile3"
fi
jsonfile4="$tempdir/encoding.json"
if [ -s "$jsonfile4" ]; then
  echo "File $jsonfile4 exists, not downloading again."
else
  $WGET "$url4" -O "$jsonfile4"
fi

#---all occurring units:
unitsfile="$tempdir/units.txt"
jq -Mc '.[]' "$jsonfile2" | tr -d '}{"' | awk -F"[:,]" '{ print $2 "\t" $4 }' > "$unitsfile"
{
while read -r line; do
  indx=`echo "$line" | awk -F"\t" '{ print $1 }'`
  unit=`echo "$line" | awk -F"\t" '{ print $2 }' | convert_units`
  printf "%s\t%s\n" "$indx" "$unit"
  units[indx]="$unit"
done < "$unitsfile"
} > "$tempdir/units_conv.txt"

#---all occurring origin centres:
centresfile="$tempdir/centres.txt"
jq -Mc '.[]' "$jsonfile3" | tr -d '}{"' | awk -F"[:,]" '{ print $2 "\t" $4 }' > "$centresfile"
declare -A centres  # need to use associative array due to negative indices
while read -r line; do
  indx=`echo "$line" | awk -F"\t" '{ print $1 }'`
  origin=`echo "$line" | awk -F"\t" '{ print $2 }'`
  centres[$indx]="$origin"
done < "$centresfile"

#---all occurring grib2 keys:
grib2keys=( `jq -Mc '.[]' "$jsonfile4" | grep '"id":"grib2"' | jq '.keys[]' | tr -d '}{"'` )
echo "${grib2keys[@]}" | tr -s " " "\n" > "$tempdir/grib2keys.txt"



gribtable="ECMWF_gribtable"
extragribtable="extra_gribtable"
##droppedtableitems="dropped_tableitems"
if [ -s "$gribtable" ];      then mv "$gribtable"      "$gribtable.old"; fi
if [ -s "$extragribtable" ]; then mv "$extragribtable" "$extragribtable.old"; fi
exec 3>"$gribtable"
exec 4>"$extragribtable"
##exec 5>"$droppedtableitems"

while read -r jsonitem1; do
  #---parse parameter info coming from the top-level json file:
  jsonparams1=`echo "$jsonitem1" | tr -d '}{"' | tr "," "\n" | sed 's/  \+/ /g'`
  param_name=`echo "$jsonparams1" | grep -m1 "^name:" | cut -d: -f2 \
    | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | sed 's/ \+$//'`

  #---we skip experimental entries:
  if [ "$param_name" == "Experimental product" ] || [ "${param_name:0:8}" == "Reserved" ]; then
    continue
  fi

  param_id=`echo "$jsonparams1" | grep -m1 "^id:" | cut -d: -f2`
  param_shortName=`echo "$jsonparams1" | grep -m1 "^shortname:" | cut -d: -f2 | tr -d " "`
  if [ -z "$param_shortName" ] || [ "$param_shortName" == "~" ]; then
    param_shortName="$param_id"
  fi
  if [ -z "$param_name" ]; then
    param_name="-"
  fi
  unit_id=`echo "$jsonparams1" | grep -m1 "^unit_id:" | cut -d: -f2`
  unit_name="${units[unit_id]}"
  if [ -z "$unit_name" ]; then
    unit_name="-"
  fi

  #---fetch individual details for $param_id:
  url="${urlbase1}${param_id}/grib2/"
  jsonfile="$tempdir/param-${param_id}.json"
  if [ -s "$jsonfile" ]; then
    echo "File $jsonfile exists, not downloading again."
  else
    $WGET "$url" -O "$jsonfile"
  fi

  #---loop over the different key variants (different centres and/or versions):
  while read -r jsonitem; do
    #---parse details for $param_id:
    jsonparams=`echo "$jsonitem" | sed 's/},{/£/g' | tr -d '}{"' | tr "," "\n" | sed 's/  \+/ /g'`
    ##echo "$jsonparams" | awk '{ printf "\t%s\n",$0 } END { printf "\n" }'

    centre_id=`echo "$jsonparams" | grep -m1 "^centre_id:" | cut -d: -f2`
    centre_name="${centres[$centre_id]}"
    version=`echo "$jsonparams" | grep -m1 "^version:" | cut -d: -f2`
    #---status of access (seems not to mean very much)
    access=`echo "$jsonitem" | jq -Mc '.access_ids[]' | tr -d '}{"' \
      | awk 'BEGIN { v=1 } { if ($1=="dissemination") { v=0 } } END { print v }'`
    #---whether we have the preferred variant among the given key variants:
    default=`echo "$jsonparams" | grep -m1 "^default:" \
      | awk -F: '{ v=$2 } END { if (v=="true") { print 0 } else { print 1 } }'`
    ##echo "$param_shortName / $centre_id / $centre_name / $version / $access / $default"

    #---read given grib2 keys and make variables out of it:
    keys=`echo "$jsonitem" | jq -Mc '.keys[]' | tr -d '}{"' | sed 's/  \+/ /g' \
      | awk -F"[:,]" '{ print $2 ":" $4 }'`
    unset "${grib2keys[@]}"
    cnt=0
    for keyname in "${grib2keys[@]}"; do
      keyval=`echo "$keys" | grep -m1 "^${keyname}:" | cut -d: -f2`
      if [ "$keyval" ]; then
        ((++cnt))
        declare "$keyname=$keyval"
        ##declare -p "$keyname"
      fi
    done

    #---test whether the essential grib2 keys are available:
    if [ -z "$discipline" ] || [ -z "$parameterCategory" ] || [ -z "$parameterNumber" ]; then
      echo "Not all needed grib2 keys found for $param_id/$centre_name/$version: $discipline /" \
           "$parameterCategory / $parameterNumber"
      continue
    fi

    if [ $discipline        -gt 191 ] || \
       [ $parameterCategory -gt 191 ] || \
       [ $parameterNumber   -gt 191 ]; then
      master=0  # experimental
      local=1   # some local table used
    else
      master=1  # some operational table used
      local=0   # no local table used
    fi

    item="$discipline:$master:0:255:$centre_id:$local:$parameterCategory:$parameterNumber"
    item+=":$param_shortName:$param_name:$unit_name:$cnt:$access:$default"
    ##echo "X-${param_id}-${centre_name}-${version}Y:$item:Z"
    if [ "$centre_id" -eq 0 ] || [ "$centre_id" -eq 98 ] ; then  # WMO or ECMWF
      echo "$item" >&3
    else
      echo "$item" >&4
    fi

  done < <( jq -Mc '.[]' "$jsonfile" )
done < <( jq -Mc '.results[]' "$jsonfile1" )

exec 3>&-   # close descriptor 3
exec 4>&-   # close descriptor 4
##exec 5>&-   # close descriptor 5

#---There are duplicate lines due to multiple keys, we sort and make them unique.
#---We prioritize low key count (i.e. more generic, column 12),
#---access=dissemination (column 13) and default=true (column 14). At the end, we
#---throw away the last 3 auxiliary columns:
LC_ALL=en_US     sort  -u -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n \
    -k12n,12n -k13n,13n -k14n,14n -k9,11 "$gribtable" | tee "$gribtable.debug" \
  | LC_ALL=en_US sort  -u -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n \
    -k12n,12n -k13n,13n -k14n,14n -k9,11 \
  | LC_ALL=en_US sort -su -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n \
  | cut -d: -f1-11 > "$gribtable.$$" \
  && mv "$gribtable.$$" "$gribtable"

LC_ALL=en_US     sort  -u -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n \
    -k12n,12n -k13n,13n -k14n,14n -k9,11 "$extragribtable" | tee "$extragribtable.debug" \
  | LC_ALL=en_US sort  -u -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n \
    -k12n,12n -k13n,13n -k14n,14n -k9,11 \
  | LC_ALL=en_US sort -su -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n \
  | cut -d: -f1-11 > "$extragribtable.$$" \
  && mv "$extragribtable.$$" "$extragribtable"

##LC_ALL=en_US sort -u -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n -k9,11 \
##  "$droppedtableitems" >"$droppedtableitems.$$" && mv "$droppedtableitems.$$" "$droppedtableitems"
##LC_ALL=en_US sort -su -t: -k1n,1n -k2n,2n -k3n,3n -k4n,4n -k5n,5n -k6n,6n -k7n,7n -k8n,8n -k9,11 \
##  "$droppedtableitems" >"$droppedtableitems.$$.2" && mv "$droppedtableitems.$$.2" "$droppedtableitems.2"

exit
