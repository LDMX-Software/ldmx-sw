#!/bin/bash

###############################################################################
# ldmx-env.sh
#   This script is intended to define all the container aliases required
#   to develop for ldmx-sw. These commands assume that the user
#   1. Has docker engine installed OR has singularity installed
#   2. Can run docker as a non-root user OR can run singularity build/run
###############################################################################

###############################################################################
# ldmx-has-required-engine
#   Checks if user has any of the supported engines for running containers
###############################################################################
function ldmx-has-required-engine() {
    if hash docker &> /dev/null
    then
        return 0
    elif hash singularity &> /dev/null
    then
        return 0
    else
        return 1
    fi
}

# check if user has a required engine
if ! ldmx-has-required-engine
then
    echo "You do not have docker or singularity installed!"
    return 1
fi

###############################################################################
# ldmx-which-os
#   Check what OS we are hosting the container on.
#   Taken from https://stackoverflow.com/a/8597411
###############################################################################
function ldmx-which-os() {
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

if ! ldmx-which-os
then
    echo "[ldmx-env.sh][WARN] : Unable to detect OS Type from '${OSTYPE}'"
    echo "  You will *not* be able to run display-connected programs."
fi

# We have gotten here after determining that we definitely have a container
# runner (either docker or singularity) and we have determined how to connect
# the display (or warn the user that we can't) via the LDMX_CONTAINER_DISPLAY
# variable.
#   All container-runners need to implement the following commands
#     - _ldmx_list_local : list images available locally
#     - _ldmx_use : setup the environment to use the specified container
#     - _ldmx_run : go to first argument and run the rest in the container
#     - _ldmx_clean : remove all containers and images on the local machine
#     - _ldmx_container_config : print configuration of container

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
    export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_image_tag}"
    if [ -z "$(docker images -q ${LDMX_DOCKER_TAG} 2> /dev/null)" ]; then
      echo "No local docker image matching the name '${LDMX_DOCKER_TAG}'."
      if [ "${_repo_name}" == "local" ]; then
        echo "  You can add another tag to your local image and match our required format:"
        echo "      docker tag my-image-tag ldmx/local:my-image-tag"
        echo "  Then you can use"
        echo "      ldmx use local my-image-tag"
      elif [ ! -z ${_pull_down} ]; then
        echo "Downloading..."
        docker pull ${LDMX_DOCKER_TAG}
        return 0
      fi
      return 1
    fi
    return 0
  }

  # Print container configuration
  _ldmx_container_config() {
    echo "Docker Version: $(docker --version)"
    echo "Docker Tag: ${LDMX_DOCKER_TAG}"
  }

  # Clean up local machine
  _ldmx_clean() {
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
      elif [ ! -z $_pull_down ]; then
        echo "Downloading..."
        singularity build \
          --force \
          ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} \
          docker://ldmx/${_repo_name}:${_image_tag}
        return 0
      fi
      return 1
    fi
    return 0
  }

  # Print container configuration
  _ldmx_container_config() {
    echo "Singularity Version: $(singularity --version)"
    echo "Singularity File: ${LDMX_BASE}/${LDMX_SINGULARITY_IMG}"
  }

  # Clean up local machine
  _ldmx_clean() {
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
  if [ "${_repo_name}" == "local" ] 
  then
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
    _ldmx_container_config
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
_ldmx_mount() {
  for d in "$@"; do
    if [[ ! -d $d ]]; then
      echo "'${d}' is not a directory!"
      return 1
    fi
    # check if sub-directory of LDMX_BASE?
    LDMX_CONTAINER_MOUNTS+=("$d")
  done
  export LDMX_CONTAINER_MOUNTS
}


###############################################################################
# ldmx
#   Launch the user into the ldmx docker container environment, passing
#   any argument along as a command to be executed.
#
#   The container is set up so that it goes to the first argument and then
#   executes the rest.
#   
#   For docker, this alias sets the first argument to $(pwd) so that
#   the container runs the users command from the same place that the user
#   intended.
#
#   For singularity, this function stores the current working directory
#   relative to the base and then goes to the ldmx base. singularity
#   mounts the current working directory to the container, so we go to
#   the base, mount it, and then inside the container go back to where we were.
###############################################################################
function ldmx() {
  _sub_command="$1"
  _sub_command_args="${@:2}"
  case $_sub_command in
    "list")
      _ldmx_list $_sub_command_args
      ;;
    "clean")
      _ldmx_clean
      ;;
    "config")
      _ldmx_config $_sub_command_args
      ;;
    "pull")
      _ldmx_use $_sub_command_args "YES_PULL"
      ;;
    "mount")
      _ldmx_mount $_sub_command_args
      ;;
    "use")
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
# Modify the list of completion options on the command line
#   Helpful discussion of this procedure from a blog post
#   https://iridakos.com/programming/2018/03/01/bash-programmable-completion-tutorial
###############################################################################
_ldmx_completions() {
  # leave if we dont have exactly one input
  if [ "${#COMP_WORDS[@]}" != "2" ]; then
    return
  fi

  local _options="list clean config pull use run mount cmake make python3 python"
  for ldmx_executable in ${LDMX_BASE}/ldmx-sw/install/bin/*; do
    _options="$_options $(basename $ldmx_executable)"
  done
  COMPREPLY=($(compgen -W $_options "${COMP_WORDS[1]}"))
}

# Tell bash the tab-complete options for our main function ldmx
complete -F _ldmx_completions ldmx

###############################################################################
# Parse CLI Arguments
###############################################################################

export _default_ldmx_env_ldmx_base="$( dirname ${BASH_SOURCE[0]} )/../../" #default backs out of ldmx-sw/scripts
export _default_ldmx_env_repo_name="dev" #default repository is development container with just the dependencies in it
export _default_ldmx_env_image_tag="latest" #default tag is the most recent major release of the dev container
export _default_ldmx_env_force_update="" #default is to NOT force updates of the container

function _ldmx_env_help() {
  echo "Environment setup script for ldmx."
  echo "  Usage: source ldmx-sw/scripts/ldmx-env.sh [-h,--help] [-f,--force] [-b,--base ldmx_base] [-r,--repo repo_name] [-t,--tag image_tag]"
  echo "    -f,--force : Force download and update of container even if it already exists (default: OFF)"
  echo "    -h,--help  : Print this help message"
  echo "    -b,--base  : ldmx_base is path to directory containing ldmx-sw"
  echo "                 (default: $_default_ldmx_env_ldmx_base)"
  echo "    -r,--repo  : name of repo  (ldmx/[repo_name]) to pull container from. "
  echo "                 There are three options: 'dev', 'pro', and 'local'"
  echo "                 Pass 'local' to use existing, locally built container (default: $_default_ldmx_env_repo_name)"
  echo "    -t,--tag   : name of tag of ldmx image to pull down and use (default: $_default_ldmx_env_image_tag)"
  echo "                 The options you can input depend on the repo:"
  echo "                 For repo_name == 'dev': $(ldmx-container-tags "dev")"
  echo "                 For repo_name == 'pro': $(ldmx-container-tags "pro")"
  return 0
}

_ldmx_base=$_default_ldmx_env_ldmx_base
_repo_name=$_default_ldmx_env_repo_name
_image_tag=$_default_ldmx_env_image_tag
_force_update=$_default_ldmx_env_force_update

function _ldmx_env_fatal_error() {
  ldmx-env-help
  echo "ERROR: $@"
  return 1
}

# loop through all the options
while [[ $# -gt 0 ]]; do
  option="$1"
  case "$option" in
    -h|--help)
      ldmx-env-help
      return 0
      ;;
    -b|--base)
      if [[ -z "$2" || "$2" =~ "-".* ]]
      then
        ldmx-env-fatal-error "The '-b','--base' flag requires an argument after it!"
        return 1
      fi
      _ldmx_base="$2"
      shift #move past flag
      shift #move past argument
      ;;
    -r|--repo)
      if [[ "$2" == "dev" || "$2" == "pro" || "$2" == "local" ]]
      then
        _repo_name="$2"
      else
        ldmx-env-fatal-error "Unsupported repo name: '$2'"
        return 2
      fi
      shift #move past flag
      shift #move past argument
      ;;
    -t|--tag)
      if [[ -z "$2" || "$2" =~ "-".* ]]
      then
        ldmx-env-fatal-error "The '-t','--tag' flag requires an argument after it!"
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
      ldmx-env-fatal-error "'$option' is not a valid option!"
      return 5
      ;;
  esac
done

# this makes sure we get the full path
_old_pwd=$OLDPWD #store move history
cd ${_ldmx_base}
export LDMX_BASE=$(pwd -P)
cd - &> /dev/null
export OLDPWD=$_old_pwd #resume history

# LDMX_BASE is definitely going to be mounted
export LDMX_CONTAINER_MOUNTS=("$LDMX_BASE")

# pull down the container if it doesn't exist on this computer yet
_ldmx_use ${_repo_name} ${_image_tag} ${_pull_if_nonempty}
