import json 
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('plot') #

import sys

inputPassName="conv"
nEv=200000     #just something large, can limit to smaller as needed 


from LDMX.TrigScint.trigScint import QIEAnalyzer


# ------------------- all set; setup in detail, and run with these settings ---------------

tsEv=QIEAnalyzer("plotMaker")
tsEv.input_pass_name=inputPassName

p.sequence = [
    tsEv
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace(".root", "_plots.root")
#analysis generates histograms, not new colllections; no "output file" needed
p.histogramFile = outname 

p.maxEvents = nEv

p.logFileName=outname.replace(".root",".log")
p.termLogLevel = 2       #warning
p.logFileLevel=1         #info

#attempt to dump all parameter settings to screen
json.dumps(p.parameterDump(), indent=2)
