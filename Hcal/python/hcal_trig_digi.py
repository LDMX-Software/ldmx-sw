"""Package to configure the Hcal trigger digitization
All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.
"""

from LDMX.Framework import Processor, processor


@processor("hcal::HcalTrigPrimDigiProducer", "Hcal")
class HcalTrigPrimDigiProducer(Processor):
    digi_coll_name: str = "HcalDigis"
    digi_pass_name: str = ""
