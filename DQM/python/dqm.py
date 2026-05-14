"""Configuration for DQM analyzers"""

from LDMX.Framework import Processor, processor


@processor("dqm::HcalGeometryVerifier", "DQM")
class HCalGeometryVerifier(Processor):
    """Configured HCalGeometryVerifier python object

    Contains an instance of the verifier that has already been configured.

    This analyzer verifies that all simhits and rechits for the hcal are within
    the bounds of the scintillator strips as set in the geometry condition.

    If the analyzer encounters an error and `stop_on_error` is true, raises an
    exception with details about the issue. Otherwise, the error is logged and
    histograms for each section is produced.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.HcalGeometryVerifier() )

    """

    rec_coll_name: str = "HcalRecHits"
    rec_pass_name: str = ""
    sim_coll_name: str = "HcalSimHits"
    sim_pass_name: str = ""
    stop_on_error: bool = False
    tolerance: float = 1e-3

    def __post_init__(self):
        section_names = ["back", "top", "bottom", "right", "left"]
        self.histogram(
            "passes_sim", "Simulated hits within scintillator bounds?", 2, 0, 2
        )
        self.histogram(
            "passes_rec", "Reconstructed hits within scintillator bounds?", 2, 0, 2
        )
        for name in section_names:
            self.histogram(
                f"passes_sim_{name}",
                f"Simulated hits within scintillator bounds? ({name})",
                2,
                0,
                2,
            )
            self.histogram(
                f"passes_rec_{name}",
                f"Reconstructed hits within scintillator bounds? Passing ({name})",
                2,
                0,
                2,
            )


@processor("dqm::ReSimVerifier", "DQM")
class ReSimVerifier(Processor):
    """Configured ReSimVerifier python object

    Contains an instance of the verifier that has already been configured. This
    analyzer does not produce anything, it just checks that the sim hits and sim
    particles between two different passes are the same.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.ReSimVerifier() )

    """

    collections: list[str] = [
        "HcalSimHits",
        "EcalSimHits",
        "TargetSimHits",
        "TriggerPad1SimHits",
        "TriggerPad2SimHits",
        "TriggerPad3SimHits",
        "RecoilSimHits",
        "TaggerSimHits",
    ]
    sim_coll_name: str = "SimParticles"
    sim_pass_name: str = ""
    resim_pass_name: str = "resim"
    stop_on_error: bool = False


@processor("dqm::HCalDQM", "DQM")
class HCalDQM(Processor):
    """Configured HCalDQM python object

    Contains an instance of HCalDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.HCalDQM() )
    """

    section: int = 0
    pe_veto_threshold: float = 8.0
    max_hit_time: float = 50.0
    rec_coll_name: str = "HcalRecHits"
    rec_pass_name: str = ""
    sim_coll_name: str = "HcalSimHits"
    sim_pass_name: str = ""
    particle_passname: str = ""
    ecal_sp_hits_passname: str = ""
    hcal_veto_passname: str = ""
    sim_particles_passname: str = ""
    sim_particles_map_passname: str = ""
    target_scoring_plane_passname: str = ""
    hit_passname: str = ""
    trig_scint_passname: str = ""

    def __post_init__(self):
        section_names = ["back", "top", "bottom", "right", "left"]
        section_name = section_names[self.section]

        # Auto-set instance_name with section suffix if still at default
        if self.instance_name == self.__dataclass_fields__["instance_name"].default:
            self.instance_name = f"hcal_dqm_{section_name}"

        pe_bins = [1500, 0, 1500]
        time_bins = [100, -100, 500]
        layer_bins = [100, 0, 100]
        multiplicity_bins = [400, 0, 400]
        energy_bins = [200, 0, 200]
        total_energy_bins = [1000, 0, 1000]
        self.histogram("sim_along_x", "x", 1200, -3000, 3000)
        self.histogram("sim_along_y", "y", 1200, -3000, 3000)
        self.histogram("sim_along_z", "z", 1200, 0, 6000)
        self.histogram("along_x", "x", 1200, -3000, 3000)
        self.histogram("along_y", "y", 1200, -3000, 3000)
        self.histogram("along_z", "z", 1200, 0, 6000)
        # Per hit
        self.histogram("pe", f"Photoelectrons in the HCal ({section_name})", *pe_bins)
        self.histogram("hit_time", f"HCal hit time ({section_name}) [ns]", *time_bins)
        self.histogram(
            "sim_hit_time", f"HCal hit time ({section_name}) [ns]", *time_bins
        )
        self.histogram("layer", f"Layer number ({section_name})", *layer_bins)
        self.histogram("sim_layer", f"Layer number ({section_name})", *layer_bins)
        self.histogram("noise", f"Is pure noise hit? ({section_name})", 2, 0, 1)

        self.histogram(
            "energy",
            f"Reconstructed hit energy in the HCal ({section_name})",
            *energy_bins,
        )

        self.histogram(
            "sim_energy",
            f"Simulated hit energy in the HCal ({section_name})",
            *energy_bins,
        )
        self.histogram(
            "sim_energy_per_bar",
            f"Simulated hit energy per bar in the HCal ({section_name})",
            *energy_bins,
        )
        # Once per event
        self.histogram(
            "total_energy",
            f"Total reconstructed energy in the HCal ({section_name})",
            *total_energy_bins,
        )
        self.histogram(
            "sim_total_energy",
            f"Total simulated energy in the HCal ({section_name})",
            *total_energy_bins,
        )
        self.histogram(
            "total_pe",
            f"Total photoelectrons in the HCal ({section_name})",
            200,
            0,
            10000,
        )
        self.histogram(
            "max_pe", f"Maximum photoelectrons in the HCal ({section_name})", *pe_bins
        )
        self.histogram(
            "max_pe_adc",
            f"Maximum photoelectrons (ADC mode only) in the HCal ({section_name})",
            *pe_bins,
        )
        self.histogram(
            "sim_layer:strip",
            f"HCal Layer ({section_name})",
            *layer_bins,
            "Back HCal Strip",
            62,
            0,
            62,
        )
        self.histogram(
            "layer:strip",
            f"HCal Layer ({section_name})",
            *layer_bins,
            "Back HCal Strip",
            62,
            0,
            62,
        )
        self.histogram(
            "hit_multiplicity",
            f"HCal hit multiplicity ({section_name})",
            *multiplicity_bins,
        )
        self.histogram(
            "sim_hit_multiplicity",
            f"HCal hit multiplicity ({section_name})",
            *multiplicity_bins,
        )
        self.histogram(
            "sim_num_bars_hit",
            f"HCal hit multiplicity ({section_name})",
            *multiplicity_bins,
        )
        self.histogram(
            "vetoable_hit_multiplicity",
            f"Multiplicity of vetoable hits at {int(self.pe_veto_threshold)} PE"
            f" ({section_name})",
            *multiplicity_bins,
        )
        self.histogram(
            "max_pe_time", f"Max PE hit time ({section_name}) [ns]", *time_bins
        )
        self.histogram(
            "max_pe_adc_time",
            f"Max PE (ADC mode only) hit time ({section_name}) [ns]",
            *time_bins,
        )
        self.histogram(
            "hit_z",
            f"Reconstructed Z position in the HCal ({section_name}) [mm]",
            1000,
            0,
            6000,
        )


@processor("dqm::HcalVetoResults", "DQM")
class HcalVetoResults(Processor):
    """Configured HcalVetoResults python object"""

    hcal_veto_name: str = "HcalVeto"
    hcal_veto_pass: str = ""
    hcal_veto_passname: str = ""

    def __post_init__(self):
        self.histogram("max_pe", "Maximal PE hit PE", 500, -0.5, 499.5)
        self.histogram(
            "total_pe", "Total number of HCAL photo-electrons", 500, -0.5, 2999.5
        )
        self.histogram(
            "num_valid_hits", "Total number of valid HCAL hits", 500, -0.5, 499.5
        )
        self.histogram(
            "max_section",
            "Maximal PE hit section",
            ["Back", "Top", "Bottom", "Right", "Left"],
        )
        self.histogram("max_pos_z", "Maximal PE hit postion Z [mm]", 6000, 200.0, 6200)
        self.histogram("veto_pass", "Event passed the HCal Veto", 2, -0.5, 1.5)


