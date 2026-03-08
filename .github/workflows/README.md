
Our auto-testing suite is getting pretty bulky, so I've decided to document it here.
GitHub's documentation on their action/workflow service is pretty good,
so I'm linking it here.

- [General Action Docs](https://docs.github.com/en/actions)
- [Events that Trigger Workflows](https://docs.github.com/en/actions/reference/events-that-trigger-workflows)
- [Context and Expression Syntax](https://docs.github.com/en/actions/reference/context-and-expression-syntax-for-github-actions)
- The GitHub CLI [gh](https://github.com/cli/cli/releases) is helpful for some cases of development

## Basic Tests

This is the quick-n-dirty tests that are run on all pushes to all branches.
Here, we simply make sure that we can

1. Compile and Install ldmx-sw using the latest development container
2. Run basic funtionality tests registered with the cmake testing suite

## Build Production Container

This is run on every push to `trunk`, all tags, and can be manually run.

When it is manually run, the user must provide three inputs.

- `branch`: The branch of ldmx-sw to compile into a production container
- `repo`: The DockerHub repository to push the image to
  - This repo needs to have the user `omarmoreno` added as a collaborator,
    so that the GitHub action has access to push images to the repo.
- `tag`: A short-name to call the new version of this image

The image is tagged following the given logic.

- If a tag has been pushed, use the GitHub tag as a Docker tag and include `latest` as a Docker tag.
  - For example: `ldmx/pro:v2.3.0,ldmx/pro:latest`
- If a commit has been pushed, use the git SHA and `edge` as the Docker tags.
  - For example: `ldmx/pro:sha-k2kfhj37,ldmx/pro:edge`
- If the workflow was manually triggered, the Docker tags are the git SHA and the input tag.
  - For example: `<repo>:sha-k2kfhj37,<repo>:<tag>`

## Generate Documentation

All pushes to `trunk` generate documentation which is pushed to `ldmx-software.github.io` for publishing.
Since `sphinx` requires the python modules to be installed for it to effectively generate documentation,
the docs are only generated if the commit pushed to `trunk` successfully compiles and installs.

## PR Validation

These validations are done on pull requests.
They are focused on validating that the developments "match" the latest release on a wide variety of physics distributions.
If the PR contains changes that are meant to alter these distributions, 
the plots generated can also be downloaded and looked through in order to determine that the alterations are only where expected.

The new PR is validated by comparing the generated histograms to the "gold" histograms in the GitHub source tree.
If a pair of histograms fail a KS test (i.e. the `TH1::KolmogorovTest` returns a value less than 0.99), we put that plot
in the "fail" directory of the validation plots package. The `check` action looks into this validation package for _any_
plots in the "fail" directory.

> **Note:** Artifacts are only persisted on 
> [GitHub for 90 days](https://docs.github.com/en/organizations/managing-organization-settings/configuring-the-retention-period-for-github-actions-artifacts-and-logs-in-your-organization),

### Local Equivalence

When validating, this action is roughly equivalent to the following procedure.
(Look at the `.github/actions/validate` directory for the details.)

- Download the latest `ci-data`: `git clone https://github.com/LDMX-Software/ci-data.git`
- Set-up ldmx to use `dev latest`: `just pull ldmx/dev:latest`
- Compile and Install ldmx-sw: `just compile`
- Go to the sample of your choosing: `cd .github/validation_samples/<sample>/`
- Run the configuration: `just setenv LDMX_RUN_NUMBER=1; just setenv LDMX_NUM_EVENTS=10000; just fire config.py`
- Generate comparison plots: `denv python3 ../../actions/validate/compare.py ../../../ci-data/gold.root $(cat ../../../ci-data/label) ../../../ci-data/hist.root <branch>`
  - `<branch>` is your current branch or whatever label you want your developments to be called

## Generate PR Gold Histograms

This action is run when a release is "released", i.e. we only run this action for _actual_ stable releases (no pre-releases).
When this action is run, we run the `validate` action on the samples, copy the histogram and log files into `ci-data`, and then push the updated files to the `ci-data` repository.

**This relies heavily on the naming conventions assumed in the `validate` action, so changes to that action should also be checked here.**

## New Pre-Release

The idea for this action would be to attempt to validate the software in a stand-a-lone manner.
This would require much larger samples than the (relatively quick) PR validation.
The question of what plots to generate and what (if anything) to compare them to is open.

Instead of using the GitHub web interface to create a pre-release, a developer would launch this action 
which would generate validation histograms to look at and upload those histograms to the pre-release for developers to look at.

## Code Formatting

### C++ Formatting
The `apply-clang-tools.yml` workflow runs on all pushes to `trunk` and any pushes to an open pull request.
It checks that the C++ format is respected using clang-format and clang-tidy and, if being run on a PR, 
pushes any format fixes.

### Python Formatting
The `apply-python-tools.yml` workflow runs on all pushes to `trunk` and any pushes to an open pull request.
It formats and lints Python code using Ruff and, if being run on a PR, pushes any fixes.
Python formatting rules are configured in `pyproject.toml`.

### Extra Detail

In order to simplify the action-development process,
I've isolated the running of the LDMX container and the
compiling of ldmx-sw into their own actions.

[This page](https://docs.github.com/en/actions/learn-github-actions/finding-and-customizing-actions#referencing-an-action-in-the-same-repository-where-a-workflow-file-uses-the-action)
was incredibly helpful for this purpose.

