# LDMX Environment Initialization

The LDMX development and running environment is defined as a container image.
Running this container image properly can be complicated on many systems,
so we've isolated the running of this container into shell scripts so that
the average user/developer does not need to worry about the intricacies
of container running.

## Legacy Solution
The first development of this solution defined a series of functions in
`bash` that wrapped `docker` and `singularity` and focused on running the
image as built by [LDMX-Software/docker](https://github.com/LDMX-Software/docker).
This solution, while functional, has a few edge-case issues and is difficult 
to test independently of ldmx-sw.
These issues motivated the development of a separate project which we can
now use for development of ldmx-sw.

## denv
[denv](https://tomeichlersmith.github.io/denv/)
is a project to generalize the original bash functions to support
more shells, more systems, and more container runners. With a ground-up
redesign, it has benefited from lessons learned while using the original
bash functions and, as a independent project, can be thoroughly tested.

Utilizing `denv` to develop and run ldmx-sw is a simpler task than
trying to use the container runners directly; however, it is a bit
more complicated than the original bash functions due to its generality.
This makes defining wrapping environment scripts a helpful tool for
easing the burden placed on any users. Below, the LDMX denv configuration
is documented to help users understand what is happening as well as
help anyone interested in developing a wrapping script to be used
within the shell of their choice.

### Initialization
In order to properly mimic the environment as defined by the `ldmx`
suite of bash functions, we need to initialize a denv with a few
options.
```
denv \
  init \ # use denv special sub-command for initialization
  --clean-env \ # avoid passing environment variables
  --name "ldmx" \ # name this denv after our experiment :)
  "ldmx/dev:latest" \ # use the dev image
  <path-to-ldmx-base> # path to directory containing ldmx-sw
```
For example, if you are in the ldmx-sw directory,
```
denv init --clean-env --name ldmx ldmx/dev:latest ..
```
The default path is the current directory, so you could also init immediately
after cloning ldmx-sw.
```
git clone --recursive git@github.com:ldmx-software/ldmx-sw.git
denv init --clean-env --name ldmx ldmx/dev:latest
```
The default `--name` of a denv is the directory where `init` was run,
so the final example (and the one I use on most of my machines) is
```
mkdir ldmx
cd ldmx
git clone --recursive git@github.com:ldmx-software/ldmx-sw.git
denv init --clean-env ldmx/dev:latest
```

### Wrapping
Wrapping `denv` can have the benefit of reducing the amount of characters
needed for the average user to type when doing common tasks. Environment
scripts wrapping denv for a specific shell should do the following.

- Deduce `LDMX_BASE` and define it as an environment variable for the user
- Make sure the LDMX denv is initialized for the user
- Define the `ldmx` function with CLI defined below

### `ldmx` CLI Specification
v0.0.0: just an idea

- `ldmx`
  - `help` : print help message describing the CLI
  - `config` : print shell version being used and run `denv config print`
  - `use` : first argument is repo (`dev` or `pro`) and then second argument is tag (e.g. `latest`)
    - deduce full docker image tag name from repo and tag and then if expand the tag to include
      the path on `/cvmfs` (if available) or just the DockerHub tag (if not)
  - `pull` : call `ldmx use` and then `denv config image pull`
  - `mount` : pass arguments to `denv config mounts`
  - `setenv` : pass arguments to `denv config env copy`
  - `compile` : pass arguments to `denv ${LDMX_BASE}/ldmx-sw/ldmx-compile.sh`
  - `recompFire` : pass arguments to `denv ${LDMX_BASE}/ldmx-sw/ldmx-recompFire.sh`
  - `<cmd>` : pass directly to `denv`
