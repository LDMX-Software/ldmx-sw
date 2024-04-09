#!/bin/bash

###############################################################################
# init.sh
#   Pre-validation initializing for dark brem signal samples
#
#   We need to produce the dark brem event library.
###############################################################################

start_group Produce Dark Brem Library
wget https://raw.githubusercontent.com/LDMX-Software/dark-brem-lib-gen/main/env.sh
source env.sh
# commented out lines are dbgen's defaults for reference
#dbgen use latest
#dbgen cache ${HOME} <- only matters for apptainer/singularity
#dbgen work /tmp
#dbgen dest $PWD
mkdir scratch
dbgen work scratch
dbgen run \
  --run 1 \
  --max_energy 8.0 \
  --apmass 0.01
end_group
