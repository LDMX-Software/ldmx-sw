
"""Configuration classes for default photonuclear models"""

from LDMX.SimCore import simcfg


class BertiniModel(simcfg.PhotonuclearModel):
    """The default model for photonuclear interactions.

    Keeps the default Bertini model from Geant4.
    """

    def __init__(self):
        super().__init__('BertiniModel',
                         'simcore::BertiniModel',
                         'SimCore_PhotonuclearModels')
class BertiniSingleNeutronModel(simcfg.PhotonuclearModel):
    """ A photonuclear model producing only topologies with no particles above a
    certain threshold.

    Uses the default Bertini model from Geant4.

    """

    def __init__(self):
        super().__init__('BertiniSingleNeutronModel',
                         'simcore::BertiniSingleNeutronModel',
                         'SimCore_PhotonuclearModels')
        self.hard_particle_threshold = 200.
        self.zmin = 0
        self.emin = 2500.
