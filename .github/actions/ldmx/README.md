# ldmx Action

Run the input command string in the ldmx container with the `LDMX_BASE` environment variable properly configured.

We deduce the `LDMX_BASE` environment variable by assuming that `ldmx-sw` has been checked out using `actions/checkout`
which puts the user in the root directory of the repository being checked out.

## Inputs

- `run` : **required**
  - String containing the inputs to the container. This includes the location to run the command int he container, the command, and the command's arguments.
  - For example: `ldmx-sw/build cmake ..`
- `image` : **required**
  - The LDMX-style container to run within.
  - Default (and example): `ldmx/dev:latest`
- `working_dir` : **required**
  - The directory to run inside of when in the container. Must be relative to the ldmx base directory.
  - Default: `.` (i.e. no change, run insid ethe base directory)
  - Example: `ldmx-sw/build`

## Example

Here is an example of translating a command you would run locally inside the container to inside this action.
This example

```bash
cd build
ldmx cmake ..
```

Becomes

```yaml
  - name: Configure the Build
    uses: actions/run_ldmx
    with:
      working_dir: 'ldmx-sw/build'
      run: 'cmake ..'
```
