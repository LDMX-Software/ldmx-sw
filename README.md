Setting up Track reconstruction
---------------------------------------------

Baseline installation

    0. Start docker deamon 
```   
git clone --recursive git@github.com:LDMX-Software/ldmx-sw.git -b tracking_dev
cd ldmx-sw
cd Tracking
git checkout tracking_full_chain
cd ..
cd cmake
git checkout 66fff74
cd ../../
source ldmx-sw/scripts/ldmx-env.sh
ldmx pull dev sha-995b3239
cd ldmx-sw; mkdir build; cd build;
ldmx cmake ..
ldmx make install -j2
```

Due to an issue with DD4Hep (which I plan to remove soon), the compilation will probably fail with an error similar to:
/bin/sh: 1: LD_LIBRARY_PATH=/Users/pbutti/sw/ldmxtest/ldmx-sw/build/Detectors:/deps/dd4hep/lib:/Users/pbutti/sw/ldmxtest/ldmx-sw/install/lib:/deps/dd4hep/lib:/deps/cernroot/lib:/deps/acts/lib:/deps/geant4/lib:/deps/xerces-c/lib:/Users/pbutti/sw/ldmxtest/ldmx-sw/install/external/*/lib: not found
Detectors/CMakeFiles/Components_Tracker.dir/build.make:72: recipe for target 'Detectors/libTracker.components' failed

If that happens just enter the docker, compile with the right path and exit the docker.
> ldmx bash
> cd $LDMX_BASE/ldmx-sw/build/Detectors && LD_LIBRARY_PATH=$LDMX_BASE/ldmx-sw/build/Detectors:/deps/dd4hep/lib:$LDMX_BASE/ldmx-sw/install/lib:/deps/dd4hep/lib:/deps/cernroot/lib:/deps/acts/lib:/deps/geant4/lib:/deps/xerces-c/lib:$LDMX_BASE/ldmx-sw/install/external/*/lib /deps/dd4hep/bin/listcomponents_dd4hep -o libTracker.components libTracker.so
> exit

Issue again:
ldmx make install -j2

and compilation should succeed. 
    

----------------------------------

If you prefer to work inside the docker with root privileges after the compilation
    
    7. I prefer to work "inside the docker" by opening a shell with root privileges. 
    To do so a slightly different version of ldmx-env.sh is needed. Take this:
    https://www.dropbox.com/s/llm1ibl6tgv0okt/ldmx-env.sh?dl=0    
    and substitute my ldmx-env.sh to the one in ldmx-sw/scripts/

    > source ldmx-sw/scripts/ldmx-env.sh
    > ldmx pull dev sha-995b3239
    > ldmx bash


    In the case you have to compile the tracking module only and the library doesn't get installed:
    - Compile the Tracking module only
    cd build; make Tracking; cp Tracking/libTracking.so ../install/lib/libTracking.so; cd ..




