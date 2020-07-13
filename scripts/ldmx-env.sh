
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
        wget -q https://registry.hub.docker.com/v1/repositories/ldmx/${_repo_name}/tags -O -  |\
            sed -e 's/[][]//g' -e 's/"//g' -e 's/ //g' |\
            tr '}' '\n'  |\
            awk -F: '{print $3}'
    fi
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
    #store current working directory relative to ldmx base
    _current_working_dir=${PWD##"${LDMX_BASE}/"} 
    cd ${LDMX_BASE} # go to ldmx base directory outside container
    if hash docker &>/dev/null; then
        docker run \
            -it \
            -e LDMX_BASE \
            -v $LDMX_BASE:$LDMX_BASE \
            -u $(id -u ${USER}):$(id -g ${USER}) \
            $LDMX_DOCKER_TAG ${_current_working_dir} "$@"
    elif hash singularity &>/dev/null; then
        # actually run the singularity image stored in the base directory 
        #  going to working directory inside container
        singularity run \
            --no-home \
            ${LDMX_SINGULARITY_IMG} ${_current_working_dir} "$@"
    fi
    cd - &> /dev/null #go back outside the container
}

###############################################################################
# Parse CLI Arguments
#   $1 - base directory for your ldmx work
#        must be directory containing ldmx-sw and ldmx-analysis
#   $2 - docker image repository (defaults to 'dev')
#   $3 - docker image tag        (defaults to 'latest')
###############################################################################

_ldmx_base="$( dirname ${BASH_SOURCE[0]} )/../../" #default backs out of ldmx-sw/scripts
_repo_name="dev" #default repository name: ldmx/_repo_name
_image_tag="latest" #default image tag in repository

if [[ "$1" = *"help" || "$1" == *"-h" ]]
then
    echo "Environment setup script for ldmx."
    echo "  Usage: source ldmx-sw/scripts/ldmx-env.sh [ldmx_base] [repo_name] [image_tag]"
    echo "    ldmx_base : path to directory containing ldmx-sw"
    echo "                (default: $_ldmx_base)"
    echo "    repo_name : name of repo  (ldmx/[repo_name]) to pull container from. "
    echo "                There are three options: 'dev', 'pro', and 'local'"
    echo "                Pass 'local' to use existing, locally built container (default: dev)"
    echo "    image_tag : name of tag of ldmx image to pull down and use (default: latest)"
    echo "                The options you can input depend on the repo:"
    echo "                For repo_name == 'dev': $(ldmx-container-tags "dev")"
    echo "                For repo_name == 'pro': $(ldmx-container-tags "pro")"
    return 0
elif [ ! -z $1 ]
then
    _ldmx_base="$1"
    # first parameter given is there a second?
    if [ ! -z $2 ]
    then
        _repo_name="$2"
        #second parameter given, is there a third?
        if [ ! -z $3 ]
        then
            _image_tag="$3"
        fi
    fi
fi

# this makes sure we get the full path
cd ${_ldmx_base}
export LDMX_BASE=$(pwd)
cd - &> /dev/null

# pull down the container if it doesn't exist on this computer yet
if hash docker &> /dev/null
then
    export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_image_tag}"
    if [ -z $(docker images -q ${LDMX_DOCKER_TAG} 2> /dev/null) ]
    then
        ldmx-container-pull ${_repo_name} ${_image_tag}
    fi
elif hash singularity &> /dev/null 
then
    export LDMX_SINGULARITY_IMG="ldmx_${_repo_name}_${_image_tag}.sif"
    if [ ! -f "${LDMX_BASE}/${LDMX_SINGULARITY_IMG}" ]
    then
        ldmx-container-pull ${_repo_name} ${_image_tag}
    fi
fi
