
###############################################################################
# ldmx-env.sh
#   This script is intended to define all the docker aliases required
#   to develop for ldmx-sw. These commands assume that the user
#   1. Has docker engine installed
#   2. Can run docker as a non-root user
###############################################################################

if [ -z $3 ]
then
    _repo_name="dev"
    #this is the name of the dockerhub repository
    #   it shouldn't change
    # except it should when we want a production container
    # also, if it's set to "local", skip downloading at all and attempt to use a locally built container
else
    _repo_name="$3"
fi
export _docker_hub_repo="ldmx/$_repo_name"
#compatibility tweak. we could also suggest that all local builds are named with prefix convention ldmx/local: instead of ldmx/dev:
if [ ${_repo_name} = "local" ]
then
    export _docker_hub_repo="ldmx/dev"
fi
echo $_docker_hub_repo
   

###############################################################################
# Get the docker tags for the repository
#   Taken from https://stackoverflow.com/a/39454426
###############################################################################
function ldmx_docker_tags() {
    wget -q https://registry.hub.docker.com/v1/repositories/${_docker_hub_repo}/tags -O -  | sed -e 's/[][]//g' -e 's/"//g' -e 's/ //g' | tr '}' '\n'  | awk -F: '{print $3}'
}

###############################################################################
# Parse CLI Arguments
#   $1 - base directory for your ldmx work
#        must be directory containing ldmx-sw and ldmx-analysis
#   $2 - docker image tag (defaults to 'latest')
###############################################################################

_ldmx_base="$1"
if [ -z ${_ldmx_base} ]; then
    _dir_name_of_script=$( dirname ${BASH_SOURCE[0]} )
    _ldmx_base="${_dir_name_of_script}/../../" #back out of ldmx-sw/scripts
elif [[ "${_ldmx_base}" = *"help" || "${_ldmx_base}" == *"-h" ]]; then
    echo "Environment setup script for ldmx."
    echo "  Usage: source ldmx-sw/scripts/ldmx-env.sh [ldmx_base] [image_tag] [repo-name]"
    echo "    ldmx_base : path to directory containing ldmx-sw (default: present working directory)"
    echo "    image_tag : name of tag of ldmx/dev image to pull down and use (default: latest)"
    echo "    repo_name : name of repo  (ldmx/[repo_name]) to pull container from. "
    echo "                Pass 'local' to use existing, locally built container (default: dev)"
    echo "The image_tag options you can input are:"
    echo $(ldmx_docker_tags)
    return 0
fi

# this makes sure we get the full path
cd ${_ldmx_base}
export LDMX_BASE=$(pwd)
cd - &> /dev/null

_dock_image="$2"
if [ -z ${_dock_image} ]; then
    _dock_image="latest"
fi
export LDMX_DOCKER_TAG="${_docker_hub_repo}:${_dock_image}"

###############################################################################
# Make sure we have the latest docker container
###############################################################################
if hash docker &>/dev/null; then
    if [ "${_repo_name}" == "local" ]
    then
	echo "will assume locally built container: $_dock_image "
    else
	docker pull ${LDMX_DOCKER_TAG}
    fi
elif hash singularity &>/dev/null;  then
    # change cache directory to be inside ldmx base directory
    export SINGULARITY_CACHEDIR=${LDMX_BASE}/.singularity
    mkdir -p ${SINGULARITY_CACHEDIR} #make sure cache directory exists

    # name the singularity image after the tag the user asked for
    export LDMX_SINGULARITY_IMG=ldmx_${_repo_name}_${_dock_image}.sif

    # build the docker container into the singularity image
    #   will prompt the user if the image already exists
    singularity build ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} docker://${LDMX_DOCKER_TAG}
else
    echo "You don't have docker or singularity installed!"
    return 1
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

