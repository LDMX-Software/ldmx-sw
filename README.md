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
    <img src="https://github.com/LDMX-Software/ldmx-sw/workflows/Docker%20Image/badge.svg" />
    <img src="https://github.com/LDMX-Software/ldmx-sw/workflows/Tests/badge.svg" />
</p>

## Quick Start 

- [Install the docker engine](https://docs.docker.com/engine/install/)
- (on Linux systems) [Manage docker as non-root user](https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user)
- Clone the repo: `git clone --recursive https://github.com/LDMX-Software/ldmx-sw.git`
- Setup the environment (in bash): `source ldmx-sw/scripts/ldmx-env.sh`
- Make a build directory: `cd ldmx-sw; mkdir build; cd build;`
- Configure the build: `ldmx cmake -DCMAKE_INSTALL_PREFIX=../install ..`
- Build and Install: `ldmx make install -j2`
- Now you can run any processor in _ldmx-sw_ through `ldmx fire myconfig.py`

## Documentation 
The full documentation for **ldmx-sw** is available on [github pages](https://ldmx-software.github.io/).

## Maintainer 

## Contributors

<a href="https://github.com/LDMX-Software/ldmx-sw/graphs/contributors">
  <img src="https://contributors-img.web.app/image?repo=LDMX-Software/ldmx-sw" />
</a>

