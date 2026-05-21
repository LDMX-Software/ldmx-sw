from LDMX.Framework import Processor, processor


@processor("simcore::GenieReweightProducer", "SimCore::Reweight")
class GenieReweightProducer(Processor):
    seed: int = 10
    n_weights: int = 100
    var_types: list[str] = ["GENIE_GENERIC"]
    message_threshold_file: str = "/usr/local/GENIE/Generator/config/Messenger.xml"
    event_weights_coll_name: str = "genieEventWeights"
    hepmc3_coll_name: str = ""
    hepmc3_pass_name: str = ""
