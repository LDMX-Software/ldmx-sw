# Load the tracking module
from LDMX.Tracking import geo, tracking
from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo


trackgeo.get_instance().setDetector('ldmx-det-v15-8gev')

# ---------------------------------------------------------------------------
# Mode selection
#
#   use_truth_smearing = False  (default)
#       Full charge digitization chain:
#           SimTrackerHit -> DigitizationProcessor (charge sim)
#                        -> RawSiStripHit
#                        -> StripFitProcessor
#                        -> FittedSiStripHit
#                        -> StripClusterProcessor
#                        -> Measurement
#
#   use_truth_smearing = True
#       Simple Gaussian smearing:
#           SimTrackerHit -> DigitizationProcessor (Gaussian smear)
#                        -> Measurement
#
# Set this flag before importing this module, or edit it here directly.
# ---------------------------------------------------------------------------
use_truth_smearing = False

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

if use_truth_smearing:
    # -----------------------------------------------------------------------
    # Mode: Gaussian smearing
    # DigitizationProcessor produces Measurements directly.
    # -----------------------------------------------------------------------
    digi_tagger = tracking.DigitizationProcessor("DigitizationProcessor")
    digi_tagger.hit_collection          = "TaggerSimHits"
    digi_tagger.out_collection          = "DigiTaggerSimHits"
    digi_tagger.tracker_hit_passname    = ""
    digi_tagger.use_charge_digitization = False
    digi_tagger.do_smearing             = True
    digi_tagger.sigma_u                 = 0.006  # mm
    digi_tagger.sigma_v                 = 0.0    # mm (strips give no V info)
    digi_tagger.merge_hits              = True

    digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
    digi_recoil.hit_collection          = "RecoilSimHits"
    digi_recoil.out_collection          = "DigiRecoilSimHits"
    digi_recoil.tracker_hit_passname    = ""
    digi_recoil.use_charge_digitization = False
    digi_recoil.do_smearing             = True
    digi_recoil.sigma_u                 = 0.006  # mm
    digi_recoil.sigma_v                 = 0.0    # mm
    digi_recoil.merge_hits              = True

    # Measurements come directly from the digitization processor
    tagger_meas_collection = digi_tagger.out_collection
    recoil_meas_collection = digi_recoil.out_collection

    digi_sequence = [digi_tagger, digi_recoil]

