# Load the tracking module
from LDMX.Tracking import geo, tracking
from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as trackgeo


trackgeo.get_instance().setDetector("ldmx-det-v15-8gev")

# Truth seeder
# Runs truth tracking producing tracks from target scoring plane hits for Recoil
# and generated electrons for Tagger.
# Truth tracks can be used for assessing tracking performance or using as seeds
truth_tracking = tracking.TruthSeedProcessor(
    debug=True,
    trk_coll_name="RecoilTruthSeeds",
    pdgIDs=[11],
    scoring_hits="TargetScoringPlaneHits",
    z_min=0.0,
    track_id=-1,
    p_cut=0.05,
    pz_cut=0.03,
    p_cutEcal=0.0,
)

# Smearing Processor - Tagger
# Runs G4 hit smearing producing measurements in the Tagger tracker.
# Hits that belong to the same sensor with the same trackID are merged together
# to reduce combinatorics
digi_tagger = tracking.DigitizationProcessor(
    instance_name="DigitizationProcessor",
    hit_collection="TaggerSimHits",
    out_collection="DigiTaggerSimHits",
)

# Smearing Processor - Recoil
digi_recoil = tracking.DigitizationProcessor(
    instance_name="DigitizationProcessorRecoil",
    hit_collection="RecoilSimHits",
    out_collection="DigiRecoilSimHits",
)

# Seed Finder Tagger
# This runs the track seed finder looking for 5 hits in consecutive sensors and
# fitting them with a parabola+linear fit. Compatibility with expected particles
# is checked by looking at the track parameters and the impact parameters at the
# target or generation point. For the tagger one should look for compatibility
# with the beam orbit / beam spot
seeder_tagger = tracking.SeedFinderProcessor(
    instance_name="SeedTagger",
    input_hits_collection=digi_tagger.out_collection,
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
    input_hits_collection=digi_recoil.out_collection,
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
tracking_tagger = tracking.CKFProcessor(
    instance_name="Tagger_TrackFinder",
    taggerTracking=True,
    seed_coll_name=seeder_tagger.out_seed_collection,
    out_trk_collection="TaggerTracks",
    measurement_collection=digi_tagger.out_collection,
    min_hits=5,
    outlier_pval_=16.5,
)

# CKF track finding for recoil tracker using seeds.
tracking_recoil = tracking.CKFProcessor(
    instance_name="Recoil_TrackFinder",
    taggerTracking=False,
    seed_coll_name=seeder_recoil.out_seed_collection,
    out_trk_collection="RecoilTracks",
    measurement_collection=digi_recoil.out_collection,
    min_hits=5,
    outlier_pval_=22.1,
)

# Greedy ambiguity solver for the tagger
greedy_solver_tagger = tracking.GreedyAmbiguitySolver(
    instance_name="GreedySolverTagger",
    out_trk_collection="TaggerTracksClean",
    track_collection=tracking_tagger.out_trk_collection,
    meas_collection=digi_tagger.out_collection,
)

# Greedy ambiguity solver for the recoil
greedy_solver_recoil = tracking.GreedyAmbiguitySolver(
    instance_name="GreedySolverRecoil",
    out_trk_collection="RecoilTracksClean",
    track_collection=tracking_recoil.out_trk_collection,
    meas_collection=digi_recoil.out_collection,
)

# Gaussian sum filter for the tagger
GSF_tagger = tracking.GSFProcessor(
    instance_name="Tagger_GSF",
    taggerTracking=True,
    track_collection=greedy_solver_tagger.out_trk_collection,
    meas_collection=digi_tagger.out_collection,
    out_trk_collection="GSFTaggerTracks",
)

# Gaussian sum filter for the recoil
GSF_recoil = tracking.GSFProcessor(
    instance_name="Recoil_GSF",
    taggerTracking=False,
    track_collection=greedy_solver_recoil.out_trk_collection,
    meas_collection=digi_recoil.out_collection,
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
    trackStates=["target"],
    title="",
    measurement_collection=digi_tagger.out_collection,
)

# DQM for the recoil with CKF
dqm_recoil_ckf = tkdqm.TrackingRecoDQM(
    instance_name="RecoilDQM",
    track_collection=tracking_recoil.out_trk_collection,
    truth_collection="RecoilTruthTracks",
    trackStates=["ecal", "target"],
    title="",
    measurement_collection=digi_recoil.out_collection,
    truth_hit_collection="RecoilSimHits",
)

# DQM for the tagger with GAS (Greedy ambiguity solver)
dqm_tagger_gas = tkdqm.TrackingRecoDQM(
    instance_name="TaggerGASDQM",
    track_collection=greedy_solver_tagger.out_trk_collection,
    truth_hit_collection="TaggerSimHits",
    truth_collection="TaggerTruthTracks",
    trackStates=["target"],
    title="",
    measurement_collection=digi_tagger.out_collection,
)

# DQM for the recoil with GAS
dqm_recoil_gas = tkdqm.TrackingRecoDQM(
    instance_name="RecoilGASDQM",
    track_collection=greedy_solver_recoil.out_trk_collection,
    truth_collection="RecoilTruthTracks",
    trackStates=["ecal", "target"],
    title="",
    measurement_collection=digi_recoil.out_collection,
    truth_hit_collection="RecoilSimHits",
)

# DQM for the tagger with GSF
dqm_tagger_gsf = tkdqm.TrackingRecoDQM(
    instance_name="TaggerGSFDQM",
    track_collection=GSF_tagger.out_trk_collection,
    truth_hit_collection="TaggerSimHits",
    truth_collection="TaggerTruthTracks",
    trackStates=["target"],
    title="",
    measurement_collection=digi_tagger.out_collection,
)

# DQM for the recoil with GSF
dqm_recoil_gsf = tkdqm.TrackingRecoDQM(
    instance_name="RecoilGSFDQM",
    track_collection=GSF_recoil.out_trk_collection,
    truth_collection="RecoilTruthTracks",
    trackStates=["ecal", "target"],
    title="",
    measurement_collection=digi_recoil.out_collection,
    truth_hit_collection="RecoilSimHits",
)

tracker_veto = tracking.TrackerVetoProcessor()

# Put it all together into a single sequence
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
    tracker_veto,
]

