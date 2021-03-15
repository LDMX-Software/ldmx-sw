#!/bin/bash

###############################################################################
# ldmx-env.sh
#   This script is intended to define all the container aliases required
#   to develop for ldmx-sw. These commands assume that the user
#     1. Has docker engine installed OR has singularity installed
#     2. Can run docker as a non-root user OR can run singularity build/run
#
#   SUGGESTION: Put something similar to the following in your '.bashrc',
#     '~/.bash_aliases', or '~/.bash_profile' so that you just have to 
#     run 'ldmx-env' to set-up this environment.
#
#   alias ldmx-env='source <full-path>/ldmx-env.sh; unalias ldmx-env'
###############################################################################

###############################################################################
# All of this setup requires us to be in a bash shell.
#   We add this check to make sure the user is in a bash shell.
###############################################################################
if [[ "$0" != "bash" ]]; then
  echo "[ldmx-env.sh] [ERROR] You aren't in a bash shell. You are in '$0'."
  [[ "$SHELL" = *"bash"* ]] || echo "  You're default shell '$SHELL' isn't bash."
  return 1
fi

###############################################################################
# _ldmx_has_required_engine
#   Checks if user has any of the supported engines for running containers
###############################################################################
_ldmx_has_required_engine() {
  if hash docker &> /dev/null; then
    return 0
  elif hash singularity &> /dev/null; then
    return 0
  else
    return 1
  fi
}

# check if user has a required engine
if ! _ldmx_has_required_engine; then
  echo "[ldmx-env.sh] [ERROR] You do not have docker or singularity installed!"
  return 1
fi

###############################################################################
# _ldmx_which_os
#   Check what OS we are hosting the container on.
#   Taken from https://stackoverflow.com/a/8597411
###############################################################################
export LDMX_CONTAINER_DISPLAY=""
_ldmx_which_os() {
  if [[ "$OSTYPE" == "linux-gnu"* || "$OSTYPE" == "freebsd"* ]]; then
    export LDMX_CONTAINER_DISPLAY=""        
    return 0
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    # Mac OSX
    export LDMX_CONTAINER_DISPLAY="docker.for.mac.host.internal"
    return 0
  fi

  return 1
}

if ! _ldmx_which_os; then
  echo "[ldmx-env.sh] [WARN] Unable to detect OS Type from '${OSTYPE}'"
  echo "    You will *not* be able to run display-connected programs."
fi

###############################################################################
# We have gotten here after determining that we definitely have a container
# runner (either docker or singularity) and we have determined how to connect
# the display (or warn the user that we can't) via the LDMX_CONTAINER_DISPLAY
# variable.
#
#   All container-runners need to implement the following commands
#     - _ldmx_list_local : list images available locally
#     - _ldmx_use : setup the environment to use the specified container
#         - Three arguments: <repo> <tag> <pull_no_matter_what>
#     - _ldmx_run : give all arguments to container's entrypoint script
#         - mounts all directories in bash array LDMX_CONTAINER_MOUNTS
#     - _ldmx_container_clean : remove all containers and images on this machine
#     - _ldmx_container_config : print configuration of container
###############################################################################

# prefer docker, so we do that first
if hash docker &> /dev/null; then
  # List containers on our machine matching the sub-string 'ldmx/local'
  _ldmx_list_local() {
    docker images -q "ldmx/local"
  }

  # Use the input container and error out if not available
  _ldmx_use() {
    local _repo_name="$1"
    local _image_tag="$2"
    local _pull_down="$3"
    export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_image_tag}"
    if [ -z "$(docker images -q ${LDMX_DOCKER_TAG} 2> /dev/null)" ]; then
      echo "No local docker image matching the name '${LDMX_DOCKER_TAG}'."
      if [ "${_repo_name}" == "local" ]; then
        echo "  You can add another tag to your local image and match our required format:"
        echo "      docker tag my-image-tag ldmx/local:my-image-tag"
        echo "  Then you can use"
        echo "      ldmx use local my-image-tag"
      else
        echo "Downloading..."
        docker pull ${LDMX_DOCKER_TAG}
        return $?
      fi
      return 1
    elif [ ! -z ${_pull_down} ]; then
      echo "Downloading..."
      docker pull ${LDMX_DOCKER_TAG}
      return $?
    fi

    return 0
  }

  # Print container configuration
  #   SHA retrieval taken from https://stackoverflow.com/a/33511811
  _ldmx_container_config() {
    echo "Docker Version: $(docker --version)"
    echo "Docker Tag: ${LDMX_DOCKER_TAG}"
    echo "  SHA: $(docker inspect --format='{{index .RepoDigests 0}}' ${LDMX_DOCKER_TAG})"
    return 0
  }

  # Clean up local machine
  _ldmx_container_clean() {
    docker container prune -f || return $?
    docker image prune -a -f  || return $?
  }

  # Run the container
  _ldmx_run() {
    local _mounts=""
    for dir_to_mount in "${LDMX_CONTAINER_MOUNTS[@]}"; do
      _mounts="$_mounts -v $dir_to_mount:$dir_to_mount"
    done
    docker run --rm -it -e LDMX_BASE \
      -e DISPLAY=${LDMX_CONTAINER_DISPLAY}:0 \
      -v /tmp/.X11-unix:/tmp/.X11-unix \
      $_mounts \
      -u $(id -u ${USER}):$(id -g ${USER}) \
      $LDMX_DOCKER_TAG "$@"
    return $?
  }
