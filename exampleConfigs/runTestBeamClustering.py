from os.path import exists
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('clusters') #

import sys

inputPassName="hits"
nEv=400000

if len(sys.argv) > 2 :
    timeSample=int(sys.argv[2])
else :
    timeSample=21
    
from LDMX.TrigScint.trigScint import TestBeamClusterProducer


tbClustersUp  =TestBeamClusterProducer("tbClusters")
tbClustersUp.input_pass_name=inputPassName
#tbClustersUp.input_collection="TestBeamHitsUp"
tbClustersUp.pad_time=0.
tbClustersUp.time_tolerance=50.
tbClustersUp.verbosity=0
p.sequence = [
    tbClustersUp
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
p.outputFiles = [ sys.argv[1].replace(".root", "_clusters.root") ]
p.maxEvents = nEv

p.termLogLevel = 2
p.logFileName = p.outputFiles[0].replace(".root", ".log") 
p.logFileLevel=0

