#!/bin/sh
#  10/2024  Public Domain  Wesley Ebisuzaki
homedir=`pwd`
PATH="$homedir:$PATH"

scriptlist=`ls $homedir/get_*.sh`
mkdir -p tables.tmp
cd tables.tmp

indx=0
for script in $scriptlist; do
  ((++indx))
  $script || {
      echo "Error in script $script"
      exit $indx
    }
done

exit
