
###############################################################################
# ldmx-env.sh
#   This script is intended to define all the docker aliases required
#   to develop for ldmx-sw. These commands assume that the user
#   1. Has docker engine installed
#   2. Can run docker as a non-root user
###############################################################################

#this is the name of the dockerhub repository
#   it shouldn't change
export _docker_hub_repo="ldmx/dev"

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
elif [ ${_ldmx_base} = "help" ]; then
    echo "Environment setup script for ldmx."
    echo "  Usage: source ldmx-sw/scripts/ldmx-env.sh [ldmx_base] [image_tag]"
    echo "    ldmx_base : path to directory containing ldmx-sw (default: present working directory)"
    echo "    image_tag : name of tag of ldmx/dev image to pull down and use (default: latest)"
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
export LDMX_DOCKER_TAG=${_dock_image}

###############################################################################
# Make sure we have the latest docker container
###############################################################################
docker pull ${_docker_hub_repo}:${LDMX_DOCKER_TAG}

###############################################################################
# ldmx
#   Launch the user into the ldmx docker container environment, passing
#   any argument along as a command to be executed.
#
#   The container is set up so that it goes to the first argument and then
#   executes the rest. This alias sets the first argument to $(pwd) so that
#   the container runs the users command from the same place that the user
#   intended.
###############################################################################
alias ldmx='docker run -i -e LDMX_BASE -v $LDMX_BASE:$LDMX_BASE -u $(id -u ${USER}):$(id -g ${USER}) ${_docker_hub_repo}:$LDMX_DOCKER_TAG $(pwd)'

