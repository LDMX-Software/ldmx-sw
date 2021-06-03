#!/bin/bash

set -e

###############################################################################
# ldmx.sh
#   Run a command inside the container
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

__main__() {
  __start_group__ Deduce Inputs
  local _dir="$1"
  local _cmd="${@:2}"
  echo "dir=${_dir}"
  echo "cmd=${_cmd}"
  __end_group__

  __start_group__ ldmx ${_cmd}
  __docker_run__ ${_dir} ${_cmd}
  __end_group__
}

__main__ $@
