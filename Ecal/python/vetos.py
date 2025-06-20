"""Configuration for Ecal veto

Examples
--------
    from LDMX.Ecal.ecalVeto import ecalVeto
    p.sequence.append( ecalVeto )
"""

from LDMX.Framework import ldmxcfg

class EcalVetoProcessor(ldmxcfg.Producer) :
    """Configuration for the ECal veto"""

    def __init__(self,name = 'ecalVeto') :
        super().__init__(name,"ecal::EcalVetoProcessor",'Ecal')

        from LDMX.Ecal.makePath import makeBDTPath, makeRoCPath
        self.num_ecal_layers = 34
        self.verbose = False
        self.feature_list_name = "input"
        self.bdt_file = makeBDTPath( "segmip" )
        self.roc_file = makeRoCPath( "RoC_v14_8gev" )
        self.beam_energy = 8000.0  # in MeV
        self.disc_cut = 0.99741
        
        self.sp_pass_name = ""
        self.collection_name = "EcalVeto"
        self.rec_pass_name = ""
        self.rec_coll_name = "EcalRecHits"
        self.recoil_from_tracking = True
        self.track_collection = "RecoilTracksClean"
        self.inverse_skim = False

        self.sp_pass_name = ""
        self.sim_particles_passname = ""
        self.track_pass_name = ""
        
        self.ecal_simhits_passname = ""
        self.ecal_digis_passname = ""
        self.ecal_rechits_passname = ""
        self.ecal_trig_digis_passname = ""

class EcalMipProcessor(ldmxcfg.Producer) :
    """Configuration for the ECal MIP processor"""

    def __init__(self,name = 'ecalMipTracking') :
        super().__init__(name,"ecal::EcalMipTrackingProcessor",'Ecal')

        self.num_ecal_layers = 34
        self.linreg_radius = 35.0 # in mm
        self.mip_collection_name = "EcalMipCollection"
        self.mip_pass_name = ""

class DNNEcalVetoProcessor(ldmxcfg.Producer) :
    """Configuration for DNN Ecal Veto

    By default, sets the disct_cut to negative 1 so
    the user is forced to decide on the cut.
    """

    def __init__(self,name = 'dnnEcalVeto') :
        super().__init__(name,"ecal::DNNEcalVetoProcessor",'Ecal')

        self.debug = False
        from LDMX.Ecal.makePath import makeBDTPath
        self.model_path = makeBDTPath("particle-net_ecal_v9")
        self.disc_cut = -1.
        self.collection_name = "EcalVetoDNN"
        self.ecal_rec_hits_passname = ""
        




