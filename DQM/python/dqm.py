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
        self.build1DHistogram('passes_sim', 'Simulated hits within scintillator bounds?', 2, 0,2)
        self.build1DHistogram('passes_rec', 'Reconstructed hits within scintillator bounds?', 2, 0,2)
        section_names = ['back', 'top', 'bottom', 'right', 'left']
        for name in section_names:
            self.build1DHistogram(f'passes_sim_{name}', f'Simulated hits within scintillator bounds? ({name})', 2, 0,2)
            self.build1DHistogram(f'passes_rec_{name}', f'Reconstructed hits within scintillator bounds? Passing ({name})', 2, 0,2)


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



    def __init__(self,name="hcal_dqm", pe_threshold=5, section=0, max_hit_time = 50.0) :
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

        pe_bins = [1500, 0, 1500]
        time_bins = [100, -100, 500]
        layer_bins = [100,0,100]
        multiplicity_bins = [400,0,400]
        energy_bins = [200,0,200]
        total_energy_bins = [1000, 0, 1000]
        self.build1DHistogram('sim_along_x', 'x', 1200, -3000,3000)
        self.build1DHistogram('sim_along_y', 'y', 1200, -3000,3000)
        self.build1DHistogram('sim_along_z', 'z', 1200, 0,6000)
        self.build1DHistogram('along_x', 'x', 1200, -3000,3000)
        self.build1DHistogram('along_y', 'y', 1200, -3000,3000)
        self.build1DHistogram('along_z', 'z', 1200, 0,6000)
        # Per hit
        self.build1DHistogram("pe",
                              f"Photoelectrons in the HCal ({section_name})",
                              *pe_bins)
        self.build1DHistogram('hit_time', f'HCal hit time ({section_name}) [ns]',
                              *time_bins)
        self.build1DHistogram('sim_hit_time', f'HCal hit time ({section_name}) [ns]',
                              *time_bins)
        self.build1DHistogram("layer", f"Layer number ({section_name})",
                              *layer_bins)
        self.build1DHistogram("sim_layer", f"Layer number ({section_name})",
                              *layer_bins)
        self.build1DHistogram("noise",
                              f"Is pure noise hit? ({section_name})", 2, 0, 1)

        self.build1DHistogram("energy",
                              f"Reconstructed hit energy in the HCal ({section_name})",
                              *energy_bins)

        self.build1DHistogram("sim_energy",
                              f"Simulated hit energy in the HCal ({section_name})",
                              *energy_bins)
        self.build1DHistogram("sim_energy_per_bar",
                              f"Simulated hit energy per bar in the HCal ({section_name})",
                              *energy_bins)
        # Once per event
        self.build1DHistogram("total_energy",
                              f"Total reconstructed energy in the HCal ({section_name})",
                              *total_energy_bins)
        self.build1DHistogram("sim_total_energy",
                              f"Total simulated energy in the HCal ({section_name})",
                              *total_energy_bins)
        self.build1DHistogram("total_pe",
                              f"Total photoelectrons in the HCal ({section_name})",
                              200,0,10000)
        self.build1DHistogram('max_pe',
                              f"Maximum photoelectrons in the HCal ({section_name})",
                              *pe_bins)
        self.build2DHistogram('sim_layer:strip',
                              f'HCal Layer ({section_name})',
                              *layer_bins,
                              'Back HCal Strip', 62,0,62 )
        self.build2DHistogram('layer:strip',
                              f'HCal Layer ({section_name})',
                              *layer_bins,
                              'Back HCal Strip', 62,0,62 )
        self.build1DHistogram("hit_multiplicity",
                              f"HCal hit multiplicity ({section_name})",
                              *multiplicity_bins)
        self.build1DHistogram("sim_hit_multiplicity",
                              f"HCal hit multiplicity ({section_name})",
                              *multiplicity_bins)
        self.build1DHistogram("sim_num_bars_hit",
                              f"HCal hit multiplicity ({section_name})",
                              *multiplicity_bins)
        self.build1DHistogram("vetoable_hit_multiplicity",
                              f"Multiplicity of vetoable hits at {pe_threshold} PE ({section_name})",
                              *multiplicity_bins)
        self.build1DHistogram('max_pe_time',
                             f"Max PE hit time ({section_name}) [ns]",
                              *time_bins)
        self.build1DHistogram('hit_z', 'Reconstructed Z position in the HCal ({section_name}) [mm]',
                              1000, 0, 6000
                              )



