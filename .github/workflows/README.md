
Our auto-testing suite is getting pretty bulky, so I've decided to document it here.
GitHub's documentation on their action/workflow service is pretty good,
so I'm linking it here.

- [General Action Docs](https://docs.github.com/en/actions)
- [Events that Trigger Workflows](https://docs.github.com/en/actions/reference/events-that-trigger-workflows)
- [Context and Expression Syntax](https://docs.github.com/en/actions/reference/context-and-expression-syntax-for-github-actions)

## Basic Tests

This is the quick-n-dirty tests that are run on all pushes to all branches.
Here, we simply make sure that we can

1. Compile and Install ldmx-sw using the latest development container
2. Run basic funtionality tests registered with the cmake testing suite

## Generate Production Container

This is run on every push to `trunk`, all tags, and can be manually run.

When it is manually run, the user must provide three inputs.

- `branch`: The branch of ldmx-sw to compile into a production container
- `repo`: The DockerHub repository to push the image to
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

## Recon Validation

These validations are done on pull requests.
They are focused on validating that the reconstruction procedure "matches" what is on `trunk`.
If the PR contains changes that are meant to alter the reconstruction, 
the plots generated can also be downloaded and looked through in order to determine that the alterations are only where expected.

In this test, the simulations **are not** being validated.
They are merely there as a method for generating a wide variety of hits that need to be successfully handled by our reconstruction pipeline.
While we aren't directly attempting to validate the simulations,
simulation plots are still produced to help debug any potential discrepancies that are observed.

> **Note:** Artifacts are only persisted on 
> [GitHub for 90 days](https://docs.github.com/en/organizations/managing-organization-settings/configuring-the-retention-period-for-github-actions-artifacts-and-logs-in-your-organization),

### Local Equivalence

When validating, this action is roughly equivalent to the following procedure.

- Set-up ldmx to use `dev latest`: `ldmx pull dev latest`
- Compile and Install ldmx-sw: `mkdir build; cd build; ldmx 'cmake .. && make install'`
- Go to workflows directory: `cd ../.github/workflows`
- Run one of the configs: `ldmx fire configs/<sample_id>.py`
- Compare to golden histograms: `ldmx python3 compare.py <sample_id>`

Since the golden histograms are generated from `ldmx/edge:pro` (and cached for future runs)
the simplest way to generate your own golden histograms locally is to do re-run the configs you wish to compare
with that production image and move the output histograms into the `.github/workflows/gold` directory.

## Deep Validation

**To be developed**

The idea for this action would be to attempt to validate the simulation (both physics and detector design).
The question of what plots to generate and what (if anything) to compare them to is open.

### Extra Detail

In order to simplify the action-development process,
I've isolated the running of the LDMX container and the
compiling of ldmx-sw into their own actions.

[This page](https://docs.github.com/en/actions/learn-github-actions/finding-and-customizing-actions#referencing-an-action-in-the-same-repository-where-a-workflow-file-uses-the-action)
was incredibly helpful for this purpose.

