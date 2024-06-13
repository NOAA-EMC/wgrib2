# wgrib

# Introduction

wgrib is deprecated and no longer supported. It should not be used in
operational workflows.

The command wgrib both inventories and decodes GRIB-1
files. There are three types of inventories (regular, short,
and verbose) which can be viewed as a human-readable index file.
The inventories can be manipulated to select the records to
decode. The output formats of wgrib include: text, binary
(system dependent), big endian IEEE and GRIB. In addition
the program can produce a non-inventory description of the GRIB
records. Information includes range of values, grid type, etc.

The program can be compiled to either use the NCEP operational
GRIB tables or the NCEP/NCAR Reanalysis GRIB table as the default
table in cases of ambiguity.

The program does not handle spectral files nor files with complex
packing.

### Program History Log
- 1997-08-19  EBISUZAKI  OPERATIONAL VERSION 1.6.0
- 1998-08-24  EBISUZAKI  OPERATIONAL VERSION 1.7.0b, added '-d all',
                        fixed -H, reanalysis ID code upgrade,
                        meta data for: rotated lat-lon grid,
                        Arakawa E-grid, O3TOT (fixed spelling),
                        fixed bitmap bug when nbits > 24
- 1999-05-13  EBISUZAKI  OPERATIONAL VERSION 1.7.3
                        Update NCEP opn grib table
                        Support for NCEP ensembles
                        Fixed err msg with thinned grids and bitmaps
                        fixed -H option
                        change "x12 hours" format to "hr" format
                        Updated (128,160) and added new (129,130,131,140,
                           150,170,180) ECMWF tables
- 2001-05-31  EBISUZAKI  OPERATIONAL VERSION 1.7.3.6
                        Added fractional mb (NCEP files only)
                        Fixed code for precision greater than 31 bits
                        Check for missing grib file (error message)
                        Polar stereo, Lambert: print direction increments (-V)
                        Undefined direction increments set to zero (-V)
                        Added new levels

- 2003-06-27 EBISUZAKI   Operational version 1.8.0.3j
                        Brings the operational version up to date.
                        Adds new tables: ncep 129, 130, 131
                           new EC, CPTEC and DWD tables
                        Fix levels 117, 205, 141
                        Decode of simple packed spectral data
                        decode Scan mode
                        AIX make file supports 2GB+ data files

- 2004-06-08 EBISUZAKI   Operational version 1.8.0.9a
			 Updates grib tables 129, 130, 131
			 adds new time codes 128-135
			 fixed scan mode, N/S vs grid wind diagnostic
			 support for ncep-style ensembles encoding
			 support for minutes

- 2006-09-15 EBISUZAKI   Operational version v1.8.0.12g
			 Adds more ncep ensemble information, revised level 113
			 Updates NCEP grib tables 128, 129, 131
			 Adds ECMWF grib table 172, DWD 204, 205
			 Adds vertical level 20, and levels for NCEP ocean model
			 Adds time ranges 6 and 8
			 Adds error message for complex packing, truncated grib messages
			 Bug fix for simple packed spectral files
			 Updated thinned Gaussian grid information

- 2007-05-01 EBISUZAKI   Update to version v1.8.0.12o
                        Updates to NCEP tables 128 and 129
                        Better support for user-defined grib tables

### Command Line Options

Options for stdout (terminal output), mutually exclusive
- -s     short inventory
- -v     verbose inventory
- -V     very verbose file/record description, not an inventory (none) regular inventory (default)

Options for stdout (terminal output), mutually exclusive
- -ncep_opn    use NCEP operational grib table as default
- -ncep_rean   use NCEP reanalysis grib table as default
- -ncep_ens    use NCEP-style ensemble information
- (none)       default grib table determined at compile time

Options for stdout
- -verf  print the verification instead of the initial time
- -4yr   print the four digit year code
- -min   print the minutes
- -PDS   print the PDS in hexadecimal
- -PDS10 print the PDS in decimal
- -GDS   print the GDS in hexadecimal
- -GDS10 print the GDS in decimal

Options for decoding, mutually exclusive
- -d [record number|all]     decode/dump one record
- -p [byte position]         decode/dump one record
- -i                         decode/dump controlled by stdin (inventory)
- (none)                     do not decode/dump

Options for decoding, mutually exclusive
- -text      write as a text file
- -bin       write as a binary file (default)
- -ieee      write as a big endian IEEE file
- -grib      write as a grib file

Options for decoding, mutually exclusive
- -nh        no header
- -h         header (default)

Options for decoding
- -o [file]  write to [file], default is "dump"
- -append    append when writing
- -H         output will include PDS and GDS (-bin/-ieee only)

More information can be obtained from
http://www.cpc.ncep.noaa.gov/products/wesley/wgrib.html.
