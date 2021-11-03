from LDMX.Framework.ldmxcfg import Producer

class SeedFinderProcessor(Producer) :
    def __init__(self, instance_name = "SeedFinderProcessor"):
        super().__init__(instance_name, 'tracking::sim::SeedFinderProcessor','Tracking')
