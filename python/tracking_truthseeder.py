from LDMX.Framework.ldmxcfg import Producer

class TruthSeedProcessor(Producer) :

    def __init__(self, instance_name = "TruthSeedProcessor"):
        super().__init__(instance_name, 'tracking::reco::TruthSeedProcessor','Tracking')
