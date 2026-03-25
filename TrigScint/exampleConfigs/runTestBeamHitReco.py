from os.path import exists
from os import path
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("hits")  #

import sys

input_pass_name = "conv"
n_ev = 400000
p.max_events = n_ev

time_sample = int(sys.argv[2]) if len(sys.argv) > 2 else 1

pulse_width = int(sys.argv[3]) if len(sys.argv) > 3 else 5

from LDMX.TrigScint.trigScint import TestBeamHitProducer


n_channels = 24
gain_list = [2e6] * n_channels

# now if there is a gain file, use that instead to read in the gain for each channel
gain_file_name = sys.argv[1].replace(".root", "_gains.txt")

# pick one file more or less at random as the fallback option
default_run = (
    "decoded_data_20251208_225935_dark_current_kicker_trigger_parsed_50k_linearize"
)
data_path = path.dirname(sys.argv[1])  # extract the path to where we keep the data
default_gain_file_name = data_path + "/" + default_run + "_gains.txt"

# if for some reason, gains are not derived for this run. probably too low
# stats --> fits
# not converging. bet on that inter-channel gain differences are larger than variations
# in channel over time; then it is better to use an old file than a flat default gain.
# also, this could be edited to become an average file.
if not exists(gain_file_name):
    gain_file_name = default_gain_file_name
print("using gain file " + gain_file_name)
if exists(gain_file_name):
    with open(gain_file_name) as f:
        for line in f.readlines():
            # values are comma separated, one channel per line: channelNB, gain
            line = line.split(",")
            # don't assume ordered
            gain_list[int(line[0].strip())] = float(line[1].strip())

print("Using this list of gains:")
print(gain_list)

ped_list = [-2.0] * n_channels
# [
#            -4.6,  #0.6,
#            -2.6, #4.4,
#            -0.6, #-1.25,
#            4.4,  #3.9,    # #3
#            1.9,  #10000., # #4: (used to be) dead channel during test beam
#            -2.3, #-2.1,   # #5
#            1.0,  #2.9,    # #6
#            -1.2, #-2,     # #7
#            4.9,  #-0.4,   # #8  dead channel in spring 2022 testbeam at CERN
#            -4.4, #-1.1,   # #9: dead channel in TTU teststand setup
#            -0.1, #1.5,    # #10
#            -1.7, #2.0,    # #11
#            3.3,  #3.7,    # #12 -- uninstrumented
#            -0.3, #2.8,    # #13 -- uninstrumented
#            1.3,  #-1.5,   # #14 -- uninstrumented
#            1.3   #1.6     # #15 -- uninstrumented
#        ]

# now if there is a ped file, use that instead to read in the ped for each channel
ped_file_name = gain_file_name.replace("gains", "peds")
default_ped_file_name = data_path + "/" + default_run + "_peds.txt"

if not exists(ped_file_name):
    ped_file_name = default_ped_file_name

if exists(ped_file_name):
    with open(ped_file_name) as f:
        for line in f.readlines():
            # values are comma separated, one channel per line: channelNB, ped
            line = line.split(",")
            ped_list[int(line[0].strip())] = float(line[1].strip())

print("Using this list of peds:")
print(ped_list)


tb_hits_up = TestBeamHitProducer("tb_hits")
tb_hits_up.input_pass_name = input_pass_name
tb_hits_up.input_collection = "QIEsamplesPad1"  # "QIEsamplesUp"
tb_hits_up.pedestals = ped_list
tb_hits_up.gain = gain_list
tb_hits_up.start_sample = time_sample
tb_hits_up.pulse_width = pulse_width  # 5 #7
tb_hits_up.pulse_width_lyso = pulse_width  # 5 #7 #9 #7 for plastic, 9 for LYSO
tb_hits_up.do_clean_hits = False  # True
tb_hits_up.n_instrumented_channels = 24
p.sequence = [tb_hits_up]


# generate on the fly
p.input_files = [sys.argv[1]]
p.output_files = [sys.argv[1].replace(".root", "_hits.root")]

p.term_log_level = 2
