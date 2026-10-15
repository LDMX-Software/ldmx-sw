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
    algorithm: str
        "clue" (uses the settings above) or "clue3d"
    clue3d_*: float
        CLUE3D settings, see Ecal/CLUE3D.h; energies in MeV, lengths in mm
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
    algorithm: str = "clue"
    clue3d_hit_ecut: float = 0.0
    clue3d_dc2d: float = 10.0
    clue3d_kernel2d: float = 0.2
    clue3d_deltas2d: float = 10.0
    clue3d_deltao2d: float = 25.0
    clue3d_rhoc2d: float = 5.0
    clue3d_sibling_layers: int = 2
    clue3d_dens_xy3d: float = 20.0
    clue3d_kernel3d: float = 0.5
    clue3d_crit_xy3d: float = 10.0
    clue3d_crit_rho3d: float = 50.0
    clue3d_self_frac3d: float = 0.15
    clue3d_outlier_mult3d: float = 2.0
    clue3d_attach_radius: float = 40.0
    clue3d_sc_ratio: float = 0.06
    clue3d_sc_radius: float = 100.0
    clue3d_sc_min_layers: int = 5
    clue3d_min_cluster_energy: float = 20.0
    clue3d_share: bool = True
    clue3d_share_iters: int = 30
    clue3d_share_min_energy: float = 300.0
    clue3d_coincide_dist: float = 4.0
    clue3d_profile_lambda: list[float] = []
    clue3d_w0: float = 4.0
    clue3d_pos_window: float = 15.0
