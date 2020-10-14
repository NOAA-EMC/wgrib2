#!/bin/sh

# check for duplicate names
# {0,1,0,255,0,0,0,0, "TMP", "Temperature", "K"},

in=gribtab
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
