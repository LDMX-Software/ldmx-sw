"""Configuration for processing pn weights"""

from LDMX.Framework import ldmxcfg

class PnWeightProcessor(ldmxcfg.Producer) :
    """Process the PN Weights into something more helpful

    Attributes
    ----------
    w_threshold : float
        Threshold above which to apply reweighting [MeV]
    theta_threshold : float
        Minimum angle for backwards going hadrons [degrees]
    """

    def __init__(self,name='pnWeight') :
        super().__init__(name , 'ldmx::PnWeightProcessor')

        from LDMX.EventProc import include
        include.library()

        self.w_threshold     = 1150.
        self.theta_threshold = 100.
