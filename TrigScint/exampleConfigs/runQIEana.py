import json
from os.path import exists
from os import path
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('plot') #

import sys

input_pass_name="conv"
n_ev=2000

if len(sys.argv) > 2 :
    start_sample=int(sys.argv[2])
else :
    start_sample=2


from LDMX.TrigScint.trigScint import QIEAnalyzer


# ------------------- all set; setup in detail, and run with these settings ---------------

n_channels=12
gain_list=[2e6]*n_channels
#now if there is a gain file, use that instead to read in the gain for each channel
gain_file_name=sys.argv[1].replace(".root", "_gains.txt")
gain_file_name=gain_file_name.replace("_adcTrig", "")  #not derived for adcTrig events

#pick one file more or less at random as the fallback option
default_run="unpacked_4gev_negativeMu_Apr03_2200_reformat_30timeSamplesFrom0_linearize"
data_path=path.dirname( sys.argv[1] ) #extract the path to where we keep the data
default_gain_file_name=data_path+"/"+default_run+"_gains.txt"

#if for some reason, gains are not derived for this run. probably too low stats --> fits not converging. bet on that inter-channel gain differences are larger than variations in channel over time; then it is better to use an old file than a flat default gain. also, this could be edited to become an average file.
if not exists(gain_file_name) :
    gain_file_name=default_gain_file_name

if exists(gain_file_name) :
    with open(gain_file_name) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, gain
            gain_list[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of gains:")
print(gain_list)

#similarly for pedestals.
ped_list=[
            -4.6,  #0.6,
            -2.6, #4.4,
            -0.6, #-1.25,
            4.4,  #3.9, 	 # #3
            1.9,  #10000., # #4: (used to be) dead channel during test beam
            -2.3, #-2.1,   # #5
            1.0,  #2.9,    # #6
            -1.2, #-2,     # #7
            4.9,  #-0.4,   # #8
            -4.4, #-1.1,   # #9: dead channel in TTU teststand setup
            -0.1, #1.5,    # #10
            -1.7, #2.0,    # #11
            3.3,  #3.7,    # #12 -- uninstrumented
            -0.3, #2.8,    # #13 -- uninstrumented
            1.3,  #-1.5,   # #14 -- uninstrumented
            1.3   #1.6     # #15 -- uninstrumented
        ]

#now if there is a ped file, use that instead to read in the ped for each channel
ped_file_name=gain_file_name.replace("gains", "peds")
default_ped_file_name=data_path+"/"+default_run+"_peds.txt"

if not exists(ped_file_name) :
    ped_file_name=default_ped_file_name

if exists(ped_file_name) :
    with open(ped_file_name) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, ped
            ped_list[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of peds:")
print(ped_list)


ts_ev=QIEAnalyzer("plotMaker")
ts_ev.input_pass_name=input_pass_name
# now in default config, too, but with test beam values :
#these are derived as the mean of gaussian fits to the "event pedestal" (average over middle two quartiles) for each channel
ts_ev.start_sample=start_sample
ts_ev.pedestals=ped_list
ts_ev.gain=gain_list

p.sequence = [
    ts_ev
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace(".root", "_plots.root")
#p.outputFiles = [ outname ]

p.histogramFile = outname #.replace(".root"

p.max_events = n_ev

p.logFileName=outname.replace(".root",".log")
p.termLogLevel = 2#0
p.logFileLevel=0

json.dumps(p.parameterDump(), indent=2)
