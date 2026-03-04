
"""Package to configure the ECal trigger digitization


All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

"""

from LDMX.Framework.ldmxcfg import Producer


class EcalTrigPrimDigiProducer(Producer) :
    """Configuration for EcalTrigPrimDigiProducer

    """

    def __init__(self, instance_name = 'ecalTrigDigis') :
        super().__init__(instance_name , 'ecal::EcalTrigPrimDigiProducer','Ecal')
        self.digi_coll_name = "EcalDigis"
        self.digi_pass_name = ""


