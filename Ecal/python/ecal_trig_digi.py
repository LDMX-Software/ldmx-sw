"""Package to configure the ECal trigger digitization"""

from LDMX.Framework import Processor, processor


@processor("ecal::EcalTrigPrimDigiProducer", "Ecal")
class EcalTrigPrimDigiProducer(Processor):
    digi_coll_name: str = "EcalDigis"
    digi_pass_name: str = ""

    def __post_init__(self):
        self.instance_name = "ecalTrigDigis"
