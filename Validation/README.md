# Validation

Python package forcused on comparing two or more "similar" LDMX event data files.

## Installation
Inside container...
```
ldmx python3 -m pip install Validation/ --target install/python/LDMX/ --no-deps --no-cache
```
Outside container
```
python3 -m pip install Validation/
```

Other helpful options
- Outside container: `--user` may need to be required
- Both: `--editable` may be helpful if developing Validation which should be provided _before_ the path to Validation
  e.g. `python3 -m pip install --editable Validation/ --user`

## Usage
_Cannot_ run from ldmx-sw directory. `import Validation` prefers
the local directory instead of the installed path so it tries to
load from the `ldmx-sw/Validation` directory.

Could fix this by renaming the package inside Validation.
