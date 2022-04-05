from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('hits') #

import sys

inputPassName="conv"
nEv=65000

if len(sys.argv) > 2 :
    timeSample=int(sys.argv[2])
else :
    timeSample=21
    
from LDMX.TrigScint.trigScint import TestBeamHitProducer


# ------------------- all set; setup in detail, and run with these settings ---------------


tbHitsUp  =TestBeamHitProducer("tbHits")
tbHitsUp.input_pass_name=inputPassName
tbHitsUp.input_collection="QIEsamplesUp"
tbHitsUp.sipm_gain=4e6
tbHitsUp.pulseWidthLYSO=5


p.sequence = [
    tbHitsUp#, tsDigisTag, tsDigisDown,
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
p.outputFiles = [ sys.argv[1].replace(".root", "_hits.root") ]
p.maxEvents = nEv

p.termLogLevel = 2
