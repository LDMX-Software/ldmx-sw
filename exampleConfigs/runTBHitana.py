import json 
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('plot') #

import sys

inputPassName="hits"
nEv=200000


from LDMX.TrigScint.trigScint import TestBeamHitAnalyzer


# ------------------- all set; setup in detail, and run with these settings ---------------

tsEv=TestBeamHitAnalyzer("plotMaker")
tsEv.input_pass_name=inputPassName
# now in default config, too, but with test beam values :
#these are derived as the mean of gaussian fits to the "event pedestal" (average over middle two quartiles) for each channel
tsEv.pedestals=[
    -4.8,  #0.6,
    -2.6, #4.4,
    -0.9, #-1.25,
    4.4,  #3.9, 	 # #3
    1.8,  #10000., # #4: (used to be) dead channel during test beam
    -2.5, #-2.1,   # #5 
    0.9,  #2.9,    # #6
    -1.5, #-2,     # #7
    4.7,  #-0.4,   # #8
    -4.4, #-1.1,   # #9: dead channel in TTU teststand setup
    -1.5, #1.5,    # #10
    -2.3, #2.0,    # #11
    3.3,  #3.7,    # #12 -- uninstrumented
    -0.3, #2.8,    # #13 -- uninstrumented
    1.3,  #-1.5,   # #14 -- uninstrumented
    1.3   #1.6     # #15 -- uninstrumented
]
                 

p.sequence = [
    tsEv
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace(".root", "_plots.root")
#p.outputFiles = [ outname ]

p.histogramFile = outname #.replace(".root"

p.maxEvents = nEv

p.logFileName=outname.replace(".root",".log")
p.termLogLevel = 2
p.logFileLevel=1#0

json.dumps(p.parameterDump(), indent=2)
