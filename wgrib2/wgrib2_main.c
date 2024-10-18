/* wgrib2 main module:  public domain 2024 w. ebisuzaki
 *
 * ancient:  wgrib2.c was the main for the wgrib2 utility.  There was no wgrib2 library, and
 *           no call to the wgrib2(..).
 *
 * old:      A user wanted to call wgrib2 from a C program.  He converted wgrib2.c to be either a main(..)
 *           or a routine wgrib2(..) depending on a flag (CALLABLE_WGRIB2).  There were two builds,
 *           one for the utility and a second for the library.
 *
 * new:      move to github
 *           Reimplement the old idea to make the wgrib2 utility a call to wgrib2(..).
 *           This involves removing all references to CALLABLE_WGRIB2
 *           This was tested in a pre-github version but never implemented because it provided no reward
 *           With github, the build system is being rebuilt from Kyle's version.  Rather than the two
 *           builds, make it a single build with the wgrib2 utility calling wgrib2(..)

 *
 * 8/2024    the main for the wgrib2 utility is moved from wgrib2.c to wgrib2_main.c
 */

#include <stdio.h>
#include "wgrib2_api.h"


int main(int argc, const char **argv) {
   return wgrib2(argc, argv);
}
