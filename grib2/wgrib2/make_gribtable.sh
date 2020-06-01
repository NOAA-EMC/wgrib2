#!/bin/sh

# Script to convert grib2 table information from the
# "gribtab" format to the "gribtable.dat" format.
#
# Usage:
#   - Zero arguments:                        "gribtab" --> "gribtable.dat"
#   - One argument "mytab":                  "mytab"   --> "mytable.dat"
#   - Two arguments "mytab" "mytab.custom":  "mytab"   --> "mytab.custom"
#
# sample "gribtab" format and corresponding "gribtable.dat" format:
# 0:0:0:255:7:1:7:193:4LFTX:Best (4 layer) Lifted Index:K
# {0,0,0,255,7,1,7,193, "4LFTX", "Best (4 layer) Lifted Index", "K"},

if [ $# -ge 1 ]; then
  infile="$1"
else
  infile="gribtab"
fi
if [ $# -ge 2 ]; then
  outfile="$2"
else
  outfile="${infile}le.dat"
fi

if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

LC_ALL=C sort -t: -k9,9 -k1,8 "$infile" | awk -F: '
  BEGIN { OFS="," }
  {
    print "{" $1,$2,$3,$4,$5,$6,$7,$8," \"" $9 "\""," \"" $10 "\""," \"" $11 "\"},"
  }' > "$outfile"

exit
