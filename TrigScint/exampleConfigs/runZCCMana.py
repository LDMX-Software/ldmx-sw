import json
from os.path import exists
from os import path
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("plot")  #

import sys

input_pass_name = "conv"
n_ev = 400000

start_sample = int(sys.argv[2]) if len(sys.argv) > 2 else 0


from LDMX.TrigScint.trig_scint import QIEAnalyzer


# ------------------- all set; setup in detail, and run with these settings
# ---------------

n_channels = 24
gain_list = [2e6] * n_channels
# now if there is a gain file, use that instead to read in the gain for each channel
gain_file_name = sys.argv[1].replace(".root", "_gains.txt")
gain_file_name = gain_file_name.replace(
    "_adcTrig", ""
)  # not derived for adcTrig events

# pick one file more or less at random as the fallback option
default_run = (
    "decoded_data_20251208_225935_dark_current_kicker_trigger_parsed_50k_linearize"
)
data_path = path.dirname(sys.argv[1])  # extract the path to where we keep the data
default_gain_file_name = data_path + "/" + default_run + "_gains.txt"

# if for some reason, gains are not derived for this run. probably too low
# stats --> fits
# not converging. bet on that inter-channel gain differences are larger than variations
# in one channel over time; then it is better to use an old file than a flat default
# gain. also, this could be edited to become an average file.
if not exists(gain_file_name):
    gain_file_name = default_gain_file_name

if exists(gain_file_name):
    with open(gain_file_name) as f:
        for line in f.readlines():
            # comma separated: channelNB, gain
            line = line.split(",")
            gain_list[int(line[0].strip())] = float(line[1].strip())

print(f"Using this list of gains, from {gain_file_name}:")
print(gain_list)

# similarly for pedestals.
ped_list = [2.0] * n_channels

# now if there is a ped file, use that instead to read in the ped for each channel
ped_file_name = gain_file_name.replace("gains", "peds")
default_ped_file_name = data_path + "/" + default_run + "_peds.txt"

if not exists(ped_file_name):
    ped_file_name = default_ped_file_name

if exists(ped_file_name):
    with open(ped_file_name) as f:
        for line in f.readlines():
            # comma separated: channelNB, ped
            line = line.split(",")
            ped_list[int(line[0].strip())] = float(line[1].strip())

print("Using this list of peds:")
print(ped_list)


ts_ev = QIEAnalyzer("plotMaker")
ts_ev.input_pass_name = input_pass_name
ts_ev.input_collection = "QIEsamplesPad1"
# now in default config, too, but with test beam values :
# these are derived as the mean of gaussian fits to the "event pedestal" (average over
# middle two quartiles) for each channel
ts_ev.start_sample = start_sample
ts_ev.pedestals = ped_list
ts_ev.gain = gain_list

p.sequence = [ts_ev]


# generate on the fly
p.input_files = [sys.argv[1]]
outname = sys.argv[1]
outname = outname.replace(".root", "_plots.root")
# p.output_files = [ outname ]

p.histogram_file = outname  # .replace(".root"

p.max_events = n_ev

p.logger.file_path = outname.replace(".root", ".log")
p.logger.term_level = 1  # 0
p.logger.file_level = 0

json.dumps(p.parameter_dump(), indent=2)
