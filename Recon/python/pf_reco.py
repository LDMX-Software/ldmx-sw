"""Configuration for pfReco producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon.pf_reco import pfEcalClusterProducer
    p.sequence.append( pfEcalClusterProducer )
"""

from LDMX.Framework import ldmxcfg


class pfEcalClusterProducer(ldmxcfg.Producer) :
    """Configuration for Ecal cluster builder for particle reco"""
    def __init__(self, name='PFEcalCluster') :
        super().__init__(name, 'recon::PFEcalClusterProducer','Recon')
        self.hit_coll_name     = 'EcalRecHits'
        self.hit_pass_name     = ''
        self.cluster_coll_name = 'PFEcalClusters'
        self.do_single_cluster = False
        self.log_energy_weight = True
        self.min_cluster_hit_mult = 2
        self.cluster_hit_dist = 50.
        self.min_hit_energy = 1.

class pfHcalClusterProducer(ldmxcfg.Producer) :
    """Configuration for Hcal cluster builder for particle reco"""
    def __init__(self, name='PFHcalCluster') :
        super().__init__(name, 'recon::PFHcalClusterProducer','Recon')
        self.hit_coll_name     = 'HcalRecHits'
        self.hit_pass_name     = ''
        self.cluster_coll_name = 'PFHcalClusters'
        self.do_single_cluster = False
        self.log_energy_weight = True
        self.min_cluster_hit_mult = 2
        self.cluster_hit_dist = 100.
        self.min_hit_energy = 0.1

class pfTrackProducer(ldmxcfg.Producer) :
    """Configuration for track selector for particle reco"""
    def __init__(self, name='PFTrack') :
        super().__init__(name, 'recon::PFTrackProducer','Recon')
        self.input_track_coll_name  = 'EcalScoringPlaneHits'
        self.input_pass_name  = ''
        self.output_track_coll_name = 'PFTracks'
        self.do_electron_tracking = False
        self.min_electron_momentum_z = 2500.
        self.max_electron_track_id = 30

class pfProducer(ldmxcfg.Producer) :
    """Configuration for particle reco"""
    def __init__(self, name='PFlow') :
        super().__init__(name, 'recon::ParticleFlow','Recon')
        self.input_ecal_coll_name  = 'PFEcalClusters'
        self.input_hcal_coll_name  = 'PFHcalClusters'
        self.input_track_coll_name = 'PFTracks'
        self.use_existing_ecal_clusters = False
        self.output_coll_name     = 'PFCandidates'
        self.single_particle     = False
        self.input_ecal_passname  = ''
        self.input_hcal_passname  = ''
        self.input_tracks_passname  = ''

class pfTruthProducer(ldmxcfg.Producer) :
    """Configuration for track selector for particle reco"""
    def __init__(self, name='PFTruth') :
        super().__init__(name, 'recon::PFTruthProducer','Recon')
        self.output_primary_coll_name = 'PFTruth'
        self.output_target_coll_name  = 'PFTruthTarget'
        self.output_ecal_coll_name    = 'PFTruthEcal'
        self.output_hcal_coll_name    = 'PFTruthHcal'

        self.target_sp_coll_name = 'TargetScoringPlaneHits'
        self.target_sp_passname = ''
        self.ecal_sp_coll_name = 'EcalScoringPlaneHits'
        self.ecal_sp_passname = ''
        self.sim_particles_coll_name = 'SimParticles'
        self.sim_particles_passname = ''
        self.sim_particles_event_passname = ''
        self.ecal_sp_hits_event_passname = ''
        self.target_sp_hits_event_passname = ''
