#!/bin/sh

tab='3.15'
no=`echo $tab | sed 's/-/./'`
tab=`echo $tab | sed 's/\./-/'`

out="CodeTable_$no.dat"

# set -x

tmp=/tmp/junk.dat

urlbase="http://www.nco.ncep.noaa.gov/pmb/docs/grib2"
url="$urlbase/grib2_table$tab.shtml"

lc=0
[ $no == '6.0' ] && lc=1
[ $no == '4.5' ] && no='4.5_ncep'
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
  -e 's=<a>.*</a>==g' -e '/^<td>[0-9]*-/d' -e 's/(see)//g' -e 's/(see[^)]*)//g' >$tmp.5

cat $tmp.5 | sed -e 's/ *$//' | \
  sed -e 's/<sup>/^/g' -e 's/<\/sup> */ /g'  -e 's/ *$//' | \
  sed 's/<td>\(.*\)<td>\(.*\)<td>\(.*\)/    case \1: string=\"\2\ [\3]"; break;/' >$tmp.6
echo >> $tmp.6

if [ $lc -eq 1 ] ; then
  cat $tmp.6 | sed -e y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/ >$out
else
  cat $tmp.6 > $out
fi
cat $out

echo "diff:"
diff $out ~/grib2/wgrib2/$out


exit
