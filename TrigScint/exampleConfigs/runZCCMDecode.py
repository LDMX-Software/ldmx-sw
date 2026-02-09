"""Test packing config"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack')
import sys

n_ev=400000

p.max_events = n_ev

p.inputFiles=[sys.argv[1]]
p.outputFiles = [sys.argv[2]]
input_pass=sys.argv[3]
n_samp=int(sys.argv[4])
log_name= p.outputFiles[0].replace(".root", "_toLDMX.log")


if len(sys.argv) > 5 :
    map_file=sys.argv[5]
else :
    map_file="toyChannelMap_4modules_14lanes.txt"

if len(sys.argv) > 6 :
    log_verbosity=int(sys.argv[6])
else :
    log_verbosity=2  #default

if len(sys.argv) > 7 :
    n_chan=int(sys.argv[7])
else :
    n_chan=14*6 #default

from LDMX.TrigScint.zccmFormat import ZCCMDecoder
dec=ZCCMDecoder(map_file)
module_map_file=map_file.replace("Channel", "Module")
module_map_file=module_map_file.replace("channel", "module")
dec.module_map_file=module_map_file
dec.input_pass_name=input_pass
dec.output_collection = dec.output_collection+"Pad"
dec.verbose=True
dec.number_channels=n_chan
dec.number_time_samples=n_samp
dec.is_real_data=True #default: False

p.sequence = [ dec ]

p.logger.termLevel = 2
p.logger.file_path = log_name
p.logger.fileLevel = log_verbosity


if log_verbosity < 2 :
    p.logger.debug(dec) #pass instance itself
if log_verbosity < 1 :
    p.logger.trace(dec)

#p.pause()
