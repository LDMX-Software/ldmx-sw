# Load the tracking module
from LDMX.Tracking import tracking
from LDMX.Tracking import geo

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
trackgeo.get_instance().setDetector('ldmx-det-v15-8gev')

# Truth seeder
# Runs truth tracking producing tracks from target scoring plane hits for Recoil
# and generated electrons for Tagger.
# Truth tracks can be used for assessing tracking performance or using as seeds
truth_tracking = tracking.TruthSeedProcessor()
vars(truth_tracking).update(
    debug = True,
    scoring_hits_coll_name = 'TargetScoringPlaneHitsOverlay',
    recoil_sim_hits_coll_name = 'RecoilSimHitsOverlay',
    tagger_sim_hits_coll_name = 'TaggerSimHitsOverlay',
    z_min = 0.,
    track_id = -1,
    p_cut = 0.05, # MeV
    pz_cut = 0.03, # MeV
    p_cut_ecal = 0.,
    ecal_sp_coll_name = 'EcalScoringPlaneHitsOverlay',
    sim_particles_coll_name = 'SimParticlesOverlay',
    beam_electrons_collection = 'beamElectronsOverlay',
    tagger_seeds_collection = 'TaggerTruthSeedsOverlay',
    tagger_truth_collection = 'TaggerTruthTracksOverlay',
    recoil_seeds_collection = 'RecoilTruthSeedsOverlay',
    recoil_truth_collection = 'RecoilTruthTracksOverlay',
)

# Smearing Processor - Tagger
# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to reduce combinatorics
digi_tagger = tracking.DigitizationProcessor("DigitizationProcessor")
digi_tagger.hit_collection = "TaggerSimHitsOverlay"
digi_tagger.out_collection = "DigiTaggerSimHitsOverlay"

# Smearing Processor - Recoil
digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digi_recoil.hit_collection = "RecoilSimHitsOverlay"
digi_recoil.out_collection = "DigiRecoilSimHitsOverlay"

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at the track
# parameters and the impact parameters at the target or generation point. For the tagger one should look
# for compatibility with the beam orbit / beam spot
seeder_tagger = tracking.SeedFinderProcessor("SeedTagger")
seeder_tagger.input_hits_collection =  digi_tagger.out_collection
seeder_tagger.out_seed_collection = "TaggerRecoSeedsOverlay"
seeder_tagger.sim_particles_coll_name = "SimParticlesOverlay"
seeder_tagger.pmin  = 0.03
seeder_tagger.pmax  =  63.0
seeder_tagger.d0min =  -36.9
seeder_tagger.d0max = 31.5
seeder_tagger.z0max = 54.6
seeder_tagger.thetacut = 0.26
seeder_tagger.phicut =  0.84

#Seed finder processor - Recoil
seeder_recoil = tracking.SeedFinderProcessor("SeedRecoil")
seeder_recoil.perigee_location = [0.,0.,0.]
seeder_recoil.input_hits_collection =  digi_recoil.out_collection
seeder_recoil.out_seed_collection = "RecoilRecoSeedsOverlay"
seeder_recoil.sim_particles_coll_name = "SimParticlesOverlay"
seeder_recoil.bfield = 1.5
seeder_recoil.pmin  =   0.04
seeder_recoil.pmax  =  819.0
seeder_recoil.d0min =  -40.2
seeder_recoil.d0max = 36.5
seeder_recoil.z0max = 40.5
seeder_recoil.thetacut =  1.5
seeder_recoil.phicut =  1.6

# CKF track finding for tagger tracker using seeds.
tracking_tagger  = tracking.CKFProcessor("Tagger_TrackFinder")
tracking_tagger.taggerTracking = True
# for truth seed based case use 
# tracking_tagger.seed_coll_name = "TaggerTruthSeeds"
tracking_tagger.seed_coll_name = seeder_tagger.out_seed_collection
tracking_tagger.out_trk_collection = "TaggerTracksOverlay"
tracking_tagger.measurement_collection = digi_tagger.out_collection
tracking_tagger.sim_particles_coll_name = "SimParticlesOverlay"
tracking_tagger.min_hits = 5
tracking_tagger.outlier_pval_ = 16.5

