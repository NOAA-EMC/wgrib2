/** @file
 * @brief Miscellaneous options.
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 */

#include <stdlib.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/* Misc.c            10/2024 Public Domain  Wesley Ebisuzaki
 *
 * misc options
 */

/** Current input type. */
extern enum input_type input;

/** File header flag. */
extern int header;

/** Mode of operation. */
extern int mode;

/** IEEE little-endian flag. */
extern int ieee_little_endian;

/** Append grib file flag. */
extern int file_append;

/** Item deliminator string. */
extern const char *item_deliminator;

/** Inventory file. */
extern struct seq_file inv_file;

/** New line character. */
extern const char *nl;

/** End of inventory string. */
extern const char *end_inv;

/** Read inventory input file. */
extern struct seq_file rd_inventory_input;

/** Use g2clib flag. */
extern int use_g2clib;

/*
 * HEADER:100:i:setup:0:read Inventory from stdin
 */

/**
 * Read inventory from stdin.
 * 
 * The -i option specifies that wgrib2 should read STDIN to determine the records to be 
 * processed. The -i_file option is similar except that wgrib2 reads file a user-specified 
 * file to determine the records to be processed. Some of the common uses of -i can also 
 * be done with the -match option. 
 * 
 * ## Usage
 * -i
 * 
 * ## Slicing and Dicing
 * Wgrib2 is a program to "slice and dice" grib2 files. Suppose you have a big grib file but 
 * you only want the 2-meter temperature and the precipitation. Rather than fill up your 
 * disk with big files, you can easily extract the required fields. 
 * 
 * The first step is to figure what is in the grib file. 
 * @code{.sh}
 * -sh-2.05b$ wgrib2 test.grb2 -s
 * 1:0:d=2005090200:HGT:1000 mb:60 hour fcst
 * 2:133907:d=2005090200:HGT:975 mb:60 hour fcst
 * 3:263511:d=2005090200:HGT:950 mb:60 hour fcst
 * 4:389058:d=2005090200:HGT:925 mb:60 hour fcst
 * 5:511037:d=2005090200:HGT:900 mb:60 hour fcst
 * 6:630256:d=2005090200:HGT:850 mb:60 hour fcst
 * 7:745505:d=2005090200:HGT:800 mb:60 hour fcst
 * ....
 * 291:37540403:d=2005090200:GPA:1000 mb:60 hour fcst
 * 292:37677072:d=2005090200:GPA:500 mb:60 hour fcst
 * 293:37791941:d=2005090200:5WAVA:500 mb:60 hour fcst
 * @endcode
 * 
 * Information overload. Lets see if you can find the desired variables.
 * 
 * @code{.sh}
 * -sh-2.05b$ wgrib2 test.grb2 -s | grep ':TMP:2 m'
 * 265:35107588:d=2005090200:TMP:2 m above ground:60 hour fcst
 * @endcode
 * 
 * Found the 2-m temperature, can we find the precipitation? 
 * 
 * @code{.sh}
 * -sh-2.05b$ wgrib2 test.grb2 -s | grep ':PRATE:'
 * 260:34814859:d=2005090200:PRATE:surface:54-60 hour fcst
 * @endcode
 * 
 * Yes. we found the fields. Now we need to combine the above into a single command using 
 * the or option of egrep; i.e., egrep '(A|B)'. 
 * 
 * @code{.sh}
 * -sh-2.05b$ wgrib2 test.grb2 -s | egrep '(:TMP:2 m|:PRATE:)'
 * 260:34814859:d=2005090200:PRATE:surface:54-60 hour fcst
 * 265:35107588:d=2005090200:TMP:2 m above ground:60 hour fcst
 * @endcode
 * 
 * Now that we have selected the records, we can send the output (inventory) back into 
 * wgrib2 to manipulate.
 * 
 * @code{.sh}
 * -sh-2.05b$  wgrib2 test.grb2 -s | egrep '(:TMP:2 m|:PRATE:)' | wgrib2 -i test.grb2 -grib small.grb
 * 260:34814859:d=2005090200:PRATE:surface:54-60 hour fcst
 * 265:35107588:d=2005090200:TMP:2 m above ground:60 hour fcst
 * @endcode
 * 
 * The above command made a grib2 file consisting of the precipation (PRATE) and 2-m 
 * temperature (TMP). 
 * 
 * @code{.sh}
 * -sh-2.05b$ ls -l test.grb2 small.grb
 * -rw-r--r--    1 wd51we   wd5        212429 2006-10-16 15:08 small.grb
 * -rwxr-xr-x    1 wd51we   wd5      37862776 2006-05-25 15:16 test.grb2
 * @endcode
 * 
 * As you can see, the new file is much smaller than the original file. 
 * 
 * @code{.sh}
 * -sh-2.05b$  wgrib2 small.grb
 * 1:0:d=2005090200:PRATE:surface:54-60 hour fcst
 * 2:110654:d=2005090200:TMP:2 m above ground:60 hour fcst
 * @endcode
 * 
 * As expected, the new grib file only has the desired two fields. 
 * 
 * ## Decoding a single record
 * Another use of the -i option is to specify the field to decode. 
 * @code{.sh}
 * -sh-2.05b$ wgrib2 test.grb2 -s | grep ':PRATE:' | wgrib2 -i test.grb2 -spread field.txt
 * 260:34814859:d=2005090200:PRATE:surface:54-60 hour fcst
 * 
 * -sh-2.05b$ head field.txt
 * lon,lat,PRATE surface d=2005090200 54-60 hour fcst
 * 0,-90,5e-06
 * 0.5,-90,5e-06
 * 1,-90,5e-06
 * 1.5,-90,5e-06
 * 2,-90,5e-06
 * 2.5,-90,5e-06
 * 3,-90,5e-06
 * 3.5,-90,5e-06
 * 4,-90,5e-06
 * @endcode
 * 
 * ## Speed
 * The following command is very common so it incorporated within wgrib2. This speeds up 
 * the operation by eliminating two program executions and duplicate reads. 
 * 
 * @code{.sh}
 * wgrib2 gribfile | grep "string" | wgrib2 -i gribfile <other options>
 * @endcode
 *
 * is equivalent to
 *
 * @code{.sh}
 * wgrib2 gribfile -match "string" <other options>
 * @endcode
 * 
 * However, the -i option can be more efficient when making multiple extractions from a 
 * file. For example, 
 * 
 * @code{.sh}
 * wgrib2 gribfile >gribfile.inv
 * grep "string1" gribfile.inv | wgrib2 -i gribfile <other options>
 * grep "string2" gribfile.inv | wgrib2 -i gribfile <other options>
 * grep "string3" gribfile.inv | wgrib2 -i gribfile <other options>
 * @endcode
 * is faster than
 * 
 * @code{.sh}
 * wgrib2 gribfile -match "string1" <other options>
 * wgrib2 gribfile -match "string2" <other options>
 * wgrib2 gribfile -match "string3" <other options>
 * @endcode
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_i(ARG0) {
    if (mode == -1) input = inv_mode;
    return 0;
}

/*
 * HEADER:100:i_file:setup:1:read Inventory from file
 */