else:
    # -----------------------------------------------------------------------
    # Mode: Full charge digitization
    # -----------------------------------------------------------------------

    # ---------------------------------------------------------------------------
    # Charge digitization — Tagger
    #
    # Replaces the simple Gaussian smearing with realistic silicon-strip charge
    # simulation (CDFSiSensorSim-style physics).  Two outputs are produced:
    #   DigiTaggerSimHits    — Measurements from the direct charge-weighted
    #                          centroid (compatible with legacy downstream).
    #   TaggerRawSiStripHits — Per-strip ADC waveforms (3 samples, CR-RC
    #                          pulse shape) for the full realistic reco chain.
    # ---------------------------------------------------------------------------
    digi_tagger = tracking.DigitizationProcessor("DigitizationProcessor")
    digi_tagger.hit_collection          = "TaggerSimHits"
    digi_tagger.out_collection          = "DigiTaggerSimHits"
    digi_tagger.tracker_hit_passname    = ""
    digi_tagger.use_charge_digitization = True
    digi_tagger.merge_hits              = True
    # Sensor bias / depletion
    digi_tagger.bias_voltage            = 200.0   # V
    digi_tagger.depletion_voltage       = 70.0    # V
    # Noise & threshold
    digi_tagger.noise_electrons         = 1000.0  # e-
    digi_tagger.threshold_electrons     = 3000.0  # e-
    digi_tagger.out_raw_collection      = "TaggerRawSiStripHits"

    # ---------------------------------------------------------------------------
    # Charge digitization — Recoil
    # ---------------------------------------------------------------------------
    digi_recoil = tracking.DigitizationProcessor("DigitizationProcessorRecoil")
    digi_recoil.hit_collection          = "RecoilSimHits"
    digi_recoil.out_collection          = "DigiRecoilSimHits"
    digi_recoil.tracker_hit_passname    = ""
    digi_recoil.use_charge_digitization = True
    digi_recoil.merge_hits              = True
    digi_recoil.bias_voltage            = 200.0
    digi_recoil.depletion_voltage       = 70.0
    digi_recoil.noise_electrons         = 1000.0
    digi_recoil.threshold_electrons     = 3000.0
    digi_recoil.out_raw_collection      = "RecoilRawSiStripHits"

    # ---------------------------------------------------------------------------
    # Pulse fit — Tagger
    #
    # Fits the CR-RC pulse shape to each RawSiStripHit to extract amplitude and
    # hit time.  Parameters must match those used in digi_tagger above.
    # ---------------------------------------------------------------------------
    fit_tagger = tracking.StripFitProcessor("StripFitTagger")
    fit_tagger.in_collection          = digi_tagger.out_raw_collection
    fit_tagger.out_collection         = "TaggerFittedSiStripHits"
    fit_tagger.t_scan_min_ns          = -50.0
    fit_tagger.t_scan_max_ns          = 150.0
    fit_tagger.t_scan_step_ns         = 1.0

    # ---------------------------------------------------------------------------
    # Pulse fit — Recoil
    # ---------------------------------------------------------------------------
    fit_recoil = tracking.StripFitProcessor("StripFitRecoil")
    fit_recoil.in_collection          = digi_recoil.out_raw_collection
    fit_recoil.out_collection         = "RecoilFittedSiStripHits"
    fit_recoil.t_scan_min_ns          = -50.0
    fit_recoil.t_scan_max_ns          = 150.0
    fit_recoil.t_scan_step_ns         = 1.0

    # ---------------------------------------------------------------------------
    # Strip clustering — Tagger
    #
    # Nearest-neighbour clustering of FittedSiStripHits into Measurements.
    # Thresholds are in units of noise_sigma (from SiStripConstants).
    # ---------------------------------------------------------------------------
    cluster_tagger = tracking.StripClusterProcessor("StripClusterTagger")
    cluster_tagger.in_collection        = fit_tagger.out_collection
    cluster_tagger.out_collection       = "TaggerClusterMeasurements"
    cluster_tagger.seed_threshold       = 4.0    # S/N to start a cluster
    cluster_tagger.neighbor_threshold   = 3.0    # S/N to extend a cluster
    cluster_tagger.cluster_threshold    = 4.0    # total S/N to keep a cluster

    # ---------------------------------------------------------------------------
    # Strip clustering — Recoil
    # ---------------------------------------------------------------------------
    cluster_recoil = tracking.StripClusterProcessor("StripClusterRecoil")
    cluster_recoil.in_collection        = fit_recoil.out_collection
    cluster_recoil.out_collection       = "RecoilClusterMeasurements"
    cluster_recoil.seed_threshold       = 4.0
    cluster_recoil.neighbor_threshold   = 3.0
    cluster_recoil.cluster_threshold    = 4.0

    # Measurements come from the clustering step
    tagger_meas_collection = cluster_tagger.out_collection
    recoil_meas_collection = cluster_recoil.out_collection

    digi_sequence = [
        digi_tagger,  digi_recoil,
        fit_tagger,   fit_recoil,
        cluster_tagger, cluster_recoil,
    ]

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting
# them with a
# parabola+linear fit. Compatibility with expected particles is checked by looking at
# the track
# parameters and the impact parameters at the target or generation point. For the tagger
# one should look
# for compatibility with the beam orbit / beam spot
seeder_tagger = tracking.SeedFinderProcessor("SeedTagger")
seeder_tagger.input_hits_collection = tagger_meas_collection
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
seeder_recoil.input_hits_collection = recoil_meas_collection
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
tracking_tagger.measurement_collection = tagger_meas_collection
tracking_tagger.min_hits = 5
tracking_tagger.outlier_pval_ = 16.5

