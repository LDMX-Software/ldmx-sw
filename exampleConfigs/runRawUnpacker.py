from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('raw') 

import sys


nEv=400000  #set some large max 
RAWfileName=sys.argv[1] 
p.outputFiles = [sys.argv[2]]
logName=p.outputFiles[0].replace(".root", "_toRoot.log")

nTimeSamples=24
if len(sys.argv) > 3 :
    nTimeSamples=int(sys.argv[3])

nChannels=16
nWords=2*nChannels*nTimeSamples+4


from LDMX.Packing import rawio

p.maxEvents = nEv # single subsystem unpacker will abort when it runs out of data, so this is pretty redundant if large enough
p.sequence = [
    rawio.SingleSubsystemUnpacker(raw_file = RAWfileName, output_name = "QIEstreamUp", detector_name="ldmx-hcal-prototype-v1.0", num_bytes_per_event = nWords)
        ]

p.termLogLevel = 1    #"info" 
p.logFileName = logName
p.fileLogLevel = 0   #"debug"