@processor("dqm::HcalInefficiencyAnalyzer", "DQM")
class HcalInefficiencyAnalyzer(Processor):
    sim_coll_name: str = "HcalSimHits"
    sim_pass_name: str = ""
    rec_coll_name: str = "HcalRecHits"
    rec_pass_name: str = ""
    pe_veto_threshold: float = 8.0
    max_hit_time: float = 50.0

    def __post_init__(self):
        num_sections = 5
        section_names = ["back", "top", "bottom", "right", "left"]
        inefficiency_layer_bins = [100, 0, 100]
        self.histogram("efficiency", "", 12, -1, 11)
        for section in range(num_sections):
            section_name = section_names[section]
            self.histogram(
                f"inefficiency_{section_name}",
                f"Inefficiency ({section_name})",
                *inefficiency_layer_bins,
            )


@processor("dqm::EcalDigiVerifier", "DQM")
class EcalDigiVerify(Processor):
    """Configured EcalDigiVerifier python object

    Contains an instance of EcalDigiVerifier that
    has already been configured.

    1. Number of SimHits per cell
       - Only including cells that have at least one hit
       - Integrates to number of rec hits
    2. Total Rec Energy in ECal
       - A perfect reconstruction would see a sharp gaussian
         around the total energy being fired into the ECal
       - Integrates to number of events
    3. SimHit Energy Deposition vs Reconstructed Hit Amplitude
       - A perfect reconstruction would see a one-to-one linear
         relationship between these two variables
       - Integrates to number of rec hits
       - Aggregates EDeps from any SimHits in the same cell
    4. RecHit - SimHit spacial residuals
    5. Number of hits in modules
    6. Noise related plots

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.EcalDigiVerify('EcalDigiVerify') )
    """

    ecal_sim_hit_coll: str = "EcalSimHits"
    ecal_sim_hit_pass: str = ""
    ecal_rec_hit_coll: str = "EcalRecHits"
    ecal_rec_hit_pass: str = ""
    ecal_presel_coll: str = "EcalPreselectionDecision"
    ecal_presel_pass: str = ""
    num_layers: int = 32

    def __post_init__(self):
        self.histogram(
            "rec_sim_hit_residual_x", "RecHit X - SimHit X [mm]", 30, -15.0, 15.0
        )
        self.histogram(
            "rec_sim_hit_residual_y", "RecHit Y - SimHit Y [mm]", 30, -15.0, 15.0
        )
        self.histogram(
            "rec_sim_hit_residual_z", "RecHit Z - SimHit Z [mm]", 49, -0.98, 0.98
        )
        self.histogram(
            "rec_sim_hit_residual_x:layer",
            "RecHit X - SimHit X [mm]",
            30,
            -15.0,
            15.0,
            "RecHit Layer",
            34,
            0.5,
            34.5,
        )
        self.histogram(
            "rec_sim_hit_residual_y:layer",
            "RecHit Y - SimHit Y [mm]",
            30,
            -15.0,
            15.0,
            "RecHit Layer",
            34,
            0.5,
            34.5,
        )
        self.histogram(
            "rec_sim_hit_residual_z:layer",
            "RecHit Z - SimHit Z [mm]",
            48,
            -0.6,
            0.6,
            "RecHit Layer",
            34,
            0.5,
            34.5,
        )
        self.histogram(
            "num_sim_hits_per_cell",
            "Number of SimHits per ECal Cell (excluding empty rec cells)",
            20,
            -0.5,
            19.5,
        )
        self.histogram("num_rec_hits", "Number of RecHits", 100, -0.5, 299.5)
        self.histogram("num_noise_hits", "Number of noisy RecHits", 100, -0.5, 99.5)
        self.histogram("is_noise_hit", "Is noise hit?", 2, -0.5, 1.5)
        self.histogram(
            "total_rec_energy",
            "Total Reconstructed Energy in ECal [MeV]",
            800,
            0.0,
            11000.0,
        )
        self.histogram(
            "num_mod_with_0hits", "Num of modules with 0 hit", 100, 140.5, 240.5
        )
        self.histogram(
            "num_mod_with_1hits", "Num of modules with 1 hit", 31, -0.5, 30.5
        )
        self.histogram(
            "num_mod_with_2hits", "Num of modules with 2 hits", 31, -0.5, 30.5
        )
        self.histogram(
            "num_mod_with_more_than_2hits",
            "Num of modules with >2 hits",
            31,
            -0.5,
            30.5,
        )
        self.histogram(
            "num_hit_if_more_than_2hits",
            "Num of hits for modules with >2 hits",
            31,
            -0.5,
            30.5,
        )
        self.histogram("preselection_passed", "Preselection Passed", ["Fail", "Pass"])
        self.histogram(
            "sim_edep:rec_amplitude",
            "Simulated Energy [MeV]",
            1000,
            0.0,
            50.0,
            "Reconstructed Amplitude [MeV]",
            1000,
            0.0,
            50.0,
        )
        self.histogram(
            "sim_edep:rec_energy",
            "Simulated Energy [MeV]",
            1000,
            0.0,
            20.0,
            "Reconstructed Energy [MeV]",
            1000,
            0.0,
            2000.0,
        )


@processor("dqm::EcalShowerFeatures", "DQM")
class EcalShowerFeatures(Processor):
    """Configured EcalShowerFeatures python object"""

    ecal_veto_name: str = "EcalVeto"
    ecal_veto_pass: str = ""

    def __post_init__(self):
        self.histogram("deepest_layer_hit", "Deepest Layer Hit", 40, 0, 40)
        self.histogram("num_readout_hits", "Num Readout Hits", 100, 0, 300)
        self.histogram("summed_det", "Total Rec Energy [MeV]", 600, 0.0, 12000.0)
        self.histogram("summed_iso", "Total Isolated Energy [MeV]", 600, 0.0, 12000.0)
        self.histogram("summed_back", "Total Back Energy [MeV]", 500, 0.0, 10000.0)
        self.histogram(
            "max_cell_dep", "Maximum Single-Cell Energy Dep [MeV]", 200, 0.0, 2000.0
        )
        self.histogram("shower_rms", "Transverse Shower RMS [mm]", 200, 0.0, 200.0)
        self.histogram("x_std", "X Std Deviation [mm]", 200, 0.0, 200.0)
        self.histogram("y_std", "Y Std Deviation [mm]", 200, 0.0, 200.0)
        self.histogram("avg_layer_hit", "Avg Layer Hit", 40, 0.0, 40.0)
        self.histogram("std_layer_hit", "Std Dev Layer Hit", 20, 0.0, 20.0)
        self.histogram(
            "e_containment_energy",
            "Electron Containment Energy [MeV]",
            200,
            0.0,
            10000.0,
        )
        self.histogram(
            "ph_containment_energy",
            "Photon Containment Energy [MeV]",
            200,
            0.0,
            10000.0,
        )
        self.histogram(
            "out_containment_energy",
            "Outside Containment Energy [MeV]",
            200,
            0.0,
            10000.0,
        )


@processor("dqm::EcalMipTrackingFeatures", "DQM")
class EcalMipTrackingFeatures(Processor):
    """Configured EcalMipTrackingFeatures python object"""

    ecal_veto_name: str = "EcalVeto"
    ecal_veto_pass: str = ""
    ecal_mip_name: str = "EcalMipInfo"
    ecal_mip_pass: str = ""

    def __post_init__(self):
        self.histogram("n_straight_tracks", "Num Straight Tracks", 30, -0.5, 29.5)
        self.histogram(
            "n_linreg_segments", "Num Linear Regression Segments", 30, -0.5, 29.5
        )
        self.histogram(
            "first_near_photon_layer", "First Near Photon Layer", 34, -0.5, 34.5
        )
        self.histogram("ep_ang", "Electron Photon Angle [degrees]", 90, 0.0, 90.0)
        self.histogram("ep_sep", "Electron Photon Separation", 180, 0.0, 180.0)
        self.histogram("recoil_pz", "Recoil electron p_{z} [MeV]", 200, -200.0, 8000.0)
        self.histogram("recoil_pt", "Recoil electron p_{T} [MeV]", 200, 0, 2000.0)
        self.histogram("recoil_x", "Recoil electron x [mm]", 100, -300.0, 300.0)
        self.histogram("recoil_y", "Recoil electron y [mm]", 100, -300.0, 300.0)
        self.histogram("n_tracking_hits", "Num Tracking Hits", 300, 0.0, 300.0)


