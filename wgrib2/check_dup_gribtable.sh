#!/bin/sh
#   10/2024    Public Domain  Wesley Ebisuzaki
#
# This script checks parameter info for wgrib2. This is a test script
# that is manually run when updating the grib2 table from the NCO web
# pages.
# 
export LC_ALL=C

# check for duplicate names
# {0,1,0,255,0,0,0,0, "TMP", "Temperature", "K"},

in=$1
awk '{if ($8< 192) print $0}' FS=':' <$in >junk
cut -f9 -d: <junk >junk2
sort -u <junk2 >junk3
sort <junk2 >junk4
diff junk4 junk3


echo "finished"
exit

in=gribtable.dat

sed -e 's/^[^"]*"//'  -e 's/".*//' <$in | sort  >junk
sort -u <junk >junk2
diff junk junk2 >junk.diff
echo "finished"
