###############################################################################
# ldmx-denv-init.sh
#   Make sure that the LDMX denv is initialized properly before proceeding.
#
#   This script is desigend to be idempotent: i.e. if the denv is already
#   properly configured, then this script should not have any effect.
###############################################################################

# print each argument on its own line with the first line
# prefixed with "ERROR: ".
_error() {
  printf >&2 "[ldmx-denv-init.sh] \033[1;31mERROR: \033[0m\033[31m%s\n" "$1"
  shift
  while [ "$#" -gt "0" ]; do
    printf >&2 "       %s\n" "$1"
    shift
  done
  printf >&2 "\033[0m"
}

if ! command -v denv &> /dev/null; then
  _error "'denv' is not installed."
  return 1
fi

if ! denv check --quiet; then
  # maybe try loading apptainer via the 'module' command?
  # https://github.com/LDMX-Software/ldmx-sw/issues/1248#issuecomment-1896618339
  denv check
  _error "'denv' unable to find a supported container runner."
  return 2
fi

###############################################################################
# Check if the LDMX denv is initialized. If not, do a default initialization.
###############################################################################
_default_denv_workspace="$(dirname ${BASH_SOURCE[0]} )/../../"
_default_denv_workspace="$(cd "${_default_denv_workspace}" && pwd -P)"
if [ -z "${LDMX_BASE+x}" ]; then
  export LDMX_BASE="${_default_denv_workspace}"
fi
if [[ ! -f ${LDMX_BASE}/.denv/config ]]; then
  denv init --clean-env --name "ldmx" "ldmx/dev:latest" "${LDMX_BASE}"
fi
