
###############################################################################
# physics_validation.sh
#   Compare the current compiled version of ldmx-sw against the provided
#   production image.  This is a very specific script meant to be used by 
#   the GitHub Action, so only tamper with it if you know what you are doing!
###############################################################################

# Deduce where LDMX_BASE is
#   This is only here to aid in users wanting to run physics_validation locally
__deduce_ldmx_base() {
  _pwd="$(pwd)"
  export LDMX_BASE=${_pwd%%ldmx-sw/*}
  echo $LDMX_BASE
}

# Deduce label for these developments
#  GitHub actions define the GITHUB_REF to be the branch/tag being pushed to 
__deduce_test_label() {
  if [[ -z "${GITHUB_REF}" ]]; then
    echo "$(git rev-parse --abbrev-ref HEAD)"
  else
    # get branch or tag name by removing prefix
    local _branch=${GITHUB_REF#refs/heads/}
    _branch=${_branch#refs/tags/}
    echo ${_branch}
  fi
}

# Deduce the label for the golden files
__deduce_gold_label() {
  local __gold_label_file="gold/label"
  if [[ -f ${__gold_label_file} ]]; then
    cat ${__gold_label_file}
  else
    echo "gold"
  fi
}

# Our custom docker run command **specificially for this script**
#   We mount only the LDMX_BASE directory and run inside the current directory
#   i.e. We assume we are running inside LDMX_BASE.
__docker_run() {
  local _container="$1"
  local _args="${@:2}"
  docker run --rm -i -v ${LDMX_BASE}:${LDMX_BASE} \
    -e LDMX_BASE -u $(id -u ${USER}):$(id -g ${USER}) \
    ${_container} $(pwd) ${_args}
  return $?
}

# Full Main
#   0. Get CLI parameters (none atm)
#   1. Go to the directory this script is in
#   2. Make sure we have the containers we need
#   3. Run through different sample IDs and compare them
__main() {
  local _old_pwd=$OLDPWD
  cd $(dirname ${BASH_SOURCE[0]})

  __deduce_ldmx_base
  __docker_run ldmx/dev:latest python3 physics.py val \
    -t $(__deduce_test_label) -g $(__deduce_gold_label) || return $?

  cd - &> /dev/null
  export OLDPWD=${_old_pwd}

  return 0
}

__main $@
