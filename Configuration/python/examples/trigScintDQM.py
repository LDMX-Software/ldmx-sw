#!/usr/bin/python

import sys

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
p=ldmxcfg.Process("plot")  

# Currently, we need to explicitly identify plugin libraries which should be
# loaded.  In future, we do not expect this be necessary
p.libraries.append("libDQM.so")            # dqm processors                                     
 
# set the maximum number of events to process                                                                     
p.maxEvents=1000

# import template DQM analyzers
from LDMX.DQM import triggerPad

# Define the sequence of event processors to be run
#   triggerPad.sim() returns a list of analyzers to put into sequence
#   since there aren't any other analyzers, we can just set the sequence
#   equal to this list
p.sequence = triggerPad.sim()

# Provide the list of output files to produce
# if it can all be in one sequence, no input file is needed
p.inputFiles=["ldmx_basicOneElectron_events.root"]   # assume this has been produced already
p.histogramFile = "ldmx_basicOneElectron_events_simDqm.root" # histograms have to go to a histogramFile, not an outputFile 

# Utility function to interpret and print out the configuration to the screen
print p
