# Load the tracking module
from LDMX.Tracking import geo, tracking
from LDMX.Tracking.geo import (
    TrackersTrackingGeometryProvider as TrackGeo,
)


TrackGeo.get_instance().set_detector("ldmx-det-v15-8gev")

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
#use_truth_smearing = True
use_truth_smearing = False

# Truth seeder
# Runs truth tracking producing tracks from target scoring plane hits for Recoil
# and generated electrons for Tagger.
# Truth tracks can be used for assessing tracking performance or using as seeds
truth_tracking = tracking.TruthSeedProcessor(
    debug=True,
    trk_coll_name="RecoilTruthSeeds",
    pdg_ids=[11],
    scoring_hits="TargetScoringPlaneHits",
    z_min=0.0,
    track_id=-1,
    p_cut=0.05,
    pz_cut=0.03,
    p_cut_ecal=0.0,
)

if use_truth_smearing:
    # -----------------------------------------------------------------------
    # Mode: Gaussian smearing
    # DigitizationProcessor produces Measurements directly.
    # -----------------------------------------------------------------------
    digi_tagger = tracking.DigitizationProcessor(
        instance_name="DigitizationProcessor",
        hit_collection="TaggerSimHits",
        out_collection="DigiTaggerSimHits",
        tracker_hit_passname="",
        use_charge_digitization=False,
        do_smearing=True,
        sigma_u=0.006,  # mm
        sigma_v=0.0,    # mm (strips give no V info)
        merge_hits=True,
    )

    digi_recoil = tracking.DigitizationProcessor(
        instance_name="DigitizationProcessorRecoil",
        hit_collection="RecoilSimHits",
        out_collection="DigiRecoilSimHits",
        tracker_hit_passname="",
        use_charge_digitization=False,
        do_smearing=True,
        sigma_u=0.006,  # mm
        sigma_v=0.0,    # mm
        merge_hits=True,
    )

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
    digi_tagger = tracking.DigitizationProcessor(
        instance_name="DigitizationProcessor",
        hit_collection="TaggerSimHits",
        out_collection="DigiTaggerSimHits",
        tracker_hit_passname="",
        use_charge_digitization=True,
        merge_hits=True,
        bias_voltage=200.0,       # V
        depletion_voltage=70.0,   # V
        noise_electrons=1000.0,   # e-
        threshold_electrons=3000.0,  # e-
        out_raw_collection="TaggerRawSiStripHits",
    )

    # ---------------------------------------------------------------------------
    # Charge digitization — Recoil
    # ---------------------------------------------------------------------------
    digi_recoil = tracking.DigitizationProcessor(
        instance_name="DigitizationProcessorRecoil",
        hit_collection="RecoilSimHits",
        out_collection="DigiRecoilSimHits",
        tracker_hit_passname="",
        use_charge_digitization=True,
        merge_hits=True,
        bias_voltage=200.0,
        depletion_voltage=70.0,
        noise_electrons=1000.0,
        threshold_electrons=3000.0,
        out_raw_collection="RecoilRawSiStripHits",
    )

    # ---------------------------------------------------------------------------
    # Pulse fit — Tagger
    #
    # Fits the CR-RC pulse shape to each RawSiStripHit to extract amplitude and
    # hit time.  Parameters must match those used in digi_tagger above.
    # ---------------------------------------------------------------------------
    fit_tagger = tracking.StripFitProcessor(
        instance_name="StripFitTagger",
        in_collection=digi_tagger.out_raw_collection,
        out_collection="TaggerFittedSiStripHits",
        t_scan_min_ns=-50.0,
        t_scan_max_ns=150.0,
        t_scan_step_ns=1.0,
    )

    # ---------------------------------------------------------------------------
    # Pulse fit — Recoil
    # ---------------------------------------------------------------------------
    fit_recoil = tracking.StripFitProcessor(
        instance_name="StripFitRecoil",
        in_collection=digi_recoil.out_raw_collection,
        out_collection="RecoilFittedSiStripHits",
        t_scan_min_ns=-50.0,
        t_scan_max_ns=150.0,
        t_scan_step_ns=1.0,
    )

    # ---------------------------------------------------------------------------
    # Strip clustering — Tagger
    #
    # Nearest-neighbour clustering of FittedSiStripHits into Measurements.
    # Thresholds are in units of noise_sigma (from SiStripConstants).
    # ---------------------------------------------------------------------------
    cluster_tagger = tracking.StripClusterProcessor(
        instance_name="StripClusterTagger",
        in_collection=fit_tagger.out_collection,
        out_collection="TaggerClusterMeasurements",
        seed_threshold=4.0,      # S/N to start a cluster
        neighbor_threshold=3.0,  # S/N to extend a cluster
        cluster_threshold=4.0,   # total S/N to keep a cluster
    )

    # ---------------------------------------------------------------------------
    # Strip clustering — Recoil
    # ---------------------------------------------------------------------------
    cluster_recoil = tracking.StripClusterProcessor(
        instance_name="StripClusterRecoil",
        in_collection=fit_recoil.out_collection,
        out_collection="RecoilClusterMeasurements",
        seed_threshold=4.0,
        neighbor_threshold=3.0,
        cluster_threshold=4.0,
    )

    # Measurements come from the clustering step
    tagger_meas_collection = cluster_tagger.out_collection
    recoil_meas_collection = cluster_recoil.out_collection

    digi_sequence = [
        digi_tagger,    digi_recoil,
        fit_tagger,     fit_recoil,
        cluster_tagger, cluster_recoil,
    ]

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and fitting
# them with a parabola+linear fit. Compatibility with expected particles is checked by
# looking at the track parameters and the impact parameters at the target or generation
# point. For the tagger one should look for compatibility with the beam orbit / beam spot
seeder_tagger = tracking.SeedFinderProcessor(
    instance_name="SeedTagger",
    input_hits_collection=tagger_meas_collection,
    out_seed_collection="TaggerRecoSeeds",
    pmin=0.03,
    pmax=63.0,
    d0min=-36.9,
    d0max=31.5,
    z0max=54.6,
    thetacut=0.26,
    phicut=0.84,
)

