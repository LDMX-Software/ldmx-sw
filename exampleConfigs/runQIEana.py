import json 
from os.path import exists
from os import path
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('plot') #

import sys

inputPassName="conv"
nEv=2000

if len(sys.argv) > 2 :
    startSample=int(sys.argv[2])
else :
    startSample=2


from LDMX.TrigScint.trigScint import QIEAnalyzer


# ------------------- all set; setup in detail, and run with these settings ---------------

nChannels=12
gainList=[2e6]*nChannels
#now if there is a gain file, use that instead to read in the gain for each channel
gainFileName=sys.argv[1].replace(".root", "_gains.txt")
gainFileName=gainFileName.replace("_adcTrig", "")  #not derived for adcTrig events 

#pick one file more or less at random as the fallback option
defaultRun="unpacked_4gev_negativeMu_Apr03_2200_reformat_30timeSamplesFrom0_linearize"
dataPath=path.dirname( sys.argv[1] ) #extract the path to where we keep the data
defaultGainFileName=dataPath+"/"+defaultRun+"_gains.txt"

#if for some reason, gains are not derived for this run. probably too low stats --> fits not converging. bet on that inter-channel gain differences are larger than variations in channel over time; then it is better to use an old file than a flat default gain. also, this could be edited to become an average file.
if not exists(gainFileName) :
    gainFileName=defaultGainFileName

if exists(gainFileName) :
    with open(gainFileName) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, gain
            gainList[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of gains:")
print(gainList)

#similarly for pedestals.
pedList=[
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
pedFileName=gainFileName.replace("gains", "peds")
defaultPedFileName=dataPath+"/"+defaultRun+"_peds.txt"

if not exists(pedFileName) :  
    pedFileName=defaultPedFileName

if exists(pedFileName) :
    with open(pedFileName) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, ped
            pedList[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of peds:")
print(pedList)


tsEv=QIEAnalyzer("plotMaker")
tsEv.inputPassName=inputPassName
# now in default config, too, but with test beam values :
#these are derived as the mean of gaussian fits to the "event pedestal" (average over middle two quartiles) for each channel
tsEv.startSample=startSample
tsEv.pedestals=pedList
tsEv.gain=gainList 

p.sequence = [
    tsEv
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace(".root", "_plots.root")
#p.outputFiles = [ outname ]

p.histogramFile = outname #.replace(".root"

p.maxEvents = nEv

p.logFileName=outname.replace(".root",".log")
p.termLogLevel = 2#0
p.logFileLevel=0

json.dumps(p.parameterDump(), indent=2)
