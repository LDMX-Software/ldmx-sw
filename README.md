<p align="center">
    <img src="https://github.com/LDMX-Software/ldmx-software.github.io/blob/trunk/src/img/ldmx_logo_dark.png" width="500">
</p>

<p align="center">
    Simulation and reconstruction framework for the Light Dark Matter eXperiment.  
</p>

<p align="center">
    <a href="http://perso.crans.org/besson/LICENSE.html" alt="GPLv3 license">
        <img src="https://img.shields.io/badge/License-GPLv3-blue.svg" />
    </a>
    <a href="https://github.com/LDMX-Software/ldmx-sw/actions/workflows/build_production_image.yml" alt="Build Production Image">
        <img src="https://github.com/LDMX-Software/ldmx-sw/actions/workflows/build_production_image.yml/badge.svg"/>
    </a>
    <img src="https://github.com/LDMX-Software/ldmx-sw/actions/workflows/basic_test.yml/badge.svg" />
</p>

## Start Up
ldmx-sw is a large software project and so it is helpful to separate _using_ it to
perform physics studies from _developing_ it to fix/improve/enable other studies.
In both cases, we use containers to share a fixed software environment, so everyone
will need a method for running these containers.

- [Install the docker engine](https://docs.docker.com/engine/install/)
  - Only necessary on personal computers. Shared computing clusters should have `apptainer` installed.
  - (on Linux personal computers) [Manage docker as non-root user](https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user)
- [Install `denv`](https://tomeichlersmith.github.io/denv/getting_started.html#installation)

### Users
In order to use ldmx-sw, no more dependencies are required!
Simply choose the version of ldmx-sw you wish to use with your project.
```
mkdir my-project
cd my-project
denv init ldmx/pro:v4.0.1 # or some other ldmx-sw version
```
And then you can run ldmx-sw with a configuration script of your choice.
```
denv fire my-config.py
```
More detail on configuration scripts and analyzing the output files
is given in the first section of the [online manual](ldmx-software.github.io).

### Developers
For development, we use a few more tools to help track our changes and share commands
that we use regularly.

> [!WARNING]
> If you are on Windows, make sure to install these tools _inside_ WSL where `docker`
> will be run and ldmx-sw will be developed. Since WSL is often a virtual Ubuntu machine,
> following the instructions for Ubuntu or Linux can be appropriate.

- Make sure `git` is installed.
  - `git` is a very common tool used by software developers and so it may already be available.
  - (on MacOS systems) Make sure `git lfs` is installed. (Test: `git lfs` prints out a help message instead of an error about `lfs` not being found.) The default installation of `git` that is included with Apple's developer tools does not include `git lfs` which is required by acts to download and unpack one of its own submodules. [GitHub has a nice tutorial](https://docs.github.com/en/repositories/working-with-files/managing-large-files/installing-git-large-file-storage?platform=mac) on how to install `git lfs` on MacOS.
- [Install `just`](https://just.systems/man/en/chapter_5.html)
  - This tool is not required but it is highly encouraged. The recipes we share via the [justfile](justfile) can be run without `just` but are longer to type.

Then, with these additional tools, developers can clone the repository and start development.
```
git clone --recursive git@github.com:LDMX-Software/ldmx-sw.git
```

> [!NOTE]
> You need to [setup an SSH-key with your GitHub account](https://docs.github.com/en/authentication/connecting-to-github-with-ssh) on the computer you are using.

```
cd ldmx-sw
just # no arguments prints out the possible options
just init # initialize a new development environment (once per clone)
just configure build test # configure ldmx-sw, build it, then test it
```

## Maintainer 

## Contributors

<a href="https://github.com/LDMX-Software/ldmx-sw/graphs/contributors">
  <img src="https://contributors-img.web.app/image?repo=LDMX-Software/ldmx-sw" />
</a>

