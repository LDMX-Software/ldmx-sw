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

  # use sed replace (by blank) to run the diff without the initial HH:MM:SS timestamp
  if ! diff  -I '^#' <(sed -e 's/^[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}/ /g' gold.log) <(sed -e 's/^[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}/ /g' output.log) > log.diff; then
    # do not error out (don't set rc here) if diff is non-zero, the timestamps printed out
    # by some processors prevent a full text diff so we do the character
    # count check below to look for big changes
    warn Text Differences Between Logs
    start_group diff gold.log output.log
    cat log.diff
    end_group
  fi

  # check character count of logs, allowing up to 0.5% difference to avoid
  # false positives from minor run-to-run output variations
  ngold=$(wc --chars gold.log | cut -f 1 -d ' ')
  nnew=$(wc --chars output.log | cut -f 1 -d ' ')
  if (( ngold != nnew )); then
    if (( ngold > 0 )); then
      char_diff_pct=$(( (nnew - ngold) * 100 / ngold ))
      char_abs_pct=${char_diff_pct#-}
      if (( char_abs_pct > 0.5 )); then
        error "Log character count differs by ${char_abs_pct}%: gold=${ngold}, new=${nnew}"
        rc=1
      else
        warn "Log character count differs by ${char_abs_pct}% (gold=${ngold}, new=${nnew}); within tolerance"
      fi
    fi
  fi

  # compare total wall-clock run time if gold.time is available in the archive
  # gold.time has one line per sample: "<sample> <seconds>"
  # timing.txt has one line for this job: "<sample> <seconds>"
  local _timing_tolerance=10
  if tar -tf ${_archive} gold.time timing.txt &>/dev/null 2>&1; then
    tar xzf ${_archive} gold.time timing.txt
    local _sample new_s gold_s
    read _sample new_s < timing.txt
    gold_s=$(awk -v s="${_sample}" '$1 == s {print $2}' gold.time)
    if [[ -z "${gold_s}" ]]; then
      warn "No gold timing entry for sample '${_sample}'; skipping timing check."
    else
      echo "Timing for ${_sample}: gold=${gold_s}s, new=${new_s}s"
      if (( gold_s > 0 )); then
        local timing_diff_pct timing_abs_pct
        timing_diff_pct=$(( (new_s - gold_s) * 100 / gold_s ))
        timing_abs_pct=${timing_diff_pct#-}
        if (( timing_abs_pct > _timing_tolerance )); then
          if (( new_s > gold_s )); then
            warn "Timing regression for ${_sample}: new=${new_s}s vs gold=${gold_s}s (+${timing_diff_pct}%, tolerance ${_timing_tolerance}%)"
          else
            warn "Timing anomaly for ${_sample}: new=${new_s}s vs gold=${gold_s}s (${timing_diff_pct}%, tolerance ${_timing_tolerance}%)"
          fi
        else
          echo "Timing within ${timing_abs_pct}% of gold (tolerance ${_timing_tolerance}%)"
        fi
      fi
    fi
  else
    warn "No gold.time found; skipping timing check. Re-generate gold data to enable timing comparisons."
  fi

  return ${rc}
}

__main__ $@
