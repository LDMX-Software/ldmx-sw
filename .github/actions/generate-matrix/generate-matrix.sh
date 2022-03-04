#!/bin/bash

set -e

###############################################################################
# generate-matrix.sh
#   Generate the matrix for the validation/generation jobs down the line.
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

# convert the input argument array into a json list
__json_list__() {
  local _lst="["
  for e in $@; do
    echo ${e}
    _lst="${_lst}, ${e}"
  done
  _lst="${_lst} ]"  
  echo "${_lst}"
}

__main__() {
  local _num_jobs_per_sample="$1"

  start_group Deduce Samples
  local _samples=($(cd ${LDMX_BASE}/ldmx-sw/.github/validation_samples; find . -mindepth 1 -maxdepth 1 -type d))
  local _json_samples="["
  for s in ${_samples[@]}; do
    if [[ ${_json_samples} == "[" ]]; then
      _json_samples="${_json_samples}\"${s#./}\""
    else
      _json_samples="${_json_samples},\"${s#./}\""
    fi
  done
  _json_samples="${_json_samples}]"
  echo "Full List: ${_json_samples}"
  end_group

  start_group Deduce Runs
  local _runs="["
  for r in $(seq 1 ${_num_jobs_per_sample}); do
    if [[ ${_runs} == "[" ]]; then
      _runs="${_runs}${r}"
    else
      _runs="${_runs},${r}"
    fi
  done
  _runs="${_runs}]"
  echo "Run List: ${_runs}"
  end_group

  local _the_matrix="{\"sample\":${_json_samples},\"run\":${_runs}}"

  set_output job_matrix "${_the_matrix}"

  return 0
}

__main__ $@
