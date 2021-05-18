# setup Action

A custom setup action for starting an ldmx-sw workflow.

After this action is run, further calls using the `ldmx` action can be done to use this compiled version of ldmx-sw to do things.

## Inputs

- `image`: **required** 
  - The ldmx-style container to run inside of
  - Default (and example) : `ldmx/dev:latest`

## Pre-requisites

Whatever branch of ldmx-sw you want to `setup` must be already checked-out.

```yaml
  - uses: actions/checkout@v2
    with:
      submodules: 'recursive'
      ref: <your-branch-or-leave-off>
```
