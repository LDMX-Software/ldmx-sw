############################# Usage #############################
# source scripts/ldmx-compile.sh.sh [-j <numCores>] [-b <buildLocation>]
# e.g. source scripts/ldmx-compile.sh
# or   source scripts/ldmx-compile.sh -j 16 -b ${LDMX_BASE}/ldmx-sw

# set number of cores to be used, build location
while [[ $# -gt 0 ]]; do
  # number of cores
  if [[ $1 == "-j"* ]]; then
      if [[ $1 =~ ^-j\s*[=]?\s*([0-9]+)$ ]]; then
          CORES=${BASH_REMATCH[1]}
      fi
  else
    CORES=$(nproc)
  fi
  # build directory
  if [[ $1 == "-b"* ]]; then
      if [[ $1 =~ ^-b\s*[=]?\s*([a-zA-Z0-9]+)$ ]]; then
          BUILD=${BASH_REMATCH[1]}
      fi
  else
      BUILD=${LDMX_BASE}/ldmx-sw
  fi
  shift
done

echo "-- Compiling ldmx-sw in ${BUILD} with ${CORES} cores"

# Compile ldmx-sw
cd ${BUILD}
ldmx cmake -B ${BUILD}/build -S .
ldmx cmake --build ${BUILD}/build --target install -j=$CORES
cd -
