from LDMX.Tracking import geo, tracking

from LDMX.Tracking.geo import TrackersTrackingGeometryProvider as TrackGeo


class TrackingSequence:
    """Return value of full_tracking_sequence() and related functions.

    Attributes
    ----------
    sequence : list
        Processor sequence (digi + seeds + CKF + ambiguity + GSF [+ veto]).
    dqm_sequence : list
        DQM processor sequence.

    Individual processors are also accessible as attributes, e.g.
    ``trk.digi_recoil``, ``trk.seeder_tagger``, ``trk.GSF_recoil``.
    Charge-digitization processors (``fit_tagger``, ``cluster_recoil``, etc.)
    are only present when ``use_truth_smearing=False``.
    """

    def __init__(self, sequence, dqm_sequence, **processors):
        self.sequence = sequence
        self.dqm_sequence = dqm_sequence
        for name, proc in processors.items():
            setattr(self, name, proc)

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
    use_truth_smearing=True,
    detector="ldmx-det-v15-8gev",
    tag="",
):
    """Build and return the full LDMX tracking processor sequence.

    Parameters
    ----------
    use_truth_smearing : bool
        If True (default), use simple Gaussian smearing (Mode 0).
        If False, use full charge digitization (Mode 1).
    detector : str
        Detector geometry tag.
    tag : str
        Optional prefix applied to all processor instance names and output
        collection names.  Use this when running two chains in the same job
        (e.g. a smear/digi comparison) to avoid name conflicts.
        Example: ``tag="Smear"`` turns ``"TaggerTracks"`` into
        ``"SmearTaggerTracks"``.

    Returns
    -------
    TrackingSequence
        Object with .sequence, .dqm_sequence, .set_overlay(), and individual
        processor attributes (e.g. .digi_recoil, .seeder_tagger, .GSF_recoil).
    """

    def tagged(name):
        return f"{tag}{name}" if tag else name

    TrackGeo.get_instance().set_detector(detector)

    # ------------------------------------------------------------------
    # Truth seeder
    # ------------------------------------------------------------------
    truth_tracking = tracking.TruthSeedProcessor(
        instance_name=tagged("TruthSeedProcessor"),
        recoil_seeds_collection=tagged("RecoilTruthSeeds"),
        tagger_seeds_collection=tagged("TaggerTruthSeeds"),
        tagger_truth_collection=tagged("TaggerTruthTracks"),
        recoil_truth_collection=tagged("RecoilTruthTracks"),
        beam_electrons_collection=tagged("beamElectrons"),
        pdg_ids=[11],
        scoring_hits_coll_name="TargetScoringPlaneHits",
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
            instance_name=tagged("DigitizationProcessor"),
            hit_collection="TaggerSimHits",
            out_collection=tagged("DigiTaggerSimHits"),
            tracker_hit_passname="",
            use_charge_digitization=False,
            do_smearing=True,
            sigma_u=0.006,
            sigma_v=0.0,
            merge_hits=True,
        )

        digi_recoil = tracking.DigitizationProcessor(
            instance_name=tagged("DigitizationProcessorRecoil"),
            hit_collection="RecoilSimHits",
            out_collection=tagged("DigiRecoilSimHits"),
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
        charge_digi_processors = {}

    else:
        digi_tagger = tracking.DigitizationProcessor(
            instance_name=tagged("DigitizationProcessor"),
            hit_collection="TaggerSimHits",
            out_collection=tagged("DigiTaggerSimHits"),
            tracker_hit_passname="",
            use_charge_digitization=True,
            merge_hits=True,
            bias_voltage=200.0,
            depletion_voltage=70.0,
            noise_electrons=1000.0,
            threshold_electrons=3000.0,
            out_raw_collection=tagged("TaggerRawSiStripHits"),
        )

        digi_recoil = tracking.DigitizationProcessor(
            instance_name=tagged("DigitizationProcessorRecoil"),
            hit_collection="RecoilSimHits",
            out_collection=tagged("DigiRecoilSimHits"),
            tracker_hit_passname="",
            use_charge_digitization=True,
            merge_hits=True,
            bias_voltage=200.0,
            depletion_voltage=70.0,
            noise_electrons=1000.0,
            threshold_electrons=3000.0,
            out_raw_collection=tagged("RecoilRawSiStripHits"),
        )

        fit_tagger = tracking.StripFitProcessor(
            instance_name=tagged("StripFitTagger"),
            in_collection=digi_tagger.out_raw_collection,
            out_collection=tagged("TaggerFittedSiStripHits"),
            t_scan_min_ns=-50.0,
            t_scan_max_ns=150.0,
            t_scan_step_ns=1.0,
        )

        fit_recoil = tracking.StripFitProcessor(
            instance_name=tagged("StripFitRecoil"),
            in_collection=digi_recoil.out_raw_collection,
            out_collection=tagged("RecoilFittedSiStripHits"),
            t_scan_min_ns=-50.0,
            t_scan_max_ns=150.0,
            t_scan_step_ns=1.0,
        )

        cluster_tagger = tracking.StripClusterProcessor(
            instance_name=tagged("StripClusterTagger"),
            in_collection=fit_tagger.out_collection,
            out_collection=tagged("TaggerClusterMeasurements"),
            seed_threshold=4.0,
            neighbor_threshold=3.0,
            cluster_threshold=4.0,
        )

        cluster_recoil = tracking.StripClusterProcessor(
            instance_name=tagged("StripClusterRecoil"),
            in_collection=fit_recoil.out_collection,
            out_collection=tagged("RecoilClusterMeasurements"),
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
        charge_digi_processors = dict(
            fit_tagger=fit_tagger,
            fit_recoil=fit_recoil,
            cluster_tagger=cluster_tagger,
            cluster_recoil=cluster_recoil,
        )

    # ------------------------------------------------------------------
    # Seeding
    # ------------------------------------------------------------------
    seeder_tagger = tracking.SeedFinderProcessor(
        instance_name=tagged("SeedTagger"),
        input_hits_collection=tagger_meas_collection,
        out_seed_collection=tagged("TaggerRecoSeeds"),
        pmin=0.03,
        pmax=63.0,
        d0min=-36.9,
        d0max=31.5,
        z0max=54.6,
        thetacut=0.26,
        phicut=0.84,
    )

    seeder_recoil = tracking.SeedFinderProcessor(
        instance_name=tagged("SeedRecoil"),
        perigee_location=[0.0, 0.0, 0.0],
        input_hits_collection=recoil_meas_collection,
        out_seed_collection=tagged("RecoilRecoSeeds"),
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
        instance_name=tagged("Tagger_TrackFinder"),
        tagger_tracking=True,
        seed_coll_name=seeder_tagger.out_seed_collection,
        out_trk_collection=tagged("TaggerTracks"),
        measurement_collection=tagger_meas_collection,
        min_hits=5,
        outlier_pval_=16.5,
    )

    tracking_recoil = tracking.CKFProcessor(
        instance_name=tagged("Recoil_TrackFinder"),
        tagger_tracking=False,
        seed_coll_name=seeder_recoil.out_seed_collection,
        out_trk_collection=tagged("RecoilTracks"),
        measurement_collection=recoil_meas_collection,
        min_hits=5,
        outlier_pval_=22.1,
    )

    # ------------------------------------------------------------------
    # Greedy ambiguity solver
    # ------------------------------------------------------------------
    greedy_solver_tagger = tracking.GreedyAmbiguitySolver(
        instance_name=tagged("GreedySolverTagger"),
        out_trk_collection=tagged("TaggerTracksClean"),
        track_collection=tracking_tagger.out_trk_collection,
        meas_collection=tagger_meas_collection,
    )

    greedy_solver_recoil = tracking.GreedyAmbiguitySolver(
        instance_name=tagged("GreedySolverRecoil"),
        out_trk_collection=tagged("RecoilTracksClean"),
        track_collection=tracking_recoil.out_trk_collection,
        meas_collection=recoil_meas_collection,
    )

    # ------------------------------------------------------------------
    # Gaussian sum filter
    # ------------------------------------------------------------------
    GSF_tagger = tracking.GSFProcessor(
        instance_name=tagged("Tagger_GSF"),
        tagger_tracking=True,
        track_collection=greedy_solver_tagger.out_trk_collection,
        meas_collection=tagger_meas_collection,
        out_trk_collection=tagged("GSFTaggerTracks"),
    )

    GSF_recoil = tracking.GSFProcessor(
        instance_name=tagged("Recoil_GSF"),
        tagger_tracking=False,
        track_collection=greedy_solver_recoil.out_trk_collection,
        meas_collection=recoil_meas_collection,
        out_trk_collection=tagged("GSFRecoilTracks"),
    )

    tracker_veto = tracking.TrackerVetoProcessor(
        instance_name=tagged("TrackerVetoProcessor"),
        tagger_track_collection=tracking_tagger.out_trk_collection,
        recoil_track_collection=tracking_recoil.out_trk_collection,
        output_collection=tagged("TrackerVeto"),
    )

    # ------------------------------------------------------------------
    # DQM
    # ------------------------------------------------------------------
    from LDMX.Tracking import dqm as tkdqm

    dqm_seed_tagger = tkdqm.TrackingRecoDQM(
        instance_name=tagged("SeedTaggerDQM"),
        track_collection=seeder_tagger.out_seed_collection,
        truth_collection=tagged("TaggerTruthTracks"),
        measurement_collection=tagger_meas_collection,
        title="",
    )

    dqm_seed_recoil = tkdqm.TrackingRecoDQM(
        instance_name=tagged("SeedRecoilDQM"),
        track_collection=seeder_recoil.out_seed_collection,
        truth_collection=tagged("RecoilTruthTracks"),
        measurement_collection=recoil_meas_collection,
        title="",
    )

    dqm_tagger_ckf = tkdqm.TrackingRecoDQM(
        instance_name=tagged("TaggerDQM"),
        track_collection=tracking_tagger.out_trk_collection,
        truth_hit_collection="TaggerSimHits",
        truth_collection=tagged("TaggerTruthTracks"),
        track_states=["target"],
        title="",
        measurement_collection=tagger_meas_collection,
    )

    dqm_recoil_ckf = tkdqm.TrackingRecoDQM(
        instance_name=tagged("RecoilDQM"),
        track_collection=tracking_recoil.out_trk_collection,
        truth_collection=tagged("RecoilTruthTracks"),
        track_states=["ecal", "target"],
        title="",
        measurement_collection=recoil_meas_collection,
        truth_hit_collection="RecoilSimHits",
    )

    dqm_tagger_gas = tkdqm.TrackingRecoDQM(
        instance_name=tagged("TaggerGASDQM"),
        track_collection=greedy_solver_tagger.out_trk_collection,
        truth_hit_collection="TaggerSimHits",
        truth_collection=tagged("TaggerTruthTracks"),
        track_states=["target"],
        title="",
        measurement_collection=tagger_meas_collection,
    )

    dqm_recoil_gas = tkdqm.TrackingRecoDQM(
        instance_name=tagged("RecoilGASDQM"),
        track_collection=greedy_solver_recoil.out_trk_collection,
        truth_collection=tagged("RecoilTruthTracks"),
        track_states=["ecal", "target"],
        title="",
        measurement_collection=recoil_meas_collection,
        truth_hit_collection="RecoilSimHits",
    )

    dqm_tagger_gsf = tkdqm.TrackingRecoDQM(
        instance_name=tagged("TaggerGSFDQM"),
        track_collection=GSF_tagger.out_trk_collection,
        truth_hit_collection="TaggerSimHits",
        truth_collection=tagged("TaggerTruthTracks"),
        track_states=["target"],
        title="",
        measurement_collection=tagger_meas_collection,
    )

    dqm_recoil_gsf = tkdqm.TrackingRecoDQM(
        instance_name=tagged("RecoilGSFDQM"),
        track_collection=GSF_recoil.out_trk_collection,
        truth_collection=tagged("RecoilTruthTracks"),
        track_states=["ecal", "target"],
        title="",
        measurement_collection=recoil_meas_collection,
        truth_hit_collection="RecoilSimHits",
    )

    dqm_digi_tagger = tkdqm.DigiDQM(
        instance_name=tagged("TaggerDigiDQM"),
        sim_coll_name="TaggerSimHits",
        digi_coll_name="" if use_truth_smearing else digi_tagger.out_collection,
        fitted_coll_name="" if use_truth_smearing else fit_tagger.out_collection,
        cluster_coll_name="" if use_truth_smearing else cluster_tagger.out_collection,
    )

    dqm_digi_recoil = tkdqm.DigiDQM(
        instance_name=tagged("RecoilDigiDQM"),
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

    return TrackingSequence(
        sequence,
        dqm_sequence,
        truth_tracking=truth_tracking,
        digi_tagger=digi_tagger,
        digi_recoil=digi_recoil,
        seeder_tagger=seeder_tagger,
        seeder_recoil=seeder_recoil,
        tracking_tagger=tracking_tagger,
        tracking_recoil=tracking_recoil,
        greedy_solver_tagger=greedy_solver_tagger,
        greedy_solver_recoil=greedy_solver_recoil,
        GSF_tagger=GSF_tagger,
        GSF_recoil=GSF_recoil,
        tracker_veto=tracker_veto,
        dqm_seed_tagger=dqm_seed_tagger,
        dqm_seed_recoil=dqm_seed_recoil,
        dqm_tagger_ckf=dqm_tagger_ckf,
        dqm_recoil_ckf=dqm_recoil_ckf,
        dqm_tagger_gas=dqm_tagger_gas,
        dqm_recoil_gas=dqm_recoil_gas,
        dqm_tagger_gsf=dqm_tagger_gsf,
        dqm_recoil_gsf=dqm_recoil_gsf,
        dqm_digi_tagger=dqm_digi_tagger,
        dqm_digi_recoil=dqm_digi_recoil,
        **charge_digi_processors,
    )


def recoil_sequence(
    use_truth_smearing=True,
    detector="ldmx-det-v15-8gev",
    tag="",
):
    """Build and return a recoil-only tracking sequence.

    Includes digi, truth seeding, seed finding, CKF, ambiguity solver, and
    GSF for the recoil tracker only. The tracker veto is omitted (it requires
    a tagger track). Useful for studies and configs where tagger reconstruction
    is not needed.

    Parameters
    ----------
    use_truth_smearing : bool
        If True (default), use simple Gaussian smearing.
        If False, use full charge digitization.
    detector : str
        Detector geometry tag.
    tag : str
        Optional prefix for instance and collection names (see
        full_tracking_sequence).

    Returns
    -------
    TrackingSequence
    """
    full = full_tracking_sequence(
        use_truth_smearing=use_truth_smearing, detector=detector, tag=tag
    )

    if use_truth_smearing:
        digi_seq = [full.digi_recoil]
    else:
        digi_seq = [full.digi_recoil, full.fit_recoil, full.cluster_recoil]

    sequence = digi_seq + [
        full.truth_tracking,
        full.seeder_recoil,
        full.tracking_recoil,
        full.greedy_solver_recoil,
        full.GSF_recoil,
    ]

    dqm_sequence = [
        full.dqm_seed_recoil,
        full.dqm_recoil_ckf,
        full.dqm_recoil_gas,
        full.dqm_recoil_gsf,
        full.dqm_digi_recoil,
    ]

    processors = {k: v for k, v in vars(full).items()
                  if k not in ("sequence", "dqm_sequence")}
    return TrackingSequence(sequence, dqm_sequence, **processors)


def tagger_sequence(
    use_truth_smearing=True,
    detector="ldmx-det-v15-8gev",
    tag="",
):
    """Build and return a tagger-only tracking sequence.

    Includes digi, truth seeding, seed finding, CKF, ambiguity solver, and
    GSF for the tagger tracker only. Useful for studies and configs where
    recoil reconstruction is not needed.

    Parameters
    ----------
    use_truth_smearing : bool
        If True (default), use simple Gaussian smearing.
        If False, use full charge digitization.
    detector : str
        Detector geometry tag.
    tag : str
        Optional prefix for instance and collection names (see
        full_tracking_sequence).

    Returns
    -------
    TrackingSequence
    """
    full = full_tracking_sequence(
        use_truth_smearing=use_truth_smearing, detector=detector, tag=tag
    )

    if use_truth_smearing:
        digi_seq = [full.digi_tagger]
    else:
        digi_seq = [full.digi_tagger, full.fit_tagger, full.cluster_tagger]

    sequence = digi_seq + [
        full.truth_tracking,
        full.seeder_tagger,
        full.tracking_tagger,
        full.greedy_solver_tagger,
        full.GSF_tagger,
    ]

    dqm_sequence = [
        full.dqm_seed_tagger,
        full.dqm_tagger_ckf,
        full.dqm_tagger_gas,
        full.dqm_tagger_gsf,
        full.dqm_digi_tagger,
    ]

    processors = {k: v for k, v in vars(full).items()
                  if k not in ("sequence", "dqm_sequence")}
    return TrackingSequence(sequence, dqm_sequence, **processors)


def __getattr__(name):
    """Backward-compatible module-level access.

    Old configs that access module-level attributes directly:
        from LDMX.Tracking import full_tracking_sequence
        p.sequence.extend(full_tracking_sequence.sequence)
        p.sequence.extend(full_tracking_sequence.dqm_sequence)
        full_tracking_sequence.set_overlay(pass_name)
        full_tracking_sequence.digi_recoil  # individual processor
    all work: the default TrackingSequence is built lazily on first access
    and every attribute is cached as a real module global for subsequent lookups.
    """
    if name.startswith("_"):
        raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

    _default = full_tracking_sequence()

    # Cache all processor attributes from the default sequence.
    for attr, val in vars(_default).items():
        globals()[attr] = val
    # Cache the methods that live on the object, not in __dict__.
    globals()["set_overlay"] = _default.set_overlay

    if name in globals():
        return globals()[name]
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")
