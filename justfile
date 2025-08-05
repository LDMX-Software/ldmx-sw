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

# install recipe for the CI, private so users know what tools they have on their computers
[private]
install-denv:
    curl -s https://raw.githubusercontent.com/tomeichlersmith/denv/main/install | sh

# prep version file
[private]
prep-version:
    git fetch --tags && git describe --tags | cut -f 1 -d '-' > VERSION

# configure how ldmx-sw will be built
# added ADDITIONAL_WARNINGS and CLANG_TIDY to help improve code quality

# base configure command defining how cmake is called, private so only experts call it
[private]
configure-base *CONFIG: prep-version
    denv cmake -B build -S . {{ CONFIG }}

# default configure of build when developing
configure *CONFIG: (configure-base "-DADDITIONAL_WARNINGS=ON -DENABLE_CLANG_TIDY=ON" CONFIG)

# configure minimal option for faster compilation
configure-quick: configure-base

# configure with Address Sanitizer (ASAN) and  UndefinedBehaviorSanitizer (UBSan)
configure-asan-ubsan: (configure-base "-DENABLE_SANITIZER_UNDEFINED_BEHAVIOR=ON -DENABLE_SANITIZER_ADDRESS=ON")

# This is the same as just configure but reports all (non-3rd-party) warnings as errors
configure-force-error: (configure "-DWARNINGS_AS_ERRORS=ON")

# Use alternative compiler and enable LTO (test compiling only, won't run properly)
configure-clang-lto: (configure "-DENABLE_LTO=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang")

configure-clang-lto-fail-on-warning: (configure "-DENABLE_LTO=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DWARNINGS_AS_ERRORS=ON")

# Keep debug symbols so running with gdb provides more helpful detail
configure-gdb: (configure-base "-DCMAKE_BUILD_TYPE=Debug")

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

# run gdb on a config file
[no-cd]
debug config_py *ARGS:
    denv gdb --args fire {{ config_py }} {{ ARGS }}

# initialize a containerized development environment
init:
    #!/usr/bin/env sh
    set -eu
    denv_major=$(denv version | sed 's/denv v//' | cut -f 1 -d.)
    denv_minor=$(denv version | sed 's/denv v//' | cut -f 2 -d.)
    if [ "${denv_major}" -lt "1" ] || [ "${denv_minor}" -lt "1" ]; then
      # denv v1.0.X or earlier, manually check for workspace
      # which may print a confusing error from denv when no workspace is found
      unset denv_workspace
      # while setting the denv_workspace is helpful for other
      # commands that can assume the denv is already initialized,
      # we need to unset this environment variable to make sure
      # the test is done appropriately.
      # just makes sure this recipe runs from the directory of
      # the justfile so we know we are in the correct location.
      if denv check --workspace --quiet; then
        echo "\033[32mWorkspace already initialized.\033[0m"
      else
        denv init --clean-env --name ldmx ldmx/dev:latest "${LDMX_BASE}"
      fi
    else
      # denv v1.1.0 and later has updated denv init to allow us
      # to avoid overwriting quietly
      denv init --clean-env --no-over --no-mkdir --name ldmx ldmx/dev:latest "${LDMX_BASE}"
    fi
    denv config print

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
    rm -r build install VERSION

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

# Now do the same but with clang tidy
tidy-cpp *ARGS='-p build -quiet --fix':
    #!/usr/bin/env sh
    set -exu
    format_list=$(mktemp)
    git ls-tree -r HEAD --name-only | egrep '(\.h|\.cxx)$'  | grep 'Biasing' > ${format_list}
    denv clang-tidy $(cat ${format_list}) {{ ARGS }}
    rm ${format_list}

# shellcheck doesn't have a "apply-formatting" option
# because it really is more of a tidier (its changes could affect code meaning)
# so only a check is implemented here
#  ISSUE: the filter implemented here gets all files that are either executable
#    or have the '.sh' extension. This includes a python script in TrigScint
#    and some bash-specific scripts as well. Not sure how to handle them.

# check the scripts for common errors and bugs
shellcheck:
    #!/usr/bin/env sh
    set -x
    format_list=$(mktemp)
    git ls-tree -r HEAD | awk '{ if ($1 == 100755 || $4 ~ /\.sh/) print $4 }' \
      > "${format_list}"
    xargs --arg-file="${format_list}" \
      shellcheck --severity style --shell sh
    rm "${format_list}"

# check a script recipe also using shellcheck
shellcheck-recipe RECIPE:
    #!/usr/bin/env sh
    source=$(mktemp)
    just -n {{ RECIPE }} 2> "${source}"
    shellcheck --severity style --shell sh "${source}"
    rm "${source}"

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

# install the dependencies of the plotting module
install-compare-plots-deps:
    denv python3 -m pip install -r ComparePlots/requirements.txt --no-cache --break-system-packages

# alias for install-compare-plots-deps
install-validation: install-compare-plots-deps

# run the ComparePlots plotting module
[no-cd]
compare-plots *args:
    denv 'PYTHONPATH={{ justfile_directory() }}:${PYTHONPATH} python3 -m ComparePlots {{ args }}'
