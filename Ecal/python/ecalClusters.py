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
        super().__init__(name,"ecal::EcalClusterProducer", 'Ecal')

        self.cutoff = 10.
        self.seedThreshold = 100.0 #MeV

        # Pass name for ecal digis and rec hits
        self.digiCollName = 'EcalDigis'
        self.digisPassName = ''
        self.recHitCollName = 'EcalRecHits'

        # Name of the algo to save to the root file 
        self.algoName = "MyClusterAlgo"

        # Name of the cluster collection to make
        self.clusterCollName = "ecalClusters"

        # Name of the cluster algo collection to make
        self.algoCollName = "ClusterAlgoResult"

        self.build1DHistogram("nLoops", "No of loops for clustering", 50, 0, 300)
        self.build1DHistogram("nClusters", "No of clusters", 20, 0, 20)
        self.build1DHistogram("nHits", "Hits per cluster", 20, 0, 200)
        self.build1DHistogram("cluster_energy", "Energy [MeV] per cluster", 100, 0, 10000)

class EcalClusterAnalyzer(ldmxcfg.Analyzer) :
    """Analyze clustering"""

    def __init__(self,name='EcalClusterAnalyzer') :
        super().__init__(name,"ecal::EcalClusterAnalyzer", 'Ecal')

        self.ecalSimHitColl = "EcalSimHits"
        self.ecalSimHitPass = "" #use whatever pass is available

        # Pass name for ecal digis and rec hits
        self.recHitCollName = 'EcalRecHits'
        self.recHitPassName = ''

        self.clusterCollName = 'ecalClusters'
        self.clusterPassName = ''

        # self.depth = 100

        self.build1DHistogram("ancestors", "Ancestors of particles", 4, 0, 3)
        self.build1DHistogram("same_ancestor", "Percentage of hits in cluster coming from the electron that produced majority of hits", 20, 50, 100)
        self.build1DHistogram("UF_same_ancestor", "Percentage of hits in cluster coming from the electron that produced majority of hits", 20, 0, 100)
        self.build1DHistogram("mixed_ancestry", "Percentage of hits in cluster being contributed to by both electron 1 and 2", 20, 0, 100)
        self.build1DHistogram("UF_mixed_ancestry", "Percentage of hits in cluster being contributed to by both electron 1 and 2", 20, 0, 100)
        self.build1DHistogram("unclear_ancestry", "Percentage of hits in cluster where ancestry is unclear", 20, 0, 100)
        self.build1DHistogram("clusterless_hits", "Number of hits not in a cluster", 10, 0, 200)