class HcalInefficiencyAnalyzer(ldmxcfg.Analyzer):
    def __init__(self,name="HcalInefficiencyAnalyzer", num_sections=5,
                 pe_threshold=5, max_hit_time=50.0):
        super().__init__(name,'dqm::HcalInefficiencyAnalyzer','DQM')

        self.sim_coll_name = "HcalSimHits"
        self.sim_pass_name = "" #use whatever pass is available

        self.rec_coll_name= "HcalRecHits"
        self.rec_pass_name= "" #use whatever pass is available

        self.pe_veto_threshold = float(pe_threshold)
        self.max_hit_time = max_hit_time

        section_names = ['back', 'top', 'bottom', 'right', 'left']
        inefficiency_depth_bins = [6000, 0., 6000.]
        inefficiency_layer_bins = [100, 0, 100]
        # Overall, Back, Side, Top, Bottom, Left, Right, Both,
        # Back only, Side Only, Neither
        self.build1DHistogram('efficiency', "", 12, -1, 11)
        for section in range(num_sections):
            section_name = section_names[section]
            self.build1DHistogram(f"inefficiency_{section_name}",
                                  f"Inefficiency ({section_name})",
                                  *inefficiency_layer_bins
                                  )

class EcalDigiVerify(ldmxcfg.Analyzer) :
    """Configured EcalDigiVerifier python object
    
    Contains an instance of EcalDigiVerifier that
    has already been configured.

    The EcalDigiVerifier fills three histograms.
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
    
    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.EcalDigiVerify('EcalDigiVerify') )
    """

    def __init__(self,name="EcalDigiVerify") :
        super().__init__(name,'dqm::EcalDigiVerifier','DQM')

        self.ecalSimHitColl = "EcalSimHits"
        self.ecalSimHitPass = "" #use whatever pass is available

        self.ecalRecHitColl = "EcalRecHits"
        self.ecalRecHitPass = "" #use whatever pass is available

        self.build1DHistogram( "num_sim_hits_per_cell" ,
                "Number SimHits per ECal Cell (excluding empty rec cells)" , 20 , 0 , 20 )
        
        self.build1DHistogram( "total_rec_energy"      ,
                "Total Reconstructed Energy in ECal [MeV]" , 800 , 0. , 8000. )
        
        self.build2DHistogram( "sim_edep__rec_amplitude" ,
                "Simulated [MeV]" , 1000 , 0. , 50. ,
                "Reconstructed [MeV]" , 1000 , 0. , 50. )

