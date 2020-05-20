#!/usr/bin/python

import sys
import os

# We need the ldmx configuration package to construct the processor objects
from LDMX.Framework import ldmxcfg

# Define the process, which must have a name which identifies this
# processing pass ("pass name").
p = ldmxcfg.Process("recon")

# Currently, we need to explicitly identify plugin libraries which should be
# loaded.  In future, we do not expect this be necessary
p.libraries.append("libEventProc.so")

# Create a processor 
myProcess = ldmxcfg.Producer("myProcessor", "ldmx::MyProcessor")

# Define the sequence of event processor to be run
p.sequence=[myProcess]

# Determine the input file from the first argument to this script.
p.inputFiles = [sys.argv[1]]

# Provide the list of output files to produce, either one to contain the results of all input files or one output file name per input file name
p.outputFiles = ["my_processor_results.root"]

# Utility function to interpret and print out the configuration to the screen
print p