/**
 * Read inventory from file.
 * 
 * The -i_file option is similar except that wgrib2 reads file a user-specified file to 
 * determine the records to be processed. 
 * 
 * ## Usage
 * -i_file FILE
 *
 * FILE is the name of the file containing the inventory.
 * 
 * The following 3 lines are equivalent.
 * 
 * @code{.sh}
 * cat FILE.inv | grep UGRD | wgrib2 -i FILE.grb -bin data.bin
 * cat FILE.inv | wgrib2 -i FILE.grb -match UGRD -bin data.bin
 * wgrib2 -i_file FILE.inv FILE.grb -match UGRD -bin data.bin 
 * @endcode
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_i_file(ARG1) {
    if (mode == -1) {
        input = inv_mode;
        if (fopen_file(&rd_inventory_input, arg1, "r") != 0) {
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        fclose_file(&rd_inventory_input);
    }
    return 0;
}

/*
 * HEADER:100:v0:misc:0:not verbose (v=0)
 */

/**
 * Not verbose mode (v=0).
 * 
 * Wgrib2 has multiple levels of verbosity. The lowest level (0) is the default and set 
 * by -v0. The next level is 1 and is set by -v. Really verbose is set by -v2. Debugging 
 * is set by level 99. Note that the verbosity can be changed multiple times on the 
 * command line. 
 * 
 * ## Usage
 * -v0
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_v0(ARG0) {
    set_mode(0);
    return 0;
}

/*
 * HEADER:100:v:misc:0:verbose (v=1)
 */