@processor("dqm::EcalVetoResults", "DQM")
class EcalVetoResults(Processor):
    """Configured EcalMipTrackingFeatures python object"""

    ecal_veto_name: str = "EcalVeto"
    ecal_veto_pass: str = ""

    def __post_init__(self):
        self.histogram("bdt_disc", "BDT discriminating score", 100, 0.0, 1.0)
        self.histogram(
            "bdt_disc_log", "-log(1-BDT discriminating score)", 100, 0.0, 5.0
        )
        self.histogram("fiducial", "Recoil eletron fiducial", 2, -0.5, 1.5)
        self.histogram("bdt_pass", "Event passed the ECal BDT", 2, -0.5, 1.5)


@processor("dqm::EcalSPElectronKinematics", "DQM")
class EcalSPElectronKinematics(Processor):
    """Kinematics of the primary electron at the ECal front-face scoring plane.

    Searches EcalScoringPlaneHits for the primary beam electron (track ID 1,
    PDG ID 11) at plane 31 and fills histograms + event-tree branches for its
    energy, momentum, and transverse position.
    """

    ecal_sp_coll_name: str = "EcalScoringPlaneHits"
    ecal_sp_pass_name: str = ""

    def __post_init__(self):
        self.histogram(
            "energy", "Primary Electron Energy at ECal SP [MeV]", 101, 0, 8080
        )
        self.histogram("pt", "Primary Electron pT at ECal SP [MeV]", 100, 0, 2000)
        self.histogram("px", "Primary Electron px at ECal SP [MeV]", 200, -200, 200)
        self.histogram("py", "Primary Electron py at ECal SP [MeV]", 200, -200, 200)
        self.histogram("x", "Primary Electron x at ECal SP [mm]", 200, -100, 100)
        self.histogram("y", "Primary Electron y at ECal SP [mm]", 200, -100, 100)


@processor("dqm::EcalPnetVetoResults", "DQM")
class EcalPnetVetoResults(Processor):
    """Configured EcalMipTrackingFeatures python object"""

    ecal_pnet_veto_name: str = "EcalPnetVeto"
    ecal_pnet_veto_pass: str = ""

    def __post_init__(self):
        self.histogram("pnet_disc", "ParticleNet discriminating score", 100, 0.0, 1.0)
        self.histogram(
            "pnet_disc_log", "-log(1-ParticleNet discriminating score)", 100, 0.0, 5.0
        )
        self.histogram("pnet_pass", "Event passed the ECal ParticleNet", 2, -0.5, 1.5)


@processor("dqm::EcalWABRecResults", "DQM")
class EcalWABRecResults(Processor):
    """Configured EcalWABRec python object"""

    ecal_wab_rec_name: str = "EcalWABRec"
    ecal_wab_rec_pass: str = ""

    def __post_init__(self):
        self.histogram(
            "ThetaDiffElectronPhoton",
            "Reco #theta Difference between Photon and Electron [Degrees]",
            92,
            -1,
            91,
            "True #theta Difference between Photon and Electron [Degrees]",
            92,
            -1,
            91,
        )
        self.histogram(
            "ThetaElectron",
            "Electron Reco #theta [Degrees]",
            92,
            -1,
            91,
            "Electron True #theta [Degrees]",
            92,
            -1,
            91,
        )
        self.histogram(
            "ThetaPhoton",
            "Photon Reco #theta [Degrees]",
            92,
            -1,
            91,
            "Photon True #theta [Degrees]",
            92,
            -1,
            91,
        )
        self.histogram(
            "PhiDiffElectronPhoton",
            "Reco #phi Difference between Photon and Electron [Degrees]",
            92,
            -1,
            91,
            "True #phi Difference between Photon and Electron [Degrees]",
            92,
            -1,
            91,
        )
        self.histogram(
            "PhiElectron",
            "Electron Reco #phi [Degrees]",
            92,
            -1,
            91,
            "Electron True #phi [Degrees]",
            92,
            -1,
            91,
        )
        self.histogram(
            "PhiPhoton",
            "Photon Reco #phi [Degrees]",
            92,
            -1,
            91,
            "Photon True #phi [Degrees]",
            92,
            -1,
            91,
        )
        self.histogram(
            "ElectronEnergy",
            "Reconstructed Recoil Electron Shower Energy [MeV]",
            80,
            0,
            4000,
            "True Recoil Electron Energy [MeV]",
            80,
            0,
            4000,
        )
        self.histogram(
            "PhotonEnergy",
            "Reconstructed Photon Shower Energy [MeV]",
            80,
            0,
            4000,
            "True Photon Energy [MeV]",
            80,
            0,
            4000,
        )
        self.histogram(
            "ElectronThetaDiff",
            "Electron True and Reconstruction #theta Difference [Degrees]",
            181,
            0,
            181,
        )
        self.histogram(
            "PhotonThetaDiff",
            "Photon True and Reconstruction #theta Difference [Degrees]",
            181,
            0,
            181,
        )
        self.histogram(
            "ElectronPhiDiff",
            "Electron True and Reconstruction #phi Difference [Degrees]",
            181,
            0,
            181,
        )
        self.histogram(
            "PhotonPhiDiff",
            "Photon True and Reconstruction #phi Difference [Degrees]",
            181,
            0,
            181,
        )
        self.histogram("ProgressNum", "Reconstruction Progress", 4, 0, 4)


@processor("dqm::VisiblesFeatureProducer", "DQM")
class VisiblesFeatureProducer(Processor):
    """Just plot the visibles features"""

    training: bool = False
    training_file: str = ""
    beam_energy: float = 8000.0
    hcal_rec_coll_name: str = "HcalRecHits"
    hcal_rec_pass_name: str = ""
    ecal_rec_coll_name: str = "EcalRecHits"
    ecal_rec_pass_name: str = ""
    recoil_from_tracking: bool = False
    track_collection: str = "RecoilTracks"
    track_pass_name: str = ""
    sp_coll_name: str = "TargetScoringPlaneHits"
    sp_pass_name: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_pass_name: str = ""

    def __post_init__(self):
        self.histogram("layers_hit", "Number of Hcal layers hit", 100, 0, 100)
        self.histogram("x_std", "Std dev x [mm]", 80, 0, 800)
        self.histogram("y_std", "Std dev y [mm]", 80, 0, 800)
        self.histogram("z_std", "Std dev z [mm]", 100, 0, 1000)
        self.histogram("x_mean", "Average hit x [mm]", 80, -800, 800)
        self.histogram("y_mean", "Average hit  y [mm]", 80, -800, 800)
        self.histogram("r_mean", "Average hit  r [mm]", 80, 0, 800)
        self.histogram("iso_hits", "Total isolated hits", 100, 0, 100)
        self.histogram("iso_energy", "Total isolated energy [MeV]", 80, 0, 800)
        self.histogram("n_hits", "Total Hcal hits", 200, 0, 200)
        self.histogram("total_energy", "Total Hcal energy [MeV]", 100, 4800, 9800)
        self.histogram(
            "photon_track", "Average distance from photon track [mm]", 80, 0, 800
        )


