# Load the tracking module
from LDMX.Tracking import tracking
from LDMX.Tracking import geo

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo
trackgeo.get_instance().setDetector('ldmx-det-v14-8gev')

# Truth seeder
# Runs truth tracking producing tracks from target scoring plane hits for Recoil
# and generated electros for Tagger.
# Truth tracks can be used for assessing tracking performance or using as seeds
truth_tracking               = tracking.TruthSeedProcessor()
truth_tracking.debug         = True
truth_tracking.trk_coll_name = "RecoilTruthSeeds"
truth_tracking.pdgIDs        = [11]
truth_tracking.scoring_hits  = "TargetScoringPlaneHits"
truth_tracking.z_min         = 0.
truth_tracking.track_id      = -1
truth_tracking.p_cut         = 0.05 # In MeV
truth_tracking.pz_cut        = 0.03
truth_tracking.p_cutEcal     = 0. # In MeV

# Smearing Processor - Tagger
# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together to reduce combinatorics
digi_tagger = tracking.DigitizationProcessor("DigitizationProcessor")
digi_tagger.hit_collection = "TaggerSimHits"
digi_tagger.out_collection = "DigiTaggerSimHits"

# Smearing Processor - Recoil
digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
digi_recoil.hit_collection = "RecoilSimHits"
digi_recoil.out_collection = "DigiRecoilSimHits"

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at the track
# parameters and the impact parameters at the target or generation point. For the tagger one should look
# for compatibility with the beam orbit / beam spot
seeder_tagger = tracking.SeedFinderProcessor("SeedTagger")
seeder_tagger.input_hits_collection =  digi_tagger.out_collection
seeder_tagger.out_seed_collection = "TaggerRecoSeeds"
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
seeder_recoil.out_seed_collection = "RecoilRecoSeeds"
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
tracking_tagger.out_trk_collection = "TaggerTracks"
tracking_tagger.measurement_collection = digi_tagger.out_collection
tracking_tagger.min_hits = 5
tracking_tagger.outlier_pval_ = 16.5

# CKF track finding for recoil tracker using seeds.
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.taggerTracking = False
tracking_recoil.seed_coll_name = seeder_recoil.out_seed_collection
tracking_recoil.out_trk_collection = "RecoilTracks"
# for truth seed based case use 
# tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.measurement_collection = digi_recoil.out_collection
tracking_recoil.min_hits = 6
tracking_recoil.outlier_pval_ =  22.1

# Greedy ambiguity solver for the tagger
greedy_solver_tagger = tracking.GreedyAmbiguitySolver("GreedySolverTagger")
greedy_solver_tagger.out_trk_collection = "TaggerTracksClean"
greedy_solver_tagger.trackCollection = tracking_tagger.out_trk_collection
greedy_solver_tagger.measCollection = digi_tagger.out_collection

# Greedy ambiguity solver for the recoil
greedy_solver_recoil = tracking.GreedyAmbiguitySolver("GreedySolverRecoil")
greedy_solver_recoil.out_trk_collection = "RecoilTracksClean"
greedy_solver_recoil.trackCollection = tracking_recoil.out_trk_collection
greedy_solver_recoil.measCollection = digi_recoil.out_collection

# Gaussian sum filter for the tagger
GSF_tagger = tracking.GSFProcessor("Tagger_GSF")
GSF_tagger.taggerTracking = True
GSF_tagger.trackCollection = greedy_solver_tagger.out_trk_collection
GSF_tagger.measCollection  = digi_tagger.out_collection 
GSF_tagger.out_trk_collection = "GSFTaggerTracks"

# Gaussian sum filter for the recoil
GSF_recoil = tracking.GSFProcessor("Recoil_GSF")
GSF_recoil.taggerTracking = False
GSF_recoil.trackCollection = greedy_solver_recoil.out_trk_collection
GSF_recoil.measCollection  = digi_recoil.out_collection 
GSF_recoil.out_trk_collection = "GSFRecoilTracks"

# Running DQM for the collections above
from LDMX.Tracking import dqm as tkdqm

# Seeder DQM for the tagger
dqm_seed_tagger = tkdqm.TrackingRecoDQM("SeedTaggerDQM")
dqm_seed_tagger.track_collection = seeder_tagger.out_seed_collection
dqm_seed_tagger.truth_collection = "TaggerTruthTracks"
dqm_seed_tagger.title = ""
dqm_seed_tagger.buildHistograms()

