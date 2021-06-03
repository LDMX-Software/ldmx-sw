#!/bin/bash

set -e

###############################################################################
# ldmx.sh
#   Run a command inside the container
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

__main__() {
  start_group Deduce Inputs
  local _dir="$1"
  local _cmd="${@:2}"
  echo "dir=${_dir}"
  echo "cmd=${_cmd}"
  end_group

  start_group ldmx ${_cmd}
  cd ${LDMX_BASE}/${_dir}`
  ldmx ${_cmd}
  end_group
}

__main__ $@
