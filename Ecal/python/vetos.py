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
        self.ecal_collection_name = "EcalVeto"
        self.ecal_pass_name = ""
        self.mip_collection_name = "EcalTrajectoryInfo"
        self.mip_pass_name = ""
        self.mip_result_name = "EcalMipInfo"

class DNNEcalVetoProcessor(ldmxcfg.Producer) :
    """Configuration for GNN Ecal Veto
        ParticleNet trained on v14 geometry ecalPN + signal
    """

    def __init__(self,name = 'dnnEcalVeto') :
        super().__init__(name,"ecal::DNNEcalVetoProcessor",'Ecal')

        self.max_num_hits = 300
        from LDMX.Ecal.makePath import makeBDTPath
        self.model_path = makeBDTPath("particle_net_ecal_v10")
        self.disc_cut = 0.74
        self.collection_name = "EcalVetoDNN"
        self.ecal_rec_hits_passname = ""
        




