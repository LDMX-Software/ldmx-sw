
###############################################################################
# ldmx-env.sh
#   This script is intended to define all the docker aliases required
#   to develop for ldmx-sw. These commands assume that the user
#   1. Has docker engine installed
#   2. Can run docker as a non-root user
###############################################################################

###############################################################################
# Parse CLI Arguments
#   $1 - base directory for your ldmx work
#        must be directory containing ldmx-sw and ldmx-analysis
#   $2 - docker image tag (defaults to 'latest')
###############################################################################

_ldmx_base="$1"
if [ -z ${_ldmx_base} ]; then
    _ldmx_base=$(pwd)
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

#this is the name of the dockerhub repository
#   it shouldn't change
export _docker_hub_repo="ldmx/dev"

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
alias ldmx='docker run -i -e LDMX_BASE -v $LDMX_BASE:$LDMX_BASE ${_docker_hub_repo}:$LDMX_DOCKER_TAG $(pwd)'

