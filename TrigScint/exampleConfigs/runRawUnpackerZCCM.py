from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('raw')

import sys

RAWfileName=sys.argv[1]
p.outputFiles = [sys.argv[2]]
logName=p.outputFiles[0].replace(".root", "_toRoot.log")
if len(sys.argv) > 3 :
    nTimeSamples=int(sys.argv[3])
else :
    nTimeSamples=128 #default
print(f"Running with {nTimeSamples} time samples")

if len(sys.argv) > 4 :
    nEv=sys.argv[4] #int(sys.argv[4])
else :
    nEv=4e6 #default
print(f"Processing {nEv} events")

if len(sys.argv) > 5 :
    nChannels=int(sys.argv[5])
else :
    nChannels=14*6 #default
print(f"Setting event readout length based on {nChannels} channels")

# get the length of all header parts
lenDAQheader=16          # DAQ time stamp and some other info
lenTimestamp=8           # TS readout data start time stamp
lenEmptyword=8           # 8B empty before event data begins
lenHeader=lenDAQheader+lenTimestamp+lenEmptyword
# now get the length of each lane message
lenTSflagsAndLane=2      # 1B flags, 1B lane nb in each message
lenEmpty=2               # 2B empty following ADC
lenADCandTDC=2*6         # 6 channels per lane
lenMessage=lenTSflagsAndLane+lenEmpty+lenADCandTDC
# find the number of messages (one per lane)
nLanes=nChannels/6
nBytes=lenHeader + nLanes*nTimeSamples*lenMessage


from LDMX.Packing import rawio

p.sequence = [
    rawio.SingleSubsystemUnpacker(raw_file = RAWfileName, output_name = "ZCCMoutput", detector_name="ldmx-hcal-prototype-v1.0", num_bytes_per_event = int(nBytes))
        ]

p.maxEvents = int(nEv) # this HAS TO be set! single subsystem unpacker will abort all events when it runs out of data so it cannot simply be left running

p.logger.termLevel = 0 #1
p.logger.fileName = logName
p.logger.fileLevel = 0

