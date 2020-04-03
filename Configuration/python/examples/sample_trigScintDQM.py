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
 
trigScintUp = ldmxcfg.Analyzer("TrigScintSimDQMUp", "ldmx::TrigScintDQM")
trigScintUp.parameters["hit_collection"] = "TriggerPadUpSimHits"
trigScintUp.parameters["pad"] = "up"

trigScintTag = ldmxcfg.Analyzer("TrigScintSimDQMTag", "ldmx::TrigScintDQM")
trigScintTag.parameters["hit_collection"] = "TriggerPadTaggerSimHits"
trigScintTag.parameters["pad"] = "tag"

trigScintDown = ldmxcfg.Analyzer("TrigScintSimDQMDown", "ldmx::TrigScintDQM")
trigScintDown.parameters["hit_collection"] = "TriggerPadDownSimHits"
trigScintDown.parameters["pad"] = "down"

# set the maximum number of events to process                                                                     
p.maxEvents=1000


# Define the sequence of event processors to be run
p.sequence=[trigScintTag,trigScintUp,trigScintDown]

# Provide the list of output files to produce
# if it can all be in one sequence, no input file is needed
p.inputFiles=["ldmx_basicOneElectron_events.root"]   # assume this has been produced already
p.histogramFile = "ldmx_basicOneElectron_events_simDqm.root" # histograms have to go to a histogramFile, not an outputFile 

# Utility function to interpret and print out the configuration to the screen
p.printMe()
