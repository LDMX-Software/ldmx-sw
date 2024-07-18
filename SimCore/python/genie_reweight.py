from LDMX.Framework import ldmxcfg

class GenieReweightProducer(ldmxcfg.Producer) :

    def __init__(self,name='genieEventWeights'):
        super().__init__(name,"simcore::GenieReweightProducer","SimCore::Reweight")

        self.seed = 10
        self.n_weights = 100
        self.var_types = ["GENIE_GENERIC"]
        self.tune = "G18_02a_02_11b"

        self.eventWeightsCollName = "genieEventWeights"