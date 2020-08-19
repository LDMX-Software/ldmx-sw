# Event Display for ldmx-sw

Currently, the event display is unable to be used within the container,
but we have configured it to be able to be built separately from everything else in ldmx-sw.

### Pre-Requisites
The event display depends on ROOT, so you will need an installation of ROOT outside of the container installed on your system.
The following procedure was developed on Ubuntu 18.04 using ROOT 6.20.00.

### Build and Install
1.  Make a directory for the event display inside of ldmx-sw: `mkdir eve; cd eve`
2. Make a build directory: `mkdir build; cd build`
3. Configure the build: `cmake -DBUILD_EVE_ONLY=ON -DCMAKE_INSTALL_PREFIX=../install ../../`
4. Build and Install: `make install`

### Environment Setup
You need to point your computer to the library and executable that the event display uses.
This entails setting two environment variables: `LD_LIBRARY_PATH` and `PATH`. In bash,
```bash
cd eve/install
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib
export PATH=$PATH:$(pwd)/bin
```
This setup will need to be done each time you open a new terminal to run the event display (even if you aren't re-compiling it).
You can see run-time helpful commands by passing the help flag.
```bash
ldmx-eve --help
```
Since we put the event display install in a different directory than the normal install,
you can continue to work on other branches without breaking your installation of the event display.
