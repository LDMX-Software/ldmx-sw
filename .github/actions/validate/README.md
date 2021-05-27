# validate Action

An action to validate the specified sample.

## Assumptions

Each "sample" is a subdirectory of `.github/validation_samples` with the following structure.

- `.github/validation_samples/`
  - `<sample-name>/`
    - `config.py`
    - `gold.root`

where `config.py` generates the _histogram file_ `hist.root` 
when run with ldmx-sw. `gold.root` is a copy of `hist.root` when `config.py`
was generated with a "verified" version of ldmx-sw (whatever that means).

We also assume that the `LDMX_DOCKER_TAG` environment variable is set to docker tag
that should be used to run the installation of ldmx-sw. This variable should be set
in the workflow or job that uses this action.

Finally, we assume that the branch we wish to validate is already built and installed.
This can be done using the `setup` action if a build package artifact is not available.

## Inputs

- `sample`: **required** 
  - The name of the sample to validate (i.e. the subdirectory name `<sample-name>`)
  - For example: `inclusive`

