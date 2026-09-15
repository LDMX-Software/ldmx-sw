from LDMX.Framework import Processor, processor


@processor("tracking::reco::AlignmentTestProcessor", "Tracking")
class AlignmentTestProcessor(Processor):
    """Producer to test Alignment constants loading and propagation to devices."""
