
###############################################################################
# check.sh
#   Check the input plots package for any failed comparison plots
###############################################################################

__main__() {
  local _archive="$1"

  local _failed_plots=($(tar -tf ${_archive} | grep "fail/.*.pdf"))
  if [[ ${#_failed_plots[@]} -gt 0 ]]; then 
    echo "::error ::${#_failed_plots[@]} plots failed the KS test against gold."
    echo "::group::List of Plots Failing KS Test"
    for p in ${_failed_plots[@]}; do
      echo $(basename ${p})
    done
    echo "::endgroup::"
    return 1
  fi

  return 0
}

__main__ $@