# CKF track finding for recoil tracker using seeds.
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.taggerTracking = False
tracking_recoil.seed_coll_name = seeder_recoil.out_seed_collection
tracking_recoil.out_trk_collection = "RecoilTracksOverlay"
# for truth seed based case use 
# tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.measurement_collection = digi_recoil.out_collection
tracking_recoil.sim_particles_coll_name = "SimParticlesOverlay"
tracking_recoil.min_hits = 4
tracking_recoil.outlier_pval_ =  22.1

# Greedy ambiguity solver for the tagger
greedy_solver_tagger = tracking.GreedyAmbiguitySolver("GreedySolverTagger")
greedy_solver_tagger.out_trk_collection = "TaggerTracksCleanOverlay"
greedy_solver_tagger.track_collection = tracking_tagger.out_trk_collection
greedy_solver_tagger.meas_collection = digi_tagger.out_collection

# Greedy ambiguity solver for the recoil
greedy_solver_recoil = tracking.GreedyAmbiguitySolver("GreedySolverRecoil")
greedy_solver_recoil.out_trk_collection = "RecoilTracksCleanOverlay"
greedy_solver_recoil.track_collection = tracking_recoil.out_trk_collection
greedy_solver_recoil.meas_collection = digi_recoil.out_collection

# Gaussian sum filter for the tagger
GSF_tagger = tracking.GSFProcessor("Tagger_GSF")
GSF_tagger.taggerTracking = True
GSF_tagger.track_collection = greedy_solver_tagger.out_trk_collection
GSF_tagger.meas_collection  = digi_tagger.out_collection 
GSF_tagger.out_trk_collection = "GSFTaggerTracksOverlay"

# Gaussian sum filter for the recoil
GSF_recoil = tracking.GSFProcessor("Recoil_GSF")
GSF_recoil.taggerTracking = False
GSF_recoil.track_collection = greedy_solver_recoil.out_trk_collection
GSF_recoil.meas_collection  = digi_recoil.out_collection 
GSF_recoil.out_trk_collection = "GSFRecoilTracksOverlay"

# Tracker veto
tracker_veto = tracking.TrackerVetoProcessor()
tracker_veto.tagger_track_collection = "TaggerTracksOverlay"
tracker_veto.recoil_track_collection = "RecoilTracksOverlay"
tracker_veto.output_collection = "TrackerVetoOverlay"

# Put it all together into a single sequance
trk_sequence = [
    digi_tagger,
    digi_recoil,
    truth_tracking,
    seeder_tagger,
    seeder_recoil,
    tracking_tagger,
    tracking_recoil,
    greedy_solver_tagger,
    greedy_solver_recoil,
    GSF_tagger,
    GSF_recoil,
    tracker_veto
]

# Running DQM for the collections above
from LDMX.Tracking import dqm as tkdqm

# Seeder DQM for the tagger
dqm_seed_tagger = tkdqm.TrackingRecoDQM("SeedTaggerDQM")
dqm_seed_tagger.track_collection = seeder_tagger.out_seed_collection
dqm_seed_tagger.truth_collection = "TaggerTruthTracksOverlay"
dqm_seed_tagger.measurement_collection = digi_tagger.out_collection
dqm_seed_tagger.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_seed_tagger.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_seed_tagger.title = ""
dqm_seed_tagger.buildHistograms()

# Seeder DQM for the recoil
dqm_seed_recoil = tkdqm.TrackingRecoDQM("SeedRecoilDQM")
dqm_seed_recoil.track_collection = seeder_recoil.out_seed_collection
dqm_seed_recoil.truth_collection = "RecoilTruthTracksOverlay"
dqm_seed_recoil.measurement_collection = digi_recoil.out_collection
dqm_seed_recoil.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_seed_recoil.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_seed_recoil.title = ""
dqm_seed_recoil.buildHistograms()

# DQM for the tagger with CKF
dqm_tagger_ckf = tkdqm.TrackingRecoDQM("TaggerDQM")
dqm_tagger_ckf.track_collection = tracking_tagger.out_trk_collection
dqm_tagger_ckf.truth_hit_collection="TaggerSimHitsOverlay"
dqm_tagger_ckf.truth_collection = "TaggerTruthTracksOverlay"
dqm_tagger_ckf.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_tagger_ckf.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_tagger_ckf.trackStates = ["target"]
dqm_tagger_ckf.title = ""
dqm_tagger_ckf.measurement_collection=digi_tagger.out_collection
dqm_tagger_ckf.buildHistograms()

