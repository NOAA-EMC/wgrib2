Development directory for wgrib2

To make development sane, most functions can be plugged into the code
automatically.  The "compile" script scan the source code to the various
"option" functions and adds them to a table so that they can be selected 
by the user.

Rules for Option Functions

  (1) filename *.c and the first character is in upper case.
  (2) Each option function MUST have a header describing the name,
       type, number of optional arguments and a sort order (not used yet)
  (3) Setup functions only get called once before processing of the GRIB data
  (4) Output functions NEED to open files in init section
  (5) Inv functions write strings to inv_out which gets written to standard out
  (6) Misc functions are none of the above
  (7) All functions are called before processing (mode = -1)
      once per grib message (mode = 0/1/2)
      and at the end of the program (mode=-2)
  (8) Functions are called in order of their appearence on the command line
  (9) name of function is f_(name)
 (10) communicate to main program through global variables
 (11) function arguments are standardized but may or may not contain good values
      for example, in the "initialization" phase, the variable data is undefined.

Header format:

  * HEADER:sort_order:name:type:n_args:description

  n_args = 0,..,4


The "sort_order" is used to sort the help page listing.

   if (sort_order == -1), this entry does not show up on help screen;
      however, you can find it by searching for it by the -help xxx option.
      Usage: for very infrequently used functions

The "name" is the option name.  That is the function is invoked by -{name}

   Normal (no equal sign in the name)

      The "name" is the option name.  That is the function is invoked by -{name}
      The name of the function that is excuted for each option is f_name* where name*
      is like name but all the periods have been replaced by underscores.

   Alias (equal sign in the name)

      The format of an alias is  "option_name:function_name"

      In this case, the option is invoked by -{option_name} and the name of
      function that is executed is {function_name}.  Alias are useful for
      making the option list more user friendly.

if type == inv
   writes text to stdout based on data file

if type == output 
   writes grid data to a file

if type == inv_output 
   writes non-grid data (metadata/inventory) to a file

if type == setup
   only executed once during the initialation phase

if type == misc
   ex. version number, format controls, verbosity


mode = -1 -> initialization
mode = -2 -> finalize
mode >= 0 -> normal mode (0 = simple, 1 = verbose)


                            Structure of wgrib2

The structure of wgrib2 has changed from wgrib.  Wgrib2 is now just a driver
that calls a list of "option" functions that were specified on the command line.
Each step of the inventory/decode is controlled by the list of option functions.

Option functions use pre-specified APIs and are "drop-in" and compile.

T
