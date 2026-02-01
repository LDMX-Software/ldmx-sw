"""Configuration for DQM analyzers"""

from LDMX.Framework import ldmxcfg


class HCalGeometryVerifier(ldmxcfg.Analyzer) :
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
    def __init__(self,name="hcal_geometry_verifier", stop_on_error=False) :
        section_names = ['back', 'top', 'bottom', 'right', 'left']
        super().__init__(name,'dqm::HcalGeometryVerifier','DQM')
        self.rec_coll_name = 'HcalRecHits'
        self.rec_pass_name = ''
        self.sim_coll_name = 'HcalSimHits'
        self.sim_pass_name = ''
        self.stop_on_error=stop_on_error
        self.tolerance=1e-3 # mm
        self.build_1d_histogram('passes_sim', 'Simulated hits within scintillator bounds?', 2, 0,2)
        self.build_1d_histogram('passes_rec', 'Reconstructed hits within scintillator bounds?', 2, 0,2)
        section_names = ['back', 'top', 'bottom', 'right', 'left']
        for name in section_names:
            self.build_1d_histogram(f'passes_sim_{name}', f'Simulated hits within scintillator bounds? ({name})', 2, 0,2)
            self.build_1d_histogram(f'passes_rec_{name}', f'Reconstructed hits within scintillator bounds? Passing ({name})', 2, 0,2)


class ReSimVerifier(ldmxcfg.Analyzer) :
    """Configured ReSimVerifier python object

    Contains an instance of the verifier that has already been configured. This
    analyzer does not produce anything, it just checks that the sim hits and sim
    particles between two different passes are the same.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.ReSimVerifier() )

    """

    def __init__(self,name="hcal_geometry_verifier", stop_on_error=False) :
        super().__init__(name,'dqm::ReSimVerifier','DQM')
        self.collections = [
            'HcalSimHits',
            'EcalSimHits',
            'TargetSimHits',
            'TriggerPad1SimHits',
            'TriggerPad2SimHits',
            'TriggerPad3SimHits',
            'RecoilSimHits',
            'TaggerSimHits',
        ]
        self.sim_coll_name = 'SimParticles'
        self.sim_pass_name = ''
        self.resim_pass_name = 'resim'
        self.stop_on_error=stop_on_error

class HCalDQM(ldmxcfg.Analyzer) :
    """Configured HCalDQM python object

    Contains an instance of HCalDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.HCalDQM() )
    """



    def __init__(self,name="hcal_dqm", pe_threshold=8, section=0, max_hit_time = 50.0) :
        self.section = section
        section_names = ['back', 'top', 'bottom', 'right', 'left']
        section_name = section_names[section]
        super().__init__(name + f'_{section_name}','dqm::HCalDQM','DQM')

        self.pe_veto_threshold = float(pe_threshold)
        self.max_hit_time = max_hit_time
        self.rec_coll_name = 'HcalRecHits'
        self.rec_pass_name = ''
        self.sim_coll_name = 'HcalSimHits'
        self.sim_pass_name = ''

        self.particle_passname = ''
        self.ecal_sp_hits_passname = ''
        self.hcal_veto_passname = ''
        self.sim_particles_passname = ''
        self.target_scoring_plane_passname = ''
        self.sim_particles_passname = ''
        self.sim_particles_map_passname = ''
        self.hit_passname = ''
        self.trig_scint_passname = ''

        pe_bins = [1500, 0, 1500]
        time_bins = [100, -100, 500]
        layer_bins = [100,0,100]
        multiplicity_bins = [400,0,400]
        energy_bins = [200,0,200]
        total_energy_bins = [1000, 0, 1000]
        self.build_1d_histogram('sim_along_x', 'x', 1200, -3000,3000)
        self.build_1d_histogram('sim_along_y', 'y', 1200, -3000,3000)
        self.build_1d_histogram('sim_along_z', 'z', 1200, 0,6000)
        self.build_1d_histogram('along_x', 'x', 1200, -3000,3000)
        self.build_1d_histogram('along_y', 'y', 1200, -3000,3000)
        self.build_1d_histogram('along_z', 'z', 1200, 0,6000)
        # Per hit
        self.build_1d_histogram("pe",
                              f"Photoelectrons in the HCal ({section_name})",
                              *pe_bins)
        self.build_1d_histogram('hit_time', f'HCal hit time ({section_name}) [ns]',
                              *time_bins)
        self.build_1d_histogram('sim_hit_time', f'HCal hit time ({section_name}) [ns]',
                              *time_bins)
        self.build_1d_histogram("layer", f"Layer number ({section_name})",
                              *layer_bins)
        self.build_1d_histogram("sim_layer", f"Layer number ({section_name})",
                              *layer_bins)
        self.build_1d_histogram("noise",
                              f"Is pure noise hit? ({section_name})", 2, 0, 1)

        self.build_1d_histogram("energy",
                              f"Reconstructed hit energy in the HCal ({section_name})",
                              *energy_bins)

        self.build_1d_histogram("sim_energy",
                              f"Simulated hit energy in the HCal ({section_name})",
                              *energy_bins)
        self.build_1d_histogram("sim_energy_per_bar",
                              f"Simulated hit energy per bar in the HCal ({section_name})",
                              *energy_bins)
        # Once per event
        self.build_1d_histogram("total_energy",
                              f"Total reconstructed energy in the HCal ({section_name})",
                              *total_energy_bins)
        self.build_1d_histogram("sim_total_energy",
                              f"Total simulated energy in the HCal ({section_name})",
                              *total_energy_bins)
        self.build_1d_histogram("total_pe",
                              f"Total photoelectrons in the HCal ({section_name})",
                              200,0,10000)
        self.build_1d_histogram('max_pe',
                              f"Maximum photoelectrons in the HCal ({section_name})",
                              *pe_bins)
        self.build_1d_histogram('max_pe_adc',
                              f"Maximum photoelectrons (ADC mode only) in the HCal ({section_name})",
                              *pe_bins)
        self.build_2d_histogram('sim_layer:strip',
                              f'HCal Layer ({section_name})',
                              *layer_bins,
                              'Back HCal Strip', 62,0,62 )
        self.build_2d_histogram('layer:strip',
                              f'HCal Layer ({section_name})',
                              *layer_bins,
                              'Back HCal Strip', 62,0,62 )
        self.build_1d_histogram("hit_multiplicity",
                              f"HCal hit multiplicity ({section_name})",
                              *multiplicity_bins)
        self.build_1d_histogram("sim_hit_multiplicity",
                              f"HCal hit multiplicity ({section_name})",
                              *multiplicity_bins)
        self.build_1d_histogram("sim_num_bars_hit",
                              f"HCal hit multiplicity ({section_name})",
                              *multiplicity_bins)
        self.build_1d_histogram("vetoable_hit_multiplicity",
                              f"Multiplicity of vetoable hits at {pe_threshold} PE ({section_name})",
                              *multiplicity_bins)
        self.build_1d_histogram('max_pe_time',
                             f"Max PE hit time ({section_name}) [ns]",
                              *time_bins)
        self.build_1d_histogram('max_pe_adc_time',
                             f"Max PE (ADC mode only) hit time ({section_name}) [ns]",
                              *time_bins)
        self.build_1d_histogram('hit_z', f"Reconstructed Z position in the HCal ({section_name}) [mm]",
                              1000, 0, 6000
                              )

