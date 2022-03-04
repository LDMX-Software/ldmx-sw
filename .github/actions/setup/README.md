# setup Action

A custom setup action for starting an ldmx-sw workflow.

After this action is run, further calls using the `ldmx` action can be done to use this compiled version of ldmx-sw to do things.

The image used to compile ldmx-sw is stored in the `LDMX_DOCKER_TAG` environment variable.
This variable should be defined in the workflow or job that is using this action.

We also checkout the golden histograms (`gold.root`) from `trunk` so this branch is compared against the most recent reference histograms.

## Pre-requisites

Whatever branch of ldmx-sw you want to `setup` must be already checked-out.

```yaml
  - uses: actions/checkout@v2
    with:
      submodules: 'recursive'
      ref: <your-branch-or-leave-off>
```
