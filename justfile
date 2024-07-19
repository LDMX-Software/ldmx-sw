# Developer Notes
#   If you are looking at this file there are a few helpful things to note.
#   - `@` is used to alter what `just` chooses to print.
#     It can largely be ignored during development.
#   - Double curly braces `{{...}}` are used for evaluating `just` variables and functions
#   - By default, these recipes are run from the directory of this file.
#     This can be changed but is very helpful for us.
#   - `just --fmt --unstable` is used to apply the canonical justfile format

_default:
    @just --list --justfile {{ justfile() }}

# configure how ldmx-sw will be built
configure *CONFIG:
    denv cmake -B build -S . {{ CONFIG }}

# compile and install ldmx-sw
build ncpu=num_cpus() *CONFIG="": (configure CONFIG)
    denv cmake --build build --target install -- -j{{ ncpu }}

# run the ldmx-sw tests
test *ARGS: build
    cd build && denv ctest {{ ARGS }}

# run ldmx-sw with the input configuration script
fire config *ARGS: build
    denv fire {{ config }} {{ ARGS }}

# initialize a containerized development environment
init:
    #!/usr/bin/env sh
    if denv check --workspace --quiet; then
      echo "\033[32mWorkspace already initialized.\033[0m"
      denv config print
    else
      denv init --clean-env --name ldmx ldmx/dev:latest ..
    fi

# check that the necessary programs for running ldmx-sw are present
check:
    #!/usr/bin/env sh
    if ! command -v denv 2>&1 > /dev/null; then
      echo "\033[31mThe program 'denv' is not present.\033[0m"
      exit 1
    else
      echo "\033[32m'denv' has been found.\033[0m"
    fi
    # denv can check for container runners it needs
    denv check

# remove the build and install directories of ldmx-sw
clean:
    rm -r build install

alias fmt := format

# format the ldmx-sw source code
format: format-cpp

# format the C++ source code of ldmx-sw
format-cpp *ARGS='-i':
    #!/usr/bin/env sh
    set -eu
    format_list=$(mktemp)
    git ls-tree -r HEAD --name-only | egrep '(\.h|\.cxx)$' > ${format_list}
    denv clang-format {{ ARGS }} $(cat ${format_list})
    rm ${format_list}

# other recipe ideas:
# - production image building
# - testing
# - run test config scripts
# - format python
# below are the mimics of ldmx <cmd>

# change which image is used for the denv
use IMAGE:
    denv config image {{ IMAGE }}

# make sure the image is pulled down
pull IMAGE:
    denv config image {{ IMAGE }} && denv config image pull

# mount a directory into the denv
mount DIR:
    denv config mounts {{ DIR }}

# pass an environment variable into the denv
setenv +ENVVAR:
    denv config env copy {{ ENVVAR }}

alias compile := build
alias recompAndFire := fire