class HcalVetoResults(ldmxcfg.Analyzer) :
    """Configured HcalVetoResults python object """

    def __init__(self,name="HcalVetoResults") :
        super().__init__(name,'dqm::HcalVetoResults','DQM')

        self.hcal_veto_name = 'HcalVeto'
        self.hcal_veto_pass = ''
        self.hcal_veto_passname = ''

        self.build_1d_histogram('max_pe',
                'Maximal PE hit PE', 500, -0.5, 499.5)
        self.build_1d_histogram('total_pe',
                'Total number of HCAL photo-electrons', 500, -0.5, 2999.5)
        self.build_1d_histogram('num_valid_hits',
                'Total number of valid HCAL hits', 500, -0.5, 499.5)
        self.build_1d_histogram('max_section',
                'Maximal PE hit section',
                ['Back', 'Top', 'Bottom', 'Right', 'Left'])
        self.build_1d_histogram('max_pos_z',
                'Maximal PE hit postion Z [mm]', 6000, 200., 6200)
        self.build_1d_histogram('veto_pass',
                'Event passed the HCal Veto', 2, -0.5, 1.5)

class HcalInefficiencyAnalyzer(ldmxcfg.Analyzer):
    def __init__(self,name="HcalInefficiencyAnalyzer", num_sections=5,
                 pe_threshold=8, max_hit_time=50.0):
        super().__init__(name,'dqm::HcalInefficiencyAnalyzer','DQM')

        self.sim_coll_name = "HcalSimHits"
        self.sim_pass_name = "" #use whatever pass is available

        self.rec_coll_name= "HcalRecHits"
        self.rec_pass_name= "" #use whatever pass is available

        self.pe_veto_threshold = float(pe_threshold)
        self.max_hit_time = max_hit_time

        section_names = ['back', 'top', 'bottom', 'right', 'left']
        # inefficiency_depth_bins = [6000, 0., 6000.]
        inefficiency_layer_bins = [100, 0, 100]
        # Overall, Back, Side, Top, Bottom, Left, Right, Both,
        # Back only, Side Only, Neither
        self.build_1d_histogram('efficiency', "", 12, -1, 11)
        for section in range(num_sections):
            section_name = section_names[section]
            self.build_1d_histogram(f"inefficiency_{section_name}",
                                  f"Inefficiency ({section_name})",
                                  *inefficiency_layer_bins
                                  )

class EcalDigiVerify(ldmxcfg.Analyzer) :
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

    def __init__(self,name="EcalDigiVerify") :
        super().__init__(name,'dqm::EcalDigiVerifier','DQM')

        self.ecal_sim_hit_coll = "EcalSimHits"
        self.ecal_sim_hit_pass = ""

        self.ecal_rec_hit_coll = "EcalRecHits"
        self.ecal_rec_hit_pass = ""

        self.ecal_presel_coll = "EcalPreselectionDecision"
        self.ecal_presel_pass = ""

        self.num_layers = 32

        self.build_1d_histogram( "rec_sim_hit_residual_x" ,
                "RecHit X - SimHit X [mm]" , 30 , -15.0 , 15.0 )

        self.build_1d_histogram( "rec_sim_hit_residual_y" ,
                "RecHit Y - SimHit Y [mm]" , 30 , -15.0 , 15.0 )

        self.build_1d_histogram( "rec_sim_hit_residual_z" ,
                "RecHit Z - SimHit Z [mm]" , 49 , -0.98 , 0.98 )

        self.build_2d_histogram( "rec_sim_hit_residual_x:layer" ,
                "RecHit X - SimHit X [mm]" , 30 , -15.0 , 15.0,
                "RecHit Layer" , 34 , 0.5 , 34.5 )

        self.build_2d_histogram( "rec_sim_hit_residual_y:layer" ,
                "RecHit Y - SimHit Y [mm]" , 30 , -15.0 , 15.0,
                "RecHit Layer" , 34 , 0.5 , 34.5  )

        self.build_2d_histogram( "rec_sim_hit_residual_z:layer" ,
                "RecHit Z - SimHit Z [mm]" , 48 , -0.6 , 0.6,
                "RecHit Layer" , 34 , 0.5 , 34.5  )

        self.build_1d_histogram( "num_sim_hits_per_cell" ,
                "Number of SimHits per ECal Cell (excluding empty rec cells)" , 20 , -0.5 , 19.5 )

        self.build_1d_histogram( "num_rec_hits" ,
                "Number of RecHits" , 100 , -0.5 , 299.5 )

        self.build_1d_histogram( "num_noise_hits" ,
                "Number of noisy RecHits" , 100 , -0.5 , 99.5 )

        self.build_1d_histogram( "is_noise_hit" ,
                "Is noise hit?" , 2 , -0.5 , 1.5 )

        self.build_1d_histogram( "total_rec_energy"      ,
                "Total Reconstructed Energy in ECal [MeV]" , 800 , 0. , 11000. )

        self.build_1d_histogram( "num_mod_with_0hits"      ,
                "Num of modules with 0 hit" , 100 , 140.5 , 240.5 )

        self.build_1d_histogram( "num_mod_with_1hits"      ,
                "Num of modules with 1 hit" , 31 , -0.5 , 30.5 )

        self.build_1d_histogram( "num_mod_with_2hits"      ,
                "Num of modules with 2 hits" , 31 , -0.5 , 30.5 )

        self.build_1d_histogram( "num_mod_with_more_than_2hits"      ,
                "Num of modules with >2 hits" , 31 , -0.5 , 30.5 )

        self.build_1d_histogram( "num_hit_if_more_than_2hits"      ,
                "Num of hits for modules with >2 hits" , 31 , -0.5 , 30.5 )

        self.build_1d_histogram( "preselection_passed" ,
                "Preselection Passed" , ["Fail", "Pass"] )

        self.build_2d_histogram( "sim_edep:rec_amplitude" ,
                "Simulated Energy [MeV]" , 1000 , 0. , 50. ,
                "Reconstructed Amplitude [MeV]" , 1000 , 0. , 50. )

        self.build_2d_histogram( "sim_edep:rec_energy" ,
                "Simulated Energy [MeV]" , 1000 , 0. , 20. ,
                "Reconstructed Energy [MeV]" , 1000 , 0. , 2000. )

