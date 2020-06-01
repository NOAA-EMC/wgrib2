#!/bin/sh

tab=$1
tab="4.5"
no=`echo $tab | sed 's/-/./'`
tab=`echo $tab | sed 's/\./-/'`

# set -x

tmp=/tmp/junk.dat

urlbase="http://www.nco.ncep.noaa.gov/pmb/docs/grib2"
url="$urlbase/grib2_table$tab.shtml"

lc=0
[ $no == '4.4' ] && lc=1
[ $no == '4.5' ] && lc=1
[ $no == '5.0' ] && lc=1
[ $no == '5.1' ] && lc=1
[ $no == '6.0' ] && lc=1
[ $no == '4.5' ] && no='4.5_ncep_exit'
[ $no == '4.9' ] && no='4.9_need_to_edit'
[ $no == '4.10' ] && no='4.10_need_to_edit'

out="CodeTable_$no.dat"


if [ 1 -eq 1 ] ; then

wget -q -O "$tmp" "$url"
if [ $? -ne 0 ]; then
  echo "Download of $url failed, exit."
  exit
fi

no_lf <$tmp  >$tmp.1

fi

cat $tmp.1 | sed -e 's=<\([a-zA-Z]*\) [^>]*>=<\1>=g' -e 's=</body>.*==' -e 's=.*<body>==' \
    -e 's=<br>==g' -e 's=<big>==g' -e 's=</big>==g' -e 's=<center>==g' -e 's=</td>==g' -e 's=</tr>==g' \
    -e 's=</center>==g' -e 's/<span>//g' -e 's=</span>==g' -e 's=<tbody>==g' -e 's=</tbody>==g' >$tmp.2

cat $tmp.2 | sed -e 's/ *</</g' -e 's=> *=>=g' | sed -e 's=.*<th>Number</th>==' -e 's=<\/table.*==' >$tmp.3

#  <tr> -> LF
cat $tmp.3 | sed -e 's=<tr>=\n=g'  >$tmp.4


cat $tmp.4 | sed -e 's/ *<td>/<td>/g' -e '/<th>/d' -e '/^ *$/d' -e 's/(see [^)])//g' \
  -e 's=<a>.*</a>==g' -e '/^<td>[0-9]*-/d' -e 's/(see)//g' >$tmp.5

cat $tmp.5 | sed -e 's/ *$//' | \
  sed 's/<td>\(.*\)<td>\(.*\)<td>.*/\1 string=\"\2\"; break;/' | \
  sed 's/   */ /' | awk  '{if ($1 >= 192)  print  $0}' | sed 's/\(^[0-9]*\)/ case \1:/' >$tmp.6
echo >> $tmp.6

if [ $lc -eq 1 ] ; then
  cat $tmp.6 | sed -e y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/ \
    >$out
else
  cat $tmp.6 > $out
fi
cat $out

out=NCEP_local_levels_test.h

echo " if (center != 7) ival = -1;" > $out
cat $tmp.5 | sed -e 's/ *$//' | \
  sed 's/<td>\(.*\)<td>\(.*\)<td>.*/\1:"\2\"/' | \
  sed -e y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/ | \
  sed 's/ c)/ C)/g' | \
  sed 's/26c/26C/g' | \
  sed 's/   */ /' | awk  -F: '{if ($1 >= 192)  print " else if (strcmp(s,"$2") == 0) ival="$1";" }'  >> $out

echo " else ival = -1;" >> $out
echo >> $out
cat $out




exit
