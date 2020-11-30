<p align="center">
    <img src="https://github.com/LDMX-Software/ldmx-software.github.io/blob/master/img/ldmx_logo_dark.png" width="500">
</p>

<p align="center">
    Simulation and reconstruction framework for the Light Dark Matter eXperiment.  
</p>

<p align="center">
    <a href="http://perso.crans.org/besson/LICENSE.html" alt="GPLv3 license">
        <img src="https://img.shields.io/badge/License-GPLv3-blue.svg" />
    </a>
    <img src="https://github.com/LDMX-Software/ldmx-sw/workflows/Tests/badge.svg" />
</p>

## Quick Start 

- [Install the docker engine](https://docs.docker.com/engine/install/)
- (on Linux systems) [Manage docker as non-root user](https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user)
- Clone the repo: `git clone --recursive https://github.com/LDMX-Software/ldmx-sw.git`
- Setup the environment (in bash): `source ldmx-sw/scripts/ldmx-env.sh`
- Make a build directory: `cd ldmx-sw; mkdir build; cd build;`
- Configure the build: `ldmx cmake ..`
- Build and Install: `ldmx make install -j2`
- Now you can run any processor in _ldmx-sw_ through `ldmx fire myconfig.py`

## Documentation 
The full documentation for **ldmx-sw** is available on [github pages](https://ldmx-software.github.io/).
A brief description of common commands is given below.

### Common Commands inside Container

Command | Purpose
---|---
`ldmx cmake ..` | Configure the ldmx-sw build
`ldmx make` | Compile/build ldmx-sw
`ldmx make install` | Install ldmx-sw
`ldmx fire config.py` | Use ldmx-sw application and processors with input python configuration
`ldmx python3 analysis.py` | Run python-based analysis
`ldmx ./bin/mg5_aMC` | Run MadGraph5 inside container which has f77 compiler installed

### Other Container Configuration Commands

The environment script defines several other shell commands to help configure and debug the container environment.

- `ldmx-container-tags repo` : List the container tags that you could use with the input repository: `dev`, `pro`, or `local`
- `ldmx-container-pull repo tag` : Setup the environment for the container 'ldmx/repo:tag' and pull down the newest version if the repo is remote
- `ldmx-container-config` : Print out how the container environment is currently configured
- `ldmx-has-required-engine` : Return 0 if computer has a supported container-running engine and 1 otherwise

## Maintainer 

## Contributors

<a href="https://github.com/LDMX-Software/ldmx-sw/graphs/contributors">
  <img src="https://contributors-img.web.app/image?repo=LDMX-Software/ldmx-sw" />
</a>

