"""Configuration for Ecal veto

Examples
--------
    from LDMX.Ecal.ecal_veto import ecal_veto
    p.sequence.append( ecal_veto )
"""

from LDMX.Framework import ldmxcfg


class EcalWABRecProcessor(ldmxcfg.Producer) :
    """Configuration for the ECal veto"""

    def __init__(self,name = 'EcalWABRec') :
        super().__init__(name,"ecal::EcalWABRecProcessor",'Ecal')

        self.sp_pass_name = ''
        self.collection_name = "EcalWABRec"
        self.rec_coll_name = 'EcalRecHits'
        self.rec_pass_name = ''
        self.track_coll_name = 'LinearRecoilTracks'
        self.track_pass_name = ''

        self.sim_particles_passname = ''