dqm_sequence = [
    dqm_seed_tagger,
    dqm_seed_recoil,
    dqm_tagger_ckf,
    dqm_recoil_ckf,
    dqm_tagger_gas,
    dqm_recoil_gas,
    dqm_tagger_gsf,
    dqm_recoil_gsf,
]

def setOverlay(pass_name:str):
    """Modifies full tracking and dqm sequences in-place so that all 
    relevant input/output collections in the tracking processors point 
    to the overlay collections."""

    collection_names_to_update = [ # first, collections that are explicitly overlaid in the OverlayProducer
        "TriggerPad1SimHits", "TriggerPad2SimHits", "TriggerPad3SimHits",
        "TargetSimHits", "EcalSimHits", "HcalSimHits", "TaggerSimHits",
        "RecoilSimHits", "EcalScoringPlaneHits", "TargetScoringPlaneHits",
        "SimParticles"
    ]
    overlay_str = 'Overlay'

    # iterate through all processors, renaming collections as we go
    for proc in sequence + dqm_sequence:
        params = vars(proc) # Python variable assignments are references by default,
                            # so this works to update the processor class variables
        for key, value in params.items():
            if str(key) in ['input_pass_name', 'track_collection_event_passname',
                            'track_passname', 'meas_collection_event_passname', 'meas_passname',
                            'measurement_passname', 'truth_events_passname', 'truth_passname',
                            'track_collection_events_passname', 'input_tagger_pass_name',
                            'input_recoil_pass_name', 'input_collection_events_passname',
                            'tagger_trks_event_collection_passname']:
                params[key] = pass_name
                continue
            if str(value) in collection_names_to_update:
                params[key] += overlay_str
                continue

