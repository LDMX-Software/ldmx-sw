
<p align="center">
    <img src="https://github.com/LDMX-Software/ldmx-software.github.io/blob/trunk/img/ldmx_logo_dark.png" width="500">
</p>

# Accessing DetectorIDs from Python 
## Building DetDescr with DetectorID binding support 
The DetDescr repository can optionally be built to enable Python bindings to most of the functionalities of the various DetectorID classes. This requires Boost.Python to be installed, which is part of the [the LDMX-software container distribution](https://github.com/LDMX-Software/docker) since [version 3.3](https://github.com/LDMX-Software/docker/releases/tag/v3.3). Since older versions of the container environment don't come with this library, the feature needs to be explicitly enabled as a CMake option. 

``` sh
ldmx cmake .. -DBUILD_DETECTORID_BINDINGS=ON
```

**Note:** Due to a bug, this option currently does not work if you built LDMX-sw with Sanitizer support (one of the `-DENABLE_SANITIZER_X=ON` settings). If your Python session crashes after trying to import the `libDetDescr`, try recompiling LDMX-sw with Sanitizer support disabled.

