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
        self.clue = True
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
        # No reclustering leads to more undercounting, reclustering leads to more
        # overcounting
        self.reclustering = False
