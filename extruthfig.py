#Uses TruthHitProducer to isolate true hits,
#then turns them to digis, then clusters, then makes clusters.txt

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('truthisolater')
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
    cluster.ampl_weighting = False
    cluster.clustering_threshold = 3.0
    
    
CVA = ldmxcfg.Analyzer.from_file('ClusterViewerAnalyzer.cxx', 
                           needs = ['TrigScint_Event', 'SimCore_Event'])
CVA.pass_name = "truthisolater"

p.sequence = [*truth_hits,
              *digis,
              *clusters, 
              CVA
              ]
