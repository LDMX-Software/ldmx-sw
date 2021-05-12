
###############################################################################
# comp.sh
#   Compare the current compiled version of ldmx-sw against the latest push
#   of trunk. This is a very specific script meant to be used by the GitHub
#   Action, so only tamper with it if you know what you are doing!
###############################################################################

export LDMX_BASE=$(cd .. && pwd)

# Deduce label for these developments
#  GitHub actions define the GITHUB_REF to be the branch/tag being pushed to 
__deduce_label() {
  if [[ -z "${GITHUB_REF}" ]]; then
    echo "$(git rev-parse --abbrev-ref HEAD)"
  else
    # get branch or tag name by removing prefix
    local _branch=${GITHUB_REF#refs/heads/}
    _branch=${_branch#refs/tags/}
    echo ${_branch}
  fi
}

# Our custom docker run command **specificially for this script**
#   We mount the current directory and run inside of it
#   The container tag is also given as an argument to the script
#   to label the output file.
__docker_run() {
  local _container="$1"
  local _args="${@:2}"
  docker run --rm -i -v ${LDMX_BASE}:${LDMX_BASE} \
    -e LDMX_BASE -u $(id -u ${USER}):$(id -g ${USER}) \
    ${_container} $(pwd) ${_args}
  return $?
}

# Compare the event trees of the two input container tags
#   and the input event file post-fix (defined in configs).
__compare() {
  local _trunk="$1"
  local _dev="$2"
  local _sample_id="$3"
  __docker_run ${_trunk} fire "${_sample_id}.py" ${_trunk} || return $?
  __docker_run ldmx/dev:latest fire "${_sample_id}.py" ${_dev} || return $?
  __docker_run ldmx/dev:latest python3 compare.py ${_trunk} ${_dev} ${_sample_id} || return $?
  return $?
}

# Full Main
#   0. Get CLI parameters (none at the moment)
#   1. Go to the directory this script is in
#   2. Make sure we have both containers we need
#   3. Run through different sample IDs and compare them
__main() {
  local _dev="$(__deduce_label)"
  local _trunk=ldmx/pro:v2.3.0

  local _old_pwd=$OLDPWD
  cd $(dirname ${BASH_SOURCE[0]})

  # Make sure we have both containers locally
  docker pull ${_trunk} || return $?
  
  for sample_id in inclusive; do
    __compare ${_trunk} ${_dev} ${sample_id} || return $?
  done

  cd - &> /dev/null
  export OLDPWD=${_old_pwd}

  return 0
}

__main $@
