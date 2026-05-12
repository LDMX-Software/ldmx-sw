from LDMX.Tracking import geo, tracking
from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as TrackGeo


class TrackingSequence:
    """Return value of full_tracking_sequence().

    Attributes
    ----------
    sequence : list
        Processor sequence (digi + seeds + CKF + ambiguity + GSF + veto).
    dqm_sequence : list
        DQM processor sequence.
    """

    def __init__(self, sequence, dqm_sequence):
        self.sequence = sequence
        self.dqm_sequence = dqm_sequence

    def set_overlay(self, pass_name: str):
        """Modify sequence and dqm_sequence in-place for overlay collections."""

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

        for proc in self.sequence + self.dqm_sequence:
            params = vars(proc)
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


def full_tracking_sequence(
    use_truth_smearing=False,
    detector="ldmx-det-v15-8gev",
):
    """Build and return the full LDMX tracking processor sequence.

    Parameters
    ----------
    use_truth_smearing : bool
        If True, use simple Gaussian smearing (Mode 0).
        If False (default), use full charge digitization (Mode 1).
    detector : str
        Detector geometry tag.

    Returns
    -------
    TrackingSequence
        Object with .sequence, .dqm_sequence, and .set_overlay().
    """

    TrackGeo.get_instance().set_detector(detector)

    # ------------------------------------------------------------------
    # Truth seeder
    # ------------------------------------------------------------------
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

    # ------------------------------------------------------------------
    # Digitization
    # ------------------------------------------------------------------
    if use_truth_smearing:
        digi_tagger = tracking.DigitizationProcessor(
            instance_name="DigitizationProcessor",
            hit_collection="TaggerSimHits",
            out_collection="DigiTaggerSimHits",
            tracker_hit_passname="",
            use_charge_digitization=False,
            do_smearing=True,
            sigma_u=0.006,
            sigma_v=0.0,
            merge_hits=True,
        )

        digi_recoil = tracking.DigitizationProcessor(
            instance_name="DigitizationProcessorRecoil",
            hit_collection="RecoilSimHits",
            out_collection="DigiRecoilSimHits",
            tracker_hit_passname="",
            use_charge_digitization=False,
            do_smearing=True,
            sigma_u=0.006,
            sigma_v=0.0,
            merge_hits=True,
        )

        tagger_meas_collection = digi_tagger.out_collection
        recoil_meas_collection = digi_recoil.out_collection
        digi_sequence = [digi_tagger, digi_recoil]

    else:
        digi_tagger = tracking.DigitizationProcessor(
            instance_name="DigitizationProcessor",
            hit_collection="TaggerSimHits",
            out_collection="DigiTaggerSimHits",
            tracker_hit_passname="",
            use_charge_digitization=True,
            merge_hits=True,
            bias_voltage=200.0,
            depletion_voltage=70.0,
            noise_electrons=1000.0,
            threshold_electrons=3000.0,
            out_raw_collection="TaggerRawSiStripHits",
        )

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

        fit_tagger = tracking.StripFitProcessor(
            instance_name="StripFitTagger",
            in_collection=digi_tagger.out_raw_collection,
            out_collection="TaggerFittedSiStripHits",
            t_scan_min_ns=-50.0,
            t_scan_max_ns=150.0,
            t_scan_step_ns=1.0,
        )

        fit_recoil = tracking.StripFitProcessor(
            instance_name="StripFitRecoil",
            in_collection=digi_recoil.out_raw_collection,
            out_collection="RecoilFittedSiStripHits",
            t_scan_min_ns=-50.0,
            t_scan_max_ns=150.0,
            t_scan_step_ns=1.0,
        )

        cluster_tagger = tracking.StripClusterProcessor(
            instance_name="StripClusterTagger",
            in_collection=fit_tagger.out_collection,
            out_collection="TaggerClusterMeasurements",
            seed_threshold=4.0,
            neighbor_threshold=3.0,
            cluster_threshold=4.0,
        )

        cluster_recoil = tracking.StripClusterProcessor(
            instance_name="StripClusterRecoil",
            in_collection=fit_recoil.out_collection,
            out_collection="RecoilClusterMeasurements",
            seed_threshold=4.0,
            neighbor_threshold=3.0,
            cluster_threshold=4.0,
        )

        tagger_meas_collection = cluster_tagger.out_collection
        recoil_meas_collection = cluster_recoil.out_collection
        digi_sequence = [
            digi_tagger,    digi_recoil,
            fit_tagger,     fit_recoil,
            cluster_tagger, cluster_recoil,
        ]

    # ------------------------------------------------------------------
    # Seeding
    # ------------------------------------------------------------------
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

    # ------------------------------------------------------------------
    # CKF track finding
    # ------------------------------------------------------------------
    tracking_tagger = tracking.CKFProcessor(
        instance_name="Tagger_TrackFinder",
        tagger_tracking=True,
        seed_coll_name=seeder_tagger.out_seed_collection,
        out_trk_collection="TaggerTracks",
        measurement_collection=tagger_meas_collection,
        min_hits=5,
        outlier_pval_=16.5,
    )

    tracking_recoil = tracking.CKFProcessor(
        instance_name="Recoil_TrackFinder",
        tagger_tracking=False,
        seed_coll_name=seeder_recoil.out_seed_collection,
        out_trk_collection="RecoilTracks",
        measurement_collection=recoil_meas_collection,
        min_hits=5,
        outlier_pval_=22.1,
    )

    # ------------------------------------------------------------------
    # Greedy ambiguity solver
    # ------------------------------------------------------------------
    greedy_solver_tagger = tracking.GreedyAmbiguitySolver(
        instance_name="GreedySolverTagger",
        out_trk_collection="TaggerTracksClean",
        track_collection=tracking_tagger.out_trk_collection,
        meas_collection=tagger_meas_collection,
    )

    greedy_solver_recoil = tracking.GreedyAmbiguitySolver(
        instance_name="GreedySolverRecoil",
        out_trk_collection="RecoilTracksClean",
        track_collection=tracking_recoil.out_trk_collection,
        meas_collection=recoil_meas_collection,
    )

    # ------------------------------------------------------------------
    # Gaussian sum filter
    # ------------------------------------------------------------------
    GSF_tagger = tracking.GSFProcessor(
        instance_name="Tagger_GSF",
        tagger_tracking=True,
        track_collection=greedy_solver_tagger.out_trk_collection,
        meas_collection=tagger_meas_collection,
        out_trk_collection="GSFTaggerTracks",
    )

    GSF_recoil = tracking.GSFProcessor(
        instance_name="Recoil_GSF",
        tagger_tracking=False,
        track_collection=greedy_solver_recoil.out_trk_collection,
        meas_collection=recoil_meas_collection,
        out_trk_collection="GSFRecoilTracks",
    )

    tracker_veto = tracking.TrackerVetoProcessor()

    # ------------------------------------------------------------------
    # DQM
    # ------------------------------------------------------------------
    from LDMX.Tracking import dqm as tkdqm

    dqm_seed_tagger = tkdqm.TrackingRecoDQM(
        instance_name="SeedTaggerDQM",
        track_collection=seeder_tagger.out_seed_collection,
        truth_collection="TaggerTruthTracks",
        title="",
    )

    dqm_seed_recoil = tkdqm.TrackingRecoDQM(
        instance_name="SeedRecoilDQM",
        track_collection=seeder_recoil.out_seed_collection,
        truth_collection="RecoilTruthTracks",
        title="",
    )

    dqm_tagger_ckf = tkdqm.TrackingRecoDQM(
        instance_name="TaggerDQM",
        track_collection=tracking_tagger.out_trk_collection,
        truth_hit_collection="TaggerSimHits",
        truth_collection="TaggerTruthTracks",
        track_states=["target"],
        title="",
        measurement_collection=tagger_meas_collection,
    )

    dqm_recoil_ckf = tkdqm.TrackingRecoDQM(
        instance_name="RecoilDQM",
        track_collection=tracking_recoil.out_trk_collection,
        truth_collection="RecoilTruthTracks",
        track_states=["ecal", "target"],
        title="",
        measurement_collection=recoil_meas_collection,
        truth_hit_collection="RecoilSimHits",
    )

    dqm_tagger_gas = tkdqm.TrackingRecoDQM(
        instance_name="TaggerGASDQM",
        track_collection=greedy_solver_tagger.out_trk_collection,
        truth_hit_collection="TaggerSimHits",
        truth_collection="TaggerTruthTracks",
        track_states=["target"],
        title="",
        measurement_collection=tagger_meas_collection,
    )

    dqm_recoil_gas = tkdqm.TrackingRecoDQM(
        instance_name="RecoilGASDQM",
        track_collection=greedy_solver_recoil.out_trk_collection,
        truth_collection="RecoilTruthTracks",
        track_states=["ecal", "target"],
        title="",
        measurement_collection=recoil_meas_collection,
        truth_hit_collection="RecoilSimHits",
    )

    dqm_tagger_gsf = tkdqm.TrackingRecoDQM(
        instance_name="TaggerGSFDQM",
        track_collection=GSF_tagger.out_trk_collection,
        truth_hit_collection="TaggerSimHits",
        truth_collection="TaggerTruthTracks",
        track_states=["target"],
        title="",
        measurement_collection=tagger_meas_collection,
    )

    dqm_recoil_gsf = tkdqm.TrackingRecoDQM(
        instance_name="RecoilGSFDQM",
        track_collection=GSF_recoil.out_trk_collection,
        truth_collection="RecoilTruthTracks",
        track_states=["ecal", "target"],
        title="",
        measurement_collection=recoil_meas_collection,
        truth_hit_collection="RecoilSimHits",
    )

    dqm_digi_tagger = tkdqm.DigiDQM(
        instance_name="TaggerDigiDQM",
        sim_coll_name="TaggerSimHits",
        digi_coll_name="" if use_truth_smearing else digi_tagger.out_collection,
        fitted_coll_name="" if use_truth_smearing else fit_tagger.out_collection,
        cluster_coll_name="" if use_truth_smearing else cluster_tagger.out_collection,
    )

    dqm_digi_recoil = tkdqm.DigiDQM(
        instance_name="RecoilDigiDQM",
        sim_coll_name="RecoilSimHits",
        digi_coll_name="" if use_truth_smearing else digi_recoil.out_collection,
        fitted_coll_name="" if use_truth_smearing else fit_recoil.out_collection,
        cluster_coll_name="" if use_truth_smearing else cluster_recoil.out_collection,
    )

    # ------------------------------------------------------------------
    # Assemble
    # ------------------------------------------------------------------
    sequence = digi_sequence + [
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
        dqm_digi_tagger,
        dqm_digi_recoil,
    ]

    return TrackingSequence(sequence, dqm_sequence)
