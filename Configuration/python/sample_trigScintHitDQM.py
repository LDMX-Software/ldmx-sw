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


trigScintUp = ldmxcfg.Analyzer("TrigScintHitDQMUp", "ldmx::TrigScintHitDQM")
trigScintUp.parameters["hit_collection"] = "trigScintDigis"
trigScintUp.parameters["pad"] = "up"

trigScintTag = ldmxcfg.Analyzer("TrigScintHitDQMTag", "ldmx::TrigScintHitDQM")
trigScintTag.parameters["hit_collection"] = "trigScintDigisTag"
trigScintTag.parameters["pad"] = "tag"

trigScintDown = ldmxcfg.Analyzer("TrigScintHitDQMDown", "ldmx::TrigScintHitDQM")
trigScintDown.parameters["hit_collection"] = "trigScintDigisDn"
trigScintDown.parameters["pad"] = "down"

# set the maximum number of events to process 
#p.maxEvents=1000


# Define the sequence of event processors to be run
p.sequence=[trigScintTag,trigScintUp,trigScintDown]

# Provide the list of output files to produce
# if it can all be in one sequence, no input file is needed
# also a single input file needs to be in brackets to not be parsed one letter at a time as a list of files 
if len(sys.argv) > 1 : 
    p.inputFiles=[ sys.argv[1] ] 
else:
    p.inputFiles=["ldmx_basicOneElectron_events.root"]

# histograms have to go to a histogramFile, not an outputFile 
if len(sys.argv) > 2 :
    p.histogramFile= sys.argv[2]
else:
    p.histogramFile = p.inputFiles[0].replace( '.root' , '_simDQM.root' ) # this, however, can't be!! then there is a no-clue segfault

print p.histogramFile[0]


# Utility function to interpret and print out the configuration to the screen
p.printMe()
