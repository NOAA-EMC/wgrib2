Pywgrib2_s is a python module to read and write grib2 using the wgrib2 shared 
library.  There will-be/are various python modules based on wgrib2 (pywgrib2_s, 
pywgrib2_lite, and pywgrib2_xr).  


At the time of release of wgrib2 v3.0.1 (2/2021), pywgrib2_ is late alpha-beta
stage of software development.  Changes to the API could occur but breaking
current python code is unlikely.  The documentation is complete, but needs to 
be polished.  Sample code examples (cookbook) have been tested but documentation 
for the cookbook needs to be done.

At this time (2/2021), pywgrib2_s works on linux, MacOS and Windows 10, using
gcc, AOCC (clang+flang), nvc, and cygwin-gcc+gfortern. As of 2/2021, the intel 
compilers on linux have not been tested.  Given that linux is already supported
by gnu, AOCC and nvc compilers, it is not a high priority.


Documentation:
   pywgrib2_*:   https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/pywgrib2.html
   pywgrib2_s:   https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/pywgrib2_s.html

Changes for pywgrib2_s from the wgrib2 v3.0.1 to v3.0.1 releases include
   support for read_sec:  read grib section
               write_sec: write grib section
               names:     change names of WMO-defined variables to either dwd, ecmwf 
                           or ncep definitions.  The dwd and ecmwf names are alpha.
