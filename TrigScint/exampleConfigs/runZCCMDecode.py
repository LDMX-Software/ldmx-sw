"""Test packing config"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack')
import sys

n_ev=400000

p.max_events = n_ev

p.input_files=[sys.argv[1]]
p.output_files = [sys.argv[2]]
input_pass=sys.argv[3]
n_samp=int(sys.argv[4])
log_name= p.output_files[0].replace(".root", "_toLDMX.log")


map_file = (sys.argv[5] if len(sys.argv) > 5
    else "toyChannelMap_4modules_14lanes.txt")

log_verbosity = int(sys.argv[6]) if len(sys.argv) > 6 else 2 #default

n_chan = int(sys.argv[7]) if len(sys.argv) > 7 else 14*6 #default

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

p.logger.term_level = 2
p.logger.file_path = log_name
p.logger.file_level = log_verbosity


if log_verbosity < 2 :
    p.logger.debug(dec) #pass instance itself
if log_verbosity < 1 :
    p.logger.trace(dec)

#p.pause()
