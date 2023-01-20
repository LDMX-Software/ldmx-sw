
from LDMX.Framework import ldmxcfg
from LDMX.Tracking import tracking 
from LDMX.Tracking import vertexing

def single_e_track_recon(sigma_u: float = 0.006, 
                         sigma_v: float = 0.00) -> ldmxcfg.Process:
    """ Setup a process to run electron track reconstruction in the Tagger and
    Recoil trackers.  Currently, this is using truth information to find seeds.

    Parameters
    ----------
    sigma_u : float 
        Smearing in the u direction.
    sigma_v : float
        Smearing in the v direction.
    """

    p = ldmxcfg.Process('TrackerReco')

    # Truth seeder - Recoil electrons
    ts_recoil = tracking.TruthSeedProcessor()
    ts_recoil.z_min = 4.4
    ts_recoil.track_id = 1
    ts_recoil.pz_cut = 0.
    ts_recoil.p_cut_ecal = 0.

    # Truth seeder - Tagger electrons
    ts_tagger = tracking.TruthSeedProcessor()
    ts_tagger.z_min = 4.4
    ts_tagger.track_id = 1
    ts_tagger.pz_cut = 0.
    ts_tagger.p_cut_ecal = 0.
    ts_tagger.out_trk_coll_name = 'TaggerTruthSeeds'
    ts_tagger.sim_hits = 'TaggerSimHits'

    # Recoil track finding
    trk_recoil = tracking.CKFProcessor('RecoilTracking')
    trk_recoil.propagator_step_size = 1. # mm
    trk_recoil.propagator_maxSteps = 2000
    trk_recoil.bfield = -0.75 # T
    trk_recoil.const_b_field = False
    trk_recoil.hit_collection = 'RecoilSimHits'
    trk_recoil.out_trk_collection = 'RecoilTracks'
    trk_recoil.seed_coll_name = 'RecoilTruthSeeds'
    trk_recoil.use_seed_perigee = True
    trk_recoil.do_smearing = True
    trk_recoil.sigma_u = sigma_u # mm 
    trk_recoil.sigma_v = sigma_v # mm 
    trk_recoil.track_id = 1 
     
    # Tagger track finding
    trk_tagger  = tracking.CKFProcessor('TaggerTracking')
    trk_tagger.propagator_step_size = 1000. # mm
    trk_tagger.bfield = -1.5  # T 
    trk_tagger.const_b_field = False
    trk_tagger.hit_collection='TaggerSimHits'
    trk_tagger.out_trk_collection = 'TaggerTracks'
    trk_tagger.seed_coll_name = 'TaggerTruthSeeds'
    trk_tagger.use_seed_perigee = True
    trk_tagger.do_smearing = True
    trk_tagger.sigma_u = sigma_u # mm
    trk_tagger.sigma_v = sigma_v # mm

    vtx = tracking.Vertexer()

    p.sequence = [ts_recoil, ts_tagger, trk_recoil, trk_tagger]

    return p

def single_e_track_recon_10um() -> ldmxcfg.Process: 
    return single_e_track_recon(0.010, 0.00)

def single_e_track_recon_15um() -> ldmxcfg.Process: 
    return single_e_track_recon(0.015, 0.00)
