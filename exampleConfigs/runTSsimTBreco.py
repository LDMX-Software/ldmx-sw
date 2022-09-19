import sys
from os.path import exists
from LDMX.Framework import ldmxcfg


thisPassName="TBreco"

p = ldmxcfg.Process(thisPassName)

startSample=14
if len(sys.argv) > 1 :
    inputFile=sys.argv[1]
else :
    print("got to specify an input file")
    exit 
if len(sys.argv) > 2 :
    inputPassName=sys.argv[2]
else :
    inputPassName="conv" #"sim"

    
p.run = 10
#p.maxEvents = 2000
p.inputFiles = [ inputFile ]
outname=inputFile.replace(".root", "_TBreco.root")
p.outputFiles = [ outname ]
print("Running over input file: "+p.inputFiles[0])
print("Producing output file: "+p.outputFiles[0])

from LDMX.TrigScint.trigScint import TrigScintRecHitProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer


#assume we have run linearizer, so we can and have derived the gains
nChannels=12

gainList=[2e6]*nChannels
pedList=[6.]*nChannels

#look them up and run analyzers and reco
#now if there is a gain file, use that instead to read in the gain for each channel
gainFileName=inputFile.replace(".root", "_gains.txt")

if exists(gainFileName) :
    with open(gainFileName) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, gain
            gainList[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of gains:")
print(gainList)

#now if there is a ped file, use that instead to read in the ped for each channel
pedFileName=inputFile.replace(".root", "_peds.txt")

if exists(pedFileName) :
    with open(pedFileName) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, ped
            pedList[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of peds:")
print(pedList)



from LDMX.TrigScint.trigScint import QIEAnalyzer

tsAna=QIEAnalyzer("QIEplotMaker")
tsAna.inputPassName=inputPassName
tsAna.startSample=0
tsAna.pedestals=pedList
tsAna.gain=gainList

outname=outname.replace(".root", "_plots.root")
p.histogramFile = outname 



from LDMX.TrigScint.trigScint import TestBeamHitProducer

tbHitsUp  =TestBeamHitProducer("tbHits")
tbHitsUp.input_pass_name=inputPassName
tbHitsUp.input_collection="QIEsamplesUp"
tbHitsUp.pedestals=pedList
tbHitsUp.gain=gainList 
tbHitsUp.startSample=startSample
tbHitsUp.pulseWidth=12 #5 
tbHitsUp.pulseWidthLYSO=14
tbHitsUp.doCleanHits=True
tbHitsUp.nInstrumentedChannels=12


from LDMX.TrigScint.trigScint import TestBeamClusterProducer

tbClustersUp  =TestBeamClusterProducer("tbClusters")
tbClustersUp.input_pass_name=thisPassName
tbClustersUp.input_collection=tbHitsUp.outputCollection
tbClustersUp.pad_time=100.
tbClustersUp.time_tolerance=999.
tbClustersUp.verbosity=0
tbClustersUp.clustering_threshold = 40.  #to add in neighboring
tbClustersUp.seed_threshold = 50.   # i think 50 cuts off the low tail of the PE distribution 


cleanClustersUp  =TestBeamClusterProducer("cleanClusters")
cleanClustersUp.input_pass_name=thisPassName
cleanClustersUp.input_collection=tbHitsUp.outputCollection
cleanClustersUp.output_collection=tbClustersUp.output_collection+"Clean"
cleanClustersUp.pad_time=100.
cleanClustersUp.time_tolerance=999.
cleanClustersUp.verbosity=0
cleanClustersUp.clustering_threshold = 40.  #to add in neighboring
cleanClustersUp.seed_threshold = 50.
cleanClustersUp.doCleanHits=True


from LDMX.TrigScint.trigScint import QualityFlagAnalyzer

flagAna=QualityFlagAnalyzer("plotMaker")
flagAna.inputEventPassName=inputPassName
flagAna.inputHitPassName=thisPassName
flagAna.startSample=0
flagAna.pedestals=pedList
flagAna.gain=gainList

p.sequence = [
    tbHitsUp,
    tbClustersUp,
    cleanClustersUp,
#    tsAna,
    flagAna
    ]



p.termLogLevel = 2 #0
