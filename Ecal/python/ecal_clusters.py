"""Configuration for the EcalClusterProducer

Examples
--------
    from LDMX.Ecal.ecalClusters import ecalClusters
    p.sequence.append( ecalClusters )
"""

from LDMX.Framework import Processor, processor


@processor("ecal::EcalClusterProducer", "Ecal")
class EcalClusterProducer(Processor):
    """Configure the clustering

    Attributes
    ----------
    rec_hit_coll_name: str
        name of input rec hits to use for cluster
    rec_hit_pass_name: str
        name of pass of rec hits to use
    cluster_coll_name: str
        name of output collection of clusters
    cutoff: float
        not clue, minimum rec energy threshold to consider for clustering
    seed_threshold: float
        not clue, minimum rec energy threshold to consider as a seed
    algo_name: str
        not clue, name of algorithm used to calculate "distance"
    algo_coll_name: str
        not clue, output collection to store algorithm information
    clue: bool
        enable using the CLUE algorithm instead of default
    nbr_of_layers: int
        number of layers to perform CLUE on
        if equals 1 (default), collapse all hits inot the same z-dimension
    dc: float
        cutoff distance in calculation of local desnity
        only used if nbr_of_layers > 1
    rhoc: float
        minimum seed energy (aka maximum outlier energy)
        not used when nbr_of_layers > 1
    deltac: float
        minimum seed separation
        not used when nbr_of_layers > 1
    deltao: float
        minimum outlier separation
    recluster: bool
        recluster merged clusters or not
        No reclustering leads to more undercounting
        reclustering leads to more overcounting
    """

    rec_hit_coll_name: str = "EcalRecHits"
    rec_hit_pass_name: str = ""
    cluster_coll_name: str = "EcalClusters"
    cutoff: float = 10.0
    seed_threshold: float = 100.0  # MeV
    algo_name: str = "MyClusterAlgo"
    algo_coll_name: str = "ClusterAlgoResult"
    clue: bool = True
    nbr_of_layers: int = 1
    dc: float = 5.0
    rhoc: float = 550.0
    deltac: float = 10.0
    deltao: float = 40.0
    reclustering: bool = False