@processor("dqm::VisiblesCutflow", "DQM")
class VisiblesCutflow(Processor):
    bdt_file: str = ""
    feature_list_name: str = "float_input"
    disc_cut: float = 0.999965
    all_cuts: bool = True
    beam_energy: float = 8000.0
    hcal_rec_coll_name: str = "HcalRecHits"
    hcal_rec_pass_name: str = ""
    ecal_rec_coll_name: str = "EcalRecHits"
    ecal_rec_pass_name: str = ""
    recoil_from_tracking: bool = False
    track_collection: str = "RecoilTracks"
    track_pass_name: str = ""
    sp_coll_name: str = "TargetScoringPlaneHits"
    sp_pass_name: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_pass_name: str = ""
    ecal_veto_coll_name: str = "EcalVeto"
    ecal_veto_pass_name: str = ""
    ecal_disc_cut: float = 0.99741

    def __post_init__(self):
        if self.bdt_file == "":
            from LDMX.Hcal.visibles import make_bdt_path

            self.bdt_file = make_bdt_path("visibles")

        self.histogram("total_events", "total_events", 40, 0, 2000)
        self.histogram("pass_acceptance", "pass_acceptance", 40, 0, 2000)
        self.histogram("pass_trigger", "pass_trigger", 40, 0, 2000)
        self.histogram("pass_ecal_energy", "pass_ecal_energy", 40, 0, 2000)
        self.histogram("pass_tracker_veto", "pass_tracker_veto", 40, 0, 2000)
        self.histogram("pass_ecal_bdt", "pass_ecal_bdt", 40, 0, 2000)
        self.histogram("pass_hcal_energy", "pass_hcal_energy", 40, 0, 2000)
        self.histogram("pass_hcal_containment", "pass_hcal_containment", 40, 0, 2000)
        self.histogram("pass_visibles_bdt", "pass_visibles_bdt", 40, 0, 2000)

        self.histogram("visibles_disc", "visibles_disc", 100, 0, 1)
        self.histogram("visibles_disc_high", "visibles_disc_high", 10000, 0.999, 1)
        self.histogram(
            "visibles_disc_high_norm", "visibles_disc_high_norm", 10000, 0.999, 1
        )
        self.histogram(
            "ecal_disc_vs_vis_disc",
            "vis_disc",
            1000,
            0.9999,
            1,
            "ecal_disc",
            1000,
            0.999,
            1,
        )
        self.histogram("roc", "roc", 10000, 0.99, 1)

        self.histogram("beam_energy_frac", "beam_energy_frac", 100, 0.5, 1)
        self.histogram("beam_angle", "beam_angle", 100, 0, 0.5)

        self.histogram("layers_hit", "Number of Hcal layers hit", 100, 0, 100)
        self.histogram("x_std", "Std dev x [mm]", 80, 0, 800)
        self.histogram("y_std", "Std dev y [mm]", 80, 0, 800)
        self.histogram("z_std", "Std dev z [mm]", 100, 0, 1000)
        self.histogram("x_mean", "Average hit x [mm]", 80, -800, 800)
        self.histogram("y_mean", "Average hit  y [mm]", 80, -800, 800)
        self.histogram("r_mean", "Average hit  r [mm]", 80, 0, 800)
        self.histogram("iso_hits", "Total isolated hits", 100, 0, 100)
        self.histogram("iso_energy", "Total isolated energy [MeV]", 80, 0, 800)
        self.histogram("n_hits", "Total Hcal hits", 200, 0, 200)
        self.histogram("total_energy", "Total Hcal energy [MeV]", 100, 4800, 9800)
        self.histogram(
            "photon_track", "Average distance from photon track [mm]", 80, 0, 800
        )


@processor("dqm::SimObjects", "DQM")
class SimObjects(Processor):
    """Configuration for sim-level objects to histogram-ize

    Attributes
    ----------
    sim_pass : str
        Pass name for the sim objects
    """

    sim_pass: str = ""
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    sim_particles_map_passname: str = ""


@processor("dqm::DarkBremInteraction", "DQM", "DarkBremDQM")
class DarkBremInteraction(Processor):
    particle_coll_name: str = "SimParticles"
    particle_passname: str = ""

    def __post_init__(self):
        self.histogram("aprime_energy", "Dark Photon Energy [MeV]", 101, 0, 8080)
        self.histogram("aprime_pt", "Dark Photon pT [MeV]", 100, 0, 2000)
        self.histogram("aprime_theta", "Dark Photon Theta [degree]", 50, 0.0, 100.0)

        self.histogram(
            "aprime_daughter_pdgid",
            "Dark Photon Daughter PDG ID",
            [
                "did not decay",
                "e- 11",
                "e+ -11",
                "mu- 13",
                "mu+ -13",
                "fcp- 17",
                "fcp+ 17",
                "pi- 211",
                "pi+ -211",
                "other",
            ],
        )
        self.histogram(
            "aprime_daughter_energy", "Dark Photon Daughter Energy [MeV]", 101, 0, 8080
        )
        self.histogram(
            "aprime_daughter_pt", "Dark Photon Daughter pT [MeV]", 100, 0, 2000
        )
        self.histogram(
            "aprime_daughter_start_z",
            "Dark Photon Daughter Creation Z [mm]",
            60,
            -100,
            500,
        )
        self.histogram(
            "aprime_daughter_element",
            "Element in which A' conversion occurred",
            [
                "did not happen",
                "H 1",
                "C 6",
                "O 8",
                "Na 11",
                "Si 14",
                "Ca 20",
                "Cu 29",
                "Y 39",
                "Lu 71",
                "W 74",
                "unlisted",
            ],
        )
        self.histogram(
            "aprime_daughter_material",
            "Material in which A' conversion occurred",
            ["Unknown", "C", "PCB", "Glue", "Si", "Al", "W / LYSO", "PVT", "Air"],
        )

        self.histogram(
            "recoil_brem_daughter_energy",
            "Recoil Brem Daughter Energy [MeV]",
            101,
            0,
            8080,
        )
        self.histogram(
            "recoil_brem_daughter_energy_ratio",
            "Recoil Brem Daughter Energy Ratio",
            100,
            0,
            1,
        )
        self.histogram(
            "recoil_brem_daughter_num", "Number of Recoil Brem Daughters", 10, 0, 10
        )

        self.histogram("recoil_energy", "Recoil Electron Energy [MeV]", 101, 0, 8080)
        self.histogram("recoil_pt", "Recoil Electron pT [MeV]", 100, 0, 2000)
        self.histogram("recoil_theta", "Recoil Electron Theta [degree]", 50, 0.0, 100.0)

        self.histogram(
            "incident_energy", "Incident Electron Energy [MeV]", 101, 0, 8080
        )
        self.histogram("incident_pt", "Incident Electron pT [MeV]", 100, 0, 2000)

        self.histogram("dark_brem_z", "Z Location of Dark Brem [mm]", 160, -7.0, 1.0)
        self.histogram(
            "dark_brem_element",
            "Element in which Dark Brem Occurred",
            [
                "did not happen",
                "H 1",
                "C 6",
                "O 8",
                "Na 11",
                "Si 14",
                "Ca 20",
                "Cu 29",
                "Y 39",
                "Lu 71",
                "W 74",
                "unlisted",
            ],
        )
        self.histogram(
            "dark_brem_material",
            "Material in which Dark Brem Occurred",
            ["Unknown", "C", "PCB", "Glue", "Si", "Al", "W / LYSO", "PVT"],
        )


@processor("dqm::HCalRawDigi", "DQM", "hcal_pedestals")
class HCalRawDigi(Processor):
    input_name: str = ""
    input_pass: str = ""


@processor("dqm::HgcrocPulseTruth", "DQM", "hgcroc_pulse_truth")
class HgcrocPulseTruth(Processor):
    input_digi_name: str = ""
    input_digi_pass: str = ""
    input_truth_name: str = ""
    input_truth_pass: str = ""

    def __post_init__(self):
        self.histogram(
            "vpeak_sumADC",
            "Pulse Peak Voltage [mV]",
            200,
            0,
            2000,
            "Digi sum of ADC",
            256,
            0,
            1024,
        )
        self.histogram(
            "vpeak_TOT",
            "Pulse Peak Voltage [mV]",
            200,
            0,
            5000,
            "Digi TOT in SOI",
            512,
            0,
            4096,
        )