# Seed finder processor - Recoil
seeder_recoil = tracking.SeedFinderProcessor(
    instance_name="SeedRecoil",
    perigee_location=[0.0, 0.0, 0.0],
    input_hits_collection=recoil_meas_collection,
    out_seed_collection="RecoilRecoSeeds",
    bfield=1.5,
    pmin=0.04,
    pmax=819.0,
    d0min=-40.2,
    d0max=36.5,
    z0max=40.5,
    thetacut=1.5,
    phicut=1.6,
)

# CKF track finding for tagger tracker using seeds.
# For truth seed based case use seed_coll_name="TaggerTruthSeeds"
tracking_tagger = tracking.CKFProcessor(
    instance_name="Tagger_TrackFinder",
    tagger_tracking=True,
    seed_coll_name=seeder_tagger.out_seed_collection,
    out_trk_collection="TaggerTracks",
    measurement_collection=tagger_meas_collection,
    min_hits=5,
    outlier_pval_=16.5,
)

# CKF track finding for recoil tracker using seeds.
# For truth seed based case use seed_coll_name="RecoilTruthSeeds"
tracking_recoil = tracking.CKFProcessor(
    instance_name="Recoil_TrackFinder",
    tagger_tracking=False,
    seed_coll_name=seeder_recoil.out_seed_collection,
    out_trk_collection="RecoilTracks",
    measurement_collection=recoil_meas_collection,
    min_hits=5,
    outlier_pval_=22.1,
)

# Greedy ambiguity solver for the tagger
greedy_solver_tagger = tracking.GreedyAmbiguitySolver(
    instance_name="GreedySolverTagger",
    out_trk_collection="TaggerTracksClean",
    track_collection=tracking_tagger.out_trk_collection,
    meas_collection=tagger_meas_collection,
)

# Greedy ambiguity solver for the recoil
greedy_solver_recoil = tracking.GreedyAmbiguitySolver(
    instance_name="GreedySolverRecoil",
    out_trk_collection="RecoilTracksClean",
    track_collection=tracking_recoil.out_trk_collection,
    meas_collection=recoil_meas_collection,
)

# Gaussian sum filter for the tagger
GSF_tagger = tracking.GSFProcessor(
    instance_name="Tagger_GSF",
    tagger_tracking=True,
    track_collection=greedy_solver_tagger.out_trk_collection,
    meas_collection=tagger_meas_collection,
    out_trk_collection="GSFTaggerTracks",
)

# Gaussian sum filter for the recoil
GSF_recoil = tracking.GSFProcessor(
    instance_name="Recoil_GSF",
    tagger_tracking=False,
    track_collection=greedy_solver_recoil.out_trk_collection,
    meas_collection=recoil_meas_collection,
    out_trk_collection="GSFRecoilTracks",
)

# Running DQM for the collections above
from LDMX.Tracking import dqm as tkdqm


# Seeder DQM for the tagger
dqm_seed_tagger = tkdqm.TrackingRecoDQM(
    instance_name="SeedTaggerDQM",
    track_collection=seeder_tagger.out_seed_collection,
    truth_collection="TaggerTruthTracks",
    title="",
)

# Seeder DQM for the recoil
dqm_seed_recoil = tkdqm.TrackingRecoDQM(
    instance_name="SeedRecoilDQM",
    track_collection=seeder_recoil.out_seed_collection,
    truth_collection="RecoilTruthTracks",
    title="",
)

