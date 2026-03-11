from LDMX.Framework import Processor, processor


@processor("tracking::dqm::TrackerDigiDQM", "Tracking")
class TrackerDigiDQM(Processor):
    """DQM analyzer for tracker digitization.

    Attributes
    ----------
    output_measurements_passname : str
        The pass name of the output measurements.
    measurements_passname : str
        The pass name of the measurements.
    """

    output_measurements_passname: str = ""
    measurements_passname: str = ""

    def __post_init__(self):
        for i in range(0, 14):
            self.histogram(
                "global_yz_l%s" % i,
                "Global y (mm)", 70, -50, 20,
                "Global z (mm)", 100, -50, 50,
            )
            self.histogram(
                "local_uv_l%s" % i,
                "u (mm)", 60, -30, 30,
                "v (mm)", 100, -50, 50,
            )
            self.histogram(
                "time_l%s" % i,
                "Time (ns)", 100, 0, 100,
            )

        self.histogram(
            "global_xy",
            "Global x (mm)", 100, -1000, 0,
            "Global y (mm)", 100, -50, 50,
        )


@processor("tracking::dqm::TrackingRecoDQM", "Tracking")
class TrackingRecoDQM(Processor):
    """DQM analyzer for tracking reconstruction.

    Attributes
    ----------
    nbins : int
        Number of bins for histograms.
    d0min : float
        Minimum d0 for histograms.
    d0max : float
        Maximum d0 for histograms.
    z0min : float
        Minimum z0 for histograms.
    z0max : float
        Maximum z0 for histograms.
    phimin : float
        Minimum phi for histograms.
    phimax : float
        Maximum phi for histograms.
    thetamin : float
        Minimum theta for histograms.
    thetamax : float
        Maximum theta for histograms.
    qopmin : float
        Minimum q/p for histograms.
    qopmax : float
        Maximum q/p for histograms.
    pmax : float
        Maximum momentum for histograms.
    trackStates : list[str]
        Track states to build histograms for.
    doTruth : bool
        Whether to build truth histograms.
    track_collection : str
        Name of the track collection.
    truth_collection : str
        Name of the truth collection.
    measurement_collection : str
        Name of the measurement collection.
    measurement_passname : str
        The pass name of the measurements.
    ecal_sp_events_passname : str
        The events pass name for ecal scoring plane.
    ecal_sp_passname : str
        The pass name for ecal scoring plane.
    target_sp_events_passname : str
        The events pass name for target scoring plane.
    target_sp_passname : str
        The pass name for target scoring plane.
    truth_events_passname : str
        The events pass name for truth.
    truth_passname : str
        The pass name for truth.
    track_collection_events_passname : str
        The events pass name for track collection.
    track_passname : str
        The pass name for tracks.
    """

    nbins: int = 1000
    d0min: float = -50.0
    d0max: float = 50.0
    z0min: float = -200.0
    z0max: float = 200.0
    phimin: float = -0.1
    phimax: float = 0.1
    thetamin: float = 1.4
    thetamax: float = 1.7
    qopmin: float = -10.0
    qopmax: float = 10.0
    pmax: float = 10.0
    trackStates: list[str] = ["ecal", "target"]
    doTruth: bool = True
    track_collection: str = "TruthTracks"
    truth_collection: str = "TaggerTruthTracks"
    truth_hit_collection: str = ""
    measurement_collection: str = "DigiTaggerSimHits"
    measurement_passname: str = ""
    title: str = ""
    ecal_sp_events_passname: str = ""
    ecal_sp_passname: str = ""
    target_sp_events_passname: str = ""
    target_sp_passname: str = ""
    truth_events_passname: str = ""
    truth_passname: str = ""
    track_collection_events_passname: str = ""
    track_passname: str = ""

    def __post_init__(self):
        nbins = self.nbins
        d0min = self.d0min
        d0max = self.d0max
        z0min = self.z0min
        z0max = self.z0max
        phimin = self.phimin
        phimax = self.phimax
        thetamin = self.thetamin
        thetamax = self.thetamax
        qopmin = self.qopmin
        qopmax = self.qopmax
        pmax = self.pmax

        self.histogram("N_tracks", "N tracks", 10, 0, 10)
        self.histogram("d0", "d0 [mm]", nbins, d0min, d0max)
        self.histogram("z0", "z0 [mm]", nbins, z0min, z0max)
        self.histogram("phi", "#phi [rad]", nbins, phimin, phimax)
        self.histogram("theta", "#theta [rad]", nbins, thetamin, thetamax)
        self.histogram("p", "P [GeV]", nbins, 0, pmax)
        self.histogram("qop", "qOverP [GeV^{-1}]", nbins, qopmin, qopmax)
        self.histogram("pt_bending", "pT bending plane [GeV]", nbins, 0.0, pmax)
        self.histogram("pt_beam", "pT beam axis [GeV]", nbins, 0, 0.5)
        self.histogram("nHits", "nHits", 15, 0, 15)
        self.histogram("layers_hit", "Num layers hit", 15, 0, 15)
        self.histogram("measurement_dedx", "Measurement dE/dx (MeV/mm)", 60, 0.0, 0.6)
        self.histogram("Chi2", "Chi2", nbins, 0, 100)
        self.histogram("ndf", "ndf", 10, 0, 10)
        self.histogram("Chi2_per_ndf", "Chi2/ndf", nbins, 0, 10)
        self.histogram("nShared", "nShared", 5, 0, 5)
        self.histogram("nHoles", "nHoles", 5, 0, 5)
        self.histogram("px", "pX [GeV]", nbins, 0.0, pmax)
        self.histogram("py", "pY [GeV]", nbins, -pmax / 20.0, pmax / 20.0)
        self.histogram("pz", "pZ [GeV]", nbins, -pmax / 20.0, pmax / 20.0)
        self.histogram("d0_err", "#sigma_{d0} [mm]", nbins, 0, 0.2)
        self.histogram("z0_err", "#sigma_{z0} [mm]", nbins, 0, 0.7)
        self.histogram("phi_err", "#sigma_{#phi} [rad]", nbins, 0, 0.04)
        self.histogram("theta_err", "#sigma_{#theta} [rad]", nbins, 0, 0.06)
        self.histogram("qop_err", "#sigma_{qOp} [GeV^{-1}]", nbins, 0, 1)
        self.histogram("p_err", "#sigma_{p} [GeV]", nbins, 0, 1)

        self.histogram(
            "d0_err_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "#sigma_{d0} [mm]", nbins, 0, 0.2,
        )
        self.histogram(
            "z0_err_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "#sigma_{z0} [mm]", nbins, 0, 0.8,
        )
        self.histogram(
            "p_err_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "#sigma_{p} [GeV]", nbins, 0, 1,
        )
        self.histogram(
            "p_err_vs_p_8hits",
            "p [GeV]", nbins, 0, pmax,
            "#sigma_{p} [GeV]", nbins, 0, 1,
        )
        self.histogram(
            "p_err_vs_p_9hits",
            "p [GeV]", nbins, 0, pmax,
            "#sigma_{p} [GeV]", nbins, 0, 1,
        )
        self.histogram(
            "p_err_vs_p_10hits",
            "p [GeV]", nbins, 0, pmax,
            "#sigma_{p} [GeV]", nbins, 0, 1,
        )
        self.histogram(
            "res_p_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "res_{p} [GeV]", nbins, -3, 3,
        )
        self.histogram(
            "res_qop_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "res_{qop} [GeV^{-1}]", nbins, -0.5, 0.5,
        )
        self.histogram(
            "res_d0_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "res_{d0} [mm]", nbins, -0.05, 0.05,
        )
        self.histogram(
            "res_z0_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "res_{z0} [mm]", nbins, -0.5, 0.5,
        )
        self.histogram(
            "res_phi_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "res_{#phi}", nbins, -0.005, 0.005,
        )
        self.histogram(
            "res_theta_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "res_{#theta}", nbins, -0.01, 0.01,
        )
        self.histogram(
            "res_p_vs_p_8hits",
            "p [GeV]", nbins, 0, pmax,
            "res_{p} [GeV]", nbins, -3, 3,
        )
        self.histogram(
            "res_p_vs_p_9hits",
            "p [GeV]", nbins, 0, pmax,
            "res_{p} [GeV]", nbins, -3, 3,
        )
        self.histogram(
            "res_p_vs_p_10hits",
            "p [GeV]", nbins, 0, pmax,
            "res_{p} [GeV]", nbins, -3, 3,
        )
        self.histogram(
            "pull_qop_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "pull_{qop}", nbins, -5, 5,
        )
        self.histogram(
            "pull_d0_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "pull_{d0}", nbins, -5, 5,
        )
        self.histogram(
            "pull_z0_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "pull_{z0}", nbins, -5, 5,
        )
        self.histogram(
            "pull_phi_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "pull_{#phi}", nbins, -5, 5,
        )
        self.histogram(
            "pull_theta_vs_p",
            "p [GeV]", nbins, 0, pmax,
            "pull_{#theta}", nbins, -5, 5,
        )
        self.histogram(
            "res_pt_beam_vs_p",
            "truth p [GeV]", nbins, 0, pmax,
            "res_{pt} beam", nbins, -0.5, 0.5,
        )

        if self.doTruth:
            self.histogram("truth_N_tracks", "truth_N tracks", 10, 0, 10)
            self.histogram("truth_nHits", "truth nHits", 15, 0, 15)
            self.histogram("truth_layers_hit", "truth layers hit", 15, 0, 15)
            self.histogram("truth_d0", "truth d0 [mm]", nbins, d0min, d0max)
            self.histogram("truth_z0", "truth z0 [mm]", nbins, z0min, z0max)
            self.histogram("truth_phi", "truth #phi", nbins, phimin, phimax)
            self.histogram("truth_theta", "truth #theta", nbins, thetamin, thetamax)
            self.histogram("truth_qop", "truth q/p [GeV^{-1}]", nbins, qopmin, qopmax)
            self.histogram("truth_p", "truth p [GeV]", nbins, 0.0, pmax)
            self.histogram("truth_beam_angle", "angle wrt beam axis", 20, 0, 2)
            self.histogram("truth_PID", "Particles", 8, -4, 4)
            self.histogram(
                "truth_measurement_dedx",
                "Truth measurement dE/dx (MeV/mm)", 60, 0.0, 0.6,
            )
            self.histogram("truth_kminus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("truth_kplus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("truth_piminus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("truth_piplus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("truth_electron_p", "truth p", nbins, 0.0, pmax)
            self.histogram("truth_positron_p", "truth p", nbins, 0.0, pmax)
            self.histogram("truth_proton_p", "truth p", nbins, 0.0, pmax)

            # Residuals
            self.histogram("res_d0", "res d0 [mm]", nbins, -0.2, 0.2)
            self.histogram("res_z0", "res z0 [mm]", nbins, -0.6, 0.6)
            self.histogram("res_phi", "res #phi", nbins, -0.02, 0.02)
            self.histogram("res_theta", "res #theta", nbins, -0.04, 0.04)
            self.histogram("res_p", "res p [GeV]", nbins, -1, 1)
            self.histogram("res_pt_beam", "res pt beam [GeV]", nbins, -1, 1)
            self.histogram("res_qop", "res q/p [GeV^{-1}]", nbins, -1, 1)

            # Pulls
            self.histogram("pull_d0", "pull d0", 100, -5, 5)
            self.histogram("pull_z0", "pull z0", 100, -5, 5)
            self.histogram("pull_phi", "pull #phi", 100, -5, 5)
            self.histogram("pull_theta", "pull #theta", 100, -5, 5)
            self.histogram("pull_p", "pull p", 100, -5, 5)
            self.histogram("pull_qop", "pull q/p", 100, -5, 5)

            # Efficiency plots
            self.histogram("match_prob", "reco truth match probability", nbins, 0.0, 1.1)
            self.histogram("match_d0", "reco match d0 [mm]", nbins, d0min, d0max)
            self.histogram("match_z0", "reco match z0 [mm]", nbins, z0min, z0max)
            self.histogram("match_phi", "reco match #phi", nbins, phimin, phimax)
            self.histogram("match_theta", "reco match #theta", nbins, thetamin, thetamax)
            self.histogram("match_qop", "truth q/p [GeV^{-1}]", nbins, qopmin, qopmax)
            self.histogram("match_p", "truth p [GeV]", nbins, 0, pmax)
            self.histogram("match_beam_angle", "angle wrt beam axis", 20, 0, 2)
            self.histogram("match_PID", "Particles", 8, -4, 4)
            self.histogram("match_nHits", "match nHits", 15, 0, 15)
            self.histogram(
                "match_measurement_dedx",
                "Match measurement dE/dx (MeV/mm)", 60, 0.0, 0.6,
            )
            self.histogram("match_layers_hit", "match layers hit", 15, 0, 15)
            self.histogram("match_kminus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("match_kplus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("match_piminus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("match_piplus_p", "truth p", nbins, 0.0, pmax)
            self.histogram("match_electron_p", "truth p", nbins, 0.0, pmax)
            self.histogram("match_positron_p", "truth p", nbins, 0.0, pmax)
            self.histogram("match_proton_p", "truth p", nbins, 0.0, pmax)

            chi2_fake_max = 500
            chi2_ndf_fake_max = 50

            # Fake Plots
            self.histogram("fake_d0", "fake d0 [mm]", nbins, d0min, d0max)
            self.histogram("fake_z0", "fake z0 [mm]", nbins, z0min, z0max)
            self.histogram("fake_phi", "fake #phi", nbins, phimin, phimax)
            self.histogram("fake_theta", "fake #theta", nbins, thetamin, thetamax)
            self.histogram("fake_p", "fake p [GeV]", nbins, 0, pmax)
            self.histogram("fake_qop", "fake qOverP [GeV^{-1}]", nbins, -40, 40)
            self.histogram("fake_nHits", "fake nHits", 15, 0, 15)
            self.histogram("fake_layers_hit", "fake layers hit", 15, 0, 15)
            self.histogram(
                "fake_measurement_dedx",
                "Fake measurement dE/dx (MeV/mm)", 60, 0.0, 0.6,
            )
            self.histogram("fake_Chi2", "fake Chi2", 100, 0, chi2_fake_max)
            self.histogram("fake_Chi2_per_ndf", "fake Chi2/ndf", 100, 0, chi2_ndf_fake_max)
            self.histogram("fake_nShared", "fake nShared", 5, 0, 5)
            self.histogram("fake_nHoles", "fake nHoles", 5, 0, 5)

            # Duplicate plots
            self.histogram("dup_d0", "dup d0 [mm]", 100, d0min, d0max)
            self.histogram("dup_z0", "dup z0 [mm]", 100, z0min, z0max)
            self.histogram("dup_phi", "dup #phi", 100, phimin, phimax)
            self.histogram("dup_theta", "dup #theta", 100, thetamin, thetamax)
            self.histogram("dup_p", "dup p [GeV]", 100, 0, pmax)
            self.histogram("dup_qop", "dup qOverP [GeV^{-1}]", nbins, qopmin, qopmax)
            self.histogram("dup_nHits", "dup nHits", 15, 0, 15)
            self.histogram("dup_layers_hit", "dup layers hit", 15, 0, 15)
            self.histogram(
                "dup_measurement_dedx",
                "Dup measurement dE/dx (MeV/mm)", 60, 0.0, 0.6,
            )
            self.histogram("dup_Chi2", "dup Chi2", 100, 0, 100)
            self.histogram("dup_Chi2_per_ndf", "dup Chi2/ndf", 100, 0, 10)
            self.histogram("dup_nShared", "dup nShared", 5, 0, 5)
            self.histogram("dup_nHoles", "dup nHoles", 5, 0, 5)

            # Track states extrapolations
            for track_state in self.trackStates:
                self.histogram(
                    "trk_" + track_state + "_loc0",
                    "trk_" + track_state + "_loc0 [mm]", 200, -50, 50,
                )
                self.histogram(
                    "trk_" + track_state + "_loc1",
                    "trk_" + track_state + "_loc1 [mm]", 200, -50, 50,
                )
                self.histogram(
                    track_state + "_sp_hit_X",
                    track_state + "_sp_hit_X [mm]", 200, -50, 50,
                )
                self.histogram(
                    track_state + "_sp_hit_Y",
                    track_state + "_sp_hit_Y [mm]", 200, -50, 50,
                )
                self.histogram(
                    "trk_" + track_state + "_loc0-sp_hit_X",
                    track_state + "_diff loc0 and hit_X [mm]", 200, -0.2, 0.2,
                )
                self.histogram(
                    "trk_" + track_state + "_loc1-sp_hit_Y",
                    track_state + "_diff loc1 and hit_Y [mm]", 200, -5, 5,
                )
                self.histogram(
                    track_state + "_Pulls_of_loc0",
                    track_state + "_pulls_of_loc0 [mm]", 200, -5, 5,
                )
                self.histogram(
                    track_state + "_Pulls_of_loc1",
                    track_state + "_pulls_of_loc1 [mm]", 200, -5, 5,
                )
                self.histogram(
                    track_state + "_res_loc0-vs-N_hits",
                    "N_hits", 5, 6.5, 11.5,
                    track_state + "_res_loc0 [mm]", 100, -0.2, 0.2,
                )
                self.histogram(
                    track_state + "_res_loc1-vs-N_hits",
                    "N_hits", 5, 6.5, 11.5,
                    track_state + "_res_loc1 [mm]", 100, -5, 5,
                )
                self.histogram(
                    track_state + "_res_loc0-vs-trk_p",
                    "trk_p", 200, 0, 5,
                    track_state + "_res_loc0 [mm]", 100, -0.2, 0.2,
                )
                self.histogram(
                    track_state + "_res_loc1-vs-trk_p",
                    "trk_p", 200, 0, 5,
                    track_state + "_res_loc1 [mm]", 100, -5, 5,
                )
                self.histogram(
                    track_state + "_pulls_loc0-vs-N_hits",
                    "N_hits", 5, 6.5, 11.5,
                    track_state + "_pulls_loc0 [mm]", 100, -3, 3,
                )
                self.histogram(
                    track_state + "_pulls_loc1-vs-N_hits",
                    "N_hits", 5, 6.5, 11.5,
                    track_state + "_pulls_loc1 [mm]", 100, -3, 3,
                )
                self.histogram(
                    track_state + "_pulls_loc0-vs-trk_p",
                    "trk_p", 200, 0, 5,
                    track_state + "_pulls_loc0 [mm]", 100, -3, 3,
                )
                self.histogram(
                    track_state + "_pulls_loc1-vs-trk_p",
                    "trk_p", 200, 0, 5,
                    track_state + "_pulls_loc1 [mm]", 100, -3, 3,
                )


@processor("tracking::dqm::StraightTracksDQM", "Tracking")
class StraightTracksDQM(Processor):
    """DQM analyzer for straight tracks.

    Attributes
    ----------
    nbins : int
        Number of bins for histograms.
    phimin : float
        Minimum phi for histograms.
    phimax : float
        Maximum phi for histograms.
    thetamin : float
        Minimum theta for histograms.
    thetamax : float
        Maximum theta for histograms.
    doTruth : bool
        Whether to build truth histograms.
    track_collection : str
        Name of the track collection.
    truth_collection : str
        Name of the truth collection.
    title : str
        Title prefix for histograms.
    subdetector : str
        Name of the subdetector.
    measurement_collection : str
        Name of the measurement collection.
    track_collection_events_passname : str
        The events pass name for track collection.
    truth_collection_events_passname : str
        The events pass name for truth collection.
    input_pass_name : str
        The pass name of the input collections.
    """

    nbins: int = 100
    phimin: float = -0.2
    phimax: float = 0.2
    thetamin: float = -0.3
    thetamax: float = 0.3
    doTruth: bool = True
    track_collection: str = "LinearRecoilTracks"
    truth_collection: str = "LinearRecoilTruthTracks"
    title: str = "recoil_lin_trk_"
    subdetector: str = "Recoil"
    measurement_collection: str = "DigiRecoilSimHits"
    track_collection_events_passname: str = ""
    truth_collection_events_passname: str = ""
    input_pass_name: str = ""

    def __post_init__(self):
        nbins = self.nbins
        phimin = self.phimin
        phimax = self.phimax
        thetamin = self.thetamin
        thetamax = self.thetamax

        self.histogram("N_tracks", "N tracks", 10, 0, 10)
        self.histogram("phi", "#phi [rad]", nbins, phimin, phimax)
        self.histogram("theta", "#theta [rad]", nbins, thetamin, thetamax)
        self.histogram("trk_PID", "Particles", 30, -15, 15)
        self.histogram("nHits", "nHits", 5, 0, 5)
        self.histogram("Chi2", "Chi2", nbins, 0, 15)
        self.histogram("ndf", "ndf", 5, 0, 5)
        self.histogram("Chi2_per_ndf", "Chi2/ndf", nbins, 0, 10)
        self.histogram(
            "dRecHit", "Distance to Nearest EcalRecHit [mm]", nbins, 0.0, 20.0,
        )
        self.histogram("phi_err", "#sigma_{#phi} [rad]", nbins, 0, 0.001)
        self.histogram("theta_err", "#sigma_{#theta} [rad]", nbins, 0, 0.03)

        if self.doTruth:
            self.histogram("truth_phi", "truth #phi", nbins, phimin, phimax)
            self.histogram("truth_theta", "truth #theta", nbins, thetamin, thetamax)
            self.histogram("truth_PID", "Particles", 30, -15, 15)

            self.histogram("res_phi", "res #phi", nbins, -0.02, 0.02)
            self.histogram("res_theta", "res #theta", nbins, -0.04, 0.04)

            self.histogram("pull_phi", "pull #phi", 100, -5, 5)
            self.histogram("pull_theta", "pull #theta", 100, -5, 5)

            chi2_fake_max = 15
            chi2_ndf_fake_max = 10

            # Fake Plots
            self.histogram("fake_phi", "fake #phi", nbins, phimin, phimax)
            self.histogram("fake_theta", "fake #theta", nbins, thetamin, thetamax)
            self.histogram("fake_nHits", "fake nHits", 5, 0, 5)
            self.histogram("fake_Chi2", "fake Chi2", nbins, 0, chi2_fake_max)
            self.histogram("fake_Chi2_per_ndf", "fake Chi2/ndf", nbins, 0, chi2_ndf_fake_max)
            self.histogram("fake_trk_PID", "Particles", 8, -4, 4)

            # Duplicate plots
            self.histogram("dup_phi", "dup #phi", 100, phimin, phimax)
            self.histogram("dup_theta", "dup #theta", 100, thetamin, thetamax)
            self.histogram("dup_nHits", "dup nHits", 5, 0, 5)
            self.histogram("dup_Chi2", "dup Chi2", 1000, 0, 100)
            self.histogram("dup_Chi2_per_ndf", "dup Chi2/ndf", 1000, 0, 100)
            self.histogram("dup_trk_PID", "Particles", 30, -15, 15)

            self.histogram("trk_target_loc0", "trk_target_loc0 [mm]", 200, -50, 50)
            self.histogram("trk_target_loc1", "trk_target_loc1 [mm]", 200, -50, 50)
            self.histogram(
                "trk_target_loc0-truth_target_loc0",
                "target trk_loc0 - truth_loc0 [mm]", 100, -0.1, 0.1,
            )
            self.histogram(
                "trk_target_loc1-truth_target_loc1",
                "target trk_loc1 - truth_loc1 [mm]", 100, -2, 2,
            )
            self.histogram("trk_ecal_loc0", "trk_ecal_loc0 [mm]", 200, -50, 50)
            self.histogram("trk_ecal_loc1", "trk_ecal_loc1 [mm]", 200, -50, 50)
            self.histogram(
                "trk_ecal_loc0-truth_ecal_loc0",
                "ecal trk_loc0 - truth_loc0 [mm]", 200, -0.3, 0.3,
            )
            self.histogram(
                "trk_ecal_loc1-truth_ecal_loc1",
                "ecal trk_loc1 - truth_loc1 [mm]", 200, -6, 6,
            )

            self.histogram("target_Pulls_of_loc0", "target_pulls_of_loc0 [mm]", 200, -5, 5)
            self.histogram("target_Pulls_of_loc1", "target_pulls_of_loc1 [mm]", 200, -5, 5)
            self.histogram("ecal_Pulls_of_loc0", "ecal_pulls_of_loc0 [mm]", 200, -5, 5)
            self.histogram("ecal_Pulls_of_loc1", "ecal_pulls_of_loc1 [mm]", 200, -5, 5)

            self.histogram(
                "target_res_loc0-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "target_res_loc0 [mm]", 100, -0.2, 0.2,
            )
            self.histogram(
                "target_res_loc1-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "target_res_loc1 [mm]", 100, -5, 5,
            )
            self.histogram(
                "ecal_res_loc0-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "ecal_res_loc0 [mm]", 100, -0.2, 0.2,
            )
            self.histogram(
                "ecal_res_loc1-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "ecal_res_loc1 [mm]", 100, -5, 5,
            )
            self.histogram(
                "target_pulls_loc0-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "target_pulls_loc0 [mm]", 100, -3, 3,
            )
            self.histogram(
                "target_pulls_loc1-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "target_pulls_loc1 [mm]", 100, -3, 3,
            )
            self.histogram(
                "ecal_pulls_loc0-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "ecal_pulls_loc0 [mm]", 100, -3, 3,
            )
            self.histogram(
                "ecal_pulls_loc1-vs-N_hits",
                "N_hits", 5, 0.0, 5.0,
                "ecal_pulls_loc1 [mm]", 100, -3, 3,
            )
