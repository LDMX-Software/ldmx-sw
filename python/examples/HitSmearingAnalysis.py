import os,sys,json
from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
p=ldmxcfg.Process("hitSmearingAna")

p.inputFiles = [ sys.argv[1] ]
p.libraries.append("libTracking.so")

from LDMX.Tracking import hitsmearing_analysis

ana=hitsmearing_analysis.HitSmearingAnalysis()
p.sequence = [ ana ]

# set the maximum number of events to process
p.maxEvents=5000

# Provide the list of output files to produce
#   When using the simulator to produce events, only one output file is necessary
p.histogramFile='hist_out.root'

# Utility function to interpret and print out the configuration to the screen
print(json.dumps(p.parameterDump(), indent=2))