# DQM for the recoil with CKF
dqm_recoil_ckf = tkdqm.TrackingRecoDQM("RecoilDQM")
dqm_recoil_ckf.track_collection = tracking_recoil.out_trk_collection
dqm_recoil_ckf.truth_collection = "RecoilTruthTracksOverlay"
dqm_recoil_ckf.truth_collection = "TaggerTruthTracksOverlay"
dqm_recoil_ckf.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_recoil_ckf.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_recoil_ckf.trackStates = ["ecal","target"]
dqm_recoil_ckf.title = ""
dqm_recoil_ckf.measurement_collection=digi_recoil.out_collection
dqm_recoil_ckf.truth_hit_collection = "RecoilSimHitsOverlay"
dqm_recoil_ckf.buildHistograms()

# DQM for the tagger with GAS  (Greedy ambiguity solver)
dqm_tagger_gas = tkdqm.TrackingRecoDQM("TaggerGASDQM")
dqm_tagger_gas.track_collection = greedy_solver_tagger.out_trk_collection
dqm_tagger_gas.truth_hit_collection="TaggerSimHitsOverlay"
dqm_tagger_gas.truth_collection = "TaggerTruthTracksOverlay"
dqm_tagger_gas.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_tagger_gas.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_tagger_gas.trackStates = ["target"]
dqm_tagger_gas.title = ""
dqm_tagger_gas.measurement_collection=digi_tagger.out_collection
dqm_tagger_gas.buildHistograms()

# DQM for the recoil with GAS
dqm_recoil_gas = tkdqm.TrackingRecoDQM("RecoilGASDQM")
dqm_recoil_gas.track_collection = greedy_solver_recoil.out_trk_collection
dqm_recoil_gas.truth_collection = "RecoilTruthTracksOverlay"
dqm_recoil_gas.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_recoil_gas.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_recoil_gas.trackStates = ["ecal","target"]
dqm_recoil_gas.title = ""
dqm_recoil_gas.measurement_collection=digi_recoil.out_collection
dqm_recoil_gas.truth_hit_collection = "RecoilSimHitsOverlay"
dqm_recoil_gas.buildHistograms()

# DQM for the tagger with GSF
dqm_tagger_gsf = tkdqm.TrackingRecoDQM("TaggerGSFDQM")
dqm_tagger_gsf.track_collection = GSF_tagger.out_trk_collection
dqm_tagger_gsf.truth_hit_collection="TaggerSimHitsOverlay"
dqm_tagger_gsf.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_tagger_gsf.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_tagger_gsf.truth_collection = "TaggerTruthTracksOverlay"
dqm_tagger_gsf.trackStates = ["target"]
dqm_tagger_gsf.title = ""
dqm_tagger_gsf.measurement_collection=digi_tagger.out_collection
dqm_tagger_gsf.buildHistograms()

# DQM for the recoil with GSF
dqm_recoil_gsf = tkdqm.TrackingRecoDQM("RecoilGSFDQM")
dqm_recoil_gsf.track_collection = GSF_recoil.out_trk_collection
dqm_recoil_gsf.truth_collection = "RecoilTruthTracksOverlay"
dqm_tagger_gsf.ecal_sp_coll_name = "EcalScoringPlaneHitsOverlay"
dqm_tagger_gsf.target_sp_coll_name = 'TargetScoringPlaneHitsOverlay'
dqm_recoil_gsf.trackStates = ["ecal","target"]
dqm_recoil_gsf.title = ""
dqm_recoil_gsf.measurement_collection=digi_recoil.out_collection
dqm_recoil_gsf.truth_hit_collection = "RecoilSimHitsOverlay"
dqm_recoil_gsf.buildHistograms()

dqm_sequence = [
    dqm_seed_tagger,
    dqm_seed_recoil,
    dqm_tagger_ckf,
    dqm_recoil_ckf,
    dqm_tagger_gas,
    dqm_recoil_gas,
    dqm_tagger_gsf,
    dqm_recoil_gsf
]