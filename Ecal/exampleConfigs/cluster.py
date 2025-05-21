from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('cluster')
import sys
p.inputFiles = sys.argv[1:]
p.outputFiles = [ 'clusters.root' ]
p.histogramFile = 'h_clusters.root'

from LDMX.Ecal.ecalClusters import *
p.sequence = [
    EcalClusterProducer(),
    EcalClusterAnalyzer()
]

