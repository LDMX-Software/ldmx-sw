from LDMX.Framework.ldmxcfg import Producer
from LDMX.Tracking.make_path import makeFieldMapPath


class AlignmentTestProcessor(Producer):
    """ Producer to test Alignment constants loading and propagation to devices.

    Parameters
    ----------

    instance_name : str
        Unique name for this instance.

    Attributes
    ----------

    """

    def __init__(self, instance_name="AlignmentTestProcessor"):
        super().__init__(instance_name,
                         'tracking::reco::AlignmentTestProcessor', 'Tracking')
        

