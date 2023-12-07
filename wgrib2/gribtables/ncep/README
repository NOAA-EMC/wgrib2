12/4/2023
 mv gribtable gribtable.12.2023
./get_gribtab.sh                                  makes gribtable
../../make_gribtable.sh                           makes gribtable.dat
../../check_dup_gribtable.sh gribtable.dat        check for duplicate names
removed .. 
0:1:0:255:0:0:4:15:var4m15:Upward UV radiation emitted/reflected from the Earths surface:-
1:0:0:255:7:1:1:196:var1h196:Binary Probability of precipitation exceeding average recurrence intervals (ARI):-
1:0:0:255:7:1:1:197:var1h197:Binary Probability of precipitation exceeding flash flood guidance values:-

4/26/2022: new get_gribtab.sh from  Manfred Schwarb *thanks*
mv gribtable.dat gribtable.dat.4.2022   save old gribtable.dat
./get_gribtab.sh                                  makes gribtable
../../make_gribtable.sh                           makes gribtable.dat
../../check_dup_gribtable.sh gribtable.dat        check for duplicate names

** The person in charge of the NCEP grib table decided to modify the 
   ncep local grib table (NCO web table) by removing the locally defined 
   soil moisture.  So if someone finds a locally defined soil moisture, 
   they may not be able figure out what it contains.  In the future, 
   the numbers could be reused.

   The person in change of the NCEP grib table was advised not to
   remove existing definitions.  But as the person once said, "You
   are not my boss."  He said that rmoving existing defintiions was
   needed for his "grib template" work.

   So Manfred's new get_gribtab.sh adds SOILM and SOIL_M


7/29/2021: new get_gribtab.sh from  Manfred Schwarb *thanks*
./get_gribtab.sh             makes gribtable
../../make_gribtable.sh      makes gribtable.dat


6/4/2021:
./get_gribtab.sh
mv gribtab gribtable
../../make_gribtable.sh

sh-4.2$ diff gribtable.dat gribtable.dat.6.2021 
540c540
< {0,0,0,255,7,1,2,221, "MAXDVV", "Hourly Maximum of Downward Vertical Velocity", "m/s"},
---
> {0,0,0,255,7,1,2,221, "MAXDVV", "Hourly Maximum of Downward Vertical Velocity in the lowest 400hPa", "m/s"},
542c542
< {0,0,0,255,7,1,16,198, "MAXREF", "Hourly Maximum of Simulated Reflectivity", "dB"},
---
> {0,0,0,255,7,1,16,198, "MAXREF", "Hourly Maximum of Simulated Reflectivity at 1 km AGL", "dB"},
544c544
< {0,0,0,255,7,1,2,220, "MAXUVV", "Hourly Maximum of Upward Vertical Velocity", "m/s"},
---
> {0,0,0,255,7,1,2,220, "MAXUVV", "Hourly Maximum of Upward Vertical Velocity in the lowest 400hPa", "m/s"},
603c603
< {0,0,0,255,7,1,7,199, "MXUPHL", "Hourly Maximum of Updraft Helicity", "m^2/s^2"},
---
> {0,0,0,255,7,1,7,199, "MXUPHL", "Hourly Maximum of Updraft Helicity over Layer 2km to 5 km AGL", "m^2/s^2"},

So the differences are in the removal of level information which 
is the correct thing to do.  These files should not have "hourly"
in their definition.  Maybe it will be corrected in the next update.



1/29/2021:  (old directory structure

./get_gribtab.sh
mv gribtab gribtable

no other changes
diff gribtable.dat gribtable.dat.1.2018 only shows additions
../../check_dup_gribtable.sh show no duplicates

No manual changes!!! did cleanuped webpages or Manfred's new download script fix the problems?