class EcalShowerFeatures(ldmxcfg.Analyzer) :
    """Configured EcalShowerFeatures python object """

    def __init__(self,name="EcalShowerFeatures") :
        super().__init__(name,'dqm::EcalShowerFeatures','DQM')

        self.ecal_veto_name = 'EcalVeto'
        self.ecal_veto_pass = ''

        self.build_1d_histogram('deepest_layer_hit',
                'Deepest Layer Hit',40,0,40)
        self.build_1d_histogram('num_readout_hits',
                'Num Readout Hits',100,0,300)
        self.build_1d_histogram('summed_det',
                'Total Rec Energy [MeV]',600,0.,12000.)
        self.build_1d_histogram('summed_iso',
                'Total Isolated Energy [MeV]',600,0.,12000.)
        self.build_1d_histogram('summed_back',
                'Total Back Energy [MeV]',500,0.,10000.)
        self.build_1d_histogram('max_cell_dep',
                'Maximum Single-Cell Energy Dep [MeV]',200,0.,2000.)
        self.build_1d_histogram('shower_rms',
                'Transverse Shower RMS [mm]',200,0.,200.)
        self.build_1d_histogram('x_std',
                'X Std Deviation [mm]',200,0.,200.)
        self.build_1d_histogram('y_std',
                'Y Std Deviation [mm]',200,0.,200.)
        self.build_1d_histogram('avg_layer_hit',
                'Avg Layer Hit',40,0.,40.)
        self.build_1d_histogram('std_layer_hit',
                'Std Dev Layer Hit',20,0.,20.)
        self.build_1d_histogram('e_containment_energy',
                'Electron Containment Energy [MeV]',200,0.,10000.)
        self.build_1d_histogram('ph_containment_energy',
                'Photon Containment Energy [MeV]',200,0.,10000.)
        self.build_1d_histogram('out_containment_energy',
                'Outside Containment Energy [MeV]',200,0.,10000.)

class EcalMipTrackingFeatures(ldmxcfg.Analyzer) :
    """Configured EcalMipTrackingFeatures python object """

    def __init__(self,name="EcalMipTrackingFeatures") :
        super().__init__(name,'dqm::EcalMipTrackingFeatures','DQM')

        self.ecal_veto_name = 'EcalVeto'
        self.ecal_veto_pass = ''

        self.ecal_mip_name = 'EcalMipInfo'
        self.ecal_mip_pass = ''

        self.build_1d_histogram('n_straight_tracks',
                'Num Straight Tracks',30,-0.5,29.5)
        self.build_1d_histogram('n_linreg_segments',
                'Num Linear Regression Segments',30,-0.5,29.5)
        self.build_1d_histogram('first_near_photon_layer',
                'First Near Photon Layer',34,-0.5,34.5)
        self.build_1d_histogram('ep_ang',
                'Electron Photon Angle [degrees]',90,0.,90.)
        self.build_1d_histogram('ep_sep',
                'Electron Photon Separation',180,0.,180.)
        self.build_1d_histogram('recoil_pz',
                'Recoil electron p_{z} [MeV]',200,-200.,8000.)
        self.build_1d_histogram('recoil_pt',
                'Recoil electron p_{T} [MeV]',200,0,2000.)
        self.build_1d_histogram('recoil_x',
                'Recoil electron x [mm]',100,-300.,300.)
        self.build_1d_histogram('recoil_y',
                'Recoil electron y [mm]',100,-300.,300.)
        self.build_1d_histogram('n_tracking_hits',
                'Num Tracking Hits',300,0.,300.)


class EcalVetoResults(ldmxcfg.Analyzer) :
    """Configured EcalMipTrackingFeatures python object """

    def __init__(self,name="EcalVetoResults") :
        super().__init__(name,'dqm::EcalVetoResults','DQM')

        self.ecal_veto_name = 'EcalVeto'
        self.ecal_veto_pass = ''

        self.build_1d_histogram('bdt_disc',
                'BDT discriminating score',100,0.,1.)
        self.build_1d_histogram('bdt_disc_log',
                '-log(1-BDT discriminating score)',100,0.,5.)
        self.build_1d_histogram('fiducial',
                'Recoil eletron fiducial',2,-0.5,1.5)
        self.build_1d_histogram('bdt_pass',
                'Event passed the ECal BDT',2,-0.5,1.5)



class EcalPnetVetoResults(ldmxcfg.Analyzer) :
    """Configured EcalMipTrackingFeatures python object """

    def __init__(self,name="EcalPnetVetoResults") :
        super().__init__(name,'dqm::EcalPnetVetoResults','DQM')

        self.ecal_pnet_veto_name = 'EcalPnetVeto'
        self.ecal_pnet_veto_pass = ''

        self.build_1d_histogram('pnet_disc',
                'ParticleNet discriminating score',100,0.,1.)
        self.build_1d_histogram('pnet_disc_log',
                '-log(1-ParticleNet discriminating score)',100,0.,5.)
        self.build_1d_histogram('pnet_pass',
                'Event passed the ECal ParticleNet',2,-0.5,1.5)


class EcalWABRecResults(ldmxcfg.Analyzer) :
    """Configured EcalWABRec python object """

    def __init__(self,name="EcalWABRecResults") :
        super().__init__(name,'dqm::EcalWABRecResults','DQM')

        self.ecal_wab_rec_name = 'EcalWABRec'
        self.ecal_wab_rec_pass = ''

        self.build_2d_histogram("ThetaDiffElectronPhoton",
                            "Reco #theta Difference between Photon and Electron [Degrees]",
                            92, -1, 91,
                            "True #theta Difference between Photon and Electron [Degrees]",
                            92, -1, 91)
        self.build_2d_histogram("ThetaElectron",
                            "Electron Reco #theta [Degrees]",
                            92, -1, 91,
                            "Electron True #theta [Degrees]",
                            92, -1, 91)
        self.build_2d_histogram("ThetaPhoton",
                            "Photon Reco #theta [Degrees]",
                            92, -1, 91,
                            "Photon True #theta [Degrees]",
                            92, -1, 91)
        self.build_2d_histogram("PhiDiffElectronPhoton",
                            "Reco #phi Difference between Photon and Electron [Degrees]",
                            92, -1, 91,
                            "True #phi Difference between Photon and Electron [Degrees]",
                            92, -1, 91)
        self.build_2d_histogram("PhiElectron",
                            "Electron Reco #phi [Degrees]",
                            92, -1, 91,
                            "Electron True #phi [Degrees]",
                            92, -1, 91)
        self.build_2d_histogram("PhiPhoton",
                            "Photon Reco #phi [Degrees]",
                            92, -1, 91,
                            "Photon True #phi [Degrees]",
                            92, -1, 91)
        self.build_2d_histogram("ElectronEnergy",
                            "Reconstructed Recoil Electron Shower Energy [MeV]",
                            80, 0, 4000,
                            "True Recoil Electron Energy [MeV]",
                            80, 0, 4000)
        self.build_2d_histogram("PhotonEnergy",
                            "Reconstructed Photon Shower Energy [MeV]",
                            80, 0, 4000,
                            "True Photon Energy [MeV]",
                            80, 0, 4000)
        self.build_1d_histogram("ElectronThetaDiff", "Electron True and Reconstruction #theta Difference [Degrees]", 181, 0, 181)
        self.build_1d_histogram("PhotonThetaDiff", "Photon True and Reconstruction #theta Difference [Degrees]", 181, 0, 181)
        self.build_1d_histogram("ElectronPhiDiff", "Electron True and Reconstruction #phi Difference [Degrees]", 181, 0, 181)
        self.build_1d_histogram("PhotonPhiDiff", "Photon True and Reconstruction #phi Difference [Degrees]", 181, 0, 181)
        self.build_1d_histogram("ProgressNum", "Reconstruction Progress", 4, 0, 4)


