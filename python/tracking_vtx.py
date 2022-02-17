from LDMX.Framework.ldmxcfg import Producer

class VertexProcessor(Producer) :

    def __init__(self, instance_name = "VertexProcessor"):
        super().__init__(instance_name, 'tracking::reco::VertexProcessor','Tracking')
        
