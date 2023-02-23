
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
