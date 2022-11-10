# LDMX ECal Simulated Geometry Validation

This repository is focused on storing configuration scripts for `fire` and python-based analyses
related to validating the simulated geometry of the ECal.

## Running the Simulation
The simulation is run via `ldmx fire` with the [config script](simulation.py) in this repository.
Generally, you want to simulate both the v12 and v14 geometries, which would require two runs.
```
ldmx fire simulation.py --geometry 12 --n-events <n-events> --out-dir <out-dir>
ldmx fire simulation.py --geometry 14 --n-events <n-events> --out-dir <out-dir>
```
In order to roughly parallelize this, a [bash script](run.sh) was written to be run within the container.
```
ldmx ./run.sh -o <out-dir> -N <n-events>
```

### Full Procedure
This is a reference on the procedure for getting the simulation up and running.
We assume that ldmx-sw has already be `git clone`d onto your computer.

Enter the ldmx-sw environment
```
source ldmx-sw/scripts/ldmx-env.sh
```

**Only if needed**, recompile ldmx-sw. This is only necessary if you `git pull`d
changes to the ldmx-sw source code.
```
cd ldmx-sw/build
ldmx make install
```

Determine the version of ldmx-sw you are running.
```
cd ldmx-sw
git describe --tags
```

Create a directory for the simulated data pertaining to the version of the
simulation you are running.
```
cd ecal-validation
mkdir -p data/<specific-version-name>
```

Run the simulation.
```
ldmx ./run.sh -o data/<specific-version-name> -N <n-events>
```

## Running the Analysis
The analysis code largely consists of filling and drawing histograms.
As a first step, use [the testing notebook](test.ipynb) as an example for
using the `comp` package.
