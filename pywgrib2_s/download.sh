#!/bin/sh

#
# download pywgrib2_s.tgz
#
# contains pywgrib2_s.py and cookbook
#

echo "loading pywgrib2_s"
wget "https://ftp.cpc.ncep.noaa.gov/wd51we/pywgrib2_s/pywgrib2_s.tgz"

echo "loading pywgrib2_s: cookbook (slow, not essential)"
wget "https://ftp.cpc.ncep.noaa.gov/wd51we/pywgrib2_s/pywgrib2_s_cookbook.tgz"
