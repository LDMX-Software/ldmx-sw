############################# Usage #############################
# source scripts/ldmx-recompileAndFire.sh <config>.py [-j <numCores>] [-b <buildLocation>]
# e.g. source scripts/ldmx-recompileAndFire.sh config.py
# or   source scripts/ldmx-recompileAndFire.sh config.py -j 16 -b ${LDMX_BASE}/ldmx-sw

# The input to fire has to be the first argument
FIREINPUT=$1
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

echo "-- Compiling ldmx-sw in ${BUILD} with ${CORES} cores, then running $FIREINPUT"

# Compile ldmx-sw
cd ${BUILD}
ldmx cmake -B ${BUILD}/build -S .
ldmx cmake --build ${BUILD}/build --target install -j=$CORES
# Run fire on the input config
cd -
ldmx fire $FIREINPUT
