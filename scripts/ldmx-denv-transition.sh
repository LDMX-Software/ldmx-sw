###############################################################################
# ldmx-denv-transition.sh
#   Replicate some of the `ldmx` commands by simply wrapping the equivalent
#   `denv` commands along with printouts explaining what is being done.
#   so that users know that the same procedures can be done with `denv`.
###############################################################################

# make sure denv is initialized
. "$(dirname "${BASH_SOURCE[0]}")/ldmx-denv-init.sh"

# mimicing the ldmx bash function requires bash, point out user
# doesn't need bash anymore if they use denv directly
if [[ -z ${BASH} ]]; then
  echo "[ldmx-env.sh] [ERROR] You aren't in a bash shell. You are in '$0'."
  [[ "${SHELL}" = *"bash"* ]] || echo "  Your default shell '${SHELL}' isn't bash."
  cat <<\HELP
  If you'd prefer to not use bash, you can use 'denv' directly rather than
  this wrapper script.
  From the ldmx-sw directory, you can setup a denv to use our development image.
  
    denv init ldmx/dev:latest ..

  And then run commands within it by prefixing them with 'denv':

    denv cmake -B build -S .
    denv cmake --build build --target install
    denv fire SimCore/test/basic.py

  Look at 'denv --help' for other denv-specific commands.
HELP
  return 1
fi

# Use the input container and error out if not available
__ldmx_use() {
  local _repo_name="$1"
  local _image_tag="$2"
  cmd="denv config image ldmx/${_repo_name}:${_image_tag}"
  echo "${cmd}"
  ${cmd}
  return $?
}

###############################################################################
# __ldmx_config
#   Print the configuration of the current setup
###############################################################################
__ldmx_config() {
  echo "LDMX base directory: ${LDMX_BASE}"
  echo "uname: $(uname -a)"
  echo "Bash version: ${BASH_VERSION}"
  echo "denv config print"
  denv config print
  return $?
}

###############################################################################
# __ldmx_mount
#   Tell us to mount the passed directory to the container when we run
#   By default, we already mount the LDMX_BASE directory, so none of
#   its subdirectories need to (or should be) specified.
###############################################################################
__ldmx_mount() {
  echo "denv config mounts $*"
  # intentionally re-splitting elements of an array
  #shellcheck disable=SC2068
  denv config mounts $@
  return $?
}

###############################################################################
# __ldmx_setenv
#   Tell us to pass an environment variable to the container when we run
#   By default, we pass the LDMX_BASE and DISPLAY variables explicitly, 
#   because their syntax is too different between docker and singularity,
#   so none of these need to (or should be) specified.
###############################################################################
__ldmx_setenv() {
  echo "denv config env copy $*"
  # intentionally re-splitting elements of an array
  #shellcheck disable=SC2068
  denv config env copy $@
  return $?
}

###############################################################################
# __ldmx_help
#   Print some helpful message to the terminal
###############################################################################
__ldmx_help() {
  cat <<\HELP

  USAGE: 
    ldmx <command> [<argument> ...]

  COMMANDS:
    help    : print this help message and exit
      ldmx help
    config  : Print the current configuration of the container
      ldmx config
    use     : Use the input repo and tag of the container for running
      ldmx use (dev | pro | local) <tag>
    pull    : Pull down the input repo and tag of the container
      ldmx pull (dev | pro | local) <tag>
    mount   : Attach the input directory to the container when running
      ldmx mount <dir>
    setenv   : Set an environment variable in the container when running
      ldmx setenv <environmentVariableName=value>
    <other> : Run the input command in your current directory in the container
      ldmx <other> [<argument> ...]
      ldmx cmake ..
      ldmx make install
      ldmx fire config.py
      ldmx python3 ana.py

HELP
  return 0
}

###############################################################################
# ldmx
#   The root command for users interacting with the ldmx container environment.
#   This function is really just focused on parsing CLI and going to the
#   corresponding subcommand.
#
#   There are lots of subcommands, go to those functions to learn the detail
#   about them.
###############################################################################
ldmx() {
  # if there are no arguments, print the help
  [[ "$#" == "0" ]] && { __ldmx_help; return $?; }
  # divide commands by number of arguments
  cmd="__ldmx_${1}"
  case "${1}" in
    help|config)
      if [[ "$#" != "1" ]]; then
        ${cmd}
        echo "ERROR: 'ldmx ${1}' takes no arguments. (It was ignored.)"
        return 1
      fi
      ${cmd}
      return $?
      ;;
    list|mount|setenv|source)
      if [[ "$#" != "2" ]]; then
        __ldmx_help
        echo "ERROR: ldmx ${1} takes one argument."
        return 1
      fi
      ${cmd} "$2"
      return $?
      ;;
    pull)
      if [[ "$#" != "3" ]]; then
        __ldmx_help
        echo "ERROR: 'ldmx pull' takes two arguments: <repo> <tag>."
        return 1
      fi
      __ldmx_use "$2" "$3"
      echo "denv config image pull"
      denv config image pull
      return $?
      ;;
    use)
      if [[ "$#" != "3" ]]; then
        __ldmx_help
        echo "ERROR: 'ldmx use' takes two arguments: <repo> <tag>."
        return 1
      fi
      __ldmx_use "$2" "$3"
      return $?
      ;;
    *)
      echo "denv $*"
      # intentionally re-splitting elements of an array
      #shellcheck disable=SC2068
      denv $@
      return $?
      ;;
  esac
}

