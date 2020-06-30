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

# go to source directory
cd /code

mkdir build
cd build

# configure the build
#   leave install prefix undefined so it ends up in PATH naturally
cmake \
    -DXercesC_DIR=$XercesC_DIR \
    -DONNXRUNTIME_ROOT=$ONNX_DIR \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    ..

# build and install
make install