/**
 * Verbose mode (v=1).
 * 
 * Wgrib2 has multiple levels of verbosity. The lowest level (0) is the default and set 
 * by -v0. The next level is 1 and is set by -v. Really verbose is set by -v2. Debugging 
 * is set by level 99. Note that the verbosity can be changed multiple times on the 
 * command line. 
 * 
 * ## Usage
 * -v
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_v(ARG0) {
    set_mode(1);
    return 0;
}

/*
 * HEADER:100:v2:misc:0:really verbose (v=2)
 */

/**
 * Really verbose mode (v=2).
 * 
 * Wgrib2 has multiple levels of verbosity. The lowest level (0) is the default and set 
 * by -v0. The next level is 1 and is set by -v. Really verbose is set by -v2. Debugging 
 * is set by level 99. Note that the verbosity can be changed multiple times on the 
 * command line. 
 * 
 * ## Usage
 * -v2
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_v2(ARG0) {
    set_mode(2);
    return 0;
}

/*
 * HEADER:-1:v97:misc:0:verbose mode for debugging only (v=97) 
 */

/**
 * Verbose mode for debugging only (v=97).
 * 
 * ## Usage
 * -v97
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_v97(ARG0) {
    set_mode(97);
    return 0;
}

/*
 * HEADER:-1:v98:misc:0:verbose mode for debugging only (v=98) 
 */

/**
 * Verbose mode for debugging only (v=98).
 * 
 * ## Usage
 * -v98
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_v98(ARG0) {
    set_mode(98);
    return 0;
}

/*
 * HEADER:-1:v99:misc:0:verbose mode for debugging only (v=99) 
 */

/**
 * Verbose mode for debugging only (v=99).
 * 
 * ## Usage
 * -v99
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_v99(ARG0) {
    set_mode(99);
    return 0;
}

/*
 * HEADER:100:header:misc:0:f77 header or nx-ny header in text output (default)
 */

/**
 * Sets the header flag.
 * 
 * When the header flag is set, binary and ieee is read and written using f77-style 
 * header and trailers. When a text file (as opposed to spread sheet or csv) is written, 
 * a preliminary line with the grid dimension, "nx ny", is written. The default is -header. 
 * 
 * ## Usage
 * -header
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, error code otherwise
 *
 * ## Example
 * ???
 *
 * @author Wesley Ebisuzaki @date 2006
 */
int f_header(ARG0) {
    header = 1;
    return 0;
}

/*
 * HEADER:100:no_header:misc:0:no f77 header or nx-ny header in text output
 */

/**
 * Clears the header flag.
 * 
 * When the header flag is set, binary and ieee is read and written using f77-style 
 * header and trailers. When a text file (as opposed to spread sheet or csv) is written, 
 * a preliminary line with the grid dimension, "nx ny", is written. The default is -header. 
 * 
 * ## Usage
 * -no_header
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, error code otherwise
 *
 * ## Example
 * ???
 *
 * @author Wesley Ebisuzaki @date 2006
 */
int f_no_header(ARG0) {
    header = 0;
    return 0;
}

/*
 * HEADER:100:nl:inv:0:inserts new line into inventory
 */
/**
 * Inserts a new line into the inventory. It is used to make the inventory prettier.
 * 
 * ## Usage
 * -nl
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_nl(ARG0) {
    if (mode >= 0) strcpy(inv_out, "\n");
    return 0;
}

/*
 * HEADER:100:nl_out:inv_output:1:write new line in file X
 */

/**
 * Writes new line in specified file.
 * 
 * ## Usage
 * -nl_out FILE
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_nl_out(ARG1) {
    struct seq_file *save;
    char nl = '\n';

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("nl_out: memory allocation","");
        if (fopen_file(save, arg1, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = *local;
        if (fwrite_file(&nl, sizeof(char), 1, save) != 1) 
            fatal_error("nl_out: write error %s", arg1);
    }
    return 0;
}

/*
 * HEADER:100:print:inv:1:inserts string (X) into inventory
 */

/**
 * Inserts a string into the inventory.
 * 
 * ## Usage
 * -print "string"
 *
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 2006
 */
int f_print(ARG1) {
    if (mode >= 0) {
        strcpy(inv_out, arg1);
    }
    return 0;
}

/*
 * HEADER:100:print_out:inv_output:2:prints string (X) in file (Y)
 */

