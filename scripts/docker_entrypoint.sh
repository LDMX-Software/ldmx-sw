#!/bin/bash

set -e

###############################################################################
# Entry point for the ldmx production container
#   The basic idea is that we want to go into the container,
#   setup the ldmx-sw working environment, and then
#   run the application with any provided arguments.
#
#   We mount and run inside the present working directory.
###############################################################################

## Bash environment script for use **within** the docker container
## Assuming the following environment variables are already defined by Dockerfile:
#   XercesC_DIR      - install of xerces-c
#   ONNX_DIR         - install of onnx runtime
#   ROOTDIR          - install of root
#   G4DIR            - install of Geant4

source $ROOTSYS/bin/thisroot.sh
source $G4DIR/bin/geant4.sh

# add ldmx-sw and ldmx-analysis installs to the various paths
export LDMX_SW_INSTALL=/usr/local/
export LD_LIBRARY_PATH=$LDMX_SW_INSTALL/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$LDMX_SW_INSTALL/lib/python:$PYTHONPATH
export PATH=$LDMX_SW_INSTALL/bin:$PATH

# go to first argument
cd "$1"

# run the rest of the arguments depending on the command
eval "${@:2}"
