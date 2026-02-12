from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('cluster')
import sys
p.input_files = sys.argv[1:]
p.output_files = [ 'clusters.root' ]
p.histogram_file = 'h_clusters.root'

from LDMX.Ecal.ecalClusters import *
p.sequence = [
    EcalClusterProducer(),
    EcalClusterAnalyzer()
]

