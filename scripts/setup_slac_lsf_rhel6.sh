#!/bin/sh

##################################################################
# Script for setting up necessary environment to run on SLAC LSF #
##################################################################

# Change this to point to whatever copy of ldmx-sw you want to use or make a matching symlink.
# It should have been built for RHEL6 64-bit or it will not run on LSF.
# You should make the installation directory the root checkout dir so it finds bin/, scripts/, etc..
export LDMXSW_DIR=~/ldmx-sw/

# Comment out if you want to get job emails.
LSB_JOB_REPORT_MAIL=N

# Setup necessary runtime libraries.
. /nfs/slac/g/ldmx/software/gcc/rhel6-64/setup.sh
. /nfs/slac/g/ldmx/software/xerces/rhel6-64/setup.sh
. /nfs/slac/g/ldmx/software/python/rhel6-64/setup.sh

# Source the setup script for ldmx-sw which will also setup ROOT and Geant4.
. $LDMXSW_DIR/bin/ldmx-setup-env.sh

# Add scripts dir to path so python scripts can be found without abs paths.
export PATH=$LDMXSW_DIR/scripts:$PATH
