#!/bin/bash

set -e

###############################################################################
# validate.sh
#   Run the validate action.
#
#   Assumptions
#     - see assumptions of 'common.sh' environment script
#     - name of sample is given as only argument on command line
#     - ldmx-sw install we should run is installed in $GITHUB_WORKSPACE/install
#       as is done with setup action
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

__main__() {
  __start_group__ Input Deduction
  local _sample="$1"
  local _sample_dir="${GITHUB_WORKSPACE}/.github/validation_samples/${_sample}"
  echo "Sample Name: ${_sample}"
  echo "Sample Dir: ${_sample_dir}"
  __end_group__

  # assume sample directory has its config called 'config.py'
  __start_group__ Run config.py
  __docker_run__ ${_sample_dir} fire config.py || return $?
  __end_group__

  __start_group__ Compare to Golden Histograms
  # assume sample directory has its gold histogram called 'gold.root'
  #   compare has 4 CLI inputs:
  #    gold_f, gold_label, test_f, test_label
  __docker_run__ ${_sample_dir} python3 $GITHUB_ACTION_PATH/compare.py \
    gold.root $(__ldmx_gold_label__) hist.root ${GITHUB_REF} || return $?

  # compare.py puts plots into the plots/ directory
  #   Package them up for upload
  cd ${_sample_dir}/plots || return $?
  tar -czf ${_sample}_recon_validation_plots.tar.gz * || return $?
  __end_group__

  # Share paths to plot archive
  __start_group__ Share Paths to Outputs
  __set_output__ plots $(pwd)/${_sample}_recon_validation_plots.tar.gz
  __set_output__ hists ${_sample_dir}/hist.root
  __set_output__ events ${_sample_dir}/events.root
  __end_group__
}

__main__ $@
