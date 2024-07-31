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
        self.growthThreshold = 50.0
        self.cellFilter = 0.

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

        self.CLUE = True
        self.dc = 0.
        self.rhoc = 550.
        self.deltac = 10.
        self.deltao = 40.

        self.debug = False

        self.build1DHistogram("nLoops", "No of loops for clustering", 50, 0, 400)
        self.build1DHistogram("nClusters", "No of clusters", 20, 0, 20)
        self.build1DHistogram("nHits", "Hits per cluster", 20, 0, 200)
        self.build1DHistogram("centroid_distances", "Distance between cluster centroid and event centroid", 20, 0, 200)
        self.build1DHistogram("cluster_energy", "Energy [MeV] per cluster", 100, 0, 10000)
        self.build2DHistogram("seed_weights", "Number of seeds", 20, 0, 100, "Minimum weight", 20, 0, 10)

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

        self.build1DHistogram("same_ancestor", "Percentage of hits in cluster coming from the electron that produced majority of hits", 10, 50, 100)
        self.build1DHistogram("energy_percentage", "Percentage of energy in cluster coming from the electron that produced majority of energy", 10, 50, 100)
        self.build1DHistogram("mixed_hit_energy", "Percentage of total energy coming from hits with energy contributions from both electrons", 10, 0, 100)
        self.build1DHistogram("mixed_ancestry", "Percentage of hits in cluster being contributed to by both electron 1 and 2", 20, 0, 100)
        self.build1DHistogram("clusterless_hits", "Number of hits not in a cluster", 10, 0, 200)

        self.build2DHistogram("total_energy_vs_hits", "Total energy", 30, 0, 150, "Hits in cluster", 20, 0, 200)
        self.build2DHistogram("total_energy_vs_purity", "Total energy", 30, 0, 150, "Energy purity %", 10, 50, 100)
        self.build2DHistogram("distance_purity", "Distance in xy-plane", 20, 0, 220, "Purity %", 10, 50, 100)
        self.build2DHistogram("distance_energy_purity", "Distance in xy-plane", 20, 0, 220, "Energy purity %", 10, 50, 100)