@processor("dqm::NtuplizeHgcrocDigiCollection", "DQM", "ntuplizehgcroc")
class NtuplizeHgcrocDigiCollection(Processor):
    input_name: str = ""
    input_pass: str = ""
    using_eid: bool = True
    already_aligned: bool = False
    pedestal_table: str = ""

    def __post_init__(self):
        from LDMX.Conditions.SimpleCSVTableProvider import (
            simple_csv_integer_table_provider,
        )

        if self.pedestal_table == "":
            self.pedestal_table = "NO_PEDESTALS"
            t = simple_csv_integer_table_provider("NO_PEDESTALS", ["PEDESTAL"])
            t.valid_for_all_rows([0])
        else:
            t = simple_csv_integer_table_provider(self.pedestal_table, ["PEDESTAL"])
            t.validForever(f"file://{self.pedestal_table}")


@processor("dqm::NtuplizeTrigScintQIEDigis", "DQM", "ts")
class NtuplizeTrigScintQIEDigis(Processor):
    input_name: str = ""
    input_pass: str = ""


@processor("dqm::PhotoNuclearDQM", "DQM")
class PhotoNuclearDQM(Processor):
    """Configured PhotoNuclearDQM python object

    Contains an instance of PhotoNuclearDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.PhotoNuclearDQM() )
    """

    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    count_light_ions: bool = True

    def __post_init__(self):
        event_type_labels = [
            "Nothing hard",  # 0
            "1 n",  # 1
            "2 n",  # 2
            "#geq 3 n",  # 3
            "1 #pi",  # 4
            "2 #pi",  # 5
            "1 #pi_{0}",  # 6
            "1 #pi A",  # 7
            "1 #pi 2 A",  # 8
            "2 #pi A",  # 9
            "1 #pi_{0} A",  # 10
            "1 #pi_{0} 2 A",  # 11
            "#pi_{0} #pi A",  # 12
            "1 p",  # 13
            "2 p",  # 14
            "pn",  # 15
            "K^{0}_{L} X",  # 16
            "K X",  # 17
            "K^{0}_{S} X",  # 18
            "exotics",  # 19
            "multi-body",  # 20
        ]
        event_type_compact_labels = [
            "1 n",  # 0
            "K#pm X",  # 1
            "1 K^{0}",  # 2
            "2 n",  # 3
            "Soft",  # 4
            "Other",  # 5
        ]

        self.histogram("event_type", "", event_type_labels)
        self.histogram("event_type_500mev", "", event_type_labels)
        self.histogram("event_type_2000mev", "", event_type_labels)
        self.histogram("event_type_compact", "", event_type_compact_labels)
        self.histogram("event_type_compact_500mev", "", event_type_compact_labels)
        self.histogram("event_type_compact_2000mev", "", event_type_compact_labels)
        self.histogram(
            "1n_event_type", "", ["nn", "pn", "#pi^{+}n", "#pi^{0}n", "other"]
        )
        self.histogram(
            "pn_vertex_volume",
            "",
            [
                "Didn't happen",
                "Else",
                "W Cooling",
                "C Cooling",
                "PCB",
                "CarbonBasePlate",
                "Absorber",
                "Sensor",
                "Glue",
                "Motherboard",
            ],
        )
        self.histogram(
            "pn_interaction_material",
            "",
            [
                "Didn't happen",
                "Else",
                "Si",
                "W",
                "FR4",
                "Steel",
                "Epoxy",
                "PVT",
                "Glue",
                "Air",
            ],
        )
        self.histogram("pn_particle_mult", "Photo-nuclear Multiplicity", 200, 0, 200)
        self.histogram(
            "pn_neutron_mult", "Photo-nuclear Neutron Multiplicity", 200, 0, 200
        )
        self.histogram("pn_gamma_energy", "#gamma Energy [MeV]", 100, 0, 10000)
        self.histogram(
            "pn_total_ke",
            "Total Kineitc Energy of Photo-Nuclear Products [MeV]",
            100,
            0,
            10000,
        )
        self.histogram(
            "pn_total_neutron_ke",
            "Total Kineitc Energy of Photo-Nuclear Neutrons  [MeV]",
            100,
            0,
            10000,
        )
        self.histogram("1n_neutron_energy", "Neutron Energy [MeV]", 100, 0, 10000)
        self.histogram("1n_energy_diff", "E(#gamma_{PN}) - E(n) [MeV]", 100, 0, 10000)
        self.histogram("1n_energy_frac", "E(n)/E(#gamma_{PN}) [MeV]", 100, 0, 1)
        self.histogram(
            "2n_n2_energy", "Energy of second hardest neutron [MeV]", 100, 0, 10000
        )
        self.histogram("2n_energy_frac", "E(n)/E(#gamma_{PN}) [MeV]", 100, 0, 1)
        self.histogram("2n_energy_other", "E_{other} [MeV]", 100, 0, 10000)
        self.histogram("1kp_energy", "Charged Kaon Energy [MeV]", 100, 0, 10000)
        self.histogram(
            "1kp_energy_diff", "E(#gamma_{PN}) - E(K#pm) [MeV]", 100, 0, 100000
        )
        self.histogram("1kp_energy_frac", "E(K#pm)/E(#gamma_{PN}) [MeV]", 100, 0, 1)
        self.histogram("1k0_energy", "K0 Energy [MeV]", 100, 0, 10000)
        self.histogram("1k0_energy_diff", "E(#gamma_{PN}) - E(K0) [MeV]", 100, 0, 10000)
        self.histogram("1k0_energy_frac", "E(K0)/E(#gamma_{PN}) [MeV]", 100, 0, 1)

        self.histogram("recoil_vertex_x", "Recoil e^{-} Vertex - x [mm]", 40, -40, 40)
        self.histogram("recoil_vertex_y", "Recoil e^{-} Vertex - y [mm]", 80, -80, 80)
        self.histogram(
            "recoil_vertex_z", "Recoil e^{-} Vertex - z [mm]", 20, -950, -850
        )

        self.histogram(
            "pn_gamma_int_x", "#gamma Interaction Vertex - x [mm]", 50, -250, 250
        )
        self.histogram(
            "pn_gamma_int_y", "#gamma Interaction Vertex - y [mm]", 50, -250, 250
        )
        self.histogram(
            "pn_gamma_int_z", "#gamma Interaction Vertex - z [mm]", 40, 200, 400
        )

        self.histogram("pn_gamma_vertex_x", "#gamma Vertex - y [mm]", 40, -40, 40)
        self.histogram("pn_gamma_vertex_y", "#gamma Vertex - y [mm]", 80, -80, 80)
        self.histogram("pn_gamma_vertex_z", "#gamma Vertex - z [mm]", 10, -5, 5)

        self.histogram(
            "hardest_ke",
            "Kinetic Energy Hardest Photo-nuclear Particle [MeV]",
            200,
            0,
            8000,
        )
        self.histogram(
            "hardest_theta",
            "#theta of Hardest Photo-nuclear Particle [Degrees]",
            180,
            0,
            180,
        )
        self.histogram(
            "hardest_p_ke",
            "Kinetic Energy Hardest Photo-nuclear Proton [MeV]",
            200,
            0,
            8000,
        )
        self.histogram(
            "hardest_p_theta",
            "#theta of Hardest Photo-nuclear Proton [Degrees]",
            180,
            0,
            180,
        )
        self.histogram(
            "hardest_n_ke",
            "Kinetic Energy Hardest Photo-nuclear Neutron [MeV]",
            200,
            0,
            8000,
        )
        self.histogram(
            "hardest_n_theta",
            "#theta of Hardest Photo-nuclear Neutron [Degrees]",
            180,
            0,
            180,
        )
        self.histogram(
            "hardest_pi_ke",
            "Kinetic Energy Hardest Photo-nuclear #pi [MeV]",
            200,
            0,
            8000,
        )
        self.histogram(
            "hardest_pi_theta",
            "#theta of Hardest Photo-nuclear #pi [Degrees]",
            180,
            0,
            180,
        )

        self.histogram(
            "h_ke_h_theta",
            "Kinetic Energy Hardest Photo-nuclear Particle [MeV]",
            200,
            0,
            8000,
            "#theta of Hardest Photo-nuclear Particle [Degrees]",
            180,
            0,
            180,
        )

        self.histogram(
            "1n_ke:2nd_h_ke",
            "Kinetic Energy of Leading Neutron [MeV]",
            200,
            0,
            8000,
            "Kinetic Energy of 2nd Hardest Particle [MeV]",
            200,
            0,
            8000,
        )

        self.histogram(
            "1kp_ke:2nd_h_ke",
            "Kinetic Energy of Leading Charged Kaon [MeV]",
            200,
            0,
            8000,
            "Kinetic Energy of 2nd Hardest Particle [MeV]",
            200,
            0,
            8000,
        )

        self.histogram(
            "1k0_ke:2nd_h_ke",
            "Kinetic Energy of Leading K0 [MeV]",
            200,
            0,
            8000,
            "Kinetic Energy of 2nd Hardest Particle [MeV]",
            200,
            0,
            8000,
        )

        self.histogram(
            "recoil_vertex_x:recoil_vertex_y",
            "Recoil electron vertex x [mm]",
            80,
            -40,
            40,
            "Recoil electron vertex y [mm]",
            160,
            -80,
            80,
        )

        self.histogram(
            "pn_gamma_int_x:pn_gamma_int_y",
            "PN gamma interaction vertex x [mm]",
            50,
            -250,
            250,
            "PN gamma interaction vertex y [mm]",
            50,
            -250,
            250,
        )


