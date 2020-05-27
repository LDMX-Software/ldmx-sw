<h3 align="center">ldmx-sw</h3>

<p align="center">
    Simulation and reconstruction framework for the Light Dark Matter eXperiment.  
</p>

## Quick Start 

- Install Geant4, ROOT, XercesC, and Python using gcc7.0+ with the C++17 standard.
- Clone the repo: `git clone https://github.com/LDMX-Software/ldmx-sw.git`
- Make a build directory: `cd ldmx-sw; mkdir build;`
- Configure the build: ``cd build; cmake -DCMAKE_INSTALL_PREFIX=../install -DXercesC_DIR=<path-to-xerces-install> -DPYTHON_EXECUTABLE=`which python` -DPYTHON_INCLUDE_DIR=${PYTHONHOME}/include/python2.7 -DPYTHON_LIBRARY=$PYTHONHOME/lib/libpython2.7.so ..
../``
- Build and Install: `make install -j4`
- Source the Environment Setup Script: `source ldmx-sw/install/bin/ldmx-env-setup.sh`
- Now you can run `ldmx-app` with any processors in `ldmx-sw`

For information on how to install the framework, examples and more, see the [ldmx-sw Wiki](https://github.com/LDMX-Software/ldmx-sw/wiki). 

## Status

## Documentation 
The full documentation for **ldmx-sw** is available on [github pages](https://ldmx-software.github.io/).

## Contributing

## Maintainer 

## Copyright and license
