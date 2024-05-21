#!/bin/bash

############################# Usage #############################
# ldmx ./scripts/ldmx-recompileAndFire.sh
# for changing the defaults
# ldmx setenv LDMX_COMPILE_CORES=<num cores to be used>, i.e.
# ldmx setenv LDMX_COMPILE_CORES=16
# or
# ldmx setenv LDMX_COMPILE_BUILD=<build location>

if [[ -z "${LDMX_COMPILE_BUILD}" ]]; then
    if [[ -z "${LDMX_BASE}" ]]; then
        echo "ERROR: LDMX_BASE is undefined." \
          " This script should be run within the containerized environment."
        exit 1
    fi
    LDMX_COMPILE_BUILD="${LDMX_BASE}/ldmx-sw"
fi

# compile with default arguments
"${LDMX_COMPILE_BUILD}"/scripts/ldmx-compile.sh
# Run fire on the input config
# shellcheck disable=SC2068
fire $@
