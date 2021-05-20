
Our auto-testing suite is getting pretty bulky, so I've decided to document it here.
GitHub's documentation on their action/workflow service is pretty good,
so I'm linking it [here](https://docs.github.com/en/actions).

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

## Generate Golden Recon Histograms

All pushes to `trunk` re-generate the "golden" recon histograms that are used in the Recon Validation action.
These histograms are uploaded as artifacts to GitHub which are then available to be downloaded by other actions.

> **Note:** Artifacts are only persisted on 
> [GitHub for 90 days](https://docs.github.com/en/organizations/managing-organization-settings/configuring-the-retention-period-for-github-actions-artifacts-and-logs-in-your-organization),
> so we may need to re-design when this generation is triggered if they are being removed often.

## Recon Validation

These validations are done on pull requests or launched manually.
They are focused on validating that the reconstruction procedure "matches" what is on `trunk`.
If the PR contains changes that are meant to alter the reconstruction, the plots generated can also be downloaded and looked through in order to determine that the alterations are only where expected.

In this test, the simulations **are not** being validated.
They are merely there as a method for generating a wide variety of hits that need to be successfully handled by our reconstruction pipeline.

## Deep Validation

**To be developed**

The idea for this action would be to attempt to validate the simulation (both physics and detector design).
The question of what plots to generate and what (if anything) to compare them to is open.
