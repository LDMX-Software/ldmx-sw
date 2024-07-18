# we assume denv is installed and denv has access to a supported container runner

_default:
  @just --list

# configure how ldmx-sw will be built
configure *CONF:
  denv cmake -B build -S . {{CONF}}

# compile and install ldmx-sw
build: configure
  denv cmake --build build --target install -- -j{{num_cpus()}}

# run ldmx-sw with the input configuration script
fire config *ARGS: build
  denv fire {{config}} {{ARGS}}

# initialize a containerized development environment
init:
  #!/usr/bin/env sh
  if ! denv check --workspace --quiet; then
    denv init --clean-env --name ldmx ldmx/dev:latest ..
  fi

# check that the necessary programs for running ldmx-sw are present
check:
  #!/usr/bin/env sh
  if ! command -v denv 2>&1 > /dev/null; then
    echo "ERROR: The program `denv` is not present."
  fi
  # denv can check for container runners it needs
  denv check

alias fmt := format

# format the source code of ldmx-sw
format *ARGS:
  #!/usr/bin/env sh
  git ls-tree -r HEAD --name-only | egrep '(\.h|\.cxx)$' > format.list
  denv clang-format {{ARGS}} $(cat format.list)

# other recipe ideas:
#  production image building

# below are the mimics of ldmx <cmd>

# change which image is used for the denv
use IMAGE:
  denv config image {{IMAGE}}

# make sure the image is pulled down
pull IMAGE:
  denv config image {{IMAGE}} && denv config image pull

# mount a directory into the denv
mount DIR:
  denv config mounts {{DIR}}

# pass an environment variable into the denv
setenv *ENVVAR:
  denv config env copy {{ENVVAR}}

alias compile := build
alias recompAndFire := fire
