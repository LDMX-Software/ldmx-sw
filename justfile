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

help_message := "shared recipes for ldmx-sw development

  USAGE:
    just <cmd> [arguments...]

  Multiple commands can be provided at once if they don't require arguments.

    just init configure-force-error build

  COMMANDS:
"

# deduce the denv_workspace corresponding to this justfile
# usually, denv deduces where the workspace is on its own by finding the .denv directory,
# but we want to set where the denv is within the some recipes so users could (for example)
# run their ldmx-sw build from within some other denv by invoking fire from just
#   just -f path/to/ldmx-sw/justfile fire config.py
# or
#   just path/to/ldmx-sw/fire config.py
# would run this denv even if there is a denv in the directory where config.py is because
# those [no-cd] recipes set denv_workspace="{{ this_denv_workspace }}"
# supporting either the old (parent_directory, LDMX_BASE path) and the new (ldmx-sw itself)
# forces a decision to be made now when just is invoked
# we default to the new path (ldmx-sw itself) for new invocations while supporting the
# old location only if it exists.

denv_workspace_in_ldmx_sw_parent := path_exists(parent_directory(justfile_directory()) / ".denv")
this_denv_workspace := if denv_workspace_in_ldmx_sw_parent == "true" {
  parent_directory(justfile_directory())
} else {
  justfile_directory()
}

# make sure APPTAINER_CACHEDIR is not in the home directory
# unless the user has already defined it
#   just 1.15

export APPTAINER_CACHEDIR := env("APPTAINER_CACHEDIR", this_denv_workspace / ".apptainer")

_default:
    @just --list --justfile {{ justfile() }} --list-heading "{{ help_message }}"

# install recipe for the CI, private so users know what tools they have on their computers
[private]
install-denv:
    curl -s https://raw.githubusercontent.com/tomeichlersmith/denv/main/install | sh

