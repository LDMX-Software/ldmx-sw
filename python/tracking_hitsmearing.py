from LDMX.Framework.ldmxcfg import Producer

class HitSmearingProcessor(Producer) :
    def __init__(self, instance_name = "HitSmearingProcessor"):
        super().__init__(instance_name, 'tracking::sim::HitSmearingProcessor','Tracking')

        self.input_hit_coll  = [""]
        self.output_hit_coll = [""] 

        self.taggerSigma_u = 0.05 #50um
        self.taggerSigma_v = 0.25 #250um
        
        self.recoilSigma_u = 0.05 #50um
        self.recoilSigma_v = 0.25 #250um

        self.fullRandom = False