# Seeder DQM for the recoil
dqm_seed_recoil = tkdqm.TrackingRecoDQM("SeedRecoilDQM")
dqm_seed_recoil.track_collection = seeder_recoil.out_seed_collection
dqm_seed_recoil.truth_collection = "RecoilTruthTracks"
dqm_seed_recoil.title = ""
dqm_seed_recoil.buildHistograms()

# DQM for the tagger with CKF
dqm_tagger_cfk = tkdqm.TrackingRecoDQM("TaggerDQM")
dqm_tagger_cfk.track_collection = tracking_tagger.out_trk_collection
dqm_tagger_cfk.truth_hit_collection="TaggerSimHits"
dqm_tagger_cfk.truth_collection = "TaggerTruthTracks"
dqm_tagger_cfk.trackStates = ["target"]
dqm_tagger_cfk.title = ""
dqm_tagger_cfk.measurement_collection=digi_tagger.out_collection
dqm_tagger_cfk.buildHistograms()

# DQM for the recoil with CKF
dqm_recoil_cfk = tkdqm.TrackingRecoDQM("RecoilDQM")
dqm_recoil_cfk.track_collection = tracking_recoil.out_trk_collection
dqm_recoil_cfk.truth_collection = "RecoilTruthTracks"
dqm_recoil_cfk.trackStates = ["ecal","target"]
dqm_recoil_cfk.title = ""
dqm_recoil_cfk.measurement_collection=digi_recoil.out_collection
dqm_recoil_cfk.truth_hit_collection = "RecoilSimHits"
dqm_recoil_cfk.buildHistograms()

# DQM for the tagger with GAS  (Greedy ambiguity solver)
dqm_tagger_gas = tkdqm.TrackingRecoDQM("TaggerGASDQM")
dqm_tagger_gas.track_collection = greedy_solver_tagger.out_trk_collection
dqm_tagger_gas.truth_hit_collection="TaggerSimHits"
dqm_tagger_gas.truth_collection = "TaggerTruthTracks"
dqm_tagger_gas.trackStates = ["target"]
dqm_tagger_gas.title = ""
dqm_tagger_gas.measurement_collection=digi_tagger.out_collection
dqm_tagger_gas.buildHistograms()

# DQM for the recoil with GAS
dqm_recoil_gas = tkdqm.TrackingRecoDQM("RecoilGASDQM")
dqm_recoil_gas.track_collection = greedy_solver_recoil.out_trk_collection
dqm_recoil_gas.truth_collection = "RecoilTruthTracks"
dqm_recoil_gas.trackStates = ["ecal","target"]
dqm_recoil_gas.title = ""
dqm_recoil_gas.measurement_collection=digi_recoil.out_collection
dqm_recoil_gas.truth_hit_collection = "RecoilSimHits"
dqm_recoil_gas.buildHistograms()

# DQM for the tagger with GSF
dqm_tagger_gsf = tkdqm.TrackingRecoDQM("TaggerGSFDQM")
dqm_tagger_gsf.track_collection = GSF_tagger.out_trk_collection
dqm_tagger_gsf.truth_hit_collection="TaggerSimHits"
dqm_tagger_gsf.truth_collection = "TaggerTruthTracks"
dqm_tagger_gsf.trackStates = ["target"]
dqm_tagger_gsf.title = ""
dqm_tagger_gsf.measurement_collection=digi_tagger.out_collection
dqm_tagger_gsf.buildHistograms()

# DQM for the recoil with GSF
dqm_recoil_gsf = tkdqm.TrackingRecoDQM("RecoilGSFDQM")
dqm_recoil_gsf.track_collection = GSF_recoil.out_trk_collection
dqm_recoil_gsf.truth_collection = "RecoilTruthTracks"
dqm_recoil_gsf.trackStates = ["ecal","target"]
dqm_recoil_gsf.title = ""
dqm_recoil_gsf.measurement_collection=digi_recoil.out_collection
dqm_recoil_gsf.truth_hit_collection = "RecoilSimHits"
dqm_recoil_gsf.buildHistograms()

tracker_veto = tracking.TrackerVetoProcessor()

# Put it all together into a single sequance
sequence = [
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

dqm_sequence = [
    dqm_seed_tagger,
    dqm_seed_recoil,
    dqm_tagger_cfk,
    dqm_recoil_cfk,
    dqm_tagger_gas,
    dqm_recoil_gas,
    dqm_tagger_gsf,
    dqm_recoil_gsf
]
