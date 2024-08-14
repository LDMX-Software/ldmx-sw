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

        from LDMX.Ecal.makePath import makeBDTPath, makeCellXYPath, makeRoCPath
        self.num_ecal_layers = 34
        self.do_bdt = True
        self.feature_list_name = "input"
        self.bdt_file = makeBDTPath( "segmip" )
        self.roc_file = makeRoCPath( 'RoC_v14_8gev' )
        self.beam_energy = 8000.0  # in MeV
        self.cellxy_file = makeCellXYPath()
        self.disc_cut = 0.99741
        self.collection_name = "EcalVeto"
        self.rec_coll_name = 'EcalRecHits'
        self.rec_pass_name = ''


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