# DQM for the tagger with CKF
dqm_tagger_ckf = tkdqm.TrackingRecoDQM(
    instance_name="TaggerDQM",
    track_collection=tracking_tagger.out_trk_collection,
    truth_hit_collection="TaggerSimHits",
    truth_collection="TaggerTruthTracks",
    track_states=["target"],
    title="",
    measurement_collection=tagger_meas_collection,
)

# DQM for the recoil with CKF
dqm_recoil_ckf = tkdqm.TrackingRecoDQM(
    instance_name="RecoilDQM",
    track_collection=tracking_recoil.out_trk_collection,
    truth_collection="RecoilTruthTracks",
    track_states=["ecal", "target"],
    title="",
    measurement_collection=recoil_meas_collection,
    truth_hit_collection="RecoilSimHits",
)

# DQM for the tagger with GAS (Greedy ambiguity solver)
dqm_tagger_gas = tkdqm.TrackingRecoDQM(
    instance_name="TaggerGASDQM",
    track_collection=greedy_solver_tagger.out_trk_collection,
    truth_hit_collection="TaggerSimHits",
    truth_collection="TaggerTruthTracks",
    track_states=["target"],
    title="",
    measurement_collection=tagger_meas_collection,
)

# DQM for the recoil with GAS
dqm_recoil_gas = tkdqm.TrackingRecoDQM(
    instance_name="RecoilGASDQM",
    track_collection=greedy_solver_recoil.out_trk_collection,
    truth_collection="RecoilTruthTracks",
    track_states=["ecal", "target"],
    title="",
    measurement_collection=recoil_meas_collection,
    truth_hit_collection="RecoilSimHits",
)

# DQM for the tagger with GSF
dqm_tagger_gsf = tkdqm.TrackingRecoDQM(
    instance_name="TaggerGSFDQM",
    track_collection=GSF_tagger.out_trk_collection,
    truth_hit_collection="TaggerSimHits",
    truth_collection="TaggerTruthTracks",
    track_states=["target"],
    title="",
    measurement_collection=tagger_meas_collection,
)

# DQM for the recoil with GSF
dqm_recoil_gsf = tkdqm.TrackingRecoDQM(
    instance_name="RecoilGSFDQM",
    track_collection=GSF_recoil.out_trk_collection,
    truth_collection="RecoilTruthTracks",
    track_states=["ecal", "target"],
    title="",
    measurement_collection=recoil_meas_collection,
    truth_hit_collection="RecoilSimHits",
)

# ---------------------------------------------------------------------------
# Digitization DQM — per-sensor sim/digi/strip/cluster characterization
# ---------------------------------------------------------------------------
dqm_digi_tagger = tkdqm.DigiDQM(
    instance_name="TaggerDigiDQM",
    sim_coll_name="TaggerSimHits",
    digi_coll_name=digi_tagger.out_collection,
    fitted_coll_name="" if use_truth_smearing else fit_tagger.out_collection,
    cluster_coll_name="" if use_truth_smearing else cluster_tagger.out_collection,
)

dqm_digi_recoil = tkdqm.DigiDQM(
    instance_name="RecoilDigiDQM",
    sim_coll_name="RecoilSimHits",
    digi_coll_name=digi_recoil.out_collection,
    fitted_coll_name="" if use_truth_smearing else fit_recoil.out_collection,
    cluster_coll_name="" if use_truth_smearing else cluster_recoil.out_collection,
)

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


def set_overlay(pass_name: str):
    """Modifies full tracking and dqm sequences in-place so that all
    relevant input/output collections in the tracking processors point
    to the overlay collections."""

    # first, collections that are explicitly overlaid in the OverlayProducer
    collection_names_to_update = [
        "TriggerPad1SimHits",
        "TriggerPad2SimHits",
        "TriggerPad3SimHits",
        "TargetSimHits",
        "EcalSimHits",
        "HcalSimHits",
        "TaggerSimHits",
        "RecoilSimHits",
        "EcalScoringPlaneHits",
        "TargetScoringPlaneHits",
        "SimParticles",
    ]
    overlay_str = "Overlay"

    # iterate through all processors, renaming collections as we go
    for proc in sequence + dqm_sequence:
        params = vars(proc)  # Python variable assignments are references by default,
        # so this works to update the processor class variables
        for key, value in params.items():
            if str(key) in [
                "input_pass_name",
                "track_collection_event_passname",
                "track_passname",
                "meas_collection_event_passname",
                "meas_passname",
                "measurement_passname",
                "truth_events_passname",
                "truth_passname",
                "track_collection_events_passname",
                "input_tagger_pass_name",
                "input_recoil_pass_name",
                "input_collection_events_passname",
                "tagger_trks_event_collection_passname",
            ]:
                params[key] = pass_name
                continue
            if str(value) in collection_names_to_update:
                params[key] += overlay_str
                continue
