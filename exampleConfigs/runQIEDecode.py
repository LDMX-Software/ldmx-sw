"""Test packing config"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack') 
import sys


nEv=65000
nChan=16
#mapFile="channelMapFrontBack_"+str(nChan)+"channels.txt"
mapFile="channelMap_LYSOback_plasticFront_12-to-16channels_rotated180.txt" # "channelMap_LYSOback_plasticFront_12-to-16channels.txt" 
p.inputFiles=[sys.argv[1]]
p.outputFiles = [sys.argv[2]]
inputPass=sys.argv[3]
nSamp=int(sys.argv[4])
logName= p.outputFiles[0].replace(".root", "_toLDMX.log")


from LDMX.TrigScint.qieFormat import QIEDecoder
dec=QIEDecoder.up(mapFile)
dec.input_pass_name=inputPass
dec.verbose=True
dec.number_channels=nChan       #default: 50
dec.number_time_samples=nSamp   #default: 50


p.sequence = [
    dec
    ]

p.maxEvents = nEv
p.termLogLevel = 1    #"info" 
p.logFileName = logName
p.fileLogLevel = 0   #"debug"