/**
 * Writes a string to a specified file.
 * 
 * ## Usage
 * -print_out "string" FILE
 * 
 * Writes "string" to FILE.
 *
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_print_out(ARG2) {
    struct seq_file *save;
    int i;

    if (mode == -1) {
        *local = save = (struct seq_file *) malloc( sizeof(struct seq_file));
        if (save == NULL) fatal_error("nl_out: memory allocation","");
        if (fopen_file(save, arg2, file_append ? "ab" : "wb") != 0) {
            free(save);
            fatal_error("Could not open %s", arg2);
        }
    }
    else if (mode == -2) {
        save = *local;
        fclose_file(save);
        free(save);
    }
    else if (mode >= 0) {
        save = *local;
        i = strlen(arg1);
        if (fwrite_file(arg1,1, i, save) != i) return 1;
    }
    return 0;
}

/*
 * HEADER:100:colon:misc:1:replace item deliminator (:) with X
 */

/**
 * Replace item deliminator (:) with specified string.
 * 
 * The wgrib2 inventory consists of many fields separated by a colon (:). For example, 
 * 
 * @code{.sh}
 * $ wgrib2 small.grb2
 * 1:0:d=2009060500:HGT:200 mb:180 hour fcst:ENS=+19
 * @endcode
 * 
 * You can change the field separator from a colon to any string. For example, 
 * 
 * @code{.sh}
 * $ wgrib2 -colon "[--]" small.grb2
 * 1:0[--]d=2009060500[--]HGT[--]200 mb[--]180 hour fcst[--]ENS=+19
 * @endcode
 * 
 * You may want to use the -colon option because you are going to be parsing the inventory 
 * with another program that uses a different field separator. (CSV file?). 
 * 
 * ## Usage
 * -colon "string"
 * 
 * The colon character will be replace with "string" in the output.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_colon(ARG1) {
    item_deliminator = arg1;
    return 0;
}

/*
 * HEADER:100:one_line:setup:0:puts all on one line (makes into inventory format)
 */

/**
 * Changes the format of the inventory so all the items are one line. This is useful in 
 * using -grid, for example, in making inventories that can be grepped. 
 * 
 * ## Usage
 * -one_line
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_one_line(ARG0) {
    nl = " ";
    return 0;
}

/*
 * HEADER:100:crlf:setup:0:make the end of the inventory a crlf (windows) instead of newline (unix)
 */

/**
 * Changes the end of the inventory to a CRLF (Windows) format instead of a newline (Unix).
 * 
 * On unix/linux machines, wgrib2 terminates the end of a inventory line by a LF (newline or "\n"). 
 * On Windows machines, wgrib2 terminates the end of a inventory line by a CRLF. The -crlf option 
 * is for unix/linux machine so that they terminate the line by CRLF. This will make the inventory 
 * Windows compatible. 
 * 
 * ## Usage
 * -crlf
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_crlf(ARG0) {
    end_inv = "\r\n";
    return 0;
}

/*
 * HEADER:100:append:setup:0:append mode, write to existing output files
 */

/**
 * Sets the append flag to write to existing output files.
 * 
 * The out options are expected to respect this flag when opening output files. So an -append 
 * option before writing (-text, -bin, etc) should append to a currently existing file.
 * 
 * ## Usage
 * -append
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_append(ARG0) {
    if (mode == -1) file_append = 1;
    return 0;
}

/*
 * HEADER:100:no_append:setup:0:not append mode, write to new output files (default)
 */

/**
 * Clears the append flag to write to new output files (default).
 * 
 * The out options are expected to respect this flag when opening output files. The 
 * -no_append directs the file to be created before use which is the default. 
 * 
 * ## Usage
 * -no_append
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_no_append(ARG0) {
    if (mode == -1) file_append = 0;
    return 0;
}

/*
 * HEADER:100:inv:setup:1:write inventory to X
 */

/**
 * Write inventory to a specific file.
 * 
 *  This option could be used with an option to send decoded data to STDOUT. 
 * 
 * @code{.sh}
 * $ wgrib2 file -inv my.inv -text - -no_header  -match ':HGT:500 mb:' |  JOB
 * @endcode
 * 
 * -inv my.inv   writes the inventory to my.inv
 * -text -       write the decoded data as a text file to stdout
 * -no_header    no header
 * JOB           is some program that reads the raw data from stdin
 * 
 * The -inv option is similar but different from saving stdout. 
 * 
 * @code{.sh}
 * $ wgrib2 file -inv my.inv
 * $ wgrib2 file >my.inv2
 * @endcode
 * 
 * In the above example, my.inv and my2.inv will be the same. However, wgrib2 can be 
 * used as a grib filter. In this case, grib data is read from stdin and written to 
 * stdout. 
 * 
 * @code{.sh}
 * $ cat FILE1 FILE2 | wgrib2 - -match ':HGT:500 mb:' -grib - -inv /dev/null | \
 *  wgrib2 - -new_grid_winds earth -new_grid ncep grid 3 newgrd.grb
 * @endcode
 * 
 * The -inv option is very specialized and helps when you want to imbed wgrib2 within 
 * another program. 
 * 
 * ## Usage
 * -inv FILE
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_inv(ARG1) {
    if (mode == -1) {
        fclose_file(&inv_file);
        if (fopen_file(&inv_file, arg1, file_append ? "a+" : "w+") != 0) {
            fatal_error("Could not open %s", arg1);
        }
    }
    else if (mode == -2) {
        fclose_file(&inv_file);
        /* reopen inv to point to stdout */
        if (fopen_file(&inv_file,"-","w") != 0) fatal_error("Could not set inv_file to stdout","");
    }
    return 0;
}

