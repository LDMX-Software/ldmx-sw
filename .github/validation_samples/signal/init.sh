#!/bin/bash

###############################################################################
# init.sh
#   Pre-validation initializing for dark brem signal samples
#
#   We need to produce the dark brem event library.
###############################################################################

start_group Produce Dark Brem Library
curl -s https://tomeichlersmith.github.io/denv/install | sh
# use docker so we don't have to tell podman to look at the docker.io registry
export DENV_RUNNER=docker
denv init ldmx/dark-brem-lib-gen:v5.1.0
denv dark-brem-lib-gen \
  --run 1 \
  --max-energy 8.0 \
  --apmass 0.01
# cleanup denv
rm -r .denv
end_group