###############################################################################
# DONE WITH NECESSARY PARTS
#   Everything below here is icing on the usability cake.
###############################################################################

###############################################################################
# Bash Tab Completion
#   This next section is focused on setting up the infrastucture for smart
#   tab completion with the ldmx command and its sub-commands.
###############################################################################

###############################################################################
# __ldmx_complete_directory
#   Some of our sub-commands take a directory as input.
#   In these cases, we can pretend to cd and use bash's internal
#   tab-complete functions.
#   
#   All this requires is for us to shift the COMP_WORDS array one to
#   the left so that the bash internal tab-complete functions don't
#   get distracted by our base command 'ldmx' at the front.
#
#   We could allow for the shift to be more than one if there is a deeper
#   tree of commands that need to be allowed in the futre.
###############################################################################
__ldmx_complete_directory() {
  local _num_words="1"
  #shellcheck disable=SC2206
  COMP_WORDS=(${COMP_WORDS[@]:_num_words})
  COMP_CWORD=$((COMP_CWORD - _num_words))
  _cd
}

###############################################################################
# __ldmx_complete_command
#   Tab-complete with a command used commonly inside the container
#
#   Search the install location of ldmx-sw for ldmx-sw executables
#   and include hard-coded common commands. Any strings passed are
#   also included.
#
#   Assumes current argument being tab completed is stored in
#   bash variable 'curr_word'.
###############################################################################
__ldmx_complete_command() {
  # generate up-to-date list of options
  local _options="$* cmake make python3 root rootbrowse"
  for ldmx_executable in "${LDMX_BASE}/ldmx-sw/install/bin"/*; do
    _options="${_options} $(basename "${ldmx_executable}")"
  done

  # match current word (perhaps empty) to the list of options
  #shellcheck disable=SC2207
  COMPREPLY=($(compgen -W "${_options}" "${curr_word}"))
}

###############################################################################
# __ldmx_complete_bash_default
#   Restore the default tab-completion in bash that uses the readline function
#   Bash default tab completion just looks for filenames
###############################################################################
__ldmx_complete_bash_default() {
  compopt -o default
  COMPREPLY=()
}

###############################################################################
# __ldmx_dont_complete
#   Don't tab complete or suggest anything if user <tab>s
###############################################################################
__ldmx_dont_complete() {
  COMPREPLY=()
}

###############################################################################
# Modify the list of completion options on the command line
#   Helpful discussion of this procedure from a blog post
#   https://iridakos.com/programming/2018/03/01/bash-programmable-completion-tutorial
#
#   Helpful Stackoverflow answer
#   https://stackoverflow.com/a/19062943
#
#   COMP_WORDS - bash array of space-separated command line inputs including base command
#   COMP_CWORD - index of current word in argument list
#   COMPREPLY  - options available to user, if only one, auto completed
###############################################################################
__ldmx_complete() {
  # disable readline filename completion
  compopt +o default

  local curr_word="${COMP_WORDS[${COMP_CWORD}]}"

  if [[ "${COMP_CWORD}" = "1" ]]; then
    # tab completing a main argument
    __ldmx_complete_command "config pull use run mount setenv"
  elif [[ "${COMP_CWORD}" = "2" ]]; then
    # tab complete a sub-argument,
    #   depends on the main argument
    case "${COMP_WORDS[1]}" in
      config|setenv)
        # no more arguments
        __ldmx_dont_complete
        ;;
      pull|use)
        # container repositories after these commands
        #shellcheck disable=SC2207
        COMPREPLY=($(compgen -W "dev pro" "${_curr_word}"))
        ;;
      run|mount)
        #directories only after these commands
        __ldmx_complete_directory
        ;;
      *)
        # files like normal tab complete after everything else
        __ldmx_complete_bash_default
        ;;
    esac
  else
    # three or more arguments
    #   check base argument to see if we should continue
    case "${COMP_WORDS[1]}" in
      config|pull|use|mount|setenv)
        # these commands shouldn't have tab complete for the third argument 
        #   (or shouldn't have the third argument at all)
        __ldmx_dont_complete
        ;;
      run)
        if [[ "${COMP_CWORD}" = "3" ]]; then
          # third argument to run should be an inside-container command
          __ldmx_complete_command
        else
          # later arguments to run should be bash default
          __ldmx_complete_bash_default
        fi
        ;;
      *)
        # everything else has bash default (filenames)
        __ldmx_complete_bash_default
        ;;
    esac
  fi
}

# Tell bash the tab-complete options for our main function ldmx
complete -F __ldmx_complete ldmx
