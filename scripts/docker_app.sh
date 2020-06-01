#!/bin/bash

set -e

###############################################################################
# Entry point for the ldmx production container
#   The basic idea is that we want to go into the container,
#   setup the ldmx-sw working environment, and then
#   run ldmx-app with any provided arguments.
#
#   We mount and run inside the present working directory.
###############################################################################

## Bash environment script for use **within** the docker container
## Assuming the following environment variables are already defined by Dockerfile:
#   XercesC_DIR      - install of xerces-c
#   ONNX_DIR         - install of onnx runtime
#   ROOTDIR          - install of root
#   G4DIR            - install of Geant4
#   LDMX_BASE        - base directory where all ldmx-sw/ldmx-analysis code is

source $ROOTDIR/bin/thisroot.sh
source $G4DIR/bin/geant4.sh

# add ldmx-sw and ldmx-analysis installs to the various paths
export LD_LIBRARY_PATH=$ONNX_DIR/lib:/usr/local/lib:$LD_LIBRARY_PATH
export PYTHONPATH=/usr/local/lib/python:$PYTHONPATH
export PATH=/usr/local/bin:$PATH

# helps simplify any cmake nonsense
export CMAKE_PREFIX_PATH=$XercesC_DIR:$ROOTDIR:$G4DIR:$ONNX_DIR:/usr/local

# go to first argument
cd "$1"

# execute the rest as a one-liner command
app "${@:2}"

