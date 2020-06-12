<p align="center">
    <img src="https://github.com/LDMX-Software/ldmx-software.github.io/blob/master/img/ldmx_logo_dark.png" width="500">
</p>

<p align="center">
    Simulation and reconstruction framework for the Light Dark Matter eXperiment.  
</p>

[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](http://perso.crans.org/besson/LICENSE.html)

## Quick Start 

- [Install the docker engine](https://docs.docker.com/engine/install/)
- [Manage docker as non-root user](https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user)
- Clone the repo: `git clone https://github.com/LDMX-Software/ldmx-sw.git`
- Setup the environment: `/bin/bash -c source ldmx-sw/scripts/ldmx-env.sh`
- Make a build directory: `cd ldmx-sw; mkdir build; cd build;`
- Configure the build: `ldmx cmake -DCMAKE_INSTALL_PREFIX=../install ..`
- Build and Install: `ldmx make install -j2`
- Now you can run any processor in _ldmx-sw_ through `ldmx app`

## Status

## Documentation 
The full documentation for **ldmx-sw** is available on [github pages](https://ldmx-software.github.io/).

## Contributing

## Maintainer 
