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
  start_group Configure the Build
  local _build=${LDMX_BASE}/ldmx-sw/build
  mkdir ${_build}
  cd ${_build}
  ldmx cmake .. || return $?
  end_group

  start_group Build and Install
  ldmx make install || return $?
  end_group

  return 0
}

__main__ $@
