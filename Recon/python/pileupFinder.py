"""Configuration for pfReco producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon import pileupFinder
    p.sequence.append( pileupFinder )
"""

from LDMX.Framework import ldmxcfg


class pileupFinder(ldmxcfg.Producer) :
    """Configuration for pileup finding from particle flow objects"""
    def __init__(self, name='PileupFinder') :
        super().__init__(name, 'recon::PileupFinder','Recon')
        self.rec_hit_coll_name = 'EcalRecHits'
        self.rec_hit_pass_name = ''
        self.cluster_coll_name = 'PFEcalClusters'
        self.cluster_pass_name = ''
        self.pf_cand_coll_name = 'PFCandidates'
        self.pf_cand_pass_name = ''
        self.output_rec_hit_coll_name  = 'EcalRecHitsNoPileup'
        self.min_momentum = 4000.
