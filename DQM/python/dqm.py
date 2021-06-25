"""Configuration for DQM analyzers"""

from LDMX.Framework import ldmxcfg

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

    def __init__(self,name="hcal_dqm") :
        super().__init__(name,'dqm::HCalDQM','DQM')

        self.rec_coll_name = 'HcalRecHits'
        self.rec_pass_name = ''
        
        # every hit in hcal
        self.build1DHistogram("pe", "Photoelectrons in an HCal Module", 1500, 0, 1500)
        self.build1DHistogram("hit_time", "HCal hit time (ns)", 1600, -100, 1500)
        self.build2DHistogram("back_pe:layer",
                "Photoelectrons in a Back HCal Layer",10,0,10,
                "Back HCal Layer",100,0,100)
        self.build2DHistogram("back_layer:strip",
                "Back HCal Layer",100,0,100,
                "Back HCal Strip",62,0,62)
        self.build2DHistogram("side_pe:layer",
                "Photoelectrons in a Side HCal Layer",10,0,10,
                "Side HCal Layer",20,0,20)
        self.build2DHistogram("side_layer:strip",
                "Side HCal Layer",20,0,20,
                "Side HCal Strip",30,0,30)
        
        # once per event
        self.build1DHistogram("n_hits", "HCal hit multiplicity", 300, 0, 300)
        self.build1DHistogram("total_pe", "Total Photoelectrons", 3000, 0, 3000)
        self.build1DHistogram("back_total_pe", "Total Photoelectrons in Back", 3000, 0, 3000)
        self.build1DHistogram("max_pe", 
                "Max Photoelectrons in an HCal Module", 1500, 0, 1500)
        self.build1DHistogram("hit_time_max_pe", 
                "Max PE hit time (ns)", 1600, -100, 1500)
        self.build2DHistogram("max_pe:time", 
                "Max Photoelectrons in an HCal Module", 1500, 0, 1500, 
                "HCal max PE hit time (ns)", 1500, 0, 1500)
        self.build1DHistogram("min_time_hit_above_thresh", 
                "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500)
        self.build2DHistogram("min_time_hit_above_thresh:pe", 
                           "Photoelectrons in an HCal Module", 1500, 0, 1500, 
                           "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500)
         
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

    def __init__(self,name='PN') :
        super().__init__(name,'dqm::PhotoNuclearDQM','DQM')

        self.build1DHistogram("event_type"         , "", 24, -1, 23)
        self.build1DHistogram("event_type_500mev"  , "", 24, -1, 23)
        self.build1DHistogram("event_type_2000mev" , "", 24, -1, 23)
        self.build1DHistogram("event_type_compact"         , "", 8, -1, 7)
        self.build1DHistogram("event_type_compact_500mev"  , "", 8, -1, 7)
        self.build1DHistogram("event_type_compact_2000mev" , "", 8, -1, 7)
        self.build1DHistogram("1n_event_type"      , "", 7,  -1, 6)
        self.build1DHistogram("pn_particle_mult"   , "Photo-nuclear Multiplicity", 100, 0, 200)
        self.build1DHistogram("pn_gamma_energy"    , "#gamma Energy (MeV)", 500, 0, 5000)
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

        

ecal_dqm = [
        EcalDigiVerify(),
        EcalShowerFeatures()
        ]

hcal_dqm = [
        HCalDQM()
        ]

recoil_dqm = [
        RecoilTrackerDQM()
        ]

trigScint_dqm = [
    TrigScintSimDQM('TrigScintSimTag','TriggerPadTaggerSimHits','tag'),
    TrigScintSimDQM('TrigScintSimUp','TriggerPadUpSimHits','up'),
    TrigScintSimDQM('TrigScintSimDn','TriggerPadDownSimHits','dn'),
    TrigScintDigiDQM('TrigScintDigiTag','trigScintDigisTag','tag'),
    TrigScintDigiDQM('TrigScintDigiUp','trigScintDigisUp','up'),
    TrigScintDigiDQM('TrigScintDigiDn','trigScintDigisDn','dn'),
    TrigScintClusterDQM('TrigScintClusterTag','TriggerPadTaggerClusters','tag'),
    TrigScintClusterDQM('TrigScintClusterUp','TriggerPadUpClusters','up'),
    TrigScintClusterDQM('TrigScintClusterDn','TriggerPadDownClusters','dn'),
    TrigScintTrackDQM('TrigScintTracks','TriggerPadTracks')
    ]


trigger_dqm = [
        Trigger()
        ]


all_dqm = ecal_dqm + hcal_dqm + recoil_dqm + trigScint_dqm + trigger_dqm
