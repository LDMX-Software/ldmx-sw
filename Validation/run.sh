#!/bin/bash
# run.sh 
#   run four simulations within container

__usage__() {
  cat <<HELP
 USAGE:
  ./run.sh [-o|--out-dir OUT_DIR] [-N|--n-events NUM_EVENTS]

 OPTIONS:
  -o, --out-dir  : Base output directory for data files (default: 'data/<git describe --tags>')
  -N, --n-events : Number of events per geometry to simulate

HELP
}

__main__() {
  if [ -z ${LDMX_BASE} ]; then
    echo "ERROR: Need to be in ldmx environment."
    echo "  source the ldmx-env.sh script in ldmx-sw."
    return 1
  fi
  #local _tag=$(git -C ${LDMX_BASE}/ldmx-sw describe --tags)
  local _output_dir=$(cd data && pwd -P)/dev
  local _n_events=10000
  while [ $# -gt 0 ]; do
    case $1 in
      -o|--out-dir|-N|--n-events)
        if [ -z $2 ]; then
          echo "ERROR: '$1' requires an argument."
          return 1
        fi
        case $1 in
          -o|--out-dir) _output_dir=$2;;
          -N|--n-events) _n_events=$2;;
        esac
        shift
        ;;
      -h|--help|-?)
        __usage__
        return 0
        ;;
      *)
        echo "ERROR: Unrecognized option '$1'."
        return 1
        ;;
    esac
    shift
  done
  
  if ! mkdir -p ${_output_dir}; then
    echo "ERROR: Could not create output directory ${_output_dir}"
    return $?
  fi

  for g in 12 14; do
    fire simulation.py \
      --n-events ${_n_events} \
      --out-dir ${_output_dir} \
      --geometry ${g} \
      &> ${_output_dir}/simulation_v${g}.log &
  done
  wait
}

__main__ $@ 
