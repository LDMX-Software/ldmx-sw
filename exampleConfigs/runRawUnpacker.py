from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('raw') 

import sys

RAWfileName=sys.argv[1] 
p.outputFiles = [sys.argv[2]]
logName=p.outputFiles[0].replace(".root", "_toRoot.log")
if len(sys.argv) > 3 :
    nTimeSamples=int(sys.argv[3])
else :
    nTimeSamples=24 #default

if len(sys.argv) > 4 :
    nEv=int(sys.argv[4])
else :
    nEv=4e6 #default         

    
nChannels=16
lenHeader=4+4+4+3+1 #UTC timeStamp s, timestamp clock ticks, time since spill, evNb, 4bit error words+4bit empty
nWords=2*nChannels*nTimeSamples+lenHeader


from LDMX.Packing import rawio

p.sequence = [
    rawio.SingleSubsystemUnpacker(raw_file = RAWfileName, output_name = "QIEstreamUp", detector_name="ldmx-hcal-prototype-v1.0", num_bytes_per_event = nWords)
        ]

p.maxEvents = nEv # apparently this HAS TO be set! single subsystem unpacker will abort all events when it runs out of data

p.termLogLevel = 0 #1 
p.logFileName = logName
p.fileLogLevel = 0 
