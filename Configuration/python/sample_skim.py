#!/usr/bin/python

import sys

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
p=ldmxcfg.Process("skim")

dummy=ldmxcfg.Analyzer("dummy", "ldmx::DummyAnalyzer")

# drop every other event
dummy.parameters["keepEventModulus"]=2
dummy.parameters["caloHitCollection"]="hcalDigis"

# Define the sequence of event processors to be run
p.sequence=[dummy]

# Provide the list of input files to run on
p.inputFiles=["ldmx_digi_events.root"]

# Provide the list of output files to produce, either one to contain the results of all input files or one output file name per input file name
p.outputFiles=["ldmx_skim_events.root"]

# Set the default behavior to be that all events should be dropped
p.skimDefaultIsDrop()

# Use the output of the dummy module to decide what to keep
p.skimConsider("dummy")

# Utility function to interpret and print out the configuration to the screen
p.printMe()
