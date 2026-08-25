# Script 2 of 3 to generate new lookup table
# Next script is runLUTana.py

# example config which makes digis, clusters, and writes clusters
# to .txt file using ClusterTripletMaker

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("nontruthclusters")
p.input_files = ["SimSamples.root"]  # for a given simulation root file
p.output_files = ["Clusters.root"]
# an additional output file called clusters.txt will be created as well

from LDMX.TrigScint.truth_hits import TruthHitProducer
from LDMX.TrigScint.trig_scint import TrigScintDigiProducer

truth_filtering = False  # truth filtering using TruthHitProducer to isolate
# beam-originating hits/clusters, can help provide
# "answer-sheet" LUT

truth_hits = [
    TruthHitProducer("beamElectronsPad1"),
    TruthHitProducer("beamElectronsPad2"),
    TruthHitProducer("beamElectronsPad3"),
]

pad_num = 1

for hits in truth_hits:
    hits.input_collection = f"TriggerPad{pad_num}SimHits"
    hits.output_collection = f"truthBeamElectronsPad{pad_num}"
    pad_num += 1

digis = [
    TrigScintDigiProducer.pad1(),
    TrigScintDigiProducer.pad2(),
    TrigScintDigiProducer.pad3(),
]

if truth_filtering:
    for digi, hits in zip(digis, truth_hits, strict=True):
        digi.input_collection = hits.output_collection
    p.sequence = [*truth_hits, *digis]
if not truth_filtering:
    p.sequence = [*digis]

from LDMX.TrigScint.trig_scint import TrigScintClusterProducer

clusters = [
    TrigScintClusterProducer.pad1(),
    TrigScintClusterProducer.pad2(),
    TrigScintClusterProducer.pad3(),
]

for cluster, digi in zip(clusters, digis, strict=True):
    cluster.input_collection = digi.output_collection
    cluster.ampl_weighting = False  # for LUT making
    cluster.clustering_threshold = 3.0  # helps remove electronics noise

from LDMX.Tools.lut_making import ClusterTripletMaker

triplets = ClusterTripletMaker()

p.sequence.extend(
    [
        *clusters,
        triplets,
    ]
)
