"""Configuration for Ecal veto

Examples
--------
    from LDMX.Ecal.ecal_veto import ecal_veto
    p.sequence.append( ecal_veto )
"""

from LDMX.Framework import ldmxcfg


class EcalVetoProcessor(ldmxcfg.Producer) :
    """Configuration for the ECal veto"""

    def __init__(self,name = 'ecal_veto') :
        super().__init__(name,"ecal::EcalVetoProcessor",'Ecal')

        from LDMX.Ecal.make_path import makeBDTPath, makeRoCPath
        self.num_ecal_layers = 32
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

        self.num_ecal_layers = 32
        self.linreg_radius = 35.0 # in mm
        self.ecal_collection_name = "EcalVeto"
        self.ecal_pass_name = ""
        self.mip_collection_name = "EcalTrajectoryInfo"
        self.mip_pass_name = ""
        self.mip_result_name = "EcalMipInfo"

class EcalPnetVetoProcessor(ldmxcfg.Producer) :
    """Configuration for ParticleNet Ecal Veto
        ParticleNet trained on v14 geometry ecalPN + signal
    """

    def __init__(self,name = 'EcalPnetVeto') :
        super().__init__(name,"ecal::EcalPnetVetoProcessor",'Ecal')

        from LDMX.Ecal.make_path import makeBDTPath
        self.model_path = makeBDTPath("particle_net_ecal_v10")
        self.disc_cut = 0.65
        self.collection_name = "EcalPnetVeto"
        self.rec_coll_name = "EcalRecHits"
        self.ecal_rec_hits_passname = ""
        self.ecal_sp_hits_passname = ""
        self.track_collection = "RecoilTracksClean"
        self.track_pass_name = ""
        self.recoil_from_tracking = True