class VisiblesFeatureProducer(ldmxcfg.Analyzer) :
    """Just plot the visibles features"""

    def __init__(self, name='VisiblesFeatureProducer') :
        super().__init__(name, 'dqm::VisiblesFeatureProducer', 'DQM')

        ## Parameters choose whether to save features to .txt file ##
        ## Useful for training a Python-based BDT                  ##
        self.training = False
        self.training_file = ""

        ## Input simulation parameters ##
        self.beam_energy = 8000.0 # in MeV
        self.hcal_rec_coll_name = "HcalRecHits"
        self.hcal_rec_pass_name = ''
        self.ecal_rec_coll_name = "EcalRecHits"
        self.ecal_rec_pass_name = ''
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.track_pass_name = ''
        self.sp_coll_name = 'TargetScoringPlaneHits'
        self.sp_pass_name = ''
        self.sim_particles_coll_name = 'SimParticles'
        self.sim_particles_pass_name = ''

        ## Feature histograms ##
        self.build_1d_histogram("layers_hit", "Number of Hcal layers hit", 100, 0, 100)
        self.build_1d_histogram("x_std", "Std dev x [mm]", 80, 0, 800)
        self.build_1d_histogram("y_std", "Std dev y [mm]", 80, 0, 800)
        self.build_1d_histogram("z_std", "Std dev z [mm]", 100, 0, 1000)
        self.build_1d_histogram("x_mean", "Average hit x [mm]", 80, -800, 800)
        self.build_1d_histogram("y_mean", "Average hit  y [mm]", 80, -800, 800)
        self.build_1d_histogram("r_mean", "Average hit  r [mm]", 80, 0, 800)
        self.build_1d_histogram("iso_hits", "Total isolated hits", 100, 0, 100)
        self.build_1d_histogram("iso_energy", "Total isolated energy [MeV]", 80, 0, 800)
        self.build_1d_histogram("n_hits", "Total Hcal hits", 200, 0, 200)
        self.build_1d_histogram("total_energy", "Total Hcal energy [MeV]", 100, 4800, 9800)
        self.build_1d_histogram("photon_track", "Average distance from photon track [mm]", 80, 0, 800)

class VisiblesCutflow(ldmxcfg.Analyzer) :
    def __init__(self, name='VisiblesCutflow') :
        super().__init__(name, 'dqm::VisiblesCutflow', 'DQM')

        from LDMX.Hcal.visibles import makeBDTPath
        self.bdt_file = makeBDTPath("visibles")
        self.feature_list_name = "float_input"
        self.disc_cut = 0.999965
        self.all_cuts = True

        self.beam_energy = 8000.0

        self.hcal_rec_coll_name = "HcalRecHits"
        self.hcal_rec_pass_name = ''
        self.ecal_rec_coll_name = "EcalRecHits"
        self.ecal_rec_pass_name = ''
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.track_pass_name = ''
        self.sp_coll_name = 'TargetScoringPlaneHits'
        self.sp_pass_name = ''
        self.sim_particles_coll_name = 'SimParticles'
        self.sim_particles_pass_name = ''

        self.ecal_veto_coll_name = "EcalVeto"
        self.ecal_veto_pass_name = ''
        self.ecal_disc_cut = 0.99741 # assumes SegMip disc

        ## Histograms for efficiency ##
        self.build_1d_histogram("total_events", "total_events", 40, 0, 2000)
        self.build_1d_histogram("pass_acceptance", "pass_acceptance", 40, 0, 2000)
        self.build_1d_histogram("pass_trigger", "pass_trigger", 40, 0, 2000)
        self.build_1d_histogram("pass_ecal_energy", "pass_ecal_energy", 40, 0, 2000)
        self.build_1d_histogram("pass_tracker_veto", "pass_tracker_veto", 40, 0, 2000)
        self.build_1d_histogram("pass_ecal_bdt", "pass_ecal_bdt", 40, 0, 2000)
        self.build_1d_histogram("pass_hcal_energy", "pass_hcal_energy", 40, 0, 2000)
        self.build_1d_histogram("pass_hcal_containment", "pass_hcal_containment", 40, 0, 2000)
        self.build_1d_histogram("pass_visibles_bdt", "pass_visibles_bdt", 40, 0, 2000)

        ## Histograms for BDT performance plots ##
        self.build_1d_histogram("visibles_disc", "visibles_disc", 100, 0, 1)
        self.build_1d_histogram("visibles_disc_high", "visibles_disc_high", 10000, 0.999, 1)
        self.build_1d_histogram("visibles_disc_high_norm", "visibles_disc_high_norm", 10000, 0.999, 1)
        self.build_2d_histogram("ecal_disc_vs_vis_disc", "vis_disc", 1000, 0.9999, 1, "ecal_disc", 1000, 0.999, 1)
        self.build_1d_histogram("roc", "roc", 10000, 0.99, 1)

        ## LLP kinematics histograms ##
        self.build_1d_histogram("beam_energy_frac", "beam_energy_frac", 100, 0.5, 1)
        self.build_1d_histogram("beam_angle", "beam_angle", 100, 0, 0.5)

        ## BDT feature histograms ##
        self.build_1d_histogram("layers_hit", "Number of Hcal layers hit", 100, 0, 100)
        self.build_1d_histogram("x_std", "Std dev x [mm]", 80, 0, 800)
        self.build_1d_histogram("y_std", "Std dev y [mm]", 80, 0, 800)
        self.build_1d_histogram("z_std", "Std dev z [mm]", 100, 0, 1000)
        self.build_1d_histogram("x_mean", "Average hit x [mm]", 80, -800, 800)
        self.build_1d_histogram("y_mean", "Average hit  y [mm]", 80, -800, 800)
        self.build_1d_histogram("r_mean", "Average hit  r [mm]", 80, 0, 800)
        self.build_1d_histogram("iso_hits", "Total isolated hits", 100, 0, 100)
        self.build_1d_histogram("iso_energy", "Total isolated energy [MeV]", 80, 0, 800)
        self.build_1d_histogram("n_hits", "Total Hcal hits", 200, 0, 200)
        self.build_1d_histogram("total_energy", "Total Hcal energy [MeV]", 100, 4800, 9800)
        self.build_1d_histogram("photon_track", "Average distance from photon track [mm]", 80, 0, 800)

class SimObjects(ldmxcfg.Analyzer) :
    """Configuration for sim-level objects to histogram-ize

    Attributes
    ----------
    sim_pass : str
        Pass name for the sim objects
    """

    def __init__(self,name='sim_dqm',sim_pass='') :
        super().__init__(name,'dqm::SimObjects','DQM')
        self.sim_pass = sim_pass
        self.sim_particles_coll_name = 'SimParticles'
        self.sim_particles_passname = ''
        self.sim_particles_map_passname = ''



