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
  start_group Input Deduction
  local _sample="$1"
  local _no_comp="$2"
  cd ${GITHUB_WORKSPACE}/.github/validation_samples/${_sample} || return $?
  local _sample_dir="$(pwd)"
  echo "Sample Name: ${_sample}"
  echo "Sample Dir: ${_sample_dir}"
  echo "Not Running Comparison? ${_no_comp}"
  end_group

  start_group Sample-Specific Initialization
  if [[ -f init.sh ]]; then
    . init.sh
  else
    echo "No 'init.sh' file in ${_sample_dir}."
  fi
  end_group

  # assume sample directory has its config called 'config.py'
  start_group Run config.py
  ldmx fire config.py || return $?
  end_group

  start_group Compare to Golden Histograms
  if [[ "${_no_comp}" == "false" ]]; then
    # assume sample directory has its gold histogram called 'gold.root'
    #   compare has 4 CLI inputs:
    #    gold_f, gold_label, test_f, test_label
    ldmx python3 $GITHUB_ACTION_PATH/compare.py \
      gold.root $(ldmx_gold_label) hist.root ${GITHUB_REF} || return $?

    # compare.py puts plots into the plots/ directory
    #   Package them up for upload
    cd ${_sample_dir}/plots || return $?
    tar -czf ${_sample}_recon_validation_plots.tar.gz * || return $?
  else
    echo "Not running comparison script."
  fi
  end_group

  # Share paths to plot archive
  start_group Share Paths to Outputs
  if [[ "${_no_comp}" == "false" ]]; then
    set_output plots $(pwd)/${_sample}_recon_validation_plots.tar.gz
  fi
  set_output hists ${_sample_dir}/hist.root
  set_output events ${_sample_dir}/events.root
  end_group
}

__main__ $@
