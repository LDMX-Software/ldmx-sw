#!/bin/bash

###############################################################################
# init.sh
#   Pre-validation initializing for dark brem signal samples
#
#   We need to produce the dark brem event library.
###############################################################################

start_group Produce Dark Brem Library
denv init ldmx/dark-brem-lib-gen:v5.1
denv dark-brem-lib-gen \
  --run 1 \
  --max_energy 8.0 \
  --apmass 0.01
# cleanup denv
rm -r .denv
end_group
