#!/bin/bash

# source common software setup script
. /nfs/slac/g/ldmx/software/setup.sh

# cmake config
cmake -DGeant4_DIR=$G4DIR -DROOT_DIR=$ROOTDIR -DXercesC_DIR=$XercesC_DIR -DPYTHON_EXECUTABLE=`which python` -DPYTHON_INCLUDE_DIR=/nfs/slac/g/ldmx/software/anaconda/include/python2.7 -DPYTHON_LIBRARY=/nfs/slac/g/ldmx/software/anaconda/lib/libpython2.7.so -DCMAKE_CXX_COMPILER=/nfs/slac/g/ldmx/software/anaconda/bin/g++ -DCMAKE_C_COMPILER=/nfs/slac/g/ldmx/software/anaconda/bin/gcc -DCMAKE_INSTALL_PREFIX=../install ..
