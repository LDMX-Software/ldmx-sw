#!/bin/bash

###############################################################################
# run.sh
#   Run the validate action.
###############################################################################

__deduce_ldmx_base__() {
  local _ldmx_base="$GITHUB_WORKSPACE/../"
  export LDMX_BASE=$(cd $_ldmx_base && pwd)
}

__deduce_sample_dir__() {
  local _sample="$1"
  export LDMX_SAMPLE_DIR_PATH="${GITHUB_WORKSPACE}/.github/validation_samples/${_sample}"
}

__docker_run__() {
  docker run \
    -i -v ${LDMX_BASE}:${LDMX_BASE} -e LDMX_BASE \
    -u $(id -u $USER):$(id -g $USER) \
    ${LDMX_DOCKER_IMAGE} ${LDMX_SAMPLE_DIR_PATH} $@
}

__set_output__() {
  local _key="$1"
  local _val="$2"
  echo "::set-output name=${_key}::${_val}"
}

__main__() {
  local _sample="$1"
  export LDMX_DOCKER_IMAGE="$2"
  
  __deduce_ldmx_base__
  __deduce_sample_dir__ $_sample

  # assume sample directory has its config called 'config.py'
  __docker_run__ fire config.py || return $?

  # assume sample directory has its gold histogram called 'gold.root'
  #   compare has 4 CLI inputs:
  #    gold_f, gold_label, test_f, test_label
  __docker_run__ python3 $GITHUB_ACTION_PATH/compare.py \
    gold.root 'gold' hist.root ${GITHUB_REF} || return $?

  # compare.py puts plots into the plots/ directory
  #   Package them up for upload
  cd ${LDMX_SAMPLE_DIR_PATH}/plots || return $?
  tar -czf ${_sample}_recon_validation_plots.tar.gz * || return $?

  # Share paths to plot archive
  __set_output__ plots $(pwd)/${_sample}_recon_validation_plots.tar.gz
  __set_output__ hists ${LDMX_SAMPLE_DIR_PATH}/hist.root
  __set_output__ events ${LDMX_SAMPLE_DIR_PATH}/events.root
}

__main__ $@
