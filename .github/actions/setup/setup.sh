#!/bin/bash

set -e

###############################################################################
# setup.sh
#   Run the setup action.
#
#   Assumptions
#     - LDMX_DOCKER_TAG is defined to the image that should be used to compile
#     - ldmx-sw is already checked out to the branch/tag to compile
#     - $GITHUB_WORKSPACE is the ldmx-sw directory (default)
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

__main__() {
  __start_group__ Configure the Build
  local _build=${LDMX_BASE}/ldmx-sw/build
  mkdir ${_build}
  __docker_run__ ${_build} cmake .. || return $?
  __end_group__

  __start_group__ Build and Install
  __docker_run__ ${_build} make install || return $?
  __end_group__

  return 0
}

__main__ $@
