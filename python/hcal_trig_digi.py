"""Package to configure the Hcal trigger digitization
All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.
"""

from LDMX.Framework.ldmxcfg import Producer

class HcalTrigPrimDigiProducer(Producer) :
    """Configuration for HcalTrigPrimDigiProducer
    """

    def __init__(self, instance_name = 'hcalTrigDigis') :
        super().__init__(instance_name , 'hcal::HcalTrigPrimDigiProducer','Hcal')
        self.digiCollName = "HcalDigis"
        self.digiPassName = ""
