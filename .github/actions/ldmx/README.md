# ldmx Action

Run the input command string in the ldmx container with the `LDMX_BASE` environment variable properly configured.

We assume that `ldmx-sw` has been checked out (or the build package unpacked correctly), so that `${GITHUB_WORKSPACE}/../` is the ldmx base directory.
This is the default behavior when using `actions/checkout@v2` to git a repository during an action.

The image used is determined by the `LDMX_DOCKER_TAG` environment variable which should be set inside of the workflow or job using this action.

## Inputs

- `run` : **required**
  - String containing the command to run inside the container. 
  - For example: `cmake ..`
- `working_dir` : optional
  - The directory to run inside of when in the container. Must be relative to the ldmx base directory.
  - Default: `.` (i.e. no change, run inside the base directory)
  - Example: `ldmx-sw/build`

## Example

Here is an example of translating a command you would run locally inside the container to inside this action.
This example

```bash
cd ldmx-sw/build
ldmx cmake ..
```

Becomes

```yaml
  - name: Configure the Build
    uses: actions/ldmx
    with:
      working_dir: ldmx-sw/build
      run: cmake ..
```