class DarkBremInteraction(ldmxcfg.Producer) :
    def __init__(self) :
        super().__init__('DarkBremDQM','dqm::DarkBremInteraction','DQM')
        
        self.particle_coll_name = 'SimParticles'
        self.particle_passname = ''

        self.build_1d_histogram('aprime_energy',
            'Dark Photon Energy [MeV]',101,0,8080)
        self.build_1d_histogram('aprime_pt',
            'Dark Photon pT [MeV]',100,0,2000)
        self.build_1d_histogram('aprime_theta',
            'Dark Photon Theta [degree]',50,0.,100.)

        self.build_1d_histogram('recoil_energy',
            'Recoil Electron Energy [MeV]',101,0,8080)
        self.build_1d_histogram('recoil_pt',
            'Recoil Electron pT [MeV]',100,0,2000)
        self.build_1d_histogram('recoil_theta',
            'Recoil Electron Theta [degree]',50,0.,100.)


        self.build_1d_histogram('incident_energy',
            'Incident Electron Energy [MeV]',101,0,8080)
        self.build_1d_histogram('incident_pt',
            'Incident Electron pT [MeV]',100,0,2000)

        # weird binning so we can see the target and trigger pads
        self.build_1d_histogram('dark_brem_z',
            'Z Location of Dark Brem [mm]', 160, -7., 1.)
        # elements are hydrogen and carbon (for trigger pads) and tungsten target
        self.build_1d_histogram('dark_brem_element',
            'Element in which Dark Brem Occurred',
            ["did not happen", "H 1", "C 6", "O 8", "Na 11", "Si 14",
             "Ca 20", "Cu 29", "Y 39", "Lu 71", "W 74", "unlisted"])
        self.build_1d_histogram('dark_brem_material',
            'Material in which Dark Brem Occurred',
            ["Unknown", "C", "PCB", "Glue", "Si", "Al", "W / LYSO", "PVT"])


class HCalRawDigi(ldmxcfg.Analyzer) :
    def __init__(self, input_name) :
        super().__init__('hcal_pedestals','dqm::HCalRawDigi','DQM')

        self.input_name = input_name
        self.input_pass = ''

class HgcrocPulseTruth(ldmxcfg.Analyzer) :
    def __init__(self, input_digi_name, input_truth_name) :
        super().__init__('hgcroc_pulse_truth', 'dqm::HgcrocPulseTruth', 'DQM')
        self.input_digi_name = input_digi_name
        self.input_digi_pass = ''
        self.input_truth_name = input_truth_name
        self.input_truth_pass = ''

        self.build_2d_histogram("vpeak_sumADC",
                            "Pulse Peak Voltage [mV]",
                            200, 0, 2000,
                            "Digi sum of ADC",
                            256, 0, 1024)

        self.build_2d_histogram("vpeak_TOT",
                            "Pulse Peak Voltage [mV]",
                            200, 0, 5000,
                            "Digi TOT in SOI",
                            512, 0, 4096)


class NtuplizeHgcrocDigiCollection(ldmxcfg.Analyzer) :
    def __init__(self,input_name, pedestal_table = None, input_pass = '',
            using_eid = None, already_aligned = False,
            name = 'ntuplizehgcroc') :
        super().__init__(name,'dqm::NtuplizeHgcrocDigiCollection','DQM')
        self.input_name = input_name
        self.input_pass = input_pass

        if using_eid is None :
            # deduce if using eid based on presence of HcalDetectorMap in conditions
            # system
            from LDMX.Framework import ldmxcfg
            from LDMX.Hcal.detector_map import HcalDetectorMap
            using_eid = True
            for cop in ldmxcfg.Process.last_process.conditions_object_providers :
                if isinstance(cop,HcalDetectorMap) :
                    using_eid = False
                    break
        self.using_eid = using_eid
        self.already_aligned = already_aligned

        from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider
        if pedestal_table is None :
            self.pedestal_table = 'NO_PEDESTALS'
            t = SimpleCSVIntegerTableProvider('NO_PEDESTALS',["PEDESTAL"])
            t.validForAllRows([0])
        else :
            self.pedestal_table = pedestal_table
            t = SimpleCSVIntegerTableProvider(pedestal_table,["PEDESTAL"])
            t.validForever(f'file://{pedestal_table}')

class NtuplizeTrigScintQIEDigis(ldmxcfg.Analyzer) :
    def __init__(self,input_name, input_pass = '', name = 'ts') :
        super().__init__(name,'dqm::NtuplizeTrigScintQIEDigis','DQM')
        self.input_name = input_name
        self.input_pass = input_pass

