"""ConditionsProvider for HcalGeometry"""
from LDMX.Framework import ldmxcfg

class HcalGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Hcal geometry (HcalGeometry)

    Parameters
    ----------
    tagName : str
        tag for generator of information

    Attributes
    ----------
    HcalGeometry : HcalGeometry
        Actual class providing Hcal layout
    __instance : HcalGeometryProvider
        Singleton instance of this object
    """

    __instance = None

    def getInstance() :
        """Get the single instance of the HcalGeometryProvider

        Returns
        -------
        HcalGeometryProvider
            Single instance of the provider
        """

        if HcalGeometryProvider.__instance == None :
            HcalGeometryProvider()

        return HcalGeometryProvider.__instance

    def __init__(self):
        if HcalGeometryProvider.__instance != None :
            raise Exception('HcalGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("HcalGeometryProvider","hcal::HcalGeometryProvider","Hcal")
            from LDMX.DetDescr import HcalGeometry
            self.HcalGeometry = HcalGeometry.HcalGeometry()
            HcalGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
HcalGeometryProvider.getInstance()


class HcalQuadGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Hcal geometry (ldmx::HcalGeometry)
    Parameters
    ----------
    tagName : str
        tag for generator of information
    Attributes
    ----------
    HcalGeometry : HcalGeometry
        Actual class providing precision cellular layout in Hcal
    __instance : HcalQuadGeometryProvider
        Singleton instance of this object
    """

    __instance = None

    def getInstance() :
        """Get the single instance of the HcalQuadGeometryProvider
        Returns
        -------
        HcalQuadGeometryProvider
            Single instance of the provider
        """

        if HcalQuadGeometryProvider.__instance == None :
            HcalQuadGeometryProvider()

        return HcalQuadGeometryProvider.__instance

    def __init__(self):
        if HcalQuadGeometryProvider.__instance != None :
            raise Exception('HcalQuadGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("HcalQuadGeometry","hcal::HcalQuadGeometryProvider","Hcal")
            from LDMX.DetDescr import HcalGeometry
            self.HcalGeometry = HcalGeometry.HcalGeometry()
            HcalQuadGeometryProvider.__instance = self 

# make sure global instance is created, this registers the condition
HcalQuadGeometryProvider.getInstance()
