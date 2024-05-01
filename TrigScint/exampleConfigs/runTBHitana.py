import json 
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('plot') #

import sys

inputPassName="hits"
nEv=200000

if len(sys.argv) > 2 :
    startSample=int(sys.argv[2])
else :
    startSample=2


from LDMX.TrigScint.trigScint import TestBeamHitAnalyzer


# ------------------- all set; setup in detail, and run with these settings ---------------

tsEv=TestBeamHitAnalyzer("plotMaker")
tsEv.inputPassName=inputPassName
# now in default config, too, but with test beam values :
#these are derived as the mean of gaussian fits to the "event pedestal" (average over middle two quartiles) for each channel
tsEv.startSample=startSample
tsEv.gain=2e6
                 

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
