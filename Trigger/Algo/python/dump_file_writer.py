
from LDMX.Framework import Processor, processor


@processor("trigger::DumpFileWriter", "Trigger")
class DumpFileWriter(Processor):
    """Configuration for DumpFileWriter"""

    ecal_trig_digis_passname: str = ""
    ecal_trig_digis_event_passname: str = ""
