"""Configuration for pfReco producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon import pileup_finder
    p.sequence.append( PileupFinder )
"""

from LDMX.Framework import Processor, processor


@processor("recon::PileupFinder", "Recon")
class PileupFinder(Processor):
    """Configuration for pileup finding from particle flow objects"""

    rec_hit_coll_name: str = "EcalRecHits"
    rec_hit_pass_name: str = ""
    cluster_coll_name: str = "PFEcalClusters"
    cluster_pass_name: str = ""
    pf_cand_coll_name: str = "PFCandidates"
    pf_cand_pass_name: str = ""
    output_rec_hit_coll_name: str = "EcalRecHitsNoPileup"
    min_momentum: float = 4000.0
