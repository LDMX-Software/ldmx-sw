# validate Action

An action to validate the specified sample.

## Assumptions

Each "sample" is a subdirectory of `.github/validation_samples` with the following structure.

- `.github/validation_samples/`
  - `<sample-name>/`
    - `config.py`
    - `gold.root`
    - `init.sh` (optional)

### config.py

This is the ldmx-sw configuration that will be run;
we assume the following about it.

- it generates the histogram file `hist.root` in the current directory
- it generates its events file `events.root` in the current directory
- it should get its run number and number of events from the environment variables
```python
import os
p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])
```
This just makes passing this information to the configuration easier when using GitHub workflows.

### gold.root
`gold.root` is a copy of `hist.root` when `config.py`
was generated with a "verified" version of ldmx-sw (whatever that means).
When introducing a new sample for validation, the user will probably need to manually commit
this `gold.root` file the first time.
Afterwards, the gold-generation workflow is run on new stable releases.

### init.sh
The `init.sh` file is an optional bash script that is run before `config.py` is run. 
This is here for sample-specific setup for example generating events for the overlay producer to use. 
Look at the `common.sh` bash script for the bash functions available to you.

### Other Assumptions

We also assume that the `LDMX_DOCKER_TAG` environment variable is set to docker tag
that should be used to run the installation of ldmx-sw. This variable should be set
in the workflow or job that uses this action.

Finally, we assume that the branch we wish to validate is already built and installed.
This can be done using the `setup` action if a build package artifact is not available.

## Inputs

- `sample`: **required** 
  - The name of the sample to validate (i.e. the subdirectory name `<sample-name>`)
  - For example: `inclusive`
- `no_comp`: optional
  - Should the comparison script be done?
  - Default: true
