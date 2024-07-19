# Developer Notes
#   If you are looking at this file there are a few helpful things to note.
#   - `@` is used to alter what `just` chooses to print.
#     It can largely be ignored during development and inserted after when tuning the UI.
#   - Double curly braces `{{...}}` are used for evaluating `just` variables and functions
#   - By default, these recipes are run from the directory of this file.
#     This can be changed but is helpful for us.
#   - `just --fmt --unstable` is used to apply the canonical justfile format
#   - just does support splitting recipes across multiple files, but
#     should be avoided for our use case to keep it as a single reference.
#
# other recipe ideas:
#   - production image building
#   - format python

help_message := "shared recipes for ldmx-sw development

    Some folks use 'ldmx' as an alias for 'just' in which case you can
    replace 'just' with 'ldmx' in the examples below.

  USAGE:
    just <cmd> [arguments...]

  Multiple commands can be provided at once and they will be run in sequence.

    just configure build test

  COMMANDS:
"

_default:
    @just --list --justfile {{ justfile() }} --list-heading "{{ help_message }}"

# configure how ldmx-sw will be built
configure *CONFIG:
    denv cmake -B build -S . {{ CONFIG }}

# compile and install ldmx-sw
build ncpu=num_cpus():
    denv cmake --build build --target install -- -j{{ ncpu }}

# run the ldmx-sw tests
test *ARGS:
    cd build && denv ctest {{ ARGS }}

# run ldmx-sw with the input configuration script
fire config_py *ARGS:
    denv fire {{ config_py }} {{ ARGS }}

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

compile ncpu=num_cpus() *CONFIG='': (configure CONFIG) (build ncpu)

recompAndFire config_py *ARGS: build (fire config_py ARGS)
