# Validation

Python package focused on comparing two or more "similar" LDMX event or histogram files.

## Installation
One may use this python module either inside or outside the ldmx-sw development container.

### Inside `ldmx-sw`
Run the dedicated `just` command to install
```
just install-validation
```

### Outside `ldmx-sw`
Outside container it is helpful to put the Validation module inside a virtual environment.
This makes it easier to keep track of what you are doing and isolate the dependencies of `Validation`
from other python packages that may be on your system.
```
python3 -m venv .venv/valid --prompt valid
. .venv/valid/bin/activate
pip install --editable Validation/
```
I store the virtual environment files in a directory called `.venv/valid` and tell `venv` to use
`valid` as the prompt so I can see in my terminal that I have access to the Validation package.
_After_ activating the virtual environment, I install an editable version of Validation.
Using `--editable` means that `python` will look at the original source files when attempting to
use the module while running which is helpful when planning to edit the Validation source files.
You can omit the `--editable` option in which case the code will be copied and (slightly) optimized
and you will need to re-`install` if you make changes to the source code.

Using Validation without a virtual environment and outside of the container is not recommended.


## Usage

### CLI
The Validation module is constructed to do some common tasks quickly on the command line.
Printing out its help message shows how to run it and gives you the details on what
parameters to provide.
```
denv python3 -m Validate -h
```

For example if you would like to compare the ECAL shower features, put the input histograms into a directory, 
e.g. `compareDir` with names that are separated with an underscore, e.g. `histo_new.root` and `histo_ref.root`,
then run the following command.
```
denv python3 -m Validate compareDir/  --systems ecal.shower_feats
```
This will produce plots in the `compareDir` directory and will include "new" and "ref" in the plot legend.

### In Script
Similar to the CLI, you can develop your own python script using Validate.
Simply `import Validate` where you want to be using it.
**Remember**: The plotting functions assume the user is in an interactive notebook
unless the `out_dir` parameter is provided.

### In Notebook
Again, accessing this module post-installation is the same as other modules `import Validate`.
This can help you develop plots that don't come pre-made within the Validation module.
**If you are developing Validate and testing within a notebook**, you will need to reboot
the python kernel anytime you wish to test changes to the Validation module. This is necessary
because Jupyter keeps modules cached in memory during normal running in order to save time
