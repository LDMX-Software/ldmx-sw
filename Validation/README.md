# Validation

Python package focused on comparing two or more "similar" LDMX event or histogram files.

## Installation
One may use this python module either inside or outside the ldmx-sw development container.

### Inside `ldmx-sw`
Run the dedicated `just` command to install
```
just install-validation
```
This is named 'install' because we used to install the Validation module as well
as its dependencies. Now we just install its dependencies and then run the Validation
module "from source".

### Outside `ldmx-sw`
Outside container it is helpful to put the Validation module inside a virtual environment.
This makes it easier to keep track of what you are doing and isolate the dependencies of `Validation`
from other python packages that may be on your system.
```
python3 -m venv .venv/valid --prompt valid
. .venv/valid/bin/activate
pip install -r Validation/requirements.txt
```
I store the virtual environment files in a directory called `.venv/valid` and tell `venv` to use
`valid` as the prompt so I can see in my terminal that I have access to the Validation package.
_After_ activating the virtual environment, I install the Validation module's requirements.

Using Validation without a virtual environment and outside of the container is not recommended.

## Usage

### CLI
The Validation module is constructed to do some common tasks quickly on the command line.
Running it can be accessed via `just` as well
```
just validate -h
```

For example if you would like to compare the ECAL shower features, put the input histograms into a directory, 
e.g. `compareDir` with names that are separated with an underscore, e.g. `histo_new.root` and `histo_ref.root`,
then run the following command.
```
just validate compareDir/  --systems ecal.shower_feats
```
This will produce plots in the `compareDir` directory and will include "new" and "ref" in the plot legend.
