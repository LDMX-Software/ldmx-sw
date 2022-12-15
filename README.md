Setting up Track reconstruction
---------------------------------------------

Baseline installation

    0. Start docker deamon 
```   
git clone --recursive git@github.com:LDMX-Software/ldmx-sw.git -b tracking_dev
source ldmx-sw/scripts/ldmx-env.sh
ldmx pull dev sha-995b3239
cd ldmx-sw; mkdir build; cd build;
ldmx cmake ..
ldmx make install -j2
```

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




