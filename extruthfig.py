#example config for truth hit producer

#Uses TruthHitProducer to isolate true hits, 
#(then turns them to digis, then clusters)

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('truthisolater')
p.input_files =  ["NewEvents10.root"]
p.output_files = ["TruthEvents10.root"] 
 
from LDMX.TrigScint.truth_hits import TruthHitProducer
from LDMX.TrigScint.trig_scint import TrigScintDigiProducer

truth_hits = [TruthHitProducer('beamElectronsPad1'),
              TruthHitProducer('beamElectronsPad2'),
              TruthHitProducer('beamElectronsPad3')
                ]

pad_num = 1

for hits in truth_hits:
    hits.input_collection=f"TriggerPad{pad_num}SimHits"
    hits.output_collection=f"truthBeamElectronsPad{pad_num}"
    pad_num+=1
    
truth_digis = [TrigScintDigiProducer.pad1(),
               TrigScintDigiProducer.pad2(),
               TrigScintDigiProducer.pad3()
               ]

for digi,hits in zip(truth_digis, truth_hits):
    digi.input_collection = hits.output_collection
    
from LDMX.TrigScint.trig_scint import TrigScintClusterProducer
    
truth_clusters = [
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        ]

for cluster, digi in zip(truth_clusters, truth_digis):
    cluster.input_collection = digi.output_collection
    cluster.ampl_weighting = True


p.sequence = [*truth_hits,
              *truth_digis,
        	*truth_clusters,
        	#trigScintTrack,
        	#count,
#		TriggerProcessor('trigger', 800.), 
	#	ldmxcfg.Producer.from_file('TruthHitProducer.cxx')
        ]




    