@processor("dqm::TrkDeDxMassEstFeatures", "DQM")
class TrkDeDxMassEstFeatures(Processor):
    """Configured TrkDeDxMassEstFeatures python object

    Contains an instance of TrkDeDxMassEstFeatures that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrkDeDxMassEstFeatures() )
    """

    mass_estimate_name: str = "TrackDeDxMassEstimate"
    mass_estimate_pass: str = ""

    def __post_init__(self):
        momentum_bins = [
            90.0,
            100.0,
            125.0,
            150.0,
            175.0,
            200.0,
            250.0,
            300.0,
            350.0,
            400.0,
            450.0,
            500.0,
            600.0,
            700.0,
            800.0,
            900.0,
            1000.0,
            1300.0,
            2000.0,
            3000.0,
            4000.0,
            6000.0,
            8000.0,
        ]
        low_momentum_bins = [
            50.0,
            70.0,
            90.0,
            100.0,
            125.0,
            150.0,
            175.0,
            200.0,
            250.0,
            300.0,
            350.0,
            400.0,
            450.0,
            500.0,
            600.0,
            700.0,
            800.0,
            900.0,
            1000.0,
            2000.0,
        ]
        self.histogram(
            "momentum:harmonic_mean_dedx",
            xlabel="Momentum [MeV]",
            xbins=momentum_bins,
            ylabel="I_{h} [MeV/cm]",
            ybins=50,
            ymin=0.0,
            ymax=30.0,
        )
        self.histogram(
            "momentum_low:harmonic_mean_dedx",
            xlabel="Momentum [MeV]",
            xbins=low_momentum_bins,
            ylabel="I_{h} [MeV/cm]",
            ybins=50,
            ymin=0.0,
            ymax=30.0,
        )
        self.histogram("harmonic_mean_dedx", "I_{h} [MeV/cm]", 50, 0.0, 30.0)
        self.histogram("mass_estimate", "Mass Estimate [MeV]", 100, 0.0, 2000.0)
        self.histogram("mass_estimate_low_p", "Mass Estimate [MeV]", 100, 0.0, 2000.0)
        self.histogram(
            "mass_estimate_very_low_p", "Mass Estimate [MeV]", 100, 0.0, 2000.0
        )
        self.histogram(
            "mass_estimate_very_low_p_electron",
            "Mass Estimate for electrons [MeV]",
            20,
            0.0,
            200.0,
        )
        self.histogram(
            "mass_estimate_very_low_p_pion",
            "Mass Estimate for pions [MeV]",
            20,
            0.0,
            200.0,
        )
        self.histogram(
            "mass_estimate_very_low_p_kaon",
            "Mass Estimate for kaons [MeV]",
            60,
            200.0,
            800.0,
        )
        self.histogram(
            "mass_estimate_very_low_p_proton",
            "Mass Estimate for proton [MeV]",
            40,
            800.0,
            1200.0,
        )
        self.histogram("track_type", "Track Type", ["Other", "Tagger", "Recoil"])


@processor("dqm::TrigScintDQM", "DQM")
class TrigScintSimDQM(Processor):
    """Configured TrigScintSimDQM python object

    Contains an instance of TrigScintSimDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintSimDQM() )
    """

    hit_collection: str = "TriggerPadUpSimHits"
    pad: str = "up"
    hit_passname: str = ""


@processor("dqm::TrigScintHitDQM", "DQM")
class TrigScintDigiDQM(Processor):
    """Configured TrigScintDigiDQM python object

    Contains an instance of TrigScintDigiDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintDigiDQM() )
    """

    hit_collection: str = "trigScintDigisUp"
    pad: str = "up"
    pass_name: str = ""


@processor("dqm::TrigScintDigiVerifier", "DQM")
class TrigScintDigiVerifierDQM(Processor):
    ts_simhit_coll: str = "TriggerPadUpSimHits"
    ts_simhit_pass: str = ""
    ts_digi_coll: str = "trigScintDigisUp"
    ts_digi_pass: str = ""

    def __post_init__(self):
        self.histogram(
            "sim_edep:rec_amplitude",
            "Simulated Energy [MeV]",
            1000,
            0.0,
            50.0,
            "Reconstructed Amplitude [MeV]",
            1000,
            0.0,
            50.0,
        )
        self.histogram(
            "sim_edep:rec_energy",
            "Simulated Energy [MeV]",
            1000,
            0.0,
            50.0,
            "Reconstructed Energy [MeV]",
            1000,
            0.0,
            50.0,
        )


@processor("dqm::TrigScintClusterDQM", "DQM")
class TrigScintClusterDQM(Processor):
    """Configured TrigScintClusterDQM python object

    Contains an instance of TrigScintClusterDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintClusterDQM() )
    """

    cluster_collection: str = "TriggerPadUpClusters"
    pad: str = "up"
    pass_name: str = ""


@processor("dqm::TrigScintTrackDQM", "DQM")
class TrigScintTrackDQM(Processor):
    """Configured TrigScintTrackDQM python object

    Contains an instance of TrigScintTrackDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintTrackDQM() )
    """

    track_collection: str = "TriggerPadTracks"
    pass_name: str = ""


@processor("dqm::Trigger", "DQM")
class Trigger(Processor):
    """Configured Trigger python object
    Contains an instance of TrigScintTrackDQM that
    has already been configured.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.Trigger() )
    """

    trigger_name: str = "Trigger"
    trigger_pass: str = ""


