#!/bin/bash

###############################################################################
# setup.sh
#   Run the setup action.
#
#   Assumptions
#     - LDMX_DOCKER_TAG is defined to the image that should be used to compile
#     - ldmx-sw is already checked out to the branch/tag to compile
#     - $GITHUB_WORKSPACE is the ldmx-sw directory (default)
###############################################################################

# docker run command
#   Assume LDMX_BASE is already defined
#   Run inside of the build directory
__docker_run__() {
  docker run \
    -i -v ${LDMX_BASE}:${LDMX_BASE} -e LDMX_BASE \
    -u $(id -u $USER):$(id -g $USER) ${LDMX_DOCKER_TAG} \
    ${LDMX_BASE}/ldmx-sw/build $@
  return $?
}

# GitHub workflow command to start a group in the logging output
__start_group__() {
  echo "::group::$@"
}

# GitHub workflow command to end group previously started
__end_group__() {
  echo "::endgroup::"
}

__main__() {

  __start_group__ Deduce LDMX_BASE
  export LDMX_BASE=$(cd ${GITHUB_WORKSPACE}/../ && pwd)
  echo "LDMX_BASE=${LDMX_BASE}"
  __end_group__

  __start_group__ Configure the Build
  mkdir ${LDMX_BASE}/ldmx-sw/build
  __docker_run__ cmake .. || return $?
  __end_group__

  __start_group__ Build and Install
  __docker_run__ make install || return $?
  __end_group__

  return 0
}

__main__ $@
