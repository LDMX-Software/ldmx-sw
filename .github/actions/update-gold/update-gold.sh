#!/bin/bash

set -e

###############################################################################
# update-gold.sh
#   Update any generated histogram files hist.root as the new golden
#   histogram files gold.root and update the label for these new gold.
###############################################################################

source ${GITHUB_ACTION_PATH}/../common.sh

__main__() {
  start_group Deduce Inputs
  echo "Samples: $@"
  end_group

  start_group Update Gold Label
  echo "Old Label: $(ldmx_gold_label)"
  echo "${GITHUB_REF##refs/tags/}" > ${LDMX_GOLD_LABEL_FILE}
  echo "New Label: $(ldmx_gold_label)"
  git add ${LDMX_GOLD_LABEL_FILE}
  end_group

  start_group Copy over new Gold Histograms
  for sample in $@; do
    echo ${sample}
    mv ../${sample}-new-gold/hist.root .github/validation_samples/${sample}/gold.root
  done
  git add .github/validation_samples/*/gold.root
  end_group

  start_group Set Outputs
  set_output new-label $(ldmx_gold_label)
  end_group
}

__main__ $@
