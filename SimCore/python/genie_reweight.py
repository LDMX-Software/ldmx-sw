from LDMX.Framework import ldmxcfg

class GenieReweightProducer(ldmxcfg.Producer) :

    def __init__(self,name='genieEventWeights'):
        super().__init__(name,"simcore::GenieReweightProducer","SimCore::Reweight")

        self.verbosity = 0
        self.seed = 10
        self.n_weights = 100
        self.var_types = ["GENIE_GENERIC"]

        self.eventWeightsCollName = "genieEventWeights"