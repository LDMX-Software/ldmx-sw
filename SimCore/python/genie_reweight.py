from LDMX.Framework import ldmxcfg


class GenieReweightProducer(ldmxcfg.Producer):
    def __init__(
        self,
        name="genieEventWeights",
        n_weights=100,
        seed=10,
        var_types=None,
        message_threshold_file="/usr/local/GENIE/Generator/config/Messenger.xml",
        event_weights_coll_name="genieEventWeights",
    ):
        if var_types is None:
            var_types = ["GENIE_GENERIC"]
        super().__init__(name, "simcore::GenieReweightProducer", "SimCore::Reweight")

        self.seed = seed
        self.n_weights = n_weights
        self.var_types = var_types
        self.message_threshold_file = message_threshold_file

        self.event_weights_coll_name = event_weights_coll_name
