#!/bin/bash

# Last modified 2021-02-16-10-00
# * added pipe fix

# Management path
MP="/mnt/buffalo-management/management"

# Tools path
TP="/mnt/buffalo-management/ica-tools"

# Check if the buffalo has the sde installed in /opt/
SDE_DIR=""

### Special case for the current installation on buffalo1
if [ "X$SDE" == "X/root/bf-sde-9.1.0" ]
then
    rm /root/bf-sde-9.1.0/install/bin/python3
fi

# echo "Find location of latest SDE"
if ls /opt/bf-sde-* 1> /dev/null 2>&1; then

  for sde_dir in `pwd` `/bin/ls -dt /opt/bf-sde-*.*.* 2> /dev/null`; do
    manifest=`/bin/ls $sde_dir/*.manifest 2> /dev/null | head -n 1`
    if [ ! -z $manifest ]; then

        echo ""
        echo "####################"
        echo "Latest SDE is installed in" $sde_dir
        echo "Call: $TP/p4_build.sh -p $1 --with-p4c=$sde_dir/install/bin/bf-p4c -D $2"
        echo "####################"
        echo ""

        # This is a workaround as our management scripts seem to somehow mess up the path/whatever configs so that p4build does not find the p4c by itself... 
        $TP/p4_build.sh -p $1 -D $2 --with-p4c=$sde_dir/install/bin/bf-p4c 
        break
    fi
  done
elif ls /root/bf-sde-* 1> /dev/null 2>&1; then
  for sde_dir in `pwd` `/bin/ls -dt /root/bf-sde-*.*.* 2> /dev/null`; do
    manifest=`/bin/ls $sde_dir/*.manifest 2> /dev/null | head -n 1`
    if [ ! -z $manifest ]; then

        echo ""
        echo "####################"
        echo "Latest SDE is installed in" $sde_dir
        echo "Call: $TP/p4_build.sh -p $1 -D $2"
        echo "####################"
        echo ""

        # Here, everything should work correctly.
        echo "Call: $TP/p4_build.sh -p $1"
        $TP/p4_build.sh -p $1 -D $2  
        break
    fi
  done
fi


### Special case for the current installation on buffalo1
if [ "X$SDE" == "X/root/bf-sde-9.1.0" ]
then
    ln -s /root/bf-sde-9.1.0/build/bf-utils/third-party/bf-python/python /root/bf-sde-9.1.0/install/bin/python3
fi

__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# remove pipes 2 and 3 from the installed conf file

python3 $MP/fix_pipes.py $1
