#!/bin/bash

set -e

###############################################################################
# check.sh
#   Check the input plots package for any failed comparison plots
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

__main__() {
  local _archive="$1"
  local rc=0

  local _failed_plots=($(tar -tf ${_archive} | grep "fail/.*.pdf"))
  if [[ ${#_failed_plots[@]} -gt 0 ]]; then 
    error ${#_failed_plots[@]} plots failed the KS test against gold.
    start_group List of Plots Failing KS Test
    for p in ${_failed_plots[@]}; do
      echo $(basename ${p})
    done
    end_group
    rc=1
  fi

  # unpack the logs so we can compare them
  tar xzf ${_archive} gold.log output.log

  if ! diff gold.log output.log > log.diff; then
    # do not error out (don't set rc here) if diff is non-zero, the timestamps printed out
    # by some processors prevent a full text diff so we do the character
    # count check below to look for big changes
    warn Text Differences Between Logs
    start_group diff gold.log output.log
    cat log.diff
    end_group
  fi

  # check character count of logs
  ngold=$(wc --chars gold.log | cut -f 1 -d ' ')
  nnew=$(wc --chars output.log | cut -f 1 -d ' ')
  if (( ngold != nnew )); then
    error "Different character counts in logs"
    rc=1
  fi

  return ${rc}
}

__main__ $@
