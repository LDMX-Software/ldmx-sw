"""Test packing config"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack') 
import sys


nEv=400000
nChan=16
#mapFile="channelMapFrontBack_"+str(nChan)+"channels.txt" #../TrigScint/data/channelMapFrontBack.txt")

p.inputFiles=[sys.argv[1]]
p.outputFiles = [sys.argv[2]]
inputPass=sys.argv[3]
nSamp=int(sys.argv[4])
logName= p.outputFiles[0].replace(".root", "_toLDMX.log")


if len(sys.argv) > 5 :
    mapFile=sys.argv[5]
else :
    mapFile="channelMap_LYSOback_plasticFront_12-to-16channels_rotated180.txt" # "channelMap_LYSOback_plasticFront_12-to-16channels.txt" # "channelMap_identity_"+str(nChan)+"channels.txt"    
if len(sys.argv) > 6 :
    logVerbosity=int(sys.argv[6])
else :
    logVerbosity=2

from LDMX.TrigScint.qieFormat import QIEDecoder
dec=QIEDecoder.up(mapFile)
#dec.input_collection="QIEstreamUp" 
dec.input_pass_name=inputPass
dec.verbose=True
dec.number_channels=nChan  #default: 50
dec.number_time_samples=nSamp  #default: 5
dec.is_real_data=True #default: False
#dec.spill_counter_conversion=125.0e6  #default: 125.e6 #not used


p.sequence = [
    dec
    ]

p.maxEvents = nEv
p.termLogLevel = 1
p.logFileName = logName
p.fileLogLevel = logVerbosity