class PhotoNuclearDQM(ldmxcfg.Analyzer) :
    """Configured PhotoNuclearDQM python object

    Contains an instance of PhotoNuclearDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.PhotoNuclearDQM() )
    """

    def __init__(self,name='PN', count_light_ions=True) :
        super().__init__(name,'dqm::PhotoNuclearDQM','DQM')

        self.sim_particles_coll_name = 'SimParticles'
        self.sim_particles_passname = ''

        event_type_labels = [
            "Nothing hard",   # 0
            "1 n",            # 1
            "2 n",            # 2
            "#geq 3 n",       # 3
            "1 #pi",          # 4
            "2 #pi",          # 5
            "1 #pi_{0}",      # 6
            "1 #pi A",        # 7
            "1 #pi 2 A",      # 8
            "2 #pi A",        # 9
            "1 #pi_{0} A",    # 10
            "1 #pi_{0} 2 A",  # 11
            "#pi_{0} #pi A",  # 12
            "1 p",            # 13
            "2 p",            # 14
            "pn",             # 15
            "K^{0}_{L} X",    # 16
            "K X",            # 17
            "K^{0}_{S} X",    # 18
            "exotics",        # 19
            "multi-body",     # 20
        ]
        event_type_compact_labels = [
            "1 n",      # 0
            "K#pm X",   # 1
            "1 K^{0}",  # 2
            "2 n",      # 3
            "Soft",     # 4
            "Other",    # 5
        ]

        self.count_light_ions=count_light_ions
        self.build_1d_histogram("event_type"         , "", event_type_labels)
        self.build_1d_histogram("event_type_500mev"  , "", event_type_labels)
        self.build_1d_histogram("event_type_2000mev" , "", event_type_labels)
        self.build_1d_histogram("event_type_compact"         , "", event_type_compact_labels)
        self.build_1d_histogram("event_type_compact_500mev"  , "", event_type_compact_labels)
        self.build_1d_histogram("event_type_compact_2000mev" , "", event_type_compact_labels)
        self.build_1d_histogram("1n_event_type"              , "", [
            "nn", "pn", "#pi^{+}n", "#pi^{0}n", "other"
        ])
        self.build_1d_histogram("pn_vertex_volume"           , "", [
            "Didn't happen", "Else", "W Cooling", "C Cooling", "PCB",
            "CarbonBasePlate", "Absorber", "Sensor", "Glue", "Motherboard"
        ])
        self.build_1d_histogram("pn_interaction_material"    , "", [
            "Didn't happen", "Else", "Si", "W", "FR4", "Steel", "Epoxy",
            "PVT", "Glue", "Air"
        ])
        self.build_1d_histogram("pn_particle_mult"   , "Photo-nuclear Multiplicity", 200, 0, 200)
        self.build_1d_histogram("pn_neutron_mult"    , "Photo-nuclear Neutron Multiplicity", 200,0, 200)
        self.build_1d_histogram("pn_gamma_energy"    , "#gamma Energy [MeV]", 100, 0, 10000)
        self.build_1d_histogram("pn_total_ke"        , "Total Kineitc Energy of Photo-Nuclear Products [MeV]", 100, 0, 10000)
        self.build_1d_histogram("pn_total_neutron_ke", "Total Kineitc Energy of Photo-Nuclear Neutrons  [MeV]", 100, 0, 10000)
        self.build_1d_histogram("1n_neutron_energy"  , "Neutron Energy [MeV]", 100, 0, 10000)
        self.build_1d_histogram("1n_energy_diff"     , "E(#gamma_{PN}) - E(n) [MeV]", 100, 0, 10000)
        self.build_1d_histogram("1n_energy_frac"     , "E(n)/E(#gamma_{PN}) [MeV]", 100, 0, 1)
        self.build_1d_histogram("2n_n2_energy"       , "Energy of second hardest neutron [MeV]", 100, 0, 10000)
        self.build_1d_histogram("2n_energy_frac"     , "E(n)/E(#gamma_{PN}) [MeV]", 100, 0, 1)
        self.build_1d_histogram("2n_energy_other"    , "E_{other} [MeV]", 100, 0, 10000)
        self.build_1d_histogram("1kp_energy"         , "Charged Kaon Energy [MeV]", 100, 0, 10000)
        self.build_1d_histogram("1kp_energy_diff"    , "E(#gamma_{PN}) - E(K#pm) [MeV]", 100, 0, 100000)
        self.build_1d_histogram("1kp_energy_frac"    , "E(K#pm)/E(#gamma_{PN}) [MeV]", 100, 0, 1)
        self.build_1d_histogram("1k0_energy"         , "K0 Energy [MeV]", 100, 0, 10000)
        self.build_1d_histogram("1k0_energy_diff"    , "E(#gamma_{PN}) - E(K0) [MeV]", 100, 0, 10000)
        self.build_1d_histogram("1k0_energy_frac"    , "E(K0)/E(#gamma_{PN}) [MeV]", 100, 0, 1)

        self.build_1d_histogram("recoil_vertex_x",   "Recoil e^{-} Vertex - x [mm]", 40, -40, 40)
        self.build_1d_histogram("recoil_vertex_y",   "Recoil e^{-} Vertex - y [mm]", 80, -80, 80)
        self.build_1d_histogram("recoil_vertex_z",   "Recoil e^{-} Vertex - z [mm]", 20, -950, -850)

        self.build_1d_histogram("pn_gamma_int_x",    "#gamma Interaction Vertex - x [mm]", 50, -250, 250)
        self.build_1d_histogram("pn_gamma_int_y",    "#gamma Interaction Vertex - y [mm]", 50, -250, 250)
        self.build_1d_histogram("pn_gamma_int_z",    "#gamma Interaction Vertex - z [mm]", 40, 200, 400)

        self.build_1d_histogram("pn_gamma_vertex_x", "#gamma Vertex - y [mm]", 40,  -40, 40)
        self.build_1d_histogram("pn_gamma_vertex_y", "#gamma Vertex - y [mm]", 80,  -80, 80)
        self.build_1d_histogram("pn_gamma_vertex_z", "#gamma Vertex - z [mm]", 10, -5,  5)

        self.build_1d_histogram("hardest_ke",       "Kinetic Energy Hardest Photo-nuclear Particle [MeV]", 200, 0, 8000)
        self.build_1d_histogram("hardest_theta",    "#theta of Hardest Photo-nuclear Particle [Degrees]", 180, 0, 180)
        self.build_1d_histogram("hardest_p_ke",     "Kinetic Energy Hardest Photo-nuclear Proton [MeV]", 200, 0, 8000)
        self.build_1d_histogram("hardest_p_theta",  "#theta of Hardest Photo-nuclear Proton [Degrees]", 180, 0, 180)
        self.build_1d_histogram("hardest_n_ke",     "Kinetic Energy Hardest Photo-nuclear Neutron [MeV]", 200, 0, 8000)
        self.build_1d_histogram("hardest_n_theta",  "#theta of Hardest Photo-nuclear Neutron [Degrees]", 180, 0, 180)
        self.build_1d_histogram("hardest_pi_ke",    "Kinetic Energy Hardest Photo-nuclear #pi [MeV]", 200, 0, 8000)
        self.build_1d_histogram("hardest_pi_theta", "#theta of Hardest Photo-nuclear #pi [Degrees]", 180, 0, 180)

        self.build_2d_histogram("h_ke_h_theta",
                            "Kinetic Energy Hardest Photo-nuclear Particle [MeV]",
                            200, 0, 8000,
                            "#theta of Hardest Photo-nuclear Particle [Degrees]",
                            180, 0, 180)

        self.build_2d_histogram("1n_ke:2nd_h_ke",
                            "Kinetic Energy of Leading Neutron [MeV]",
                            200, 0, 8000,
                            "Kinetic Energy of 2nd Hardest Particle [MeV]",
                            200, 0, 8000)

        self.build_2d_histogram("1kp_ke:2nd_h_ke",
                            "Kinetic Energy of Leading Charged Kaon [MeV]",
                            200, 0, 8000,
                            "Kinetic Energy of 2nd Hardest Particle [MeV]",
                            200, 0, 8000)

        self.build_2d_histogram("1k0_ke:2nd_h_ke",
                            "Kinetic Energy of Leading K0 [MeV]",
                            200, 0, 8000,
                            "Kinetic Energy of 2nd Hardest Particle [MeV]",
                            200, 0, 8000)

        self.build_2d_histogram("recoil_vertex_x:recoil_vertex_y",
                           "Recoil electron vertex x [mm]",
                           80, -40, 40,
                           "Recoil electron vertex y [mm]",
                           160, -80, 80)

        self.build_2d_histogram("pn_gamma_int_x:pn_gamma_int_y",
                           "PN gamma interaction vertex x [mm]",
                           50, -250, 250,
                           "PN gamma interaction vertex y [mm]",
                           50, -250, 250)


class TrkDeDxMassEstFeatures(ldmxcfg.Analyzer) :
    """Configured TrkDeDxMassEstFeatures python object

    Contains an instance of TrkDeDxMassEstFeatures that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrkDeDxMassEstFeatures() )
    """

    def __init__(self,name='TrkDeDxMassEstFeatures') :
        super().__init__(name, "dqm::TrkDeDxMassEstFeatures",'DQM')

        self.mass_estimate_name = "TrackDeDxMassEstimate"
        self.mass_estimate_pass = ""

        momentum_bins = [90.,100.,125.,150.,175.,200.,250.,300.,350.,400.,450.,500., 600.,700.,800.,900.,1000.,1300.,2000.,3000.,4000.,6000.,8000.]
        low_momentum_bins = [50.,70.,90.,100.,125.,150.,175.,200.,250.,300.,350.,400.,450.,500., 600.,700.,800.,900.,1000.,2000.]
        self.build_2d_histogram('momentum:harmonic_mean_dedx' ,
                xlabel='Momentum [MeV]', xbins=momentum_bins,
                ylabel='I_{h} [MeV/cm]', ybins=50, ymin=0., ymax=30. )
        self.build_2d_histogram('momentum_low:harmonic_mean_dedx' ,
                xlabel='Momentum [MeV]', xbins=low_momentum_bins,
                ylabel='I_{h} [MeV/cm]', ybins=50, ymin=0., ymax=30. )
        self.build_1d_histogram("harmonic_mean_dedx", "I_{h} [MeV/cm]", 50, 0., 30.)
        self.build_1d_histogram("mass_estimate", "Mass Estimate [MeV]", 100, 0., 2000.)
        self.build_1d_histogram("mass_estimate_low_p", "Mass Estimate [MeV]", 100, 0., 2000.)
        self.build_1d_histogram("mass_estimate_very_low_p", "Mass Estimate [MeV]", 100, 0., 2000.)
        self.build_1d_histogram("mass_estimate_very_low_p_electron", "Mass Estimate for electrons [MeV]", 20, 0., 200.)
        self.build_1d_histogram("mass_estimate_very_low_p_pion", "Mass Estimate for pions [MeV]", 20, 0., 200.)
        self.build_1d_histogram("mass_estimate_very_low_p_kaon", "Mass Estimate for kaons [MeV]", 60, 200., 800.)
        self.build_1d_histogram("mass_estimate_very_low_p_proton", "Mass Estimate for proton [MeV]", 40, 800., 1200.)
        self.build_1d_histogram("track_type", "Track Type", ['Other', 'Tagger', 'Recoil'])


