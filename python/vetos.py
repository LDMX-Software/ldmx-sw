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

        from LDMX.Ecal.makePath import makeBDTPath, makeCellXYPath
        self.num_ecal_layers = 34
        self.do_bdt = True
        self.bdt_file = makeBDTPath( "gabrielle" )
        self.cellxy_file = makeCellXYPath()
        self.disc_cut = 0.99
        self.collection_name = "EcalVeto"
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




