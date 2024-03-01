/* wgrib2 utility:  public domain 2024 w. ebisuzaki
 *
 * this calls the wgrib2 rountine
 */

#include <stdio.h>


int main(int argc, const char **argv) {
  int i;
  fprintf(stderr,"wgrib2_main argc=%d\n", argc);
  i=wgrib2(argc, argv);
  fprintf(stderr,"wgrib2_main ret code %d\n", i);
  return i;
}