# CKF track finding for recoil tracker using seeds.
tracking_recoil  = tracking.CKFProcessor("Recoil_TrackFinder")
tracking_recoil.taggerTracking = False
tracking_recoil.seed_coll_name = seeder_recoil.out_seed_collection
tracking_recoil.out_trk_collection = "RecoilTracks"
# for truth seed based case use
# tracking_recoil.seed_coll_name = "RecoilTruthSeeds"
tracking_recoil.measurement_collection = recoil_meas_collection
tracking_recoil.min_hits = 5
tracking_recoil.outlier_pval_ =  22.1

# Greedy ambiguity solver for the tagger
greedy_solver_tagger = tracking.GreedyAmbiguitySolver("GreedySolverTagger")
greedy_solver_tagger.out_trk_collection = "TaggerTracksClean"
greedy_solver_tagger.track_collection = tracking_tagger.out_trk_collection
greedy_solver_tagger.meas_collection = tagger_meas_collection

# Greedy ambiguity solver for the recoil
greedy_solver_recoil = tracking.GreedyAmbiguitySolver("GreedySolverRecoil")
greedy_solver_recoil.out_trk_collection = "RecoilTracksClean"
greedy_solver_recoil.track_collection = tracking_recoil.out_trk_collection
greedy_solver_recoil.meas_collection = recoil_meas_collection

# Gaussian sum filter for the tagger
GSF_tagger = tracking.GSFProcessor("Tagger_GSF")
GSF_tagger.taggerTracking = True
GSF_tagger.track_collection = greedy_solver_tagger.out_trk_collection
GSF_tagger.meas_collection  = tagger_meas_collection
GSF_tagger.out_trk_collection = "GSFTaggerTracks"

# Gaussian sum filter for the recoil
GSF_recoil = tracking.GSFProcessor("Recoil_GSF")
GSF_recoil.taggerTracking = False
GSF_recoil.track_collection = greedy_solver_recoil.out_trk_collection
GSF_recoil.meas_collection  = recoil_meas_collection
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
dqm_tagger_ckf = tkdqm.TrackingRecoDQM("TaggerDQM")
dqm_tagger_ckf.track_collection = tracking_tagger.out_trk_collection
dqm_tagger_ckf.truth_hit_collection="TaggerSimHits"
dqm_tagger_ckf.truth_collection = "TaggerTruthTracks"
dqm_tagger_ckf.trackStates = ["target"]
dqm_tagger_ckf.title = ""
dqm_tagger_ckf.measurement_collection = tagger_meas_collection
dqm_tagger_ckf.buildHistograms()

# DQM for the recoil with CKF
dqm_recoil_ckf = tkdqm.TrackingRecoDQM("RecoilDQM")
dqm_recoil_ckf.track_collection = tracking_recoil.out_trk_collection
dqm_recoil_ckf.truth_collection = "RecoilTruthTracks"
dqm_recoil_ckf.trackStates = ["ecal","target"]
dqm_recoil_ckf.title = ""
dqm_recoil_ckf.measurement_collection = recoil_meas_collection
dqm_recoil_ckf.truth_hit_collection = "RecoilSimHits"
dqm_recoil_ckf.buildHistograms()

# DQM for the tagger with GAS  (Greedy ambiguity solver)
dqm_tagger_gas = tkdqm.TrackingRecoDQM("TaggerGASDQM")
dqm_tagger_gas.track_collection = greedy_solver_tagger.out_trk_collection
dqm_tagger_gas.truth_hit_collection="TaggerSimHits"
dqm_tagger_gas.truth_collection = "TaggerTruthTracks"
dqm_tagger_gas.trackStates = ["target"]
dqm_tagger_gas.title = ""
dqm_tagger_gas.measurement_collection = tagger_meas_collection
dqm_tagger_gas.buildHistograms()

