
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

## Getting documentation from within a Python session
``` python
```

Boost.Python will automatically generate some documentation for each kind of
DetectorID that you can access through the built-in help system in Python.

``` python
# For documentation of the whole module, try 
help(libDetDescr)
# or for a particular type of ID 
help(EcalID)
```

When you describe a module or object, you'll get a brief description of each of the available methods including their documentation and corresponding C++ signatures. This documentation might look a little bit silly at first glance, since most methods will contain an additional first argument. This is the implicit `this`/`self` parameter and you can ignore it when using the DetectorID functionality.

As an example, the `Cell()` member function of the EcalID class is described as. 

``` 
|  cell(...)
|      cell( (EcalID)self) -> int :
|          Get the value of the cell field from the ID.
|      
|          C++ signature :
|              int cell(ldmx::EcalID {lvalue})
```

The ways you can construct a given DetectorID type is documented by the `__init__` function. Since C++ supports overloading constructors while python doesn't, the signature will list the different versions one after another inside a wrapper `__init__(...)` function. 

For EcalID, this would give us 

``` 
 |  __init__(...)
 |      __init__( (object)self) -> None :
 |          Empty ECAL id (but not null!)
 |      
 |          C++ signature :
 |              void __init__(_object*)
 |      
 |      __init__( (object)self, (int)rawid) -> None :
 |          Create from raw number
 |      
 |          C++ signature :
 |              void __init__(_object*,unsigned int)
 |      
 |      __init__( (object)self, (int)layer, (int)module, (int)cell) -> None :
 |          Create from pieces
 |      
 |          C++ signature :
 |              void __init__(_object*,unsigned int,unsigned int,unsigned int)
 |      
 |      __init__( (object)self, (int)layer, (int)module, (int)u, (int)v) -> None :
 |          Create from pieces including u/v cell
 |      
 |          C++ signature :
 |              void __init__(_object*,unsigned int,unsigned int,unsigned int,unsigned int)
 |      
 |      __init__( (object)self, (int)layer, (int)module, (object)uv) -> None :
 |          Create from pieces including u/v cell
 |      
 |          C++ signature :
 |              void __init__(_object*,unsigned int,unsigned int,std::pair<unsigned int, unsigned int>)

```

This tells us that we can construct an EcalID from 
- Nothing 
- A raw value 
- A layer, a module, and a cell value 
- A layer, a module, a u, and a v value 
- A layer, a module, and a u/v pair object


