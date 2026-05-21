"""Test packing config"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("unpack")
import sys


n_ev = 400000
# map_file="channelMapFrontBack_"+str(n_chan)+"channels.txt"
##../TrigScint/data/channelMapFrontBack.txt")

# p.max_events = n_ev

p.input_files = [sys.argv[1]]
p.output_files = [sys.argv[2]]
input_pass = sys.argv[3]
n_samp = int(sys.argv[4])
log_name = p.output_files[0].replace(".root", "_toLDMX.log")


if len(sys.argv) > 5:
    map_file = sys.argv[5]
else:
    map_file = "channelMap_LYSOback_plasticFront_12-to-16channels_rotated180.txt"
log_verbosity = int(sys.argv[6]) if len(sys.argv) > 6 else 2

n_chan = int(sys.argv[7]) if len(sys.argv) > 7 else 16  # default

from LDMX.TrigScint.qieFormat import QIEDecoder

dec = QIEDecoder.up(map_file)
# dec.input_collection="QIEstreamUp"
dec.input_pass_name = input_pass
dec.verbose = True
dec.number_channels = n_chan  # default: 50
dec.number_time_samples = n_samp  # default: 5
dec.is_real_data = True  # default: False
# dec.spill_counter_conversion=125.0e6  #default: 125.e6 #not used


p.sequence = [dec]

# p.term_log_level = 1
p.logger.term_level = 1
# p.log_file_name = log_name
p.logger.FilePath = log_name
# p.file_log_level = log_verbosity
p.logger.file_level = log_verbosity


# p.logger.log_rules=[ "QIEDecoder", 0]
# p.logger.debug("QIEDecoder") #wrong way, don't pass instance name
if log_verbosity < 2:
    p.logger.debug(dec)  # pass instance itself
