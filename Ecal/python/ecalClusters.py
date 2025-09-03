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
        self.rec_hit_coll_name = 'EcalRecHits'
        self.rec_hit_pass_name = ''

        # Name of the cluster collection to make
        self.cluster_coll_name = "EcalClusters"

        # --- EXISTING ALGORITHM ---
        self.cutoff = 10.
        self.seed_threshold = 100.0 #MeV

        # Name of the algo to save to the root file 
        self.algo_name = "MyClusterAlgo"
        # Name of the cluster algo collection to make
        self.algo_coll_name = "ClusterAlgoResult"

        # --- CLUE ALGORITHM ---
        # Enable CLUE algorithm
        self.CLUE = True
        # Nbr of layers to perform CLUE on
        # = 1 collapses all hits into same z-dimension, gives best results atm
        self.nbr_of_layers = 1
        # Cutoff distance in calculation of local density
        # Currently only used when nbrOfLayers > 1
        self.dc = 5.
        # Minimum seed energy/maximum outlier energy
        # Not used when nbrOfLayers > 1
        self.rhoc = 550.
        # Minimum seed separation
        # Not used when nbrOfLayers > 1
        self.deltac = 10.
        # Minimum outlier separation
        self.deltao = 40.
        # Recluster merged clusters or not
        # No reclustering leads to more undercounting, reclustering leads to more overcounting
        self.reclustering = False

        # not applicable for CLUE
        self.build1DHistogram("nLoops", "Number of loops for clustering", 50, 0, 400)
         # not applicable for CLUE
        self.build2DHistogram("seed_weights", "Number of seeds", 20, 0, 100, "Minimum weight", 20, 0, 10) 
        # not applicable for simplified algo
        self.build2DHistogram("recluster", "Initial number of clusters", 20, 0, 20, "Number of clusters after reclustering", 20, 0, 20) 
        self.build1DHistogram("rhoc", "rho_c", 20, 0, 2000)
        self.build1DHistogram("deltac", "delta_c", 20, 0, 100)
        self.build1DHistogram("deltao", "delta_o", 20, 0, 200)

