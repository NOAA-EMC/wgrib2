#!/bin/sh

homedir=`pwd`
PATH="$homedir:$PATH"
dat=`date "+%Y%m%d"`

scriptlist=`ls $homedir/get_*.sh`
mkdir -p tables_$dat
cd tables_$dat

indx=0
for script in $scriptlist; do
  ((++indx))
  $script || {
      echo "Error in script $script"
      exit $indx
    }
done

exit