@processor("dqm::SampleValidation", "DQM")
class SampleValidation(Processor):
    """Configured Sample Validation python object
    Package used to validate samples

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append(dqm.SampleValidation())
    """

    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""
    target_scoring_plane_coll_name: str = "TargetScoringPlaneHits"
    target_scoring_plane_passname: str = ""

    def __post_init__(self):
        pdgid_bin_labels = [
            "e^{+}",  # 0
            "e^{-}",  # 1
            "#mu^{+}",  # 2
            "#mu^{-}",  # 3
            "#gamma",  # 4
            "p^{+}",  # 5
            "n^{0}",  # 6
            "#pi^{+}",  # 7
            "#pi^{-}",  # 8
            "#pi^{0}",  # 9
            "K^{+}",  # 10
            "K^{-}",  # 11
            "K_{L}",  # 12
            "K_{S}",  # 13
            "light-N",  # 14
            "heavy-N",  # 15
            "#Lambda / #Sigma / #Xi",  # 16
            "A'",  # 17
            "fcp+",  # 18
            "fcp-",  # 19
            "else",  # 20
        ]

        self.histogram("primaries_pdgid", "ID of primary particles", pdgid_bin_labels)
        self.histogram(
            "primaries_energy", "Energy of primary particles [MeV]", 90, 0, 9000
        )
        self.histogram("primaries_theta", "Theta [degree]", 50, 0.0, 100.0)
        self.histogram("primaries_pt", "Transverse Momentum [MeV]", 200, 0.0, 2000.0)

        self.histogram("beam_smear", "x [mm]", 30, -150, 150, "y [mm]", 30, -150, 150)
        self.histogram(
            "primarydaughters_pdgid", "ID of primary daughters", pdgid_bin_labels
        )
        self.histogram(
            "daughterphoton_energy",
            "Energy spectrum of all photons from primary [MeV]",
            170,
            0,
            8500,
        )

        self.histogram(
            "harddaughters_pdgid", "ID of primary daughters", pdgid_bin_labels
        )
        self.histogram(
            "harddaughters_startZ",
            "Start z position of hard primary daughter [mm]",
            100,
            -500,
            500,
        )
        self.histogram(
            "harddaughters_endZ",
            "End z position of hard primary daughter [mm]",
            100,
            -500,
            500,
        )
        self.histogram(
            "harddaughters_energy",
            "Energy spectrum of hard primary daughter [MeV]",
            130,
            2000,
            8500,
        )
        self.histogram("harddaughters_theta", "Theta [degree]", 25, 0.0, 50.0)
        self.histogram("harddaughters_pt", "Transverse Momentum [MeV]", 50, 0.0, 100.0)

        self.histogram(
            "hardbremdaughters_pdgid", "ID of hard brem daughters", pdgid_bin_labels
        )
        self.histogram(
            "hardbremdaughters_startZ",
            "Start z position of hard brem daughters  [mm]",
            200,
            -1000,
            1000,
        )
        self.histogram(
            "hardbremdaughters_endZ",
            "End z position of hard brem daughters  [mm]",
            70,
            -1000,
            6000,
        )
        self.histogram(
            "hardbremdaughters_energy",
            "Energy of hard brem daughters  [MeV]",
            170,
            0,
            8500,
        )
        self.histogram("hardbremdaughters_theta", "Theta [degree]", 50, 0.0, 100.0)
        self.histogram(
            "hardbremdaughters_pt", "Transverse Momentum [MeV]", 200, 0.0, 1000.0
        )


@processor("dqm::GenieTruthDQM", "DQM")
class GenieTruthDQM(Processor):
    """Configured GenieTruthDQM python object

    Contains an instance of GenieTruthDQM that
    has already been configured.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.GenieTruthDQM() )
    """

    hepmc3_coll_name: str = ""
    hepmc3_pass_name: str = ""


sample_validation_dqm = [SampleValidation()]


@processor("dqm::EcalClusterAnalyzer", "DQM")
class EcalClusterAnalyzer(Processor):
    """Analyze clustering"""

    use_simulated_electron_number: bool = False
    nbr_of_electrons: int = 2
    ecal_sim_hit_coll: str = "EcalSimHits"
    ecal_sim_hit_pass: str = ""
    rec_hit_coll_name: str = "EcalRecHits"
    rec_hit_pass_name: str = ""
    cluster_coll_name: str = "EcalClusters"
    cluster_pass_name: str = ""
    ecal_sp_hits_coll_name: str = "EcalScoringPlaneHits"
    ecal_sp_hits_pass_name: str = ""
    mixed_hit_cutoff: float = 0.05
    inverse_skim: bool = False
    n_ecal_clusters_min: int = 1

    def __post_init__(self):
        self.histogram(
            "number_of_clusters_first_layer",
            "Number of CLUE clusters on the first layer",
            5,
            -0.5,
            4.5,
        )
        self.histogram(
            "number_of_clusters_per_layer",
            "Number of CLUE clusters per layer",
            5,
            -0.5,
            4.5,
        )
        self.histogram(
            "number_of_clusters", "Total number of CLUE clusters", 51, -0.5, 50.5
        )
        self.histogram(
            "correctly_predicted_events",
            "Correct Cluster Count",
            ["Underpredicted", "Correct", "Overpredicted"],
        )

        self.histogram("ancestors", "Ancestors of particles", 4, 0.0, 4.0)

        self.histogram(
            "same_ancestor",
            "Percentage of hits in cluster coming from the electron"
            " that produced most hits",
            21,
            0.0,
            105.0,
        )
        self.histogram(
            "energy_percentage",
            "Percentage of energy in cluster coming from the electron"
            " that produced most of energy",
            21,
            0.0,
            105.0,
        )
        self.histogram(
            "mixed_hit_energy",
            "Percentage of total energy coming from hits with energy"
            " contributions from more than one electron",
            21,
            0.0,
            105.0,
        )
        self.histogram(
            "unclustered_hits", "Number of hits not in a cluster", 10, 0.0, 200.0
        )
        self.histogram(
            "unclustered_hits_percentage",
            "Percentage of hits not in a cluster",
            21,
            0.0,
            105.0,
        )
        self.histogram("total_rechits_in_event", "RecHits per event", 20, 0.0, 500.0)

        self.histogram(
            "total_energy_vs_hits",
            "Total energy (edep) [MeV]",
            30,
            0.0,
            150.0,
            "Hits in cluster",
            30,
            0.0,
            300.0,
        )
        self.histogram(
            "total_energy_vs_purity",
            "Total energy (edep) [MeV]",
            30,
            0.0,
            150.0,
            "Energy purity %",
            21,
            0,
            105.0,
        )
        self.histogram(
            "sp_ele_distance_vs_purity",
            "SP ele distance in xy-plane [mm]",
            50,
            0,
            250,
            "Energy purity %",
            21,
            0.0,
            105.0,
        )
        self.histogram(
            "sp_clue_distance_vs_layer",
            "CLUE centroid to SP ele distance in xy-plane [mm]",
            125,
            0.0,
            250.0,
            "Layer",
            33,
            -0.5,
            32.5,
        )

        self.histogram(
            "sp_clue_distance",
            "CLUE centroid to SP ele distance in xy-plane [mm]",
            125,
            0.0,
            250.0,
        )
        self.histogram(
            "sp_clue_x_residual", "CLUE centroid X - SP ele X [mm]", 250, -250.0, 250.0
        )
        self.histogram(
            "sp_clue_y_residual", "CLUE centroid Y - SP ele Y [mm]", 250, -250.0, 250.0
        )

        self.histogram("sp_distance", "dR(SPhit_1, SPhit_2)", 100, -1, 202)
        self.histogram("cluster_distance", "dR(cl_1, cl_2)", 100, -1, 202)
        self.histogram("cluster_RMSX", "RMS(hits in cluster) X", 100, -1, 202)
        self.histogram("cluster_RMSY", "RMS(hits in cluster) Y", 100, -1, 202)
        self.histogram(
            "dE_cl2_vs_cl1",
            "E_{cl}-E_{true}^{SP}, cluster 1 [MeV]",
            100,
            -10000,
            10000,
            "E_{cl}-E_{true}^{SP}, cluster 2 [MeV]",
            100,
            -10000,
            10000,
        )

        mixed_pct = int(100 * (1.0 - self.mixed_hit_cutoff))
        self.histogram(
            "tag0frac_vs_SPdist",
            "dR(SPhit_1, SPhit_2)",
            251,
            -1,
            250,
            f"Fraction of mixed (purity less than {mixed_pct}%) ancestors",
            200,
            0,
            1,
        )