class TrigScintSimDQM(ldmxcfg.Analyzer) :
    """Configured TrigScintSimDQM python object

    Contains an instance of TrigScintSimDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintSimDQM() )
    """

    def __init__(self,name='TrigScintSimUp',hit_coll='TriggerPadUpSimHits',pad='up') :
        super().__init__(name,'dqm::TrigScintDQM','DQM')

        self.hit_collection = hit_coll
        self.pad = pad
        self.hit_passname = ''

class TrigScintDigiDQM(ldmxcfg.Analyzer) :
    """Configured TrigScintDigiDQM python object

    Contains an instance of TrigScintDigiDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintDigiDQM() )
    """

    def __init__(self,name='TrigScintDigiUp',hit_coll='trigScintDigisUp',pad='up') :
        super().__init__(name,'dqm::TrigScintHitDQM','DQM')

        self.hit_collection = hit_coll
        self.pad = pad
        self.trig_scint_passname = ''

class TrigScintDigiVerifierDQM(ldmxcfg.Analyzer) :
    def __init__(self, name = 'TrigScintDigiVerifier', ts_simhit_coll = 'TriggerPadUpSimHits', ts_digi_coll = 'trigScintDigisUp') :
        super().__init__(name,'dqm::TrigScintDigiVerifier','DQM')

        self.ts_simhit_coll = ts_simhit_coll
        self.ts_simhit_pass = ''
        self.ts_digi_coll = ts_digi_coll
        self.ts_digi_pass = ''

        self.build_2d_histogram( "sim_edep:rec_amplitude" ,
                "Simulated Energy [MeV]" , 1000 , 0. , 50. ,
                "Reconstructed Amplitude [MeV]" , 1000 , 0. , 50. )

        self.build_2d_histogram( "sim_edep:rec_energy" ,
                "Simulated Energy [MeV]" , 1000 , 0. , 50. ,
                "Reconstructed Energy [MeV]" , 1000 , 0. , 50. )


class TrigScintClusterDQM(ldmxcfg.Analyzer) :
    """Configured TrigScintClusterDQM python object

    Contains an instance of TrigScintClusterDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintClusterDQM() )
    """

    def __init__(self,name='TrigScintClusterUp',coll='TriggerPadUpClusters',pad='up') :
        super().__init__(name,'dqm::TrigScintClusterDQM','DQM')

        self.cluster_collection = coll
        self.pad = pad
        self.pass_name = ''


class TrigScintTrackDQM(ldmxcfg.Analyzer) :
    """Configured TrigScintTrackDQM python object

    Contains an instance of TrigScintTrackDQM that
    has already been configured.

    Builds the necessary histograms as well.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.TrigScintTrackDQM() )
    """

    def __init__(self,name='TrigScintTrack',coll='TriggerPadTracks') :
        super().__init__(name,'dqm::TrigScintTrackDQM','DQM')

        self.track_collection = coll
        self.pass_name = ''


class Trigger(ldmxcfg.Analyzer) :
    """Configured Trigger python object
    Contains an instance of TrigScintTrackDQM that
    has already been configured.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.Trigger() )
    """

    def __init__(self,name='Trigger',coll='Trigger') :
        super().__init__(name,'dqm::Trigger','DQM')

        self.trigger_name = coll
        self.trigger_pass = ''


class SampleValidation(ldmxcfg.Analyzer) :
    """Configured Sample Validation python object
    Package used to validate samples

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append(dqm.SampleValidation())
    """
    def __init__(self, name='SampleValidation') :
        super().__init__(name, 'dqm::SampleValidation', 'DQM')

        self.sim_particles_coll_name = 'SimParticles'
        self.sim_particles_passname = ''
        self.target_scoring_plane_coll_name = 'TargetScoringPlaneHits'
        self.target_scoring_plane_passname = ''

        pdgid_bin_labels = [
            "e^{+}",                   # 0
            "e^{-}",                   # 1
            "#mu^{+}",                 # 2
            "#mu^{-}",                 # 3
            "#gamma",                  # 4
            "p^{+}",                   # 5
            "n^{0}",                   # 6
            "#pi^{+}",                 # 7
            "#pi^{-}",                 # 8
            "#pi^{0}",                 # 9
            "K^{+}",                   # 10
            "K^{-}",                   # 11
            "K_{L}",                   # 12
            "K_{S}",                   # 13
            "light-N",                 # 14
            "heavy-N",                 # 15
            "#Lambda / #Sigma / #Xi",  # 16
            "A'",                      # 17
            "else",                    # 18
        ]

        #primary histograms
        self.build_1d_histogram("primaries_pdgid", "ID of primary particles", pdgid_bin_labels)
        self.build_1d_histogram("primaries_energy", "Energy of primary particles [MeV]", 90, 0, 9000) # range applicable for 4 GeV beam
        self.build_1d_histogram("primaries_theta", "Theta [degree]", 50,0.,100.)
        self.build_1d_histogram("primaries_pt", "Transverse Momentum [MeV]", 200,0.,2000.)

        self.build_2d_histogram("beam_smear", "x [mm]", 30, -150, 150, "y [mm]", 30, -150, 150)
        self.build_1d_histogram("primarydaughters_pdgid", "ID of primary daughters", pdgid_bin_labels)
        self.build_1d_histogram("daughterphoton_energy", "Energy spectrum of all photons from primary [MeV]", 170, 0, 8500)

        #primary daughter of interest(brem / dark brem) histograms
        self.build_1d_histogram("harddaughters_pdgid", "ID of primary daughters", pdgid_bin_labels)
        self.build_1d_histogram("harddaughters_startZ", "Start z position of hard primary daughter [mm]", 100, -500, 500)
        self.build_1d_histogram("harddaughters_endZ", "End z position of hard primary daughter [mm]", 100, -500, 500)
        self.build_1d_histogram("harddaughters_energy", "Energy spectrum of hard primary daughter [MeV]", 130, 2000, 8500)
        self.build_1d_histogram("harddaughters_theta", "Theta [degree]", 25,0.,50.)
        self.build_1d_histogram("harddaughters_pt", "Transverse Momentum [MeV]", 50,0.,100.)

        #daughters of hard brem histograms
        self.build_1d_histogram("hardbremdaughters_pdgid", "ID of hard brem daughters", pdgid_bin_labels)
        self.build_1d_histogram("hardbremdaughters_startZ", "Start z position of hard brem daughters  [mm]", 200, -1000, 1000)
        self.build_1d_histogram("hardbremdaughters_endZ", "End z position of hard brem daughters  [mm]", 70, -1000, 6000)
        self.build_1d_histogram("hardbremdaughters_energy", "Energy of hard brem daughters  [MeV]", 170, 0, 8500)
        self.build_1d_histogram("hardbremdaughters_theta", "Theta [degree]", 50,0.,100.)
        self.build_1d_histogram("hardbremdaughters_pt", "Transverse Momentum [MeV]", 200,0.,1000.)


