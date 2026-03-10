"""Conditions object providers for a mapping between ecal electronics IDs and detector IDs"""

from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider


@conditions_object_provider("EcalDetectorMap", "ecal::EcalDetectorMap", "Ecal")
class EcalDetectorMap(ConditionsObjectProvider):
    """The COP that maps between Electronic and Detector IDs.

    The mapping is defined in three 'tiers'.

    1. cell - Each cell of a module has specific ROC elink and channel
    2. motherboard - Each module has a specific ROC elink and polarfire
    3. layer - Each layer has specific daq optical link

    So three CSV tables are necessary for providing this mapping.
    We can't have multiple different detector maps during a single
    run, so this class is meant to be a singleton.
    """

    cell_map: str
    motherboard_map: str
    layer_map: str
    want_d2e: bool = False
