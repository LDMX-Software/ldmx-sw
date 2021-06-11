#!/bin/bash

set -e

###############################################################################
# update-gold.sh
#   Update any generated histogram files hist.root as the new golden
#   histogram files gold.root and update the label for these new gold.
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

__main__() {
  start_group Update Gold Label
  echo "Old Label: $(ldmx_gold_label)"
  echo "${GITHUB_REF##refs/tags/}" > ${LDMX_GOLD_LABEL_FILE}
  echo "New Label: $(ldmx_gold_label)"
  git add ${LDMX_GOLD_LABEL_FILE}
  end_group

  start_group Copy over new Gold Histograms
  cd .github/pr_validation_samples
  for sample_dir in *; do
    mv ${sample_dir}/hist.root ${sample_dir}/gold.root
    echo "${sample_dir}"
  done
  git add */gold.root
  end_group

  start_group Set Outputs
  set_output new-label $(ldmx_gold_label)
  end_group
}

__main__ $@
