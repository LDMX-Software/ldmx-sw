###############################################################################
# ldmx-denv-init.sh
#   Make sure that the LDMX denv is initialized properly before proceeding.
#
#   This script is desigend to be idempotent: i.e. if the denv is already
#   properly configured, then this script should not have any effect.
###############################################################################

# print each argument on its own line with the first line
# prefixed with "ERROR [ldmx-denv-init.sh]: ".
_error() {
  printf >&2 "\033[1;31mERROR [ldmx-denv-init.sh]: \033[0m\033[31m%s\n" "$1"
  shift
  while [ "$#" -gt "0" ]; do
    printf >&2 "       %s\n" "$1"
    shift
  done
  printf >&2 "\033[0m"
}

# print the question passed to us and process user input
# continues in infinite loop if user is not explicitly answering yes or no
# Arguments
#  1 - question to ask the user (yes or no)
# Output
#  returns 0 if user answers yes and 1 if user answers no
_user_confirm() {
  question="$*"
  while true; do
    printf "\033[1m%s\033[0m [Y/n] " "${question}"
    read -r ans
    case "${ans}" in
      Y|y|yes)
        return 0
        ;;
      N|n|no)
        return 1
        ;;
      *)
        _error "${ans} is not one of Y, y, yes, N, n, or no."
        ;;
    esac
  done
}

if ! command -v denv > /dev/null 2>&1; then
  _error "'denv' is not installed."
  if _user_confirm "Do you wish to try to install 'denv' now?"; then
    curl -s https://raw.githubusercontent.com/tomeichlersmith/denv/main/install | sh 
  else
    printf "Not attempting to install 'denv'. Please follow the instructions at\n  %s\n" \
      "https://tomeichlersmith.github.io/denv/getting_started.html#installation"
    unset -f _error _user_confirm
    return 1
  fi
fi

if ! denv check --quiet; then
  # maybe try loading apptainer via the 'module' command?
  # https://github.com/LDMX-Software/ldmx-sw/issues/1248#issuecomment-1896618339
  denv check
  _error "'denv' unable to find a supported container runner." \
    "Install one of the container runners 'denv' checked for above."
  unset -f _error _user_confirm
  return 2
fi

###############################################################################
# Check if the LDMX denv is initialized. If not, do a default initialization.
###############################################################################
_default_denv_workspace="$(dirname "${BASH_SOURCE[0]}" )/../../"
_default_denv_workspace="$(cd "${_default_denv_workspace}" && pwd -P)"
if [ -z "${LDMX_BASE+x}" ]; then
  export LDMX_BASE="${_default_denv_workspace}"
fi
if [ ! -f "${LDMX_BASE}/.denv/config" ]; then
  denv init --clean-env --name "ldmx" "ldmx/dev:latest" "${LDMX_BASE}"
fi
unset -f _error _user_confirm
