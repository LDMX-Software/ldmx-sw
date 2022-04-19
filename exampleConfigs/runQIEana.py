import json 
from os.path import exists
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

if "data/teststand" in gainFileName :
    defaultGainFileName="data/teststand/unpacked_teststand_Mar08_1100_cosmic_reformat_30timeSamplesFrom0_linearize_gains.txt"
else :
    defaultGainFileName="data/testbeam22/unpacked_randoms_Mar27_1654_reformat_30timeSamplesFrom0_linearize_gains.txt"


if not exists(gainFileName) :  #for some reason, not derived. probably too low stats --> fits not converging. bet on that inter-channel gain differences are larger than variations in channel over time; then it is better to use an old file than a flat default gain. also, this could be edited to become an average file.
    gainFileName=defaultGainFileName

if exists(gainFileName) :
    with open(gainFileName) as f:
        for line in f.readlines() :#        line = f.readline()
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, gain
            #        print(line[1:])
            gainList[ int(line[0].strip()) ] = float(line[1].strip())
            #for line in lines :

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
pedFileName=sys.argv[1].replace(".root", "_peds.txt")
pedFileName=pedFileName.replace("_adcTrig", "")  #not derived for adcTrig events 

if "data/teststand" in gainFileName :
    defaultPedFileName="data/teststand/unpacked_teststand_Mar08_1100_cosmic_reformat_30timeSamplesFrom0_linearize_peds.txt"
else :
    defaultPedFileName="data/testbeam22/unpacked_randoms_Mar27_1654_reformat_30timeSamplesFrom0_linearize_peds.txt"

if not exists(pedFileName) :  #for some reason, not derived. probably too low stats --> fits not converging. bet on that inter-channel ped differences are larger than variations in channel over time; then it is better to use an old file than a flat default ped. also, this could be edited to become an average file.
    pedFileName=defaultPedFileName

if exists(pedFileName) :
    with open(pedFileName) as f:
        for line in f.readlines() :#        line = f.readline()
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, ped
            #        print(line[1:])
            pedList[ int(line[0].strip()) ] = float(line[1].strip())
            #for line in lines :

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
