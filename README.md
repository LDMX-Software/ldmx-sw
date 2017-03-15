# ldmx-sw

The *ldmx-sw* github repository contains a C++ software framework for the proposed [Light Dark Matter Experiment (LDMX)](https://confluence.slac.stanford.edu/display/MME/Light+Dark+Matter+Experiment) based at [SLAC](https://slac.stanford.edu).

The `ldmx-sim` simulation application can read a [GDML](http://gdml.web.cern.ch/GDML/) detector description and write out [ROOT](https://root.cern.ch) files containing the simulated hits and particles.  The output file can be loaded into the ROOT environment for analysis.

There is also an `ldmx-app` program that can run analysis jobs using Python for the configuration file.

## Prerequisites

You will need the following build tools available in your environment before beginning the installation.

### Linux

The software has been built and tested on CentOS7 and RHEL6.  Using an older Linux releases such as RHEL6 or SLC6 will require you to install and use a more up to date compiler (gcc 4.8 is preferable) than the default system one.

### CMake

You should have at least CMake 3.0 installed on your machine, and preferably a current version from the [CMake website](https://cmake.org).  As of this writing, the current CMake version is the 3.6.2 release.  The installation will not work with any 2.x version of cmake, which is too old.

### GCC

You will need a version of GCC that supports the C++-11 standard.  I believe that ROOT6 is not compatible with GCC 5 so a 4.7 or 4.8 release is preferrable.  The default compiler on CentOS7 or RHEL7 should suffice.

## External Packages

You will first need to install or have available at your site a number of external dependencies before building the actual framework.

## Xerces

The [Xerces C++](http://xerces.apache.org/xerces-c/download.cgi) framework is required for GDML support in Geant4 so it must be installed first.

You can install it from scratch using a series of commands such as the following:

``` bash
wget http://download.nextag.com/apache/xerces/c/3/sources/xerces-c-3.1.4.tar.gz
tar -zxvf xerces-c-3.1.4.tar.gz
cd xerces-c-3.1.4
./configure --prefix=$PWD
make install
export XERCESDIR=$PWD
```

The *XERCESDIR* environment variable is optional and for convenience.  Where you see these types of variables in these instructions, you may also substitute the actual full path to your local installation of that dependency.

## Geant4

You need to have a local Geant4 installation available with GDML enabled.  You can check for this by looking for the header files in the Geant4 include dir, e.g. by doing `ls G4GDML*.hh` from that directory.  If no files are found, then it is not enabled in your installation.

Assuming you have [downloaded a Geant4 tarball](http://geant4.web.cern.ch/geant4/support/download.shtml), the installation procedure is like the following:

``` bash
tar -zxvf geant4.10.02.p02.tar.gz
cd geant4.10.02.p02
mkdir build; cd build
cmake -DGEANT4_USE_GDML=ON -DGEANT4_INSTALL_DATA=ON -DXERCESC_ROOT_DIR=$XERCESDIR \
    -DGEANT4_USE_OPENGL_X11=ON -DCMAKE_INSTALL_PREFIX=../../geant4.10.02.p02-install ..
make install
cd ../../geant4.10.02.p02-install
export G4DIR=$PWD
```

If you get errors about Xerces not being found, then check that the path you provided is correct and that the directory contains a lib dir with the Xerces so (shared library) files.

## ROOT

LDMX is standardizing on ROOT 6, and no support for ROOT 5 is planned.

ROOT has many installation options and optional dependencies, and the [building ROOT documentation](https://root.cern.ch/building-root) covers this in full detail.

These commands should install a current version of ROOT locally:

``` bash
wget https://root.cern.ch/download/root_v6.06.08.source.tar.gz
tar -zxvf root_v6.06.08.source.tar.gz
mkdir root-6.06.08-build
cd root-6.06.08-build
cmake -Dgdml=ON ../root-6.06.08
make 
export ROOTDIR=$PWD
```

Depending on what extra tools you want to use in ROOT, you should supply your own extra CMake arguments to enable them.

## Building ldmx-sw

Now that Geant4 is installed with GDML support along with ROOT, you should be able to compile the LDMX software framework.

These commands should install the software locally:

``` bash
git clone https://github.com/LDMXAnalysis/ldmx-sw.git
cd ldmx-sw
mkdir build; cd build
cmake -DGeant4_DIR=$G4DIR -DROOT_DIR=$ROOTDIR -DCMAKE_INSTALL_PREFIX=../ldmx-sw-install ..
make install
```

If you need to compile with a custom Python environment, e.g. not the system installation, then the following additional definitions should be added to the above `cmake` command.

``` bash
-DPYTHON_EXECUTABLE=`which python` -DPYTHON_INCLUDE_DIR=${PYTHONHOME}/include/python2.7 -DPYTHON_LIBRARY=$PYTHONHOME/lib/libpython2.7.so ..
```

Now you should have an installation of *ldmx-sw* in the *ldmx-sw-install* directory.

## Setting Up the Environment 

The build will generate a script for setting up the shell environment that is necessary to run the binaries.  The script is automatically copied to the `bin` directory of your installation directory when running `make install`.

Assuming your source and installation directories are the same, you can source the generated setup script as follows:

``` bash
. ./ldmx-sw/bin/ldmx-setup-env.sh
```

The only additional step is adding the Xerces library directory to the load library path:

``` bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/xerces/install/dir/lib
```

Replace the above path with the actual full path to your Xerces library directory.

Finally, you may want to set the following environment variable to avoid certain memory errors when running the programs:

``` bash
export MALLOC_CHECK_=0
```

Once you have executed the above commands in your shell, you should be able to execute programs like `ldmx-sim` without any additional setup.

## Running the LDMX Simulation Application

There is currently one main binary program created by the framework which is the LDMX Simulation Application.

It can be run from the command line in interactive mode using the `ldmx-sim` command or in batch mode by supplying a macro like `ldmx-sim run.mac`.

A sample macro might include the following to generate events using the particle gun:

```
/persistency/gdml/read detector.gdml
/run/initialize
/gun/particle e-
/gun/energy 4 GeV
/gun/position -27.926 5 -700 mm
/gun/direction 0.3138 0 3.9877 GeV
/run/beamOn 1000
```

LHE input events in XML format can also be used for event generation:

```
/persistency/gdml/read detector.gdml
/run/initialize
/ldmx/generators/lhe/open ./events.lhe
/run/beamOn 1000
```

The detector file is located in the *Detectors* module data directory and the easiest way to access this is by setting some sym links in your current directory using `ln -s ldmx-sw/Detectors/data/ldmx-det-full-v0/*.gdml .`, and then the program should be able to find all the detector files.

## Running the LDMX Analysis Application

The `ldmx-app` command will run an analysis job using an input configuration file.

Sample configuration files can be found in `Configuration/python` in the git repository.

## Contributing

To contribute code to the project, you will need to create an account on [github](https://github.com/) if you don't have one already, and then request to be added to the [LDMXAnalysis](https://github.com/orgs/LDMXAnalysis/) organization.

When adding new code, you should do this on a branch created by a command like `git checkout -b johndoe-dev` in order to make sure you don't apply changes directly to the master (replace "johndoe" with your user name).  We typically create branches based on issue names in the github bug tracker, so "Issue 1234" turns into the branch name `iss1234`.

Then you would `git add` and `git commit` your changes to this branch.

You can then merge in these changes to the local master by doing `git checkout master` and then `git merge johndoe-dev` which will apply your branch updates.

If you don't already have SSH keys configured, you can set this up as follows:

1. [Generate a new SSH key pair](https://help.github.com/articles/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent/).
2. Add the new SSH key to your github account under settings in the [SSH and GPG keys](https://github.com/settings/keys) tab.
3. Add the key to your SSH config file.

My *~/.ssh/config* file looks like this:

```
Host github.com
  Hostname github.com
  User git
  IdentityFile ~/.ssh/id_rsa_github
  PreferredAuthentications publickey
```

Now you can push changes to the master using the command `git push git@github.com:LDMXAnalysis/ldmx-sw.git` without needing to type your user name or password. 

## Pull Requests

We prefer that any major code contributions are submitted via [pull requests](https://help.github.com/articles/creating-a-pull-request/) so that they can be reviewed before changes are merged into the master.

Before you start, an [issue should be added to the ldmx-sw issue tracker](https://github.com/LDMXAnalysis/ldmx-sw/issues/new).

Then you should make a local branch from the master using a command like `git checkout -b iss1234` where _1234_ is the issue number from the issue tracker.

Once you have committed your local changes to this branch using the `git add` and `git commit` commands, then push your branch to github using a command like `git push -u origin iss1234`.

Finally, [submit a pull request](https://github.com/LDMXAnalysis/ldmx-sw/compare) to integrate your changes by selecting your branch in the _compare_ dropdown box and clicking the green buttons to make the PR.  This should be reviewed shortly and merged or changes may be requested before the code can be integrated into the master.

## Help

Comments, suggestions or cries for help can be sent to [Jeremy McCormick](mailto:jeremym@slac.stanford.edu) or posted in the [#simulation channel](https://ldmxsoftware.slack.com/messages/simulation/) of the [LDMX Software Slack](https://ldmxsoftware.slack.com/).  

If you plan on starting a major (sub)project within the repository like adding a new code module, you should give advance notice and explain your plains beforehand.  :)

## References

* [LDMX Simulation Framework](https://www.dropbox.com/s/oosmuyo553kvlce/LDMX%20Simulation%20Framework.pptx?dl=0) - Powerpoint presentation
* [LDMX-SW Doxygen Documentation](https://ldmxanalysis.github.io/ldmx-sw/html/index.html)
