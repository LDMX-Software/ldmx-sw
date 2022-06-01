Installation of ldmx-sw from docker with acts

    0. Start docker deamon 
     
    1. Open a new terminal window (this terminal will be called w1) and  make a main folder for the project

    > mkdir sw

    2. Clone acts
    > cd sw
    > git clone git@github.com:acts-project/acts.git


    3. Get the proper branch
    > cd acts
    > git checkout v18.0.0

    4. Change the CMakeLists.txt with this one, modified to fix a DD4HEP CXX issue.
    https://www.dropbox.com/s/frl0ep0p7flr65v/CMakeLists.txt?dl=0
    Just copy it and substitute the CMakeLists.txt with that one. 
    

    5. Open a new terminal window (this terminal will be called w2) and install ldmx-sw
    (follow the instructions here https://github.com/LDMX-Software/ldmx-sw)

    > cd sw
    > git clone --recursive git@github.com:LDMX-Software/ldmx-sw.git -b tracking_dev

    6. Switch to the right branches
    > cd ldmx-sw
    > cd Tracking
    > git checkout tracking_full_chain
    > cd ..
    > cd cmake
    > git checkout 66fff74
 #   > cd ..
 #   > cd Ecal
 #   > git checkout 956077b239a80dfe6ebef7ecf4e1d56265d0ef33
    
    
    7. I prefer to work "inside the docker" by opening a shell with root privileges. You don't need to do that, but I just prefer this way.
    To do so a slightly different version of ldmx-env.sh is needed. Take this:
    https://www.dropbox.com/s/llm1ibl6tgv0okt/ldmx-env.sh?dl=0
    
    and substitute my ldmx-env.sh to the one in ldmx-sw/scripts/

    

    8. Start docker with the right image in w2

    > source ldmx-sw/scripts/ldmx-env.sh
    > ldmx pull dev sha-995b3239
    > ldmx bash

    9. In w2, change directory to the acts installation
    > cd path_to/acts/
    > mkdir build; mkdir install
    > cmake -DCMAKE_INSTALL_PREFIX=../install ..
    > make install


    10. Once acts has been installed in w2 go to ldmx-sw
    > cd ../../ldmx-sw
    > rm -rf build
    > mkdir build
    > cd build
    > source   /path_to/acts/install/bin/this_acts.sh 
    > cmake -DActs_DIR=/path_to/acts/install/lib/cmake/Acts ..
    > make -j 4 install


    11. In the case you have to compile the tracking module only and the library doesn't get installed:
    - Compile the Tracking module only
    cd build; make Tracking; cp Tracking/libTracking.so ../install/lib/libTracking.so; cd ..




=============TROUBLESHOOTING=====================

If this error appears:
/bin/sh: 1: LD_LIBRARY_PATH=/Users/pbutti/sw2/ldmx-sw/build/Detectors:/deps/dd4hep/lib:/Users/pbutti/sw2/acts/install/lib:/Users/pbutti/sw2/ldmx-sw/install/lib:/deps/dd4hep/lib:/deps/cernroot/lib:/deps/acts/lib:/deps/geant4/lib:/deps/xerces-c/lib:/Users/pbutti/sw2/ldmx-sw/install/external/*/lib: not found
Detectors/CMakeFiles/Components_Tracker.dir/build.make:72: recipe for target 'Detectors/libTracker.components' failed
make[2]: *** [Detectors/libTracker.components] Error 127
CMakeFiles/Makefile2:1501: recipe for target 'Detectors/CMakeFiles/Components_Tracker.dir/all' failed
make[1]: *** [Detectors/CMakeFiles/Components_Tracker.dir/all] Error 2
Makefile:145: recipe for target 'all' failed
make: *** [all] Error 2


it might be due to a badly formed command.
To fix it:
> make VERBOSE=1

It will show the failed command. For me it's:

root@ed94821e3bba:/Users/pbutti/sw2/ldmx-sw/build# cd /Users/pbutti/sw2/ldmx-sw/build/Detectors && "LD_LIBRARY_PATH=/Users/pbutti/sw2/ldmx-sw/build/Detectors:/deps/dd4hep/lib:/Users/pbutti/sw2/acts/install/lib:/Users/pbutti/sw2/ldmx-sw/install/lib:/deps/dd4hep/lib:/deps/cernroot/lib:/deps/acts/lib:/deps/geant4/lib:/deps/xerces-c/lib:/Users/pbutti/sw2/ldmx-sw/install/external/*/lib" /deps/dd4hep/bin/listcomponents_dd4hep -o libTracker.components libTracker.so
bash: LD_LIBRARY_PATH=/Users/pbutti/sw2/ldmx-sw/build/Detectors:/deps/dd4hep/lib:/Users/pbutti/sw2/acts/install/lib:/Users/pbutti/sw2/ldmx-sw/install/lib:/deps/dd4hep/lib:/deps/cernroot/lib:/deps/acts/lib:/deps/geant4/lib:/deps/xerces-c/lib:/Users/pbutti/sw2/ldmx-sw/install/external/*/lib: No such file or directory

Just remove the quotes at LD_LIBRARY_PATH and run:


root@ed94821e3bba:/Users/pbutti/sw2/ldmx-sw/build/Detectors# cd /Users/pbutti/sw2/ldmx-sw/build/Detectors
root@ed94821e3bba:/Users/pbutti/sw2/ldmx-sw/build/Detectors# LD_LIBRARY_PATH=/Users/pbutti/sw2/ldmx-sw/build/Detectors:/deps/dd4hep/lib:/Users/pbutti/sw2/acts/install/lib:/Users/pbutti/sw2/ldmx-sw/install/lib:/deps/dd4hep/lib:/deps/cernroot/lib:/deps/acts/lib:/deps/geant4/lib:/deps/xerces-c/lib:/Users/pbutti/sw2/ldmx-sw/install/external/*/lib 
root@ed94821e3bba:/Users/pbutti/sw2/ldmx-sw/build/Detectors# /deps/dd4hep/bin/listcomponents_dd4hep -o libTracker.components libTracker.so
