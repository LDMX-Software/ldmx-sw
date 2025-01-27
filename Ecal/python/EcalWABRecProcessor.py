"""Configuration for Ecal veto

Examples
--------
    from LDMX.Ecal.ecalVeto import ecalVeto
    p.sequence.append( ecalVeto )
"""

from LDMX.Framework import ldmxcfg

class EcalWABRecProcessor(ldmxcfg.Producer) :
    """Configuration for the ECal veto"""

    def __init__(self,name = 'EcalWABRec') :
        super().__init__(name,"ecal::EcalWABRecProcessor",'Ecal')

        self.collection_name = "EcalWABRec"
        self.rec_coll_name = 'EcalRecHits'
        self.rec_pass_name = ''
        self.data = 0
        self.WAB = 0
