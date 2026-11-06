"""Conditions object providers for a mapping between ecal
electronics IDs and detector IDs
"""

from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider


@conditions_object_provider("EcalDetectorMap", "ecal::EcalDetectorMapLoader", "Ecal")
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

    @staticmethod
    def esa26():
        """ESA 2026 cosmic stand, ECal layers 2 and 3

        Electronics IDs as written by ECONDDecoder with fiber 2 (L2, 6 ROCs)
        and fiber 3 (L3, ROC0, 2, 3). Channel to cell from cosmic muons
        (L2 x L3 pairs, HCal tracks), see data/esa26/README.md.
        """
        d = "@CMAKE_INSTALL_PREFIX@/data/Ecal/esa26"
        return EcalDetectorMap(
            cell_map=f"{d}/cell_map.csv",
            motherboard_map=f"{d}/motherboard_map.csv",
            layer_map=f"{d}/layer_map.csv",
        )
