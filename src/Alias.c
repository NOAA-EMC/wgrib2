/* a list of aliases
 *
 * public domain 2/2007 Wesley Ebisuzaki
 */

#include <stdio.h>
/*
 * HEADER:200:disc=f_code_table_0_0:inv:0:discipline (code table 0.0)
 * HEADER:-1:v1=f_v:misc:0:verbose (v=1)
 * HEADER:-1:quit=f_end:misc:0:stop after first (sub)message (save time)
 * HEADER:200:process=f_code_table_4_3:inv:0:Process type (code table 4.3)
 * HEADER:200:-version=f_version:misc:0:print version
 * HEADER:200:elseif=f_if:Elseif:1:elseif X (POSIX regular expression) conditional on match, -if ... -elseif ... -endif
 * HEADER:200:elseif_fs=f_if_fs:Elseif:1:elseif X (fixed string) conditional execution 
 * HEADER:200:elseif_n=f_if_n:Elseif:1:elseif (inv numbers in range), X=(start:end:step)
 * HEADER:200:elseif_rec=f_if_rec:Elseif:1:elseif (record numbers in range), X=(start:end:step)
 * HEADER:200:elseif_reg=f_if_reg:Elseif:1:elseif rpn registers defined, X = A, A:B, A:B:C, etc A = register number
 */

// removed   2/2015 * HEADER:200:pdt=f_code_table_4_0:inv:0:product definition template (PDT)
