#!/bin/bash

###############################################################################
# ldmx-env.sh
#   This script is intended to define all the container aliases required
#   to develop for ldmx-sw. These commands assume that the user
#     1. Has docker engine installed OR has singularity installed
#     2. Can run docker as a non-root user OR can run singularity build/run
###############################################################################

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
  echo "You do not have docker or singularity installed!"
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
  echo "[ldmx-env.sh][WARN] : Unable to detect OS Type from '${OSTYPE}'"
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
        return 0
      fi
      return 1
    elif [ ! -z ${_pull_down} ]; then
      echo "Downloading..."
      docker pull ${LDMX_DOCKER_TAG}
      return 0
    fi

    return 0
  }

  # Print container configuration
  #   SHA retrieval taken from https://stackoverflow.com/a/33511811
  _ldmx_container_config() {
    echo "Docker Version: $(docker --version)"
    echo "Docker Tag: ${LDMX_DOCKER_TAG}"
    echo "  SHA: $(docker inspect --format='{{index .RepoDigests 0}}' ${LDMX_DOCKER_TAG})"
  }

  # Clean up local machine
  _ldmx_container_clean() {
    docker container prune -f
    docker image prune -a -f
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
  }
elif hash singularity &> /dev/null; then
  # List all '.sif' files in LDMX_BASE directory
  _ldmx_list_local() {
    ls -Alh ${LDMX_BASE} | grep ".*local.*sif"
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
        return 0
      fi
      return 1
    elif [ ! -z $_pull_down ]; then
      singularity build \
        --force \
        ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} \
        docker://ldmx/${_repo_name}:${_image_tag}
      return 0
    fi
    return 0
  }

  # Print container configuration
  _ldmx_container_config() {
    echo "Singularity Version: $(singularity --version)"
    echo "Singularity File: ${LDMX_BASE}/${LDMX_SINGULARITY_IMG}"
  }

  # Clean up local machine
  _ldmx_container_clean() {
    rm $LDMX_BASE/*.sif
    rm -r $SINGULARITY_CACHEDIR
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
    echo "" #new line
  fi
}

###############################################################################
# ldmx-container-config
#   Print the configuration of the current setup
###############################################################################
function _ldmx_config() {
  echo "LDMX base directory: ${LDMX_BASE}"
  echo "Display Port (empty on Linux): ${LDMX_CONTAINER_DISPLAY}"
  echo "Container Mounts: ${LDMX_CONTAINER_MOUNTS[@]}"
  _ldmx_container_config
}

###############################################################################
# _ldmx_is_mounted
#   Check if the input directory is mounted to the container
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
  _current_working_dir=${_pwd##"${LDMX_BASE}/"} 
  _ldmx_run $_current_working_dir "$@"
  cd - &> /dev/null
  export OLDPWD=$_old_pwd
}

###############################################################################
# _ldmx_mount
#   Tell us to mount all of the passed directories to the container
#   By default, we already mount the LDMX_BASE directory, so none of
#   its subdirectories need to (or should be) specified.
###############################################################################
export LDMX_CONTAINER_MOUNTS=()
_ldmx_mount() {
  for d in "$@"; do
    if [[ ! -d $d ]]; then
      echo "'${d}' is not a directory!"
      continue
    fi

    if _ldmx_is_mounted $d; then
      echo "'$d' is already mounted"
      continue
    fi

    LDMX_CONTAINER_MOUNTS+=($(realpath "$d"))
  done
  export LDMX_CONTAINER_MOUNTS
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

  _new_base=$(realpath $_new_base)

  export LDMX_BASE=$_new_base
  _ldmx_mount $LDMX_BASE
}

###############################################################################
# _ldmx_clean
#   Clean up the computing environment for ldmx
#   The input argument defines what should be cleaned
###############################################################################
_ldmx_clean() {
  _what="$1"

  if [[ "$_what" = "container" ]] || [[ "$_what" = "all" ]]; then
    _ldmx_container_clean
  fi

  if [[ "$_what" = "env" ]] || [[ "$_what" = "all" ]]; then
    unset LDMX_BASE
    unset LDMX_CONTAINER_MOUNTS
    unset LDMX_CONTAINER_DISPLAY
  fi

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
      ldmx list <repo>
    clean   : Reset ldmx computing environment
      ldmx clean [all | container]
    config  : Print the current configuration of the container
      ldmx config
    use     : Use the input repo and tag of the container for running
      ldmx use <repo> <tag>
    pull    : Pull down the input repo and tag of the container
      ldmx pull <repo> <tag>
    run     : Run a command at an input location in the container
      ldmx run <directory> <sub-command> [<argument> ...]
    <other> : Run the input command in your current directory in the container
      ldmx <other> [<argument> ...]
HELP
}

###############################################################################
# _ldmx_error
#   Print help and short description of what went wrong.
###############################################################################
_ldmx_error() {
  _ldmx_help
  echo "ERROR: $@"
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

  case $_sub_command in
    "help")
      _ldmx_help
      ;;
    "list")
      if [[ "$#" != "2" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx list' takes one argument."
        return 1
      fi
      _ldmx_list $_sub_command_args
      ;;
    "base")
      if [[ "$#" != "2" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx base' takes one argument."
        return 1
      fi
      _ldmx_base $_sub_command_args
      ;;
    "clean")
      if [[ "$#" != "2" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx clean' takes one argument."
        return 1
      fi
      _ldmx_clean 
      ;;
    "config")
      if [[ ! -z "$_sub_command_args" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx config' takes no arguments."
        return 1
      fi
      _ldmx_config 
      ;;
    "pull")
      if [[ "$#" != "3" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx pull' takes two arguments: <repo> <tag>."
        return 1
      fi
      _ldmx_use $_sub_command_args "YES_PULL"
      ;;
    "mount")
      _ldmx_mount $_sub_command_args
      ;;
    "use")
      if [[ "$#" != "3" ]]; then
        _ldmx_help
        echo "ERROR: 'ldmx use' takes two arguments: <repo> <tag>."
        return 1
      fi
      _ldmx_use $_sub_command_args
      ;;
    "run")
      _ldmx_run $_sub_command_args
      ;;
    *)
      _ldmx_run_here $_sub_command $_sub_command_args
      ;;
  esac
}

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
#
###############################################################################
_ldmx_complete() {
  # disable readline filename completion
  compopt +o default

  local curr_word="${COMP_WORDS[$COMP_CWORD]}"

  if [[ "$COMP_CWORD" = "1" ]]; then
    # tab completing a main argument
    # generate up-to-date list of options
    local _options="list clean config pull use run mount base cmake make python3 python"
    for ldmx_executable in ${LDMX_BASE}/ldmx-sw/install/bin/*; do
      _options="$_options $(basename $ldmx_executable)"
    done

    # match current word (perhaps empty) to the list of options
    COMPREPLY=($(compgen -W "$_options" "$curr_word"))
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
#     ldmx pull dev latest
###############################################################################

#default backs out of ldmx-sw/scripts
export _default_ldmx_env_ldmx_base="$( dirname ${BASH_SOURCE[0]} )/../../" 
#default repository is development container with just the dependencies in it
export _default_ldmx_env_repo_name="dev" 
#default tag is the most recent major release of the dev container
export _default_ldmx_env_image_tag="latest"
#default is to NOT force updates of the container (empty == don't pull)
export _default_ldmx_env_force_update=""

function _ldmx_env_help() {
  cat <<\HELP
    Environment setup script for ldmx

  USAGE: 
    source ldmx-sw/scripts/ldmx-env.sh [-h,--help] [-f,--force] 
                  [-b,--base ldmx_base] [-r,--repo repo_name] 
                  [-t,--tag image_tag]

  OPTIONS:
    -f,--force : Force download of container even if it already exists
    -h,--help  : Print this help message
    -b,--base  : ldmx_base is path to directory containing ldmx-sw
    -r,--repo  : name of repo  (ldmx/[repo_name]) to pull container from
                 There are three options: 'dev', 'pro', and 'local'
                 Pass 'local' to use existing, locally built container 
    -t,--tag   : name of tag of ldmx image to pull down and use 
                 The options you can input depend on the repo. 
                 Use 'ldmx list <repo>' to list the options.
HELP
  return 0
}

_the_base=$_default_ldmx_env_ldmx_base
_repo_name=$_default_ldmx_env_repo_name
_image_tag=$_default_ldmx_env_image_tag
_force_update=$_default_ldmx_env_force_update

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