@processor("dqm::EcalTrackAnalyzer", "DQM")
class EcalTrackAnalyzer(Processor):
    """Analyze ECAL tracks produced by ACTS tracking"""

    track_collection: str = "EcalTracks"
    track_pass_name: str = ""
    rec_hit_collection: str = "EcalRecHits"
    rec_hit_pass_name: str = ""

    def __post_init__(self):
        self.histogram("n_tracks", "Number of ECAL Tracks", 20, 0, 20)
        self.histogram("track_chi2", "Track Chi2", 100, 0, 100)
        self.histogram("track_chi2_ndf", "Track Chi2/NDF", 100, 0, 10)
        self.histogram("track_ndf", "Track NDF", 50, 0, 50)
        self.histogram("track_nhits", "Track Number of Hits", 40, 0, 40)
        self.histogram("track_d0", "Track d0 [mm]", 100, -50, 50)
        self.histogram("track_z0", "Track z0 [mm]", 100, 200, 400)
        self.histogram("track_phi", "Track phi [rad]", 100, -3.2, 3.2)
        self.histogram("track_theta", "Track theta [rad]", 100, 0, 3.2)
        self.histogram("track_qop", "Track q/p [1/MeV]", 100, -0.01, 0.01)
        self.histogram("track_p", "Track momentum [MeV]", 100, 0, 5000)
        self.histogram("track_pt", "Track pT [MeV]", 100, 0, 2000)
        self.histogram("track_px", "Track px [MeV]", 100, -1000, 1000)
        self.histogram("track_py", "Track py [MeV]", 100, -1000, 1000)
        self.histogram("track_pz", "Track pz [MeV]", 100, 0, 5000)
        self.histogram("track_charge", "Track charge", 5, -2.5, 2.5)
        self.histogram("track_x", "Track x [mm]", 100, -300, 300)
        self.histogram("track_y", "Track y [mm]", 100, -300, 300)
        self.histogram("track_p_multitracks", "Track p (multi-track events) [MeV]", 100, 0, 5000)
        self.histogram("track_separation", "Track angular separation [rad]", 100, 0, 3.2)
        self.histogram(
            "track_xy",
            "Track x [mm]",
            100,
            -300,
            300,
            "Track y [mm]",
            100,
            -300,
            300,
        )
        self.histogram(
            "track_p_vs_theta",
            "Track momentum [MeV]",
            100,
            0,
            5000,
            "Track theta [rad]",
            100,
            0,
            3.2,
        )
        self.histogram(
            "track_chi2_vs_nhits",
            "Track Chi2",
            100,
            0,
            100,
            "Track Number of Hits",
            40,
            0,
            40,
        )
        self.histogram(
            "track_nhits_vs_chi2",
            "Track Number of Hits",
            40,
            0,
            40,
            "Track Chi2",
            100,
            0,
            100,
        )
        self.histogram(
            "track_nhits_vs_chi2_ndf",
            "Track Number of Hits",
            40,
            0,
            40,
            "Track Chi2/NDF",
            100,
            0,
            10,
        )
        self.histogram(
            "track_p_vs_chi2",
            "Track momentum [MeV]",
            100,
            0,
            5000,
            "Track Chi2",
            100,
            0,
            100,
        )
        self.histogram(
            "track_p_vs_nhits",
            "Track momentum [MeV]",
            100,
            0,
            5000,
            "Track Number of Hits",
            40,
            0,
            40,
        )


@processor("dqm::EcalSPTrackCompare", "DQM")
class EcalSPTrackCompare(Processor):
    """Compare primary electron at ECal scoring plane with reconstructed EcalTracks."""

    ecal_sp_coll_name: str = "EcalScoringPlaneHits"
    ecal_sp_pass_name: str = ""
    track_collection: str = "EcalTracks"
    track_pass_name: str = ""

    def __post_init__(self):
        # SP electron truth
        self.histogram("sp_x", "SP Electron x [mm]", 200, -200, 200)
        self.histogram("sp_y", "SP Electron y [mm]", 200, -200, 200)
        self.histogram("sp_p", "SP Electron p [MeV]", 100, 0, 5000)
        self.histogram("sp_pt", "SP Electron pT [MeV]", 100, 0, 2000)
        self.histogram("sp_theta", "SP Electron theta [rad]", 100, 0, 0.5)
        self.histogram("sp_phi", "SP Electron phi [rad]", 100, 0, 6.3)

        # Event-level
        self.histogram("n_tracks", "Number of ECAL Tracks", 20, 0, 20)
        self.histogram("has_match", "Has track match", 2, -0.5, 1.5)

        # Best-match track
        self.histogram("trk_x", "Track x [mm]", 200, -200, 200)
        self.histogram("trk_y", "Track y [mm]", 200, -200, 200)
        self.histogram("trk_p", "Track p [MeV]", 100, 0, 5000)
        self.histogram("trk_pt", "Track pT [MeV]", 100, 0, 2000)
        self.histogram("trk_theta", "Track theta [rad]", 100, 0, 0.5)
        self.histogram("trk_phi", "Track phi [rad]", 100, 0, 6.3)

        # Residuals (track - truth)
        self.histogram("delta_x", "Track x - SP x [mm]", 200, -100, 100)
        self.histogram("delta_y", "Track y - SP y [mm]", 200, -100, 100)
        self.histogram("delta_theta", "Track theta - SP theta [rad]", 200, -0.2, 0.2)
        self.histogram("delta_phi", "Track phi - SP phi [rad]", 200, -0.5, 0.5)
        self.histogram("delta_p", "Track p - SP p [MeV]", 200, -2000, 2000)
        self.histogram("delta_pt", "Track pT - SP pT [MeV]", 200, -1000, 1000)
        self.histogram("delta_p_frac", "(Track p - SP p) / SP p", 200, -2, 2)
        self.histogram("delta_r", "Track-SP position distance [mm]", 100, 0, 200)
        self.histogram("delta_angle", "Track-SP angular distance [rad]", 100, 0, 1.0)

        # 2D correlations
        self.histogram(
            "sp_x_vs_trk_x", "SP x [mm]", 100, -200, 200,
            "Track x [mm]", 100, -200, 200,
        )
        self.histogram(
            "sp_y_vs_trk_y", "SP y [mm]", 100, -200, 200,
            "Track y [mm]", 100, -200, 200,
        )
        self.histogram(
            "sp_p_vs_trk_p", "SP p [MeV]", 100, 0, 5000,
            "Track p [MeV]", 100, 0, 5000,
        )
        self.histogram(
            "sp_theta_vs_trk_theta", "SP theta [rad]", 100, 0, 0.5,
            "Track theta [rad]", 100, 0, 0.5,
        )


ecal_dqm = [
    EcalDigiVerify(),
    EcalShowerFeatures(),
    EcalMipTrackingFeatures(),
    EcalVetoResults(),
    EcalPnetVetoResults(),
    EcalSPElectronKinematics(),
]

hcal_dqm = [
    HCalDQM(pe_veto_threshold=8.0, section=0),
    HCalDQM(pe_veto_threshold=8.0, section=1),
    HCalDQM(pe_veto_threshold=8.0, section=2),
    HCalDQM(pe_veto_threshold=8.0, section=3),
    HCalDQM(pe_veto_threshold=8.0, section=4),
    HcalInefficiencyAnalyzer(),
    HcalVetoResults(),
]

dedx_dqm = [TrkDeDxMassEstFeatures()]

trig_scint_dqm = [
    TrigScintSimDQM(
        instance_name="TrigScintSimPad1",
        hit_collection="TriggerPad1SimHits",
        pad="pad1",
    ),
    TrigScintSimDQM(
        instance_name="TrigScintSimPad2",
        hit_collection="TriggerPad2SimHits",
        pad="pad2",
    ),
    TrigScintSimDQM(
        instance_name="TrigScintSimPad3",
        hit_collection="TriggerPad3SimHits",
        pad="pad3",
    ),
    TrigScintDigiDQM(
        instance_name="TrigScintDigiPad1",
        hit_collection="trigScintDigisPad1",
        pad="pad1",
    ),
    TrigScintDigiDQM(
        instance_name="TrigScintDigiPad2",
        hit_collection="trigScintDigisPad2",
        pad="pad2",
    ),
    TrigScintDigiDQM(
        instance_name="TrigScintDigiPad3",
        hit_collection="trigScintDigisPad3",
        pad="pad3",
    ),
    TrigScintClusterDQM(
        instance_name="TrigScintClusterPad1",
        cluster_collection="TriggerPad1Clusters",
        pad="pad1",
    ),
    TrigScintClusterDQM(
        instance_name="TrigScintClusterPad2",
        cluster_collection="TriggerPad2Clusters",
        pad="pad2",
    ),
    TrigScintClusterDQM(
        instance_name="TrigScintClusterPad3",
        cluster_collection="TriggerPad3Clusters",
        pad="pad3",
    ),
    TrigScintTrackDQM(
        instance_name="TrigScintTracks", track_collection="TriggerPadTracks"
    ),
]

trigger_dqm = [Trigger()]

all_dqm = sample_validation_dqm + ecal_dqm + hcal_dqm + trig_scint_dqm + trigger_dqm
