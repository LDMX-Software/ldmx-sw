
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
    wget -q https://registry.hub.docker.com/v1/repositories/ldmx/${_repo_name}/tags -O -  |\
        sed -e 's/[][]//g' -e 's/"//g' -e 's/ //g' |\
        tr '}' '\n'  |\
        awk -F: '{print $3}'
}

###############################################################################
# ldmx-container-pull
#   Pull down the ldmx container depending on inputs and
#   sets the bash variable LDMX_DOCKER_TAG to the correctly formatted value
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
if hash docker &>/dev/null
then
    function ldmx-container-pull() {
        _repo_name="$1"
        _tag="$2"

        export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_tag}"
        if [ "${_repo_name}" == "local" ]
        then
            if [ -z "$(docker images -q ${LDMX_DOCKER_TAG} 2> /dev/null)" ]
            then
                echo "No local docker image matching the name '${LDMX_DOCKER_TAG}'."
                return 1
            fi
        else
            docker pull ${LDMX_DOCKER_TAG}
        fi
    }
elif hash singularity &>/dev/null
then
    function ldmx-container-pull() {
        _repo_name="$1"
        _tag="$2"

        # change cache directory to be inside ldmx base directory
        export SINGULARITY_CACHEDIR=${LDMX_BASE}/.singularity
        mkdir -p ${SINGULARITY_CACHEDIR} #make sure cache directory exists
    
        # name the singularity image after the tag the user asked for
        export LDMX_SINGULARITY_IMG=ldmx_${_repo_name}_${_tag}.sif
    
        # build the docker container into the singularity image
        #   will prompt the user if the image already exists
        export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_tag}"
        if [ "${_repo_name}" == "local" ]
        then
            if [ ! -f "${LDMX_BASE}/${LDMX_SINGULARITY_IMG}" ]
            then
                echo "No local singularity image at '${LDMX_BASE}/${LDMX_SINGULARITY_IMG}'."
                return 1
            fi
        else
            singularity build ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} docker://${LDMX_DOCKER_TAG}
        fi
    }
fi

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
if hash docker &>/dev/null; then
    alias ldmx='docker run -it -e LDMX_BASE -v $LDMX_BASE:$LDMX_BASE -u $(id -u ${USER}):$(id -g ${USER}) $LDMX_DOCKER_TAG $(pwd)'
elif hash singularity &>/dev/null; then
    function ldmx() {
        _current_working_dir=${PWD##"${LDMX_BASE}/"} #store current working directory relative to ldmx base
        cd ${LDMX_BASE} # go to ldmx base directory outside container
        # actually run the singularity image stored in the base directory going to working directory inside container
        singularity run --no-home ${LDMX_SINGULARITY_IMG} ${_current_working_dir} "$@"
        cd - &> /dev/null #go back outside the container
    }
fi

###############################################################################
# Parse CLI Arguments
#   $1 - base directory for your ldmx work
#        must be directory containing ldmx-sw and ldmx-analysis
#   $2 - docker image repository (defaults to 'dev')
#   $3 - docker image tag        (defaults to 'latest')
###############################################################################

_ldmx_base="$1"
if [ -z ${_ldmx_base} ]
then
    _dir_name_of_script=$( dirname ${BASH_SOURCE[0]} )
    _ldmx_base="${_dir_name_of_script}/../../" #back out of ldmx-sw/scripts
elif [[ "${_ldmx_base}" = *"help" || "${_ldmx_base}" == *"-h" ]]
then
    echo "Environment setup script for ldmx."
    echo "  Usage: source ldmx-sw/scripts/ldmx-env.sh [ldmx_base] [repo-name] [image_tag]"
    echo "    ldmx_base : path to directory containing ldmx-sw (default: present working directory)"
    echo "    repo_name : name of repo  (ldmx/[repo_name]) to pull container from. "
    echo "                Pass 'local' to use existing, locally built container (default: dev)"
    echo "    image_tag : name of tag of ldmx/dev image to pull down and use (default: latest)"
    echo "The image_tag options you can input are below."
    echo "For the 'dev' repository:"
    echo $(ldmx-container-tags "dev") 
    echo "For the 'pro' repository:"
    echo $(ldmx-container-tags "pro") 
    return 0
fi

# this makes sure we get the full path
cd ${_ldmx_base}
export LDMX_BASE=$(pwd)
cd - &> /dev/null

_repo_name="$2"
if [ -z $_repo_name ]
then
    _repo_name="dev"
    # this is the name of the dockerhub repository
    # it should change when we want a production container 
    # also, if it's set to "local", skip downloading at all and attempt to use a locally built container
fi

_image_tag="$3"
if [ -z $_image_tag ]; then
    _image_tag="latest"
fi

# pull down the container if it doesn't exist on this computer yet
if hash docker &> /dev/null
then
    if [ -z $(docker images -q ldmx/${_repo_name}:${_image_tag} 2> /dev/null) ]
    then
        ldmx-container-pull ${_repo_name} ${_image_tag}
    fi
elif hash singularity &> /dev/null 
then
    if [ ! -f "${LDMX_BASE}/ldmx_${_repo_name}_${image_tag}" ]
    then
        ldmx-container-pull ${_repo_name} ${_image_tag}
    fi
fi
