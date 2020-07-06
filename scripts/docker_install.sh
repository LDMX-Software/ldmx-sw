#!/bin/bash

set -e

###############################################################################
# docker_install.sh
#   This bash script is meant to run as the installation script for ldmx-sw
#   while building the production docker image.
###############################################################################

# setup environment within container
source $G4DIR/bin/geant4.sh
source $ROOTDIR/bin/thisroot.sh

# go to where the Dockerfile put the code
cd /code

# make and enter a build directory
mkdir build
cd build

# configure the build
#   install it to a path already in PATH
cmake \
    -DXercesC_DIR=$XercesC_DIR \
    -DONNXRUNTIME_ROOT=$ONNX_DIR \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    ..

# build and install
make install

cd .. #back out of build directory
cd .. #back out of code directory
rm -rf code #delete all that nonsense