class GenieTruthDQM(ldmxcfg.Analyzer) :
    """Configured GenieTruthDQM python object

    Contains an instance of GenieTruthDQM that
    has already been configured.

    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.GenieTruthDQM() )
    """

    def __init__(self,name='GenieTruthDQM',coll_name="",pass_name="") :
        super().__init__(name,'dqm::GenieTruthDQM','DQM')

        self.hepmc3_coll_name = coll_name
        self.hepmc3_pass_name = pass_name


sample_validation_dqm = [
        SampleValidation()
        ]

class EcalClusterAnalyzer(ldmxcfg.Analyzer) :
    """Analyze clustering"""

    def __init__(self,name='EcalClusterAnalyzer') :
        super().__init__(name, "dqm::EcalClusterAnalyzer", 'DQM')

        self.use_simulated_electron_number = False
        self.nbr_of_electrons = 2

        self.ecal_sim_hit_coll = "EcalSimHits"
        self.ecal_sim_hit_pass = "" #use whatever pass is available

        #Pass name for ecal digis and rec hits
        self.rec_hit_coll_name = 'EcalRecHits'
        self.rec_hit_pass_name = ''

        self.cluster_coll_name = 'EcalClusters'
        self.cluster_pass_name = ''

        self.ecal_sp_hits_coll_name = 'EcalScoringPlaneHits'
        self.ecal_sp_hits_pass_name = ''
        self.mixed_hit_cutoff = 0.05

        self.inverse_skim = False

        self.n_ecal_clusters_min = 1


        self.build_1d_histogram("number_of_clusters_first_layer", "Number of CLUE clusters on the first layer", 5, -0.5, 4.5)
        self.build_1d_histogram("number_of_clusters_per_layer", "Number of CLUE clusters per layer", 5, -0.5, 4.5)
        self.build_1d_histogram("number_of_clusters", "Total number of CLUE clusters", 51, -0.5, 50.5)
        self.build_1d_histogram("correctly_predicted_events", "Correct Cluster Count",
                              ["Underpredicted", "Correct", "Overpredicted"])

        #Need to mod for more than two electrons
        self.build_1d_histogram("ancestors", "Ancestors of particles", 4, 0., 4.)

        self.build_1d_histogram("same_ancestor", "Percentage of hits in cluster coming from the electron that produced most hits", 21, 0., 105.)
        self.build_1d_histogram("energy_percentage", "Percentage of energy in cluster coming from the electron that produced most of energy", 21, 0., 105.)
        self.build_1d_histogram("mixed_hit_energy", "Percentage of total energy coming from hits with energy contributions from more than one electron", 21, 0., 105.)
        self.build_1d_histogram("unclustered_hits", "Number of hits not in a cluster", 10, 0., 200.)
        self.build_1d_histogram("unclustered_hits_percentage", "Percentage of hits not in a cluster", 21, 0., 105.)
        self.build_1d_histogram("total_rechits_in_event", "RecHits per event", 20, 0., 500.)

        self.build_2d_histogram("total_energy_vs_hits", "Total energy (edep) [MeV]", 30, 0., 150., "Hits in cluster", 30, 0., 300.)
        self.build_2d_histogram("total_energy_vs_purity", "Total energy (edep) [MeV]", 30, 0., 150., "Energy purity %", 21, 0, 105.)
        self.build_2d_histogram("sp_ele_distance_vs_purity", "SP ele distance in xy-plane [mm]", 50, 0, 250, "Energy purity %", 21, 0., 105.)
        self.build_2d_histogram("sp_clue_distance_vs_layer", "CLUE centroid to SP ele distance in xy-plane [mm]", 125, 0., 250., "Layer", 33, -0.5, 32.5)

        self.build_1d_histogram("sp_clue_distance", "CLUE centroid to SP ele distance in xy-plane [mm]", 125, 0., 250.)
        self.build_1d_histogram("sp_clue_x_residual", "CLUE centroid X - SP ele X [mm]", 250, -250., 250.)
        self.build_1d_histogram("sp_clue_y_residual", "CLUE centroid Y - SP ele Y [mm]", 250, -250., 250.)

        self.build_1d_histogram("sp_distance", "dR(SPhit_1, SPhit_2)", 100, -1, 202)
        self.build_1d_histogram("cluster_distance", "dR(cl_1, cl_2)", 100, -1, 202)
        self.build_1d_histogram("cluster_RMSX", "RMS(hits in cluster) X", 100, -1, 202)
        self.build_1d_histogram("cluster_RMSY", "RMS(hits in cluster) Y", 100, -1, 202)
        self.build_2d_histogram("dE_cl2_vs_cl1", "E_{cl}-E_{true}^{SP}, cluster 1 [MeV]", 100, -10000, 10000, "E_{cl}-E_{true}^{SP}, cluster 2 [MeV]", 100, -10000, 10000)

        self.build_2d_histogram("tag0frac_vs_SPdist", "dR(SPhit_1, SPhit_2)", 251, -1, 250,  "Fraction of mixed (purity less than {int(100*(1.-self.mixed_hit_cutoff))}%) ancestors", 200, 0, 1)

ecal_dqm = [
        EcalDigiVerify(),
        EcalShowerFeatures(),
        EcalMipTrackingFeatures(),
        EcalVetoResults(),
        EcalPnetVetoResults()
        ]

hcal_dqm = [
        HCalDQM(pe_threshold=8,
                section=0
                ),
        HCalDQM(pe_threshold=8,
                section=1
                ),
        HCalDQM(pe_threshold=8,
                section=2
                ),
        HCalDQM(pe_threshold=8,
                section=3
                ),
        HCalDQM(pe_threshold=8,
                section=4
                ),
        HcalInefficiencyAnalyzer(),
        HcalVetoResults(),
  ]

dEdx_dqm = [
        TrkDeDxMassEstFeatures()
        ]

trigScint_dqm = [
    TrigScintSimDQM('TrigScintSimPad1','TriggerPad1SimHits','pad1'),
    TrigScintSimDQM('TrigScintSimPad2','TriggerPad2SimHits','pad2'),
    TrigScintSimDQM('TrigScintSimPad3','TriggerPad3SimHits','pad3'),
    TrigScintDigiDQM('TrigScintDigiPad1','trigScintDigisPad1','pad1'),
    TrigScintDigiDQM('TrigScintDigiPad2','trigScintDigisPad2','pad2'),
    TrigScintDigiDQM('TrigScintDigiPad3','trigScintDigisPad3','pad3'),
    TrigScintClusterDQM('TrigScintClusterPad1','TriggerPad1Clusters','pad1'),
    TrigScintClusterDQM('TrigScintClusterPad2','TriggerPad2Clusters','pad2'),
    TrigScintClusterDQM('TrigScintClusterPad3','TriggerPad3Clusters','pad3'),
    TrigScintTrackDQM('TrigScintTracks','TriggerPadTracks')
    ]


trigger_dqm = [
        Trigger()
        ]


all_dqm = sample_validation_dqm + ecal_dqm + hcal_dqm + trigScint_dqm + trigger_dqm
