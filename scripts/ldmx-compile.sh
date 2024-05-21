#!/bin/bash

############################# Usage #############################
# ldmx ./scripts/ldmx-compile.sh
# for changing the defaults
# ldmx setenv LDMX_COMPILE_CORES=<num cores to be used>, i.e.
# ldmx setenv LDMX_COMPILE_CORES=16
# or
# ldmx setenv LDMX_COMPILE_BUILD=<build location>

if [[ -z "${LDMX_COMPILE_CORES}" ]]; then
    LDMX_COMPILE_CORES=$(nproc)
fi

if [[ -z "${LDMX_COMPILE_BUILD}" ]]; then
    if [[ -z "${LDMX_BASE}" ]]; then
        echo "ERROR: LDMX_BASE is undefined." \
          " This script should be run within the containerized environment."
        exit 1
    fi
    LDMX_COMPILE_BUILD="${LDMX_BASE}/ldmx-sw"
fi

echo "-- Compiling ldmx-sw in ${LDMX_COMPILE_BUILD} with ${LDMX_COMPILE_CORES} cores"

# Compile ldmx-sw
# any arguments are passed to cmake to configure the build
# shellcheck disable=SC2068
cmake -B "${LDMX_COMPILE_BUILD}/build" -S "${LDMX_COMPILE_BUILD}" $@
cmake --build "${LDMX_COMPILE_BUILD}/build" --target install -j="${LDMX_COMPILE_CORES}"
