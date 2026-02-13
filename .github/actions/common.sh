#!/bin/bash

###############################################################################
# common.sh
#   Shared bash functions between many different actions.
#
#   Assumptions
#     - The env variable LDMX_DOCKER_TAG has the image we should be using
#     - The label for the gold plots is in ci-data/label
#     - ldmx-sw has been checked out using actions/checkout@v2
#       OR the build package was unpacked to mimic this effect
#       (this assumption basically means that GITHUB_WORKSPACE points to ldmx-sw)
#
#   Usage
#     GitHub defines GITHUB_ACTION_PATH to be the full path to the diretory
#     for the action that is being run. This means, since this file is one
#     below that directory, we can source it like so
#       source ${GITHUB_ACTION_PATH}/../common.sh
#     https://docs.github.com/en/actions/reference/environment-variables
###############################################################################

# GitHub workflow command to set an output key,val pair
set_output() {
  local _key="$1"
  local _val="$2"
  echo "${_key} = ${_val}"
  echo "${_key}=${_val}" >> $GITHUB_OUTPUT
}

# GitHub workflow command to start an group of output messages
start_group() {
  echo "::group::$@"
}

# GitHub workflow command to end previously started group
end_group() {
  echo "::endgroup::"
}

# GitHub workflow command to flag message as an error
error() {
  echo "::error::$@"
}

warn() {
  echo "::warning::$@"
}

start_group Deduce Common Environment Variables
export LDMX_BASE=$(cd ${GITHUB_WORKSPACE}/../ && pwd)
export CI_DATA=$(cd ${GITHUB_WORKSPACE}/ci-data && pwd)
echo "LDMX_BASE=${LDMX_BASE}"
echo "GITHUB_WORKSPACE=${GITHUB_WORKSPACE}"
echo "CI_DATA=${CI_DATA}"
end_group