elif hash singularity &> /dev/null; then
  # List all '.sif' files in LDMX_BASE directory
  _ldmx_list_local() {
    ls -Alh ${LDMX_BASE} | grep ".*local.*sif"
    return 0
  }

  # Use the input container in your workflow
  _ldmx_use() {
    local _repo_name="$1"
    local _image_tag="$2"
    local _pull_down="$3"

    # change cache directory to be inside ldmx base directory
    export SINGULARITY_CACHEDIR=${LDMX_BASE}/.singularity
    mkdir -p ${SINGULARITY_CACHEDIR} #make sure cache directory exists
  
    # name the singularity image after the tag the user asked for
    export LDMX_SINGULARITY_IMG=ldmx_${_repo_name}_${_image_tag}.sif
  
    if [ ! -f "${LDMX_BASE}/${LDMX_SINGULARITY_IMG}" ]; then
      echo "No local singularity image at '${LDMX_BASE}/${LDMX_SINGULARITY_IMG}'."
      if [ "${_repo_name}" == "local" ]; then
        echo "  You can point ldmx to your singularity image in the correct format using symlinks."
        echo "      cd $LDMX_BASE"
        echo "      ln -s <path-to-local-singularity-image>.sif ldmx_local_my-image-tag.sif"
        echo "  Then you can use"
        echo "      ldmx-container-pull local my-image-tag"
      else
        echo "Downloading..."
        singularity build \
          --force \
          ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} \
          docker://ldmx/${_repo_name}:${_image_tag}
        return $?
      fi
      return 1
    elif [ ! -z $_pull_down ]; then
      singularity build \
        --force \
        ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} \
        docker://ldmx/${_repo_name}:${_image_tag}
      return $?
    fi
    return 0
  }

  # Print container configuration
  _ldmx_container_config() {
    echo "Singularity Version: $(singularity --version)"
    echo "Singularity File: ${LDMX_BASE}/${LDMX_SINGULARITY_IMG}"
    return 0
  }

  # Clean up local machine
  _ldmx_container_clean() {
    rm $LDMX_BASE/*.sif || return $?
    rm -r $SINGULARITY_CACHEDIR || return $?
  }

  # Run the container
  _ldmx_run() {
    local csv_list=""
    for dir_to_mount in "${LDMX_CONTAINER_MOUNTS[@]}"; do
      csv_list="$dir_to_mount,$csv_list"
    done
    csv_list="${csv_list%,}"
    singularity run --no-home --cleanenv --env LDMX_BASE=${LDMX_BASE} \
      --bind ${csv_list} ${LDMX_SINGULARITY_IMG} "$@"
    return $?
  }
fi

###############################################################################
# _ldmx_list
#   Get the docker tags for the repository
#   Taken from https://stackoverflow.com/a/39454426
# If passed repo-name is 'local',
#   the list of container options is runner-dependent
###############################################################################
function _ldmx_list() {
  _repo_name="$1"
  if [ "${_repo_name}" == "local" ]; then
    _ldmx_list_local
    return $?
  else
    #line-by-line description
    # download tag json
    # strip unnecessary information
    # break tags into their own lines
    # pick out tags using : as separator
    # put tags back onto same line
    wget -q https://registry.hub.docker.com/v1/repositories/ldmx/${_repo_name}/tags -O -  |\
        sed -e 's/[][]//g' -e 's/"//g' -e 's/ //g' |\
        tr '}' '\n'  |\
        awk -F: '{print $3}' |\
        tr '\n' ' '
    local rc=${PIPESTATUS[0]}
    echo "" #new line
    return ${rc}
  fi
}

###############################################################################
# _ldmx_config
#   Print the configuration of the current setup
###############################################################################
function _ldmx_config() {
  echo "LDMX base directory: ${LDMX_BASE}"
  echo "Display Port (empty on Linux): ${LDMX_CONTAINER_DISPLAY}"
  echo "Container Mounts: ${LDMX_CONTAINER_MOUNTS[@]}"
  _ldmx_container_config
  return $?
}

###############################################################################
# _ldmx_is_mounted
#   Check if the input directory will be accessible by the container
###############################################################################
_ldmx_is_mounted() {
  local full=$(realpath "$1")
  for _already_mounted in ${LDMX_CONTAINER_MOUNTS[@]}; do
    if [[ $full/ = $_already_mounted/* ]]; then
      return 0
    fi
  done
  return 1
}

###############################################################################
# _ldmx_run_here
#   Call the run method with some fancy directory movement around it
###############################################################################
_ldmx_run_here() {
  #store last directory to resume history later
  _old_pwd=$OLDPWD
  #store current working directory relative to ldmx base
  _pwd=$(pwd -P)/.

  # check if container will be able to see where we are
  if ! _ldmx_is_mounted $_pwd; then
    echo "You aren't in a directory mounted to the container!"
    return 1
  fi

  cd ${LDMX_BASE} # go to ldmx base directory outside container
  # go to working directory inside container
  _ldmx_run $_pwd "$@"
  local rc=$?
  cd - &> /dev/null
  export OLDPWD=$_old_pwd
  return ${rc}
}

###############################################################################
# _ldmx_mount
#   Tell us to mount the passed directory to the container when we run
#   By default, we already mount the LDMX_BASE directory, so none of
#   its subdirectories need to (or should be) specified.
###############################################################################
export LDMX_CONTAINER_MOUNTS=()
_ldmx_mount() {
  local _dir_to_mount="$1"
  
  if [[ ! -d $_dir_to_mount ]]; then
    echo "$_dir_to_mount is not a directory!"
    return 1
  fi

  if _ldmx_is_mounted $_dir_to_mount; then
    echo "$_dir_to_mount is already mounted"
    return 0
  fi

  export LDMX_CONTAINER_MOUNTS+=($(realpath "$_dir_to_mount"))
  return 0
}

###############################################################################
# _ldmx_base
#   Define the base directory of ldmx software
###############################################################################
export LDMX_BASE=""
_ldmx_base() {
  _new_base="$1"
  if [[ ! -d $_new_base ]]; then
    echo "'$_new_base' is not a directory!"
    return 1
  fi

  export LDMX_BASE=$(cd $_new_base; pwd -P)
  _ldmx_mount $LDMX_BASE
  return $?
}

###############################################################################
# _ldmx_clean
#   Clean up the computing environment for ldmx
#   The input argument defines what should be cleaned
###############################################################################
_ldmx_clean() {
  _what="$1"

  local cleaned_something=false
  local rc=0
  if [[ "$_what" = "container" ]] || [[ "$_what" = "all" ]]; then
    _ldmx_container_clean
    cleaned_something=true
    rc=$?
  fi

  if [[ "$_what" = "env" ]] || [[ "$_what" = "all" ]]; then
    unset LDMX_BASE
    unset LDMX_CONTAINER_MOUNTS
    unset LDMX_CONTAINER_DISPLAY
    cleaned_something=true
  fi

  if ! $cleaned_something; then
    echo "ERROR: $_what is not one of the clean options."
    rc=1
  fi

  return ${rc}
}

###############################################################################
# _ldmx_help
#   Print some helpful message to the terminal
###############################################################################
_ldmx_help() {
  cat <<\HELP
  USAGE: 
    ldmx <command> [<argument> ...]

  COMMANDS:
    help    : print this help message and exit
      ldmx help
    list    : List the tag options for the input container repository
      ldmx list (dev | pro | local)
    clean   : Reset ldmx computing environment
      ldmx clean (all | container | env)
    config  : Print the current configuration of the container
      ldmx config
    use     : Use the input repo and tag of the container for running
      ldmx use (dev | pro | local) <tag>
    pull    : Pull down the input repo and tag of the container
      ldmx pull (dev | pro | local) <tag>
    run     : Run a command at an input location in the container
      ldmx run <directory> <sub-command> [<argument> ...]
    <other> : Run the input command in your current directory in the container
      ldmx <other> [<argument> ...]
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
function ldmx() {
  _sub_command="$1"
  _sub_command_args="${@:2}"

  # divide commands by number of arguments
  case $_sub_command in
    help)
      _ldmx_help
      return $?
      ;;
    config)
      if [[ ! -z "$_sub_command_args" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx config' takes no arguments."
        return 1
      fi
      _ldmx_config 
      return $?
      ;;
    list|base|clean|mount)
      if [[ "$#" != "2" ]]; then
        _ldmx_help
        echo "ERROR: ldmx ${_sub_command} takes one argument."
        return 1
      fi
      _ldmx_${_sub_command} $_sub_command_args
      return $?
      ;;
    pull)
      if [[ "$#" != "3" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx pull' takes two arguments: <repo> <tag>."
        return 1
      fi
      _ldmx_use $_sub_command_args "PULL_NO_MATTER_WHAT"
      return $?
      ;;
    use)
      if [[ "$#" != "3" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx use' takes two arguments: <repo> <tag>."
        return 1
      fi
      _ldmx_use $_sub_command_args
      return $?
      ;;
    run)
      _ldmx_run $_sub_command_args
      return $?
      ;;
    *)
      _ldmx_run_here $_sub_command $_sub_command_args
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
# _ldmx_complete_directory
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
_ldmx_complete_directory() {
  local _num_words="1"
  COMP_WORDS=(${COMP_WORDS[@]:_num_words})
  COMP_CWORD=$((COMP_CWORD - _num_words))
  _cd
}

###############################################################################
# _ldmx_complete_command
#   Tab-complete with a command used commonly inside the container
#
#   Search the install location of ldmx-sw for ldmx-sw executables
#   and include hard-coded common commands. Any strings passed are
#   also included.
#
#   Assumes current argument being tab completed is stored in
#   bash variable 'curr_word'.
###############################################################################
_ldmx_complete_command() {
  # generate up-to-date list of options
  local _options="$@ cmake make python3 python"
  for ldmx_executable in ${LDMX_BASE}/ldmx-sw/install/bin/*; do
    _options="$_options $(basename $ldmx_executable)"
  done

  # match current word (perhaps empty) to the list of options
  COMPREPLY=($(compgen -W "$_options" "$curr_word"))
}

###############################################################################
# _ldmx_complete_bash_default
#   Restore the default tab-completion in bash that uses the readline function
#   Bash default tab completion just looks for filenames
###############################################################################
_ldmx_complete_bash_default() {
  compopt -o default
  COMPREPLY=()
}

###############################################################################
# _ldmx_dont_complete
#   Don't tab complete or suggest anything if user <tab>s
###############################################################################
_ldmx_dont_complete() {
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
_ldmx_complete() {
  # disable readline filename completion
  compopt +o default

  local curr_word="${COMP_WORDS[$COMP_CWORD]}"

  if [[ "$COMP_CWORD" = "1" ]]; then
    # tab completing a main argument
    _ldmx_complete_command "list clean config pull use run mount base"
  elif [[ "$COMP_CWORD" = "2" ]]; then
    # tab complete a sub-argument,
    #   depends on the main argument
    case "${COMP_WORDS[1]}" in
      config)
        # no more arguments
        _ldmx_dont_complete
        ;;
      clean)
        # arguments from special set
        COMPREPLY=($(compgen -W "all container env" "$curr_word"))
        ;;
      list|pull|use)
        # container repositories after these commands
        COMPREPLY=($(compgen -W "dev pro local" "$_curr_word"))
        ;;
      run|mount|base)
        #directories only after these commands
        _ldmx_complete_directory
        ;;
      *)
        # files like normal tab complete after everything else
        _ldmx_complete_bash_default
        ;;
    esac
  else
    # three or more arguments
    #   check base argument to see if we should continue
    case "${COMP_WORDS[1]}" in
      list|base|clean|config|pull|use|mount)
        # these commands shouldn't have tab complete for the third argument 
        #   (or shouldn't have the third argument at all)
        _ldmx_dont_complete
        ;;
      run)
        if [[ "$COMP_CWORD" = "3" ]]; then
          # third argument to run should be an inside-container command
          _ldmx_complete_command
        else
          # later arguments to run should be bash default
          _ldmx_complete_bash_default
        fi
        ;;
      *)
        # everything else has bash default (filenames)
        _ldmx_complete_bash_default
        ;;
    esac
  fi
}

# Tell bash the tab-complete options for our main function ldmx
complete -F _ldmx_complete ldmx

###############################################################################
# Parse CLI Arguments
#
#   Now that we have defined all the necessary bash functions,
#   we can move on to parsing the command line inputs for this script
#   in particular, setting up the environment in a default configuration.
#
# NOTE: We could remove all the nonsense below this point, but then
#   users would have to do multiple commands to get set up.
#
#   For example, to use dev latest, *without what is below*, the user
#   would have to run the following
#     source ldmx-sw/scripts/ldmx-env.sh
#     ldmx base .
#     ldmx use dev latest
###############################################################################

function _ldmx_env_help() {
  cat <<\HELP
    Environment setup script for ldmx

  USAGE: 
    source ldmx-sw/scripts/ldmx-env.sh [-h,--help] [-f,--force] 
                  [-b,--base ldmx_base] [-r,--repo repo_name] 
                  [-t,--tag image_tag] [--no-init]

  OPTIONS:
    -f,--force : Force download of container even if it already exists
    -h,--help  : Print this help message and exit
    -b,--base  : ldmx_base is path to directory containing ldmx-sw
    -r,--repo  : name of repo  (ldmx/[repo_name]) to pull container from
                 There are three options: 'dev', 'pro', and 'local'
                 Pass 'local' to use existing, locally built container 
    -t,--tag   : name of tag of ldmx image to pull down and use 
                 The options you can input depend on the repo. 
                 Use 'ldmx list <repo>' to list the options.
    --no-init  : Define container-interacting bash functions, but do not
                 initialize container environment.
HELP
  return 0
}

#default backs out of ldmx-sw/scripts
_the_base="$( dirname ${BASH_SOURCE[0]} )/../../" 
#default repository is development container with just the dependencies in it
_repo_name="dev"
#default tag is the most recent major release of the dev container
_image_tag="latest"
#default is to NOT force updates of the container (empty == don't pull)
_force_update=""

function _ldmx_env_fatal_error() {
  _ldmx_env_help
  echo "ERROR: $@"
  return 1
}

# loop through all the options
while [[ $# -gt 0 ]]; do
  option="$1"
  case "$option" in
    -h|--help)
      _ldmx_env_help
      return 0
      ;;
    -b|--base)
      if [[ -z "$2" || "$2" =~ "-".* ]]
      then
        _ldmx_env_fatal_error "The '-b','--base' flag requires an argument after it!"
        return 1
      fi
      _the_base="$2"
      shift #move past flag
      shift #move past argument
      ;;
    -r|--repo)
      if [[ "$2" == "dev" || "$2" == "pro" || "$2" == "local" ]]
      then
        _repo_name="$2"
      else
        _ldmx_env_fatal_error "Unsupported repo name: '$2'"
        return 2
      fi
      shift #move past flag
      shift #move past argument
      ;;
    -t|--tag)
      if [[ -z "$2" || "$2" =~ "-".* ]]
      then
        _ldmx_env_fatal_error "The '-t','--tag' flag requires an argument after it!"
        return 3
      fi
      _image_tag="$2"
      shift #move past flag
      shift #move past argument
      ;;
    -f|--force)
      _force_update="ON"
      shift #move past flag
      ;;
    --no-init)
      # we've already defined the necessary bash functions above,
      #   so we can just leave now to not initialize the container
      return 0
      ;;
    *)
      _ldmx_env_fatal_error "'$option' is not a valid option!"
      return 5
      ;;
  esac
done

# pull down the container if it doesn't exist on this computer yet
_ldmx_use ${_repo_name} ${_image_tag} ${_force_update}
# mount the base directory to the container so we can see the code
_ldmx_base ${_the_base}