# check a validation plots archive for failures
[no-cd]
check-validation archive:
    #!/usr/bin/env bash
    set -e
    source "{{ justfile_directory() }}/.github/actions/common.sh"
    _archive="{{ archive }}"
    rc=0
    _failed_plots=($(tar -tf ${_archive} | grep "fail/.*.pdf")) || true
    if [[ ${#_failed_plots[@]} -gt 0 ]]; then
      error ${#_failed_plots[@]} plots failed the KS test against gold.
      start_group List of Plots Failing KS Test
      for p in ${_failed_plots[@]}; do
        echo $(basename ${p})
      done
      end_group
      rc=1
    fi
    # unpack the logs so we can compare them
    tar xzf ${_archive} gold.log output.log
    # use sed replace (by blank) to run the diff without the initial HH:MM:SS timestamp
    if ! diff  -I '^#' <(sed -e 's/^[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}/ /g' gold.log) <(sed -e 's/^[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}/ /g' output.log) > log.diff; then
      # do not error out (don't set rc here) if diff is non-zero, the timestamps printed out
      # by some processors prevent a full text diff so we do the character
      # count check below to look for big changes
      _n_diff_lines=$(grep -c '^[<>]' log.diff || echo 0)
      warn "Text Differences Between Logs (${_n_diff_lines} lines differ)"
      start_group diff gold.log output.log
      cat log.diff
      end_group
    fi
    # check character count of logs, allowing up to 0.5% difference to avoid
    # false positives from minor run-to-run output variations
    ngold=$(wc --chars gold.log | cut -f 1 -d ' ')
    nnew=$(wc --chars output.log | cut -f 1 -d ' ')
    if (( ngold != nnew )); then
      if (( ngold > 0 )); then
        char_diff_pct=$(( (nnew - ngold) * 100 / ngold ))
        char_abs_pct=${char_diff_pct#-}
        if (( char_abs_pct > 0 )); then
          error "Log character count differs by ${char_abs_pct}%: gold=${ngold}, new=${nnew}"
          rc=1
        else
          warn "Log character count differs by ${char_abs_pct}% (gold=${ngold}, new=${nnew}); within tolerance"
        fi
      fi
    fi
    # compare total wall-clock run time if gold.time is available in the archive
    # gold.time has one line per sample: "<sample> <seconds>"
    # timing.txt has one line for this job: "<sample> <seconds>"
    _timing_tolerance=10
    if tar -tf ${_archive} gold.time timing.txt &>/dev/null 2>&1; then
      tar xzf ${_archive} gold.time timing.txt
      read _sample new_s < timing.txt
      gold_s=$(awk -v s="${_sample}" '$1 == s {v=$2} END {print v}' gold.time)
      if [[ -z "${gold_s}" ]]; then
        warn "No gold timing entry for sample '${_sample}'; skipping timing check."
      else
        echo "Timing for ${_sample}: gold=${gold_s}s, new=${new_s}s"
        if (( gold_s > 0 )); then
          timing_diff_pct=$(( (new_s - gold_s) * 100 / gold_s ))
          timing_abs_pct=${timing_diff_pct#-}
          if (( timing_abs_pct > _timing_tolerance )); then
            if (( new_s > gold_s )); then
              warn "Timing regression for ${_sample}: new=${new_s}s vs gold=${gold_s}s (+${timing_diff_pct}%, tolerance ${_timing_tolerance}%)"
            else
              warn "Timing anomaly for ${_sample}: new=${new_s}s vs gold=${gold_s}s (${timing_diff_pct}%, tolerance ${_timing_tolerance}%)"
            fi
          else
            echo "Timing within ${timing_abs_pct}% of gold (tolerance ${_timing_tolerance}%)"
          fi
        fi
      fi
    else
      warn "No gold.time found; skipping timing check. Re-generate gold data to enable timing comparisons."
    fi
    exit ${rc}

# run a validation sample and optionally compare against golden histograms
[no-cd]
[private]
run-validation sample no_comp='false':
    #!/usr/bin/env bash
    set -e
    set -o pipefail
    export GITHUB_WORKSPACE="${GITHUB_WORKSPACE:-{{ justfile_directory() }}}"
    source "{{ justfile_directory() }}/.github/actions/common.sh"
    _sample="{{ sample }}"
    _no_comp="{{ no_comp }}"
    _ref_dir="${CI_DATA}/${_sample}"
    start_group Input Deduction
    cd ${GITHUB_WORKSPACE}/.github/validation_samples/${_sample} || exit $?
    _sample_dir="$(pwd)"
    echo "Ref Dir: ${_ref_dir}"
    echo "Sample Name: ${_sample}"
    echo "Sample Dir: ${_sample_dir}"
    echo "Not Running Comparison? ${_no_comp}"
    denv config env copy LDMX_NUM_EVENTS LDMX_RUN_NUMBER LDMX_LOG_LEVEL CI_DATA
    end_group
    start_group Sample-Specific Initialization
    if [[ -f init.sh ]]; then
      . init.sh
    else
      echo "No 'init.sh' file in ${_sample_dir}."
    fi
    end_group
    # assume sample directory has its config called 'config.py'
    start_group Run config.py
    _t0=${SECONDS}
    denv fire config.py | tee output.log || exit $?
    echo "${_sample} $(( SECONDS - _t0 ))" > timing.txt
    end_group
    start_group Compare to Golden Histograms
    if [[ "${_no_comp}" == "false" ]]; then
      # assume sample directory has its gold histogram called 'gold.root'
      #   compare has 4 CLI inputs:
      #    gold_f, gold_label, test_f, test_label
      denv python3 {{ justfile_directory() }}/.github/actions/validate/compare.py \
        ${_ref_dir}/gold.root \
        $(cat ${CI_DATA}/label) \
        hist.root \
        ${HEAD_REF} \
        || exit $?
      # print log diff into output directory
      cp -t ${_sample_dir}/plots ${_ref_dir}/gold.log output.log timing.txt || exit $?
      # include the shared gold timing reference if it exists
      if [[ -f ${CI_DATA}/gold.time ]]; then
        cp ${CI_DATA}/gold.time ${_sample_dir}/plots/
      fi
      # compare.py puts plots into the plots/ directory
      #   Package them up for upload
      cd ${_sample_dir}/plots || exit $?
      tar -czf ${_sample}_recon_validation_plots.tar.gz * || exit $?
    else
      echo "Not running comparison script."
    fi
    end_group
    # Share paths to plot archive
    start_group Share Paths to Outputs
    if [[ "${_no_comp}" == "false" ]]; then
      set_output plots $(pwd)/${_sample}_recon_validation_plots.tar.gz
    fi
    set_output log ${_sample_dir}/output.log
    set_output timing ${_sample_dir}/timing.txt
    set_output hists ${_sample_dir}/hist.root
    set_output events ${_sample_dir}/events.root
    end_group

# prep version file
[private]
prep-version:
    git fetch --tags && git describe --tags | cut -f 1 -d '-' > VERSION
    git rev-parse HEAD > COMMIT_SHA

# configure how ldmx-sw will be built
# added ADDITIONAL_WARNINGS and CLANG_TIDY to help improve code quality

# base configure command defining how cmake is called, private so only experts call it
[private]
configure-base *CONFIG: prep-version
    denv cmake -B build -S . {{ CONFIG }}

# default configure of build when developing
configure *CONFIG: (configure-base "-DADDITIONAL_WARNINGS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON" CONFIG)

# configure minimal option for faster compilation
configure-quick: configure-base

# configure with Address Sanitizer (ASAN) and  UndefinedBehaviorSanitizer (UBSan)
configure-asan-ubsan: (configure-base "-DENABLE_SANITIZER_UNDEFINED_BEHAVIOR=ON -DENABLE_SANITIZER_ADDRESS=ON")

# configure with clang-tidy ON
configure-clang-tidy:  (configure-base "-DENABLE_CLANG_TIDY=ON")

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
    denv_workspace="{{ this_denv_workspace }}" denv fire {{ config_py }} {{ ARGS }}

# multiple runs of ldmx-sw fire with same input configuration script
[no-cd]
fire-parallel config_py *ARGS:
    denv_workspace="{{ this_denv_workspace }}" denv fire-parallel {{ config_py }} {{ ARGS }}

# run gdb on a config file
[no-cd]
debug config_py *ARGS:
    denv_workspace="{{ this_denv_workspace }}" denv gdb --args fire {{ config_py }} {{ ARGS }}

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
        denv init --clean-env --name ldmx ldmx/dev:latest
      fi
    else
      # denv v1.1.0 and later has updated denv init to allow us
      # to avoid overwriting quietly
      denv init --clean-env --no-over --no-mkdir --name ldmx ldmx/dev:latest
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
    rm -r build install VERSION COMMIT_SHA

# format the ldmx-sw source code
format: format-cpp-all format-just

_clang-tool-impl file_list_cmd *tool_cmd_and_args:
    #!/usr/bin/env sh
    set -eu
    cpp_list="$(mktemp)"
    if ! {{ file_list_cmd }} | grep -E '\.(h|cxx)$' > "${cpp_list}"; then
      echo "no C++ files (extensions .h and .cxx) to apply {{ tool_cmd_and_args }}"
    else
      xargs --arg-file="${cpp_list}" denv {{ tool_cmd_and_args }}
    fi
    rm "${cpp_list}"


# format the C++ source code of ldmx-sw
format-cpp-all *ARGS='-i': (_clang-tool-impl "git ls-files | grep -v HLS_Arbitrary_Precision_Types" "clang-format" ARGS)

# formatting is quick enough that the format-cpp shortcut can be used
format-cpp: format-cpp-all

# format only the C++ files that have changed relative to trunk
format-cpp-diff *args='-i': (_clang-tool-impl "git diff --name-only --diff-filter=d origin/trunk | grep -v HLS_Arbitrary_Precision_Types" "clang-format" args)

# format the Python source code
format-python:
    denv ruff format

# format only the Python files that have changed relative to trunk
format-python-diff:
    #!/usr/bin/env sh
    set -eu
    py_list="$(mktemp)"
    if ! git diff --name-only --diff-filter=d origin/trunk | grep -E '\.py$' > "${py_list}"; then
      echo "no Python files to format"
    else
      xargs --arg-file="${py_list}" denv ruff format
    fi
    rm "${py_list}"

# lint the Python source code with ruff
lint-python:
    denv ruff check

# lint and auto-fix the Python source code
lint-python-fix:
    denv ruff check --fix

# format the justfile
format-just:
    @just --fmt --unstable --justfile {{ justfile() }}

default_tidy_args := '-p build --fix -fix-errors --quiet'

# tidy all C++ files of ldmx-sw
tidy-cpp-all *args=default_tidy_args: (_clang-tool-impl "git ls-files | grep -v HLS_Arbitrary_Precision_Types" "clang-tidy" args)

# tidy C++ files that are different relative to trunk
tidy-cpp-diff *args=default_tidy_args: (_clang-tool-impl "git diff --name-only --diff-filter=d origin/trunk | grep -v HLS_Arbitrary_Precision_Types" "clang-tidy" args)

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

# execute g4-vis
g4-vis gdml_file macro_file="":
    denv g4-vis {{gdml_file}} {{macro_file}}

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

# configure and build ldmx-sw the quick way
compile-quick ncpu=num_cpus() *CONFIG='': (configure-quick) (build ncpu)

# re-build ldmx-sw and then run a config
recompFire config_py *ARGS: compile (fire config_py ARGS)

# print out the environment configuration
print-config:
    denv config print

# install the dependencies of the plotting module
install-compare-plots-deps:
    denv python3 -m pip install -r ComparePlots/requirements.txt --no-cache --break-system-packages

# alias for install-compare-plots-deps
install-validation: install-compare-plots-deps

# run the ComparePlots plotting module
[no-cd]
compare-plots *args:
    denv_workspace="{{ this_denv_workspace }}" denv 'PYTHONPATH="{{ justfile_directory() }}:${PYTHONPATH}" python3 -m ComparePlots {{ args }}'
