import sys
from os.path import exists
from LDMX.Framework import ldmxcfg


thisPassName="interCalib"

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
outname=inputFile.replace(".root", "_interCalib.root")
p.outputFiles = [ outname ]
print("Running over input file: "+p.inputFiles[0])
print("Producing output file: "+p.outputFiles[0])

from LDMX.TrigScint.trigScint import TrigScintRecHitProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer


#assume we have run linearizer, so we can and have derived the gains
nChannels=12

gainList=[2e6]*nChannels
pedList=[6.]*nChannels
responseList=[1.]*nChannels

#look them up and run analyzers and reco
#now if there is a gain file, use that instead to read in the gain for each channel
gainFileName=inputFile.replace(".root", "_gains.txt")
gainFileName=gainFileName.replace("_TBreco", "")

if exists(gainFileName) :
    with open(gainFileName) as f:
        for line in f.readlines() :#        line = f.readline()
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, gain
            #        print(line[1:])
            gainList[ int(line[0].strip()) ] = float(line[1].strip())
            #for line in lines :

print("Using this list of gains:")
print(gainList)

#now if there is a ped file, use that instead to read in the ped for each channel
pedFileName=gainFileName.replace("gains", "peds")

if exists(pedFileName) :
    with open(pedFileName) as f:
        for line in f.readlines() :#        line = f.readline()
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, ped
            pedList[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of peds:")
print(pedList)

#now if there is a response file, use that instead to read in the response for each channel
responseFileName=inputFile.replace(".root", "_response.txt")

if exists(responseFileName) :
    with open(responseFileName) as f:
        for line in f.readlines() :#        line = f.readline()
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, response
            responseList[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of relative response correction factors:")
print(responseList)


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
tbHitsUp.MIPresponse=responseList
tbHitsUp.startSample=startSample
tbHitsUp.pulseWidth=12 #5 
tbHitsUp.pulseWidthLYSO=12 #14
tbHitsUp.doCleanHits=True
tbHitsUp.nInstrumentedChannels=12


from LDMX.TrigScint.trigScint import TestBeamClusterProducer

cleanClustersUp  =TestBeamClusterProducer("cleanClusters")
cleanClustersUp.input_pass_name=thisPassName
cleanClustersUp.input_collection=tbHitsUp.outputCollection
cleanClustersUp.output_collection=cleanClustersUp.output_collection+"Clean"
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
    cleanClustersUp,
    tsAna,
    flagAna
    ]



p.termLogLevel = 2 #0
