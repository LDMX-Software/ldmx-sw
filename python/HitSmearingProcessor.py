from LDMX.Framework.ldmxcfg import Producer

class HitSmearingProcessor(Producer) :
    """Producer that smears simulated tracker hits.

    Parameters
    ----------
    instance_name : str
        Unique name for this instance.
    """

    def __init__(self, instance_name = "HitSmearingProcessor"):
        super().__init__(instance_name, 'tracking::sim::HitSmearingProcessor','Tracking')

        # Name of the input sim hit collection. 
        self.input_hit_coll = ''

        # Name of the output sim hit collection.
        self.output_hit_coll = ''

        # The deviation in the u direction. 
        self.sigma_u = 0.05 # 50 um

        # The deviation in the v direction.
        self.sigma_v = 0.25 # 250 um
