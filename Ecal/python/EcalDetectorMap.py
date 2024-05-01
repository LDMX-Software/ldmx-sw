"""Conditions object providers for a mapping between ecal electronics IDs and detector IDs"""

from LDMX.Framework import ldmxcfg

class EcalDetectorMap(ldmxcfg.ConditionsObjectProvider) :
    """The COP that maps between Electronic and Detector IDs.

    The mapping is defined in three 'tiers'. 

    1. cell - Each cell of a module has specific ROC elink and channel
    2. motherboard - Each module has a specific ROC elink and polarfire
    3. layer - Each layer has specific daq optical link

    So three CSV tables are necessary for providing this mapping.
    We can't have multiple different detector maps during a single
    run, so this class is meant to be a singleton.

    Attributes
    ----------
    __instance : EcalDetectorMap
        Singleton instance of this object
    """

    __instance = None

    def get() :
        """Get the single instance of the EcalDetectorMap

        Returns
        -------
        EcalDetectorMap
            Single instance of the provider
        """
        return EcalGeometryProvider.__instance

    def __init__(self, cell_map, motherboard_map, layer_map, want_d2e = False) :
        if EcalDetectorMap.__instance != None :
            raise Exception('EcalDetectorMap is a singleton class and should only be created once. You can retrieve the single instance with EcalDetectorMap.get()')
        else:
            # the name needs to match the conditions object name in EcalDetectorMap.h
            super().__init__("EcalDetectorMap","ecal::EcalDetectorMapLoader","Ecal")
            self.cell_map = cell_map
            self.motherboard_map = motherboard_map
            self.layer_map = layer_map
            self.want_d2e = want_d2e

