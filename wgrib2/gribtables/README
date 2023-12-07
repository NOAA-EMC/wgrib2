README for gribtables (for variable/field names)

Gribtables are stored in (center)/(center)_gribtable.dat or ncep/gribtable.dat (for NCEP)

There are two categories of gribtables
  1) only include the locally defined names
  2) include WMO and locally defined names

The second category includes DWD, ECMWF and NCEP.  You access these
tables by the -names (center) option.

                    Naming Convention

For the WMO defined fields, the -names (center) sets the naming convention.

For locally defined fields, the local center sets the naming convention.


                    To add more gribtables


change wgrib2/grb2.h (to add center code), wgrib2/cnames.c, wgrib2/gribtab.c, wgrib2/Mod_grib.c 
and perhaps wgrib2/Names.c (for category 2).  Please keep the location of the gribtable
to center/(center)_gribtable.dat because the wgrib2/makefile uses it.

