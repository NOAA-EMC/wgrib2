#!/bin/sh

## @file
## @brief This script runs all get_*.sh scripts in the current directory
## to create WMO tables.
## @author Public Domain; Wesley Ebisuzaki @date 10/2024

## @cond
homedir=`pwd`
PATH="$homedir:$PATH"

scriptlist=`ls $homedir/get_*.sh`
mkdir -p tables.tmp
cd tables.tmp

indx=0
for script in $scriptlist; do
  indx=$((indx+1))
  $script || {
      echo "Error in script $script"
      exit $indx
    }
done

exit
## @endcond
