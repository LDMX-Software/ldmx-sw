from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('reco') 

import sys

inputPassName="unpack"
nEv=65000

if len(sys.argv) > 2 :
    timeSample=int(sys.argv[2])
else :
    timeSample=21
    
from LDMX.TrigScint.trigScint import TrigScintRecHitProducer


# ------------------- all set; setup in detail, and run with these settings ---------------


tsRecHitsUp  =TrigScintRecHitProducer.up()
tsRecHitsUp.input_pass_name=inputPassName
tsRecHitsUp.input_collection="decodedQIEUp"
tsRecHitsUp.sipm_gain=4e6
tsRecHitsUp.pedestal = 4. 
tsRecHitsUp.sample_of_interest=timeSample

p.sequence = [
    tsRecHitsUp
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
p.outputFiles = [ sys.argv[1].replace(".root", "_reco.root") ]
p.maxEvents = nEv

p.termLogLevel = 2     #warning
