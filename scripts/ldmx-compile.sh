#!/bin/bash

############################# Usage #############################
# ldmx ./scripts/ldmx-compile.sh
# for changing the defaults
# ldmx setenv LDMX_COMPILE_CORES=<num cores to be used>, i.e.
# ldmx setenv LDMX_COMPILE_CORES=16
# or
# ldmx setenv LDMX_COMPILE_BUILD=<build location>

if [ -z "$LDMX_COMPILE_CORES" ]; then
    LDMX_COMPILE_CORES=$(nproc)
fi

if [ -z "$LDMX_COMPILE_BUILD" ]; then
    LDMX_COMPILE_BUILD=${LDMX_BASE}/ldmx-sw/
fi

echo "-- Compiling ldmx-sw in ${LDMX_COMPILE_BUILD} with ${LDMX_COMPILE_CORES} cores"

# Compile ldmx-sw
cmake -B ${LDMX_COMPILE_BUILD}/build -S ${LDMX_COMPILE_BUILD}
cmake --build ${LDMX_COMPILE_BUILD}/build --target install -j=${LDMX_COMPILE_CORES}