class EcalShowerFeatures(ldmxcfg.Analyzer) :
    """Configured EcalShowerFeatures python object """

    def __init__(self,name="EcalShowerFeatures") :
        super().__init__(name,'dqm::EcalShowerFeatures','DQM')

        self.ecal_veto_name = 'EcalVeto'
        self.ecal_veto_pass = ''

        self.build1DHistogram('deepest_layer_hit',
                'Deepest Layer Hit',40,0,40)
        self.build1DHistogram('num_readout_hits',
                'Num Readout Hits',100,0,300)
        self.build1DHistogram('summed_det',
                'Total Rec Energy [MeV]',800,0.,8000.)
        self.build1DHistogram('summed_iso',
                'Total Isolated Energy [MeV]',400,0.,4000.)
        self.build1DHistogram('summed_back',
                'Total Back Energy [MeV]',400,0.,4000.)
        self.build1DHistogram('max_cell_dep',
                'Maximum Single-Cell Energy Dep [MeV]',100,0.,1000.)
        self.build1DHistogram('shower_rms',
                'Transverse Shower RMS [mm]',200,0.,200.)
        self.build1DHistogram('x_std',
                'X Std Deviation [mm]',200,0.,200.)
        self.build1DHistogram('y_std',
                'Y Std Deviation [mm]',200,0.,200.)
        self.build1DHistogram('avg_layer_hit',
                'Avg Layer Hit',40,0.,40.)
        self.build1DHistogram('std_layer_hit',
                'Std Dev Layer Hit',20,0.,20.)
        self.build1DHistogram('e_containment_energy',
                'Electron Containment Energy [MeV]',200,0.,8000.)
        self.build1DHistogram('ph_containment_energy',
                'Photon Containment Energy [MeV]',200,0.,8000.)
        self.build1DHistogram('out_containment_energy',
                'Outside Containment Energy [MeV]',200,0.,8000.)

class EcalMipTrackingFeatures(ldmxcfg.Analyzer) :
    """Configured EcalMipTrackingFeatures python object """

    def __init__(self,name="EcalMipTrackingFeatures") :
        super().__init__(name,'dqm::EcalMipTrackingFeatures','DQM')

        self.ecal_veto_name = 'EcalVeto'
        self.ecal_veto_pass = ''

        self.build1DHistogram('n_straight_tracks',
                'Num Straight Tracks',30,0,30)
        self.build1DHistogram('n_linreg_tracks',
                'Num Linear Regression Tracks',15,0,15)
        self.build1DHistogram('first_near_photon_layer',
                'First Near Photon Layer',34,0,34)
        self.build1DHistogram('ep_ang',
                'Electron Photon Angle',90,0.,90.)
        self.build1DHistogram('ep_sep',
                'Electron Photon Separation',180,0.,180.)
        self.build1DHistogram('recoil_pz',
                'Recoil electron pz',200,-200.,8000.)
        self.build1DHistogram('recoil_pt',
                'Recoil electron p_{T}',200,0,2000.)
        self.build1DHistogram('recoil_x',
                'Recoil electron x',100,-300.,300.)
        self.build1DHistogram('recoil_y',
                'Recoil electron y',100,-300.,300.)
        

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


class DarkBremInteraction(ldmxcfg.Producer) :
    def __init__(self) :
        super().__init__('db_kinematics','dqm::DarkBremInteraction','DQM')

        self.build1DHistogram('aprime_energy',
            'Dark Photon Energy [MeV]',101,0,8080)
        self.build1DHistogram('aprime_pt',
            'Dark Photon pT [MeV]',100,0,2000)

        self.build1DHistogram('recoil_energy',
            'Recoil Electron Energy [MeV]',101,0,8080)
        self.build1DHistogram('recoil_pt',
            'Recoil Electron pT [MeV]',100,0,2000)

        self.build1DHistogram('incident_energy',
            'Incident Electron Energy [MeV]',101,0,8080)
        self.build1DHistogram('incident_pt',
            'Incident Electron pT [MeV]',100,0,2000)

        # weird binning so we can see the target and trigger pads
        self.build1DHistogram('dark_brem_z',
            'Z Location of Dark Brem [mm]',
            [-5.0, -4.6752, -3.5502, -2.4252, -1.3002, -0.1752, 0.1752, 1.]) 
        # elements are hydrogen and carbon (for trigger pads) and tungsten target
        self.build1DHistogram('dark_brem_element',
            'Element in which Dark Brem Occurred',
            10, 0, 10)
        self.build1DHistogram('dark_brem_material',
            'Material in which Dark Brem Occurred',
            8, 0, 8)


class HCalRawDigi(ldmxcfg.Analyzer) :
    def __init__(self, input_name) :
        super().__init__('hcal_pedestals','dqm::HCalRawDigi','DQM')

        self.input_name = input_name
        self.input_pass = ''

