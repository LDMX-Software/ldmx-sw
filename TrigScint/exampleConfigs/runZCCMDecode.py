"""Test packing config"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack')
import sys

nEv=400000

p.maxEvents = nEv

p.inputFiles=[sys.argv[1]]
p.outputFiles = [sys.argv[2]]
inputPass=sys.argv[3]
nSamp=int(sys.argv[4])
logName= p.outputFiles[0].replace(".root", "_toLDMX.log")


if len(sys.argv) > 5 :
    mapFile=sys.argv[5]
else :
    mapFile="toyChannelMap_4modules_14lanes.txt"

if len(sys.argv) > 6 :
    logVerbosity=int(sys.argv[6])
else :
    logVerbosity=2  #default

if len(sys.argv) > 7 :
    nChan=int(sys.argv[7])
else :
    nChan=14*6 #default

from LDMX.TrigScint.zccmFormat import ZCCMDecoder
dec=ZCCMDecoder(mapFile)
moduleMapFile=mapFile.replace("Channel", "Module")
moduleMapFile=moduleMapFile.replace("channel", "module")
dec.module_map_file=moduleMapFile
dec.input_pass_name=inputPass
dec.output_collection = dec.output_collection+"Pad"
dec.verbose=True
dec.number_channels=nChan
dec.number_time_samples=nSamp
dec.is_real_data=True #default: False


p.sequence = [
    dec
    ]

p.logger.termLevel = 2
p.logger.filePath = logName
p.logger.fileLevel = logVerbosity


if logVerbosity < 2 :
    p.logger.debug(dec) #pass instance itself
if logVerbosity < 1 :
    p.logger.trace(dec)

#p.pause()
