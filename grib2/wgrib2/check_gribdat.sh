#!/bin/sh

if [ $# -ne 2 ] ; then
  echo "$0 gribtab.dat.1 gribtab.dat.2"
  exit 8
fi
s=/tmp/junk.$$.1
t=/tmp/junk.$$.2

cat $1 |  sed 's/",.*/"/' >$s
cat $2 |  sed 's/",.*/"/' >$t

diff $s $t 

rm $s $t
