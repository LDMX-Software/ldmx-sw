"""Package to configure the Hcal trigger digitization
All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.
"""

from LDMX.Framework.ldmxcfg import Producer

class HcalTrigPrimDigiProducer(Producer) :
    """Configuration for HcalTrigPrimDigiProducer
    """

    def __init__(self, instance_name = 'hcalTrigPrimDigis') :
        super().__init__(instance_name , 'hcal::HcalTrigPrimDigiProducer','Hcal')
        self.digiCollName = "HcalDigis"
        self.digiPassName = ""
        
# class HcalTrigSuperQuadProducer(Producer) :
#     """Configuration for HcalTrigSuperQuadProducer
#     """

#     def __init__(self, instance_name = 'hcalTrigSuperQuads') :
#         super().__init__(instance_name , 'hcal::HcalTrigSuperQuadProducer','Hcal')
#         self.quadCollName = "hcalTrigQuadDigis"
#         self.quadPassName = ""