class NtuplizeHgcrocDigiCollection(ldmxcfg.Analyzer) :
    def __init__(self,input_name, pedestal_table = None, input_pass = '', 
            using_eid = None, already_aligned = False,
            name = 'ntuplizehgcroc') :
        super().__init__(name,'dqm::NtuplizeHgcrocDigiCollection','DQM')
        self.input_name = input_name
        self.input_pass = input_pass

        if using_eid is None :
            # deduce if using eid based on presence of HcalDetectorMap in conditions system
            from LDMX.Framework import ldmxcfg
            from LDMX.Hcal.DetectorMap import HcalDetectorMap
            using_eid = True
            for cop in ldmxcfg.Process.lastProcess.conditionsObjectProviders :
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

    def __init__(self,name='PN', verbose=False, count_light_ions=True) :
        super().__init__(name,'dqm::PhotoNuclearDQM','DQM')

        self.count_light_ions=count_light_ions
        self.verbose = verbose
        self.build1DHistogram("event_type"         , "", 24, -1, 23)
        self.build1DHistogram("event_type_500mev"  , "", 24, -1, 23)
        self.build1DHistogram("event_type_2000mev" , "", 24, -1, 23)
        self.build1DHistogram("event_type_compact"         , "", 8, -1, 7)
        self.build1DHistogram("event_type_compact_500mev"  , "", 8, -1, 7)
        self.build1DHistogram("event_type_compact_2000mev" , "", 8, -1, 7)
        self.build1DHistogram("1n_event_type"      , "", 7,  -1, 6)
        self.build1DHistogram("pn_particle_mult"   , "Photo-nuclear Multiplicity", 200, 0, 200)
        self.build1DHistogram("pn_neutron_mult", "Photo-nuclear Neutron Multiplicity", 200,0, 200)
        self.build1DHistogram("pn_gamma_energy"    , "#gamma Energy (MeV)", 500, 0, 5000)
        self.build1DHistogram("pn_total_ke"  , "Total Kineitc Energy of Photo-Nuclear Products(MeV)", 500, 0, 5000)
        self.build1DHistogram("pn_total_neutron_ke"  , "Total Kineitc Energy of Photo-Nuclear Neutrons  (MeV)", 500, 0, 5000)
        self.build1DHistogram("1n_neutron_energy"  , "Neutron Energy (MeV)", 500, 0, 5000)
        self.build1DHistogram("1n_energy_diff"     , "E(#gamma_{PN}) - E(n) (MeV)", 500, 0, 5000)
        self.build1DHistogram("1n_energy_frac"     , "E(n)/E(#gamma_{PN}) (MeV)", 500, 0, 1)
        self.build1DHistogram("2n_n2_energy"       , "Energy of second hardest neutron (MeV)", 500, 0, 5000)
        self.build1DHistogram("2n_energy_frac"     , "E(n)/E(#gamma_{PN}) (MeV)", 500, 0, 1)
        self.build1DHistogram("2n_energy_other"    , "E_{other} (MeV)", 500, 0, 5000)
        self.build1DHistogram("1kp_energy"         , "Charged Kaon Energy (MeV)", 500, 0, 5000)
        self.build1DHistogram("1kp_energy_diff"    , "E(#gamma_{PN}) - E(K#pm) (MeV)", 500, 0, 5000)
        self.build1DHistogram("1kp_energy_frac"    , "E(K#pm)/E(#gamma_{PN}) (MeV)", 500, 0, 1)
        self.build1DHistogram("1k0_energy"         , "K0 Energy (MeV)", 500, 0, 5000)
        self.build1DHistogram("1k0_energy_diff"    , "E(#gamma_{PN}) - E(K0) (MeV)", 500, 0, 5000)
        self.build1DHistogram("1k0_energy_frac"    , "E(K0)/E(#gamma_{PN}) (MeV)", 500, 0, 1)

        self.build1DHistogram("recoil_vertex_x",   "Recoil e^{-} Vertex - x (mm)", 40, -40, 40)
        self.build1DHistogram("recoil_vertex_y",   "Recoil e^{-} Vertex - y (mm)", 80, -80, 80)
        self.build1DHistogram("recoil_vertex_z",   "Recoil e^{-} Vertex - z (mm)", 20, -750, -650)

        self.build1DHistogram("pn_gamma_int_z",    "#gamma Interaction Vertex (mm)", 50, 200, 400)
        self.build1DHistogram("pn_gamma_vertex_z", "#gamma Vertex (mm)", 1000, -5,  5)
        self.build1DHistogram("pn_gamma_vertex_x", "#gamma Vertex (mm)", 80,   -40, 40)
        self.build1DHistogram("pn_gamma_vertex_y", "#gamma Vertex (mm)", 160,  -80, 80)

        self.build1DHistogram("hardest_ke",       "Kinetic Energy Hardest Photo-nuclear Particle (MeV)", 400, 0, 4000)
        self.build1DHistogram("hardest_theta",    "#theta of Hardest Photo-nuclear Particle (Degrees)", 360, 0, 180)
        self.build1DHistogram("hardest_p_ke",     "Kinetic Energy Hardest Photo-nuclear Proton (MeV)", 400, 0, 4000)
        self.build1DHistogram("hardest_p_theta",  "#theta of Hardest Photo-nuclear Proton (Degrees)", 360, 0, 180)
        self.build1DHistogram("hardest_n_ke",     "Kinetic Energy Hardest Photo-nuclear Neutron (MeV)", 400, 0, 4000)
        self.build1DHistogram("hardest_n_theta",  "#theta of Hardest Photo-nuclear Neutron (Degrees)", 360, 0, 180)
        self.build1DHistogram("hardest_pi_ke",    "Kinetic Energy Hardest Photo-nuclear #pi (MeV)", 400, 0, 4000)
        self.build1DHistogram("hardest_pi_theta", "#theta of Hardest Photo-nuclear #pi (Degrees)", 360, 0, 180)

        self.build2DHistogram("h_ke_h_theta", 
                            "Kinetic Energy Hardest Photo-nuclear Particle (MeV)",
                            400, 0, 4000, 
                            "#theta of Hardest Photo-nuclear Particle (Degrees)",
                            360, 0, 180)
        
        self.build2DHistogram("1n_ke:2nd_h_ke", 
                            "Kinetic Energy of Leading Neutron (MeV)",
                            400, 0, 4000, 
                            "Kinetic Energy of 2nd Hardest Particle",
                            400, 0, 4000)
        
        self.build2DHistogram("1kp_ke:2nd_h_ke", 
                            "Kinetic Energy of Leading Charged Kaon (MeV)",
                            400, 0, 4000, 
                            "Kinetic Energy of 2nd Hardest Particle",
                            400, 0, 4000)
        
        self.build2DHistogram("1k0_ke:2nd_h_ke", 
                            "Kinetic Energy of Leading K0 (MeV)",
                            400, 0, 4000, 
                            "Kinetic Energy of 2nd Hardest Particle",
                            400, 0, 4000)
        
        self.build2DHistogram("recoil_vertex_x:recoil_vertex_y", 
                           "Recoil electron vertex x (mm)", 
                           160, -40, 40, 
                           "Recoil electron vertex y (mm)", 
                           320, -80, 80)

