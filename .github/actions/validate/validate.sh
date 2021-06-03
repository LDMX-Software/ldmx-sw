#!/bin/bash

###############################################################################
# validate.sh
#   Run the validate action.
#
#   Assumptions
#     - LDMX_DOCKER_TAG is defined to be image we run inside of
#     - name of sample is given as only argument on command line
#     - ldmx-sw install we should run is installed in $GITHUB_WORKSPACE/install
#       as is done with setup action
###############################################################################

__ldmx_gold_label__() {
  cat ${GITHUB_WORKSPACE}/.github/actions/validate/gold_label
}

__deduce_ldmx_base__() {
  local _ldmx_base="$GITHUB_WORKSPACE/../"
  export LDMX_BASE=$(cd $_ldmx_base && pwd)
}

__deduce_sample_dir__() {
  local _sample="$1"
  export LDMX_SAMPLE_DIR_PATH="${GITHUB_WORKSPACE}/.github/validation_samples/${_sample}"
}

# container running command
#   - Assume LDMX_DOCKER_TAG is the image we run in
#   - Run inside of the sample directory
__docker_run__() {
  docker run \
    -i -v ${LDMX_BASE}:${LDMX_BASE} -e LDMX_BASE \
    -u $(id -u $USER):$(id -g $USER) \
    ${LDMX_DOCKER_TAG} ${LDMX_SAMPLE_DIR_PATH} $@
}

# GitHub workflow command to set an output key,val pair
__set_output__() {
  local _key="$1"
  local _val="$2"
  echo "${_key} = ${_val}"
  echo "::set-output name=${_key}::${_val}"
}

# GitHub workflow command to start an group of output messages
__start_group__() {
  echo "::group::$@"
}

# GitHub workflow command to end previously started group
__end_group__() {
  echo "::endgroup::"
}

__main__() {
  __start_group__ Input Deduction
  local _sample="$1"
  __deduce_ldmx_base__
  __deduce_sample_dir__ $_sample
  echo "Sample Name: ${_sample}"
  echo "LDMX_BASE: ${LDMX_BASE}"
  echo "Sample Dir: ${LDMX_SAMPLE_DIR_PATH}"
  __end_group__

  # assume sample directory has its config called 'config.py'
  __start_group__ Run config.py
  __docker_run__ fire config.py || return $?
  __end_group__

  __start_group__ Compare to Golden Histograms
  # assume sample directory has its gold histogram called 'gold.root'
  #   compare has 4 CLI inputs:
  #    gold_f, gold_label, test_f, test_label
  __docker_run__ python3 $GITHUB_ACTION_PATH/compare.py \
    gold.root $(__ldmx_gold_label__) hist.root ${GITHUB_REF} || return $?

  # compare.py puts plots into the plots/ directory
  #   Package them up for upload
  cd ${LDMX_SAMPLE_DIR_PATH}/plots || return $?
  tar -czf ${_sample}_recon_validation_plots.tar.gz * || return $?
  __end_group__

  # Share paths to plot archive
  __start_group__ Share Paths to Outputs
  __set_output__ plots $(pwd)/${_sample}_recon_validation_plots.tar.gz
  __set_output__ hists ${LDMX_SAMPLE_DIR_PATH}/hist.root
  __set_output__ events ${LDMX_SAMPLE_DIR_PATH}/events.root
  __end_group__
}

__main__ $@
