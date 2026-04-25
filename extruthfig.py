#Uses TruthHitProducer to isolate true hits,
#then turns them to digis, then clusters, then saves clusters to .txt file

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('uptoclustering')
p.input_files =  ["SimSamples.root"]
p.output_files = ["TruthSamples.root"]
#an additional output file called clusters.txt will be created as well

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

digis = [TrigScintDigiProducer.pad1(),
         TrigScintDigiProducer.pad2(),
         TrigScintDigiProducer.pad3()
         ]

for digi,hits in zip(digis, truth_hits):
    digi.input_collection = hits.output_collection

from LDMX.TrigScint.trig_scint import TrigScintClusterProducer

clusters = [TrigScintClusterProducer.pad1(),
                  TrigScintClusterProducer.pad2(),
                  TrigScintClusterProducer.pad3(),
                  ]

for cluster, digi in zip(clusters, digis):
    cluster.input_collection = digi.output_collection
    cluster.ampl_weighting = False #for LUT making for tracking
    cluster.clustering_threshold = 3.0 #prevents false "extra" clusters

p.sequence = [*truth_hits,
              *digis,
              *clusters, 
              ldmxcfg.Analyzer.from_file('ClusterViewerAnalyzer.cxx', 
                            needs = ['TrigScint_Event', 'SimCore_Event'])
              ]