# DQM for the recoil with GAS
dqm_recoil_gas = tkdqm.TrackingRecoDQM("RecoilGASDQM")
dqm_recoil_gas.track_collection = greedy_solver_recoil.out_trk_collection
dqm_recoil_gas.truth_collection = "RecoilTruthTracks"
dqm_recoil_gas.trackStates = ["ecal","target"]
dqm_recoil_gas.title = ""
dqm_recoil_gas.measurement_collection = recoil_meas_collection
dqm_recoil_gas.truth_hit_collection = "RecoilSimHits"
dqm_recoil_gas.buildHistograms()

# DQM for the tagger with GSF
dqm_tagger_gsf = tkdqm.TrackingRecoDQM("TaggerGSFDQM")
dqm_tagger_gsf.track_collection = GSF_tagger.out_trk_collection
dqm_tagger_gsf.truth_hit_collection="TaggerSimHits"
dqm_tagger_gsf.truth_collection = "TaggerTruthTracks"
dqm_tagger_gsf.trackStates = ["target"]
dqm_tagger_gsf.title = ""
dqm_tagger_gsf.measurement_collection = tagger_meas_collection
dqm_tagger_gsf.buildHistograms()

# DQM for the recoil with GSF
dqm_recoil_gsf = tkdqm.TrackingRecoDQM("RecoilGSFDQM")
dqm_recoil_gsf.track_collection = GSF_recoil.out_trk_collection
dqm_recoil_gsf.truth_collection = "RecoilTruthTracks"
dqm_recoil_gsf.trackStates = ["ecal","target"]
dqm_recoil_gsf.title = ""
dqm_recoil_gsf.measurement_collection = recoil_meas_collection
dqm_recoil_gsf.truth_hit_collection = "RecoilSimHits"
dqm_recoil_gsf.buildHistograms()

# ---------------------------------------------------------------------------
# Digitization DQM — per-sensor sim/digi/strip/cluster characterization
# ---------------------------------------------------------------------------
dqm_digi_tagger = tkdqm.DigiDQM("TaggerDigiDQM")
dqm_digi_tagger.sim_coll_name     = "TaggerSimHits"
dqm_digi_tagger.digi_coll_name    = digi_tagger.out_collection
dqm_digi_tagger.fitted_coll_name  = "" if use_truth_smearing else fit_tagger.out_collection
dqm_digi_tagger.cluster_coll_name = "" if use_truth_smearing else cluster_tagger.out_collection
dqm_digi_tagger.buildHistograms()

dqm_digi_recoil = tkdqm.DigiDQM("RecoilDigiDQM")
dqm_digi_recoil.sim_coll_name     = "RecoilSimHits"
dqm_digi_recoil.digi_coll_name    = digi_recoil.out_collection
dqm_digi_recoil.fitted_coll_name  = "" if use_truth_smearing else fit_recoil.out_collection
dqm_digi_recoil.cluster_coll_name = "" if use_truth_smearing else cluster_recoil.out_collection
dqm_digi_recoil.buildHistograms()

tracker_veto = tracking.TrackerVetoProcessor()

# Put it all together into a single sequence
sequence = (
    digi_sequence
    + [
        truth_tracking,
        seeder_tagger,
        seeder_recoil,
        tracking_tagger,
        tracking_recoil,
        greedy_solver_tagger,
        greedy_solver_recoil,
        GSF_tagger,
        GSF_recoil,
        tracker_veto,
    ]
)

dqm_sequence = [
    dqm_seed_tagger,
    dqm_seed_recoil,
    dqm_tagger_ckf,
    dqm_recoil_ckf,
    dqm_tagger_gas,
    dqm_recoil_gas,
    dqm_tagger_gsf,
    dqm_recoil_gsf,
    dqm_digi_tagger,
    dqm_digi_recoil,
]