class RecoilTrackerDQM(ldmxcfg.Analyzer) :
    """Configured RecoilTrackerDQM python object
    
    Contains an instance of RecoilTrackerDQM that
    has already been configured.
    
    Builds the necessary histograms as well.
    
    Examples
    --------
        from LDMX.DQM import dqm
        p.sequence.append( dqm.RecoilTrackerDQM() )
    """

    def __init__(self,name='RecoilTracker') :
        super().__init__(name, "dqm::RecoilTrackerDQM",'DQM')
        
        self.build1DHistogram("track_count", "Track Multiplicity", 10, 0, 10)
        self.build1DHistogram("loose_track_count", "Track Multiplicity", 10, 0, 10)
        self.build1DHistogram("axial_track_count", "Track Multiplicity", 10, 0, 10)
        
        self.build1DHistogram("recoil_vx", "Recoil e^{-} Vertex x (mm)", 120, -30, 30) 
        self.build1DHistogram("recoil_vy", "Recoil e^{-} Vertex y (mm)", 200, -100, 100) 
        self.build1DHistogram("recoil_vz", "Recoil e^{-} Vertex z (mm)", 40, -2, 0)
        
        titles = ['', '_track_veto', '_bdt', '_hcal', '_track_bdt', '_vetoes']
        for t in titles: 
            self.build1DHistogram("tp%s" % t,  "Recoil e^{-} Truth p (MeV)", 255, -50, 2500)
            self.build1DHistogram("tpt%s" % t, "Recoil e^{-} Truth p_{t} (MeV)", 300, -50, 100)
            self.build1DHistogram("tpx%s" % t, "Recoil e^{-} Truth p_{x} (MeV)", 100, -10, 10)
            self.build1DHistogram("tpy%s" % t, "Recoil e^{-} Truth p_{y} (MeV)", 100, -10, 10)
            self.build1DHistogram("tpz%s" % t, "Recoil e^{-} Truth p_{z} (MeV)", 260, -100, 2500)

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
        self.passName = ''

        
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
        self.passName = ''


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

        # primary histograms
        self.build1DHistogram("pdgid_primaries", "ID of primary particles", 20, 0, 20)
        self.build1DHistogram("energy_primaries", "Energy of primary particles [MeV]", 90, 0, 9000) # range applicable for 4 GeV beam
        self.build2DHistogram("beam_smear", "x [mm]", 30, -150, 150, "y [mm]", 30, -150, 150)
        self.build1DHistogram("pdgid_primarydaughters", "ID of primary daughters", 20, 0, 20)
        self.build1DHistogram("energy_daughterphoton", "Energy spectrum of all photons from primary [MeV]", 170, 0, 8500)

        # primary daughter of interest (brem / dark brem) histograms
        self.build1DHistogram("pdgid_harddaughters", "ID of primary daughters", 20, 0, 20)
        self.build1DHistogram("startZ_hardbrem", "Start z position of hard primary daughter [mm]", 100, -500, 500)
        self.build1DHistogram("endZ_hardbrem", "End z position of hard primary daughter [mm]", 100, -500, 500)
        self.build1DHistogram("energy_hardbrem", "Energy spectrum of hard primary daughter [MeV]", 130, 2000, 8500)

        # daughters of hard brem histograms
        self.build1DHistogram("pdgid_hardbremdaughters", "ID of hard brem daughters", 20, 0, 20)
        self.build1DHistogram("startZ_hardbremdaughters", "Start z position of hard brem daughters  [mm]", 200, -1000, 1000)
        

ecal_dqm = [
        EcalDigiVerify(),
        EcalShowerFeatures(),
        EcalMipTrackingFeatures()
        ]

hcal_dqm = [
        HCalDQM(pe_threshold=5,
                section=0
                ),
        HCalDQM(pe_threshold=5,
                section=1
                ),
        HCalDQM(pe_threshold=5,
                section=2
                ),
        HCalDQM(pe_threshold=5,
                section=3
                ),
        HCalDQM(pe_threshold=5,
                section=4
                ),
        HcalInefficiencyAnalyzer(),
  ]

recoil_dqm = [
        RecoilTrackerDQM()
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


all_dqm = ecal_dqm + hcal_dqm + recoil_dqm + trigScint_dqm + trigger_dqm
