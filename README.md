<h3 align="center">ldmx-sw</h3>

<p align="center">
    Simulation and reconstruction framework for the Light Dark Matter eXperiment.  
</p>

## Quick Start 

- [Install the docker engine](https://docs.docker.com/engine/install/)
- [Manage docker as non-root user](https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user)
- Clone the repo: `git clone https://github.com/LDMX-Software/ldmx-sw.git`
- Setup the environment: `/bin/bash source ldmx-sw/scripts/ldmx-env.sh`
- Make a build directory: `cd ldmx-sw; mkdir ldmx-sw; mkdir build; cd build;`
- Configure the build: `ldmx-cmake`
- Build and Install: `ldmx make install -j2`
- Now you can run any processor in _ldmx-sw_ through `ldmx app`

## Status

## Documentation 

## Contributing

## Maintainer 

## Copyright and license
