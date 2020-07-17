#!/bin/sh
#
# this sets up the functions for wgrib2
#
# makes fnlist.c, fnlist.h
#
# set -x

# get list of all callable functions
grep "^ \* HEADER:" [A-Z]*.c | cut -f3- -d: | sort -t: -k3,3 -k2,2 >fnlist

cat >fnlist.c <<EOF
#include <stdio.h>
#include <stdlib.h>
#include "fnlist.h"

struct function functions[] = {
EOF


cat >fnlist.h <<EOF
/* headers for wgrib callable functions */

enum fntype {inv, output, inv_output, misc, setup};

struct function {const char *name; int (*fn)(); enum fntype type; int nargs; const char *desc; int sort;};

extern struct function functions[];

extern int nfunctions;

EOF


{
   read line
   while [ "$line" != "" ]
   do
      sort="`echo $line | cut -f1 -d:`"
      v="`echo $line | cut -f2 -d:`"
#     check for alias
      if [ `echo "$v" | grep -c =` -eq 0 ] ; then
          var="$v";
          var1="f_`echo "$v" | sed -e 's/\./_/g'`"
      else
          var=`echo "$v" | sed 's/=.*//'`
          var1=`echo "$v" | sed 's/.*=//'`
      fi
echo "var=$v $var $var1"
      type="`echo $line | cut -f3 -d:`"
      nargs="`echo $line | cut -f4 -d:`"
      desc="`echo $line | cut -f5- -d:`"
#      echo " var=$var  type=$type  desc=($desc)"
      echo "   {\"$var\",$var1, $type, $nargs, \"$desc\", $sort}," >>fnlist.c

      t="int $var1(int mode, unsigned char **sec, float *data, unsigned int ndata, char *inv, void **local"

      arg=1
      while [ $nargs -gt 0 ] ; do
          t="$t, const char *arg$arg"
          nargs="`expr $nargs - 1`"
          arg=`expr $arg + 1`
      done
      t="$t);"
#       echo "$t"
      echo "$t" >> fnlist.h
      read line
   done
} < fnlist

echo "};" >>fnlist.c

echo " " >> fnlist.c
echo "int nfunctions = sizeof functions / sizeof functions[0];" >> fnlist.c


