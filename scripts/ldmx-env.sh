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
# ldmx-container-tags
#   Get the docker tags for the repository
#   Taken from https://stackoverflow.com/a/39454426
###############################################################################
function ldmx-container-tags() {
    _repo_name="$1"
    if [ "${_repo_name}" == "local" ] 
    then
        if hash docker &> /dev/null
        then
            docker images -q "ldmx/local"
        else
            ll ${LDMX_BASE} | grep ".*local.*sif"
        fi
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
# ldmx-container-pull
#   Pull down the ldmx container depending on inputs and
#   sets the ldmx environment to be use the input image
#
#   This function has two inputs:
#       1. the name of the docker hub repo to pull from (or local)
#       2. the name of the tag of the image
#
#   Examples
#   - Pull down the latest dev image: ldmx-container-pull dev latest
#   - Pull down the 2.1.0 pro image:  ldmx-container-pull pro v2.1.0
#   - Use a local image you built:    ldmx-container-pull local my-tag
#
#   The local images should be tagged like 'ldmx/local:my-tag',
#   so the build instruction would be
#       docker build . -t ldmx/local:my-tag
###############################################################################
function ldmx-container-pull() {
    _repo_name="$1"
    _image_tag="$2"
    export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_image_tag}"

    if hash docker &>/dev/null
    then
        if [ "${_repo_name}" == "local" ]
        then
            if [ -z "$(docker images -q ${LDMX_DOCKER_TAG} 2> /dev/null)" ]
            then
                echo "No local docker image matching the name '${LDMX_DOCKER_TAG}'."
                echo "  You can add another tag to your local image and match our required format:"
                echo "      docker tag my-image-tag ldmx/local:my-image-tag"
                echo "  Then you can use"
                echo "      ldmx-container-pull local my-image-tag"
                return 1
            fi
        else
            docker pull ${LDMX_DOCKER_TAG}
        fi

    elif hash singularity &>/dev/null
    then

        # change cache directory to be inside ldmx base directory
        export SINGULARITY_CACHEDIR=${LDMX_BASE}/.singularity
        mkdir -p ${SINGULARITY_CACHEDIR} #make sure cache directory exists
    
        # name the singularity image after the tag the user asked for
        export LDMX_SINGULARITY_IMG=ldmx_${_repo_name}_${_image_tag}.sif
    
        # build the docker container into the singularity image
        #   will prompt the user if the image already exists
        if [ "${_repo_name}" == "local" ]
        then
            if [ ! -f "${LDMX_BASE}/${LDMX_SINGULARITY_IMG}" ]
            then
                echo "No local singularity image at '${LDMX_BASE}/${LDMX_SINGULARITY_IMG}'."
                echo "  You can point ldmx to your singularity image in the correct format using symlinks."
                echo "      cd $LDMX_BASE"
                echo "      ln -s <path-to-local-singularity-image>.sif ldmx_local_my-image-tag.sif"
                echo "  Then you can use"
                echo "      ldmx-container-pull local my-image-tag"
                return 1
            fi
        else
            singularity build \
                --force \
                ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} \
                docker://${LDMX_DOCKER_TAG}
        fi
    fi
}

###############################################################################
# ldmx-container-config
#   Print the configuration of the current setup
###############################################################################
function ldmx-container-config() {
    echo "LDMX base directory: ${LDMX_BASE}"
    if hash docker &> /dev/null
    then
        echo "Using $(docker --version)"
        echo "Docker tag: ${LDMX_DOCKER_TAG}"
    elif hash singularity &> /dev/null
    then
        echo "Using $(singularity --version)"
        echo "Singularity Image: ${LDMX_BASE}/${LDMX_SINGULARITY_IMG}"
    fi
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
    #store last directory to resume history later
    _old_pwd=$OLDPWD
    #store current working directory relative to ldmx base
    _pwd=$(pwd -P)/.
    cd ${LDMX_BASE} # go to ldmx base directory outside container
    if hash docker &>/dev/null
    then
        docker run \
            -it \
            -e LDMX_BASE \
            -v $LDMX_BASE:$LDMX_BASE \
            -u $(id -u ${USER}):$(id -g ${USER}) \
            $LDMX_DOCKER_TAG ${_pwd} "$@"
    elif hash singularity &>/dev/null
    then
        # actually run the singularity image stored in the base directory 
        #  going to working directory inside container
        _current_working_dir=${_pwd##"${LDMX_BASE}/"} 
        singularity run \
            --no-home \
            --bind ${LDMX_BASE} \
            ${LDMX_SINGULARITY_IMG} ${_current_working_dir} "$@"
    fi
    cd - &> /dev/null #go back outside the container
    export OLDPWD=$_old_pwd #restore history
}

###############################################################################
# Parse CLI Arguments
###############################################################################

export _default_ldmx_env_ldmx_base="$( dirname ${BASH_SOURCE[0]} )/../../" #default backs out of ldmx-sw/scripts
export _default_ldmx_env_repo_name="dev" #default repository is development container with just the dependencies in it
export _default_ldmx_env_image_tag="v1.0" #default tag is the most recent major release of the dev container
export _default_ldmx_env_force_update="OFF" #default is to NOT force updates of the container

function ldmx-env-help() {
    echo "Environment setup script for ldmx."
    echo "  Usage: source ldmx-sw/scripts/ldmx-env.sh [-h,--help] [-f,--force] [-b,--base ldmx_base] [-r,--repo repo_name] [-t,--tag image_tag]"
    echo "    -f,--force : Force download and update of container even if it already exists (default: $_default_ldmx_env_force_update)"
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

function ldmx-env-fatal-error() {
    echo "ERROR: $@"
    ldmx-env-help
    return 1
}

while [[ $# -gt 0 ]] #while still have more options
do
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

# pull down the container if it doesn't exist on this computer yet
if hash docker &> /dev/null
then
    export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_image_tag}"
    if [[ $_force_update == *"ON"* || -z $(docker images -q ${LDMX_DOCKER_TAG} 2> /dev/null) ]]
    then
        ldmx-container-pull ${_repo_name} ${_image_tag}
    fi
elif hash singularity &> /dev/null
then
    export LDMX_SINGULARITY_IMG="ldmx_${_repo_name}_${_image_tag}.sif"
    if [[ $_force_update == *"ON"* || ! -f "${LDMX_BASE}/${LDMX_SINGULARITY_IMG}" ]]
    then
        ldmx-container-pull ${_repo_name} ${_image_tag}
    fi
fi