/*
 * HEADER:100:little_endian:misc:0:sets ieee output to little endian (default is big endian)
 */

/**
 * Sets the IEEE output to little endian (default is big endian).
 * 
 * This does not affect the order in which binary numbers are read/written. 
 * 
 * ## Usage
 * -little_endian
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_little_endian(ARG0) {
    ieee_little_endian = 1;
    return 0;
}

/*
 * HEADER:100:big_endian:misc:0:sets ieee output to big endian (default is big endian)
 */

/**
 * Sets the ieee output to big endian (default).
 * 
 * This does not affect the order in which binary numbers are read/written. 
 * 
 * ## Usage
 * -big_endian
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0.
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_big_endian(ARG0) {
    ieee_little_endian = 0;
    return 0;
}

/*
 * HEADER:100:g2clib:setup:1:X=0/1/2 0=WMO std 1=emulate g2clib 2=use g2clib
 */

/**
 * Sets the g2clib usage mode.
 * 
 * Originally wgrib2 used the g2clib library for unpacking the grid point data (GDS). 
 * The first break from a complete dependence on the g2clib was adding support for 
 * ieee-type grib files. Then the optimized grib1-style unpacking from wgrib was added 
 * to the internal routines. Then jpeg, png and complex packing were added to the internal 
 * decoder. Finally more packing schemes that were not in g2lib/g2clib were added (run-length 
 * and log scaling). 
 * 
 * As of wgrib2 v1.9.8, a third option was added, use the internal decoder in g2clib 
 * emulation mode. The options were also relabeled. The new default is to use the internal 
 * decoder in g2clib emulation mode (-g2clib 1). To use use the internal decoder you still 
 * use -g2clib 0. To get the g2clib decoder, use -g2clib 2 which is now a compile-time option. 
 * The changes were made because 
 * 
 * 1. internal routines are faster for complex packing (1 cpu)
 * 2. some internal routines were parallelized for decoding (complex and simple packing)
 * 3. some distributions were linking the official g2clib rather than the patched version 
 * included with the wgrib2 sources. This caused seg faults.
 * 
 * Wgrib2 has always used its own routines for encoding grib files. The encoding follows the 
 * WMO standard but are compatible with the NCEP libraries (g2clib and g2lib). 
 * 
 * The -g2clib option allows you to select the internal/g2clib decoder. If X is 0, then internal 
 * routines are selected. If X is 1, then the internal decoder with g2clib emulation is selected 
 * (g2clib is used with older wgrib2 versions). When X is 2, then the g2clib decoder is used if 
 * the g2clib decoder were compiled into the executable. Note that g2clib doesn't support ieee, 
 * RLE, and irregular structured grids. 
 * 
 * ## Usage
 * -g2clib 0
 *
 * Use WMO standard (for all files)
 * -g2clib 1
 * 
 * Default mode. Emulate NCEP g2/g2c bug for decoding constant values fields for files from NCEP 
 * (center=NCEP (7)). Files not from NCEP are unaffected.
 * 
 * -g2clib 2
 * 
 * Use g2clib. Only available if compiled with g2clib. This option should only be used for testing.
 * Some distributions will seg fault using this option. Files from non-NCEP sources may have the 
 * wrong constant values.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int f_g2clib(ARG1) {
    use_g2clib = atoi(arg1);
    if (use_g2clib == 0 || use_g2clib == 1) return 0;
#ifdef USE_G2CLIB
    if (use_g2clib == 2) return 0;
#endif
    if (use_g2clib == 2) fatal_error("g2clib not installed","");
    fatal_error("illegal g2clib option %s", arg1);
    return 0;
}

