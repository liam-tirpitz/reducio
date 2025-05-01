#!/bin/bash

DIRECTORY=$(cd `dirname $0` && pwd)
export PATH=$SDE_INSTALL/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/lib:$SDE_INSTALL/lib:$LD_LIBRARY_PATH

#sudo -E env "PATH=$PATH" "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" "$DIRECTORY/build/tofino_switch_control" -m combinedcluster -c config
sudo -E env "PATH=$PATH" "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" "$DIRECTORY/build/tofino_switch_control" $1 $2

#params: 
# we essentially only have one usable param currently: mode mode