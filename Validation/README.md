# Validation

Python package forcused on comparing two or more "similar" LDMX event data files.

## Installation
Inside container...
```
ldmx python3 -m pip install Validation/ --target install/python/ --no-cache
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

### CLI
The Validation module is constructed to do some common tasks quickly on the command line.
Printing out its help message shows how to run it and gives you the details on what
parameters to provide.
```
python3 -m Validation -h
```
which should be run with `ldmx` if the module was installed in the container.

### In Script
Similar to the CLI, you can develop your own python script using Validation.
Simply `import Validation` where you want to be using it.
**Remember**: The plotting functions assume the user is in an interactive notebook
unless the `out_dir` parameter is provided.

### In Notebook
Again, accessing this module post-installation is the same as other modules `import Validation`.
This can help you develop plots that don't come pre-made within the Validation module.
**If you are developing Validation and testing within a notebook**, you will need to reboot
the python kernel anytime you wish to test changes to the Validation module. This is necessary
because Python keeps modules cached in memory during normal running.
