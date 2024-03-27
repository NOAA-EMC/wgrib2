#!/bin/sh
# This is a test script for the wgrib2 project.
#
# Ed Hartnett, 3/27/24

set -e
echo ""
echo "*** Running wgrib2 test"

# Just run executable. For some reason it returns 8.
ls -l ../wgrib2
../wgrib2/wgrib2 > out.txt  && exit 1
[ $? -ne 8 ] && exit 2

echo "*** SUCCESS!"
exit 0
