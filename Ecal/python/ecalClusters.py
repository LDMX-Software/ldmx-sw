"""Configuration for the EcalClusterProducer

Examples
--------
    from LDMX.Ecal.ecalClusters import ecalClusters
    p.sequence.append( ecalClusters )
"""

from LDMX.Framework import ldmxcfg

class EcalClusterProducer(ldmxcfg.Producer) :
    """Configure the clustering"""

    def __init__(self,name='ecalClusters') :
        super().__init__(name,"ecal::EcalClusterProducer")

        self.cutoff = 10.
        self.seedThreshold = 100.0 #MeV

        # Pass name for ecal digis
        self.digisPassName = "recon"

        # Name of the algo to save to the root file 
        self.algoName = "MyClusterAlgo"

        # Name of the cluster collection to make
        self.clusterCollName = "ecalClusters"

        # Name of the cluster algo collection to make
        self.algoCollName = "ClusterAlgoResult"

