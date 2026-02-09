"""Test packing config"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack')
import sys


n_ev=400000
#map_file="channelMapFrontBack_"+str(n_chan)+"channels.txt" #../TrigScint/data/channelMapFrontBack.txt")

#p.max_events = n_ev

p.inputFiles=[sys.argv[1]]
p.outputFiles = [sys.argv[2]]
input_pass=sys.argv[3]
n_samp=int(sys.argv[4])
log_name= p.outputFiles[0].replace(".root", "_toLDMX.log")


if len(sys.argv) > 5 :
    map_file=sys.argv[5]
else :
    map_file="channelMap_LYSOback_plasticFront_12-to-16channels_rotated180.txt" # "channelMap_LYSOback_plasticFront_12-to-16channels.txt" # "channelMap_identity_"+str(n_chan)+"channels.txt"
if len(sys.argv) > 6 :
    log_verbosity=int(sys.argv[6])
else :
    log_verbosity=2

if len(sys.argv) > 7 :
    n_chan=int(sys.argv[7])
else :
    n_chan=16 #default

from LDMX.TrigScint.qieFormat import QIEDecoder
dec=QIEDecoder.up(map_file)
#dec.input_collection="QIEstreamUp"
dec.input_pass_name=input_pass
dec.verbose=True
dec.number_channels=n_chan  #default: 50
dec.number_time_samples=n_samp  #default: 5
dec.is_real_data=True #default: False
#dec.spill_counter_conversion=125.0e6  #default: 125.e6 #not used


p.sequence = [
    dec
    ]

#p.termLogLevel = 1
p.logger.termLevel = 1
#p.logFileName = log_name
p.logger.FilePath = log_name
#p.fileLogLevel = log_verbosity
p.logger.fileLevel = log_verbosity


#p.logger.logRules=[ "QIEDecoder", 0]
#p.logger.debug("QIEDecoder") #wrong way, don't pass instance name
if log_verbosity < 2 :
    p.logger.debug(dec) #pass instance itself
