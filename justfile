# Developer Notes
#   If you are looking at this file there are a few helpful things to note.
#   - `@` is used to alter what `just` chooses to print.
#     It can largely be ignored during development and inserted after when tuning the UI.
#   - Double curly braces `{{...}}` are used for evaluating `just` variables and functions
#   - By default, these recipes are run from the directory of this file.
#     This can be changed but is helpful for us in most recipes.
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

    just init configure build test

  COMMANDS:
"

# inherited from ldmx-env bash functions
# we could look into removing this and instead having the denv_workspace be
# the justfile_directory() itself but that is a larger change than introducing just
# the denv workspace is colloquially known as LDMX_BASE

export LDMX_BASE := parent_directory(justfile_directory())

# tell denv where the workspace is
# usually, denv deduces where the workspace is by finding the .denv directory,
# but we want to set where the denv is within the justfile so users could (for example)
# run their ldmx-sw build from within some other denv by invoking fire from just
#   just -f path/to/ldmx-sw/justfile fire config.py
# would run this denv even if there is a denv in the directory where config.py is.

export denv_workspace := LDMX_BASE

# make sure APPTAINER_CACHEDIR is not in the home directory
# unless the user has already defined it
#   just 1.15

export APPTAINER_CACHEDIR := env("APPTAINER_CACHEDIR", LDMX_BASE / ".apptainer")

_default:
    @just --list --justfile {{ justfile() }} --list-heading "{{ help_message }}"

# this install is private since I'd prefer users knowing what tools they are installing;

# however, the CI needs to install denv before it can run any testing
[private]
install-denv:
    curl -s https://raw.githubusercontent.com/tomeichlersmith/denv/main/install | sh

# configure how ldmx-sw will be built
configure *CONFIG:
    denv cmake -B build -S .  -DADDITIONAL_WARNINGS=ON  -DENABLE_CLANG_TIDY=ON  {{ CONFIG }}

configure-force-error *CONFIG:
    just  configure -DWARNINGS_AS_ERRORS=ON {{ CONFIG }}

configure-clang-lto:
    denv cmake -B build -S . -DADDITIONAL_WARNINGS=ON  -DENABLE_CLANG_TIDY=ON  -DENABLE_LTO=ON  -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang 

# compile and install ldmx-sw
build ncpu=num_cpus():
    denv cmake --build build --target install -- -j{{ ncpu }}

# run the ldmx-sw tests
test *ARGS:
    cd build && denv ctest {{ ARGS }}

# run ldmx-sw with the input configuration script
[no-cd]
fire config_py *ARGS:
    denv fire {{ config_py }} {{ ARGS }}

# initialize a containerized development environment
init:
    #!/usr/bin/env sh
    # while setting the denv_workspace is helpful for other
    # commands that can assume the denv is already initialized,
    # we need to unset this environment variable to make sure
    # the test is done appropriately.
    # just makes sure this recipe runs from the directory of
    # the justfile so we know we are in the correct location.
    unset denv_workspace
    if denv check --workspace --quiet; then
      echo "\033[32mWorkspace already initialized.\033[0m"
      denv config print
    else
      denv init --clean-env --name ldmx ldmx/dev:latest ${LDMX_BASE}
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

# confirm(PROMPT) just 1.23

# remove the build and install directories of ldmx-sw
[confirm("This will remove the build and install directories. Are you sure?")]
clean:
    rm -r build install

# format the ldmx-sw source code
format: format-cpp format-just

# format the C++ source code of ldmx-sw
format-cpp *ARGS='-i':
    #!/usr/bin/env sh
    set -exu
    format_list=$(mktemp)
    git ls-tree -r HEAD --name-only | egrep '(\.h|\.cxx)$' > ${format_list}
    denv clang-format {{ ARGS }} $(cat ${format_list})
    rm ${format_list}

# format the justfile
format-just:
    @just --fmt --unstable --justfile {{ justfile() }}

# shellcheck doesn't have a "apply-formatting" option
# because it really is more of a tidier (its changes could affect code meaning)
# so only a check is implemented here
#  ISSUE: the filter implemented here gets all files that are either executable
#    or have the '.sh' extension. This includes a python script in TrigScint
#    and some bash-specific scripts as well. Not sure how to handle them.

# check the scripts for common errors and bugs
shellcheck:
    #!/usr/bin/env sh
    set -exu
    format_list=$(mktemp)
    git ls-tree -r HEAD | awk '{ if ($1 == 100755 || $4 ~ /\.sh/) print $4 }' > ${format_list}
    shellcheck --severity style --shell sh $(cat ${format_list})

# below are the mimics of ldmx <cmd>
# we could think about removing them if folks are happy with committing to the
# just-style commands above

# open the ROOT shell within the software environment
root *ARGS="":
    denv root {{ ARGS }}

# open a ROOT file with a graphical browser
rootbrowse FILE:
    denv rootbrowse {{ FILE }}

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

# configure and build ldmx-sw
compile ncpu=num_cpus() *CONFIG='': (configure CONFIG) (build ncpu)

# re-build ldmx-sw and then run a config
recompFire config_py *ARGS: compile (fire config_py ARGS)

# install the validation module
# `python3 -m pip install Validation/` is the standard `pip` install method.
# We add `--upgrade` to tell `pip` it should overwrite the package if it already has been
# # installed before which is helpful in the case where someone is updating the code and running
# # the new code within the container. The `--target install/python/` arguments tell `pip`
# # where to install the package. This directory is where we currently store our python modules
# # and is where the container expects them to be. The `--no-cache` argument tells `pip` to
# # not use a cache for downloading any dependencies from the internet which is necessary since
# # `pip` will not be able to write to the cache location within the container.
# # install the python Validation plotting module
install-validation:
    denv python3 -m pip install Validation/ --upgrade --target install/python/ --no-cache
