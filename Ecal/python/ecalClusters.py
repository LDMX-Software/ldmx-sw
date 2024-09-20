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
        # Pass name rec hits
        self.recHitCollName = 'EcalRecHits'
        self.recHitPassName = ''

        self.simHitCollName = 'EcalSimHits'
        self.simHitPassName = ''

        # Name of the cluster collection to make
        self.clusterCollName = "ecalClusters"

        # --- EXISTING ALGORITHM ---
        self.cutoff = 10.
        self.seedThreshold = 100.0 #MeV

        # Name of the algo to save to the root file 
        self.algoName = "MyClusterAlgo"
        # Name of the cluster algo collection to make
        self.algoCollName = "ClusterAlgoResult"

        # --- CLUE ALGORITHM ---
        # Enable CLUE algorithm
        self.CLUE = True
        # Nbr of layers to perform CLUE on
        # = 1 collapses all hits into same z-dimension, gives best results atm
        self.nbrOfLayers = 1
        # Cutoff distance in calculation of local density
        # Currently only used when nbrOfLayers > 1
        self.dc = 5.
        # Minimum seed energy/maximum outlier energy
        self.rhoc = 550.
        # Minimum seed separation
        self.deltac = 10.
        # Minimum outlier separation
        self.deltao = 40.
        # Recluster merged clusters or not
        # No reclustering leads to more undercounting, reclustering leads to more overcounting
        self.reclustering = False
        # very verbose debug
        self.debug = False

        self.build1DHistogram("nLoops", "Number of loops for clustering", 50, 0, 400) # not applicable for CLUE
        self.build1DHistogram("nClusters", "Number of clusters", 20, 0, 20)
        self.build1DHistogram("nHits", "Hits per cluster", 20, 0, 300)
        self.build1DHistogram("cluster_energy", "Energy [MeV] per cluster", 100, 0, 20000)
        self.build2DHistogram("seed_weights", "Number of seeds", 20, 0, 100, "Minimum weight", 20, 0, 10) # not applicable for CLUE
        self.build2DHistogram("recluster", "Initial number of clusters", 20, 0, 20, "Number of clusters after reclustering", 20, 0, 20) # not applicable for existing algo

class EcalClusterAnalyzer(ldmxcfg.Analyzer) :
    """Analyze clustering"""

    def __init__(self,name='EcalClusterAnalyzer') :
        super().__init__(name,"ecal::EcalClusterAnalyzer", 'Ecal')

        self.nbrOfElectrons = 2

        self.ecalSimHitColl = "EcalSimHits"
        self.ecalSimHitPass = "" #use whatever pass is available

        # Pass name for ecal digis and rec hits
        self.recHitCollName = 'EcalRecHits'
        self.recHitPassName = ''

        self.clusterCollName = 'ecalClusters'
        self.clusterPassName = ''
        
        # Need to mod for more than two electrons
        self.build1DHistogram("ancestors", "Ancestors of particles", 4, 0, 3)

        self.build1DHistogram("same_ancestor", "Percentage of hits in cluster coming from the electron that produced most hits", 21, 0, 105)
        self.build1DHistogram("energy_percentage", "Percentage of energy in cluster coming from the electron that produced most of energy", 21, 0, 105)
        self.build1DHistogram("mixed_hit_energy", "Percentage of total energy coming from hits with energy contributions from more than one electron", 21, 0, 105)
        self.build1DHistogram("clusterless_hits", "Number of hits not in a cluster", 10, 0, 200)
        self.build1DHistogram("clusterless_hits_percentage", "Percentage of hits not in a cluster", 21, 0, 105)
        self.build1DHistogram("total_rechits_in_event", "Rechits per event", 20, 0, 500)
        self.build1DHistogram("correctly_predicted_events", "Correctly predicted events", 3, 0, 3)

        self.build2DHistogram("total_energy_vs_hits", "Total energy (edep)", 30, 0, 150, "Hits in cluster", 20, 0, 200)
        self.build2DHistogram("total_energy_vs_purity", "Total energy (edep)", 30, 0, 150, "Energy purity %", 21, 0, 105)
        self.build2DHistogram("distance_energy_purity", "Distance in xy-plane", 20, 0, 220, "Energy purity %", 21, 0, 105)