<h3 align="center">ldmx-sw</h3>

<p align="center">
    Simulation and reconstruction framework for the Light Dark Matter eXperiment.  
</p>

## Quick Start 

- Install Geant4, ROOT, XercesC, and Python using gcc7.0+ with the C++17 standard.
- Clone the repo: `git clone https://github.com/LDMX-Software/ldmx-sw.git`
- Make a build directory: `cd ldmx-sw; mkdir build; cd build`
- Configure the build: `cmake -DCMAKE_INSTALL_PREFIX=../install -DXercesC_DIR=<path-to-xerces-install> ../`
- Build and Install: `make install`
  - _Tip:_ Use the `-j` option to tell `make` how many processors it can use. For example, `make -j4` tells `make` to use four processors.
- Source the Environment Setup Script: `source ldmx-sw/install/bin/ldmx-env-setup.sh`
- Now you can run `ldmx-app` with any processors in `ldmx-sw`

For information on how to install the framework, examples and more, see the [ldmx-sw Wiki](https://github.com/LDMX-Software/ldmx-sw/wiki). 

## Status

## Documentation 

## Contributing

## Maintainer 

## Copyright and license
