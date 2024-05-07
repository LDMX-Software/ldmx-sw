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
# Attempt to deduce LDMX_BASE
# Unfortunately, deducing the path to a script being sourced is not possible
# using POSIX-compliant methods[1], so we need to fallback to separating
# different running scenarios.
#
# [1]: https://stackoverflow.com/a/29835459
#
# Users can avoid all of this complication by pre-defining the LDMX_BASE
# environment variable.
###############################################################################
if [ -z "${LDMX_BASE+x}" ]; then
  _full_path() (
    CDPATH=
    cd -- "${1}" && pwd -P
  )
  _base_from_script() {
    _full_path "$(dirname -- "${1}")/../../" 
  } 
  if [ -n "${ZSH_VERSION}" ]; then
    # zsh defines ZSH_EVAL_CONTEXT holding the details on how
    # this code is being executed
    # disable warnings about undefined variable and string indexing
    # shellcheck disable=SC2154,SC3057
    LDMX_BASE="$(_base_from_script "${ZSH_EVAL_CONTEXT:file}")"
  elif [ -n "${KSH_VERSION}" ]; then
    # Korn defines the _ variable to the the file being sourced
    # disable warning about undefined '_'
    # shellcheck disable=SC3028
    LDMX_BASE="$(_base_from_script "${_}")"
  elif [ -n "${BASH_VERSION}" ]; then
    # running from bash, bash defines the BASH_SOURCE array to help us
    # find the location of this file
    # disable warning about undefined variable and array references
    # shellcheck disable=SC3028,SC3054
    LDMX_BASE="$(_base_from_script "${BASH_SOURCE[0]}")"
  else
    # unable to deduce shell, resort to pwd
    if expr "${PWD}" : ".*ldmx-sw$"; then
      LDMX_BASE="$(_full_path ..)"
    elif expr "${PWD}" : ".*ldmx-sw/scripts$"; then
      LDMX_BASE="$(_full_path ../..)"
    fi
  fi
  unset -f _full_path _base_from_script
fi
# re-export LDMX_BASE in case of user does inline variable definition like
#   LDMX_BASE=/path/to/ldmx/base source /full/path/to/ldmx-denv-init.sh
export LDMX_BASE

###############################################################################
# Check if the LDMX denv is initialized. If not, do a default initialization.
# TODO: update to `denv check --workspace` introduced in denv v0.7.0 to avoid
#       reliance on internal implementation on how denv stores its config
###############################################################################
if ! denv check --workspace --quiet; then
  denv init --clean-env --name "ldmx" "ldmx/dev:latest" "${LDMX_BASE}"
fi
unset -f _error _user_confirm
