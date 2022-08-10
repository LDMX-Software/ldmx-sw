
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

## Example usage 

To use the DetectorID bindings, you import the `libDetDescr.so` shared library as if it was a regular Python module. 
``` python
# Inside a python3 session within the LDMX container environment 
import libDetDescr 
# or 
from libDetDescr import EcalID, HcalID # etc 
```

To create a particular kind of DetectorID, you can either create a default ID, create it from a Raw integer (which you would typically obtain from the LDMX-sw events), or from the different pieces of the ID

``` python
default_id = EcalID() # cell, layer, module = 0. Raw value = 335544320
from_raw = EcalID(335810563) # cell = 3, layer = 2, module = 1
from_pieces = EcalID(cell=1, layer=2, module=3) # Raw value = 335818753
```

Once you have a valid DetectorID, you can retrieve the corresponding pieces from methods with the same name as the piece and the raw value from the `.raw()` method. For example, say that we have an `HcalDigiID` that we read from an HcalDigis collection and we wanted to know what the corresponding `HcalID` would be. 

``` python
from libDetDescr import HcalID, HcalDigiID
rawHcalDigiID = 411042050 # Presumably from an HcalDigis collection somewhere
digiID = HcalDigiID(rawHcalDigiID) 
hcalID = HcalID(section=digiID.section(), layer=digiID.layer(), strip=digiID.strip())
print(f"HcalDigiID [raw, section, layer, strip, end]  \
({digiID.raw()}, {digiID.section()}, {digiID.layer()}, {digiID.strip()}, {digiID.end()})\
\n\t->HcalID [raw, section, layer, strip] \
({hcalID.raw()}, {hcalID.section()}, {hcalID.layer()}, {hcalID.strip()})")
```
This would tell us that 

``` 
HcalDigiID [raw, section, layer, strip, end]  (411042050, 0, 1, 2, 0) 
        ->HcalID [raw, section, layer, strip] (402654210, 0, 1, 2)
```
