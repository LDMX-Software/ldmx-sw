import sys
from os.path import exists
from LDMX.Framework import ldmxcfg


this_pass_name="TBreco"

p = ldmxcfg.Process(this_pass_name)

start_sample=14
if len(sys.argv) > 1 :
    input_file=sys.argv[1]
else :
    print("got to specify an input file")
    exit
if len(sys.argv) > 2 :
    input_pass_name=sys.argv[2]
else :
    input_pass_name="conv" #"sim"


p.run = 10
#p.max_events = 2000
p.input_files = [ input_file ]
outname=input_file.replace(".root", "_TBreco.root")
p.output_files = [ outname ]
print("Running over input file: "+p.input_files[0])
print("Producing output file: "+p.output_files[0])

from LDMX.TrigScint.trigScint import TrigScintRecHitProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer


#assume we have run linearizer, so we can and have derived the gains
n_channels=12

gain_list=[2e6]*n_channels
ped_list=[6.]*n_channels

#look them up and run analyzers and reco
#now if there is a gain file, use that instead to read in the gain for each channel
gain_file_name=input_file.replace(".root", "_gains.txt")

if exists(gain_file_name) :
    with open(gain_file_name) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, gain
            gain_list[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of gains:")
print(gain_list)

#now if there is a ped file, use that instead to read in the ped for each channel
ped_file_name=input_file.replace(".root", "_peds.txt")

if exists(ped_file_name) :
    with open(ped_file_name) as f:
        for line in f.readlines() :
            line=line.split(',')  #values are comma separated, one channel per line: channelNB, ped
            ped_list[ int(line[0].strip()) ] = float(line[1].strip())

print("Using this list of peds:")
print(ped_list)



from LDMX.TrigScint.trigScint import QIEAnalyzer

ts_ana=QIEAnalyzer("QIEplotMaker")
ts_ana.input_pass_name=input_pass_name
ts_ana.start_sample=0
ts_ana.pedestals=ped_list
ts_ana.gain=gain_list

outname=outname.replace(".root", "_plots.root")
p.histogram_file = outname



from LDMX.TrigScint.trigScint import TestBeamHitProducer

tb_hits_up  =TestBeamHitProducer("tb_hits")
tb_hits_up.input_pass_name=input_pass_name
tb_hits_up.input_collection="QIEsamplesUp"
tb_hits_up.pedestals=ped_list
tb_hits_up.gain=gain_list
tb_hits_up.start_sample=start_sample
tb_hits_up.pulse_width=12 #5
tb_hits_up.pulseWidthLYSO=14
tb_hits_up.doCleanHits=True
tb_hits_up.nInstrumentedChannels=12


from LDMX.TrigScint.trigScint import TestBeamClusterProducer

tbClustersUp  =TestBeamClusterProducer("tb_clusters")
tbClustersUp.input_pass_name=this_pass_name
tbClustersUp.input_collection=tb_hits_up.outputCollection
tbClustersUp.pad_time=100.
tbClustersUp.time_tolerance=999.
tbClustersUp.verbosity=0
tbClustersUp.clustering_threshold = 40.  #to add in neighboring
tbClustersUp.seed_threshold = 50.   # i think 50 cuts off the low tail of the PE distribution


clean_clusters_up  =TestBeamClusterProducer("cleanClusters")
clean_clusters_up.input_pass_name=this_pass_name
clean_clusters_up.input_collection=tb_hits_up.outputCollection
clean_clusters_up.output_collection=tbClustersUp.output_collection+"Clean"
clean_clusters_up.pad_time=100.
clean_clusters_up.time_tolerance=999.
clean_clusters_up.verbosity=0
clean_clusters_up.clustering_threshold = 40.  #to add in neighboring
clean_clusters_up.seed_threshold = 50.
clean_clusters_up.doCleanHits=True


from LDMX.TrigScint.trigScint import QualityFlagAnalyzer

flag_ana=QualityFlagAnalyzer("plotMaker")
flag_ana.inputEventPassName=input_pass_name
flag_ana.inputHitPassName=this_pass_name
flag_ana.start_sample=0
flag_ana.pedestals=ped_list
flag_ana.gain=gain_list

p.sequence = [
    tb_hits_up,
    tbClustersUp,
    clean_clusters_up,
#    ts_ana,
    flag_ana
    ]



p.term_log_level = 2 #0
