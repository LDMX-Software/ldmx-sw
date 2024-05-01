"""ConditionsProvider for EcalGeometry and other Ecal geometry-related aspects"""
from LDMX.Framework import ldmxcfg

class EcalGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Ecal geometry (ecal::EcalGeometry)

    Parameters
    ----------
    tagName : str
        tag for generator of information

    Attributes
    ----------
    EcalGeometry : EcalGeometry
        Actual class providing precision cellular layout in Ecal
    __instance : EcalGeometryProvider
        Singleton instance of this object
    """

    __instance = None

    def getInstance() :
        """Get the single instance of the EcalGeometryProvider

        Returns
        -------
        EcalGeometryProvider
            Single instance of the provider
        """

        if EcalGeometryProvider.__instance == None :
            EcalGeometryProvider()

        return EcalGeometryProvider.__instance

    def __init__(self):
        if EcalGeometryProvider.__instance != None :
            raise Exception('EcalGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("EcalGeometryProvider","ecal::EcalGeometryProvider","Ecal")
            from LDMX.DetDescr.EcalGeometry import EcalGeometry
            self.geometries = EcalGeometry.geometries()
            EcalGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
EcalGeometryProvider.getInstance()

class EcalTriggerGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Ecal geometry (ldmx::EcalGeometry)

    Parameters
    ----------
    tagName : str
        tag for generator of information

    Attributes
    ----------
    __instance : EcalTriggerGeometryProvider
        Singleton instance of this object
    """

    __instance = None

    def getInstance() :
        """Get the single instance of the EcalTriggerGeometryProvider

        Returns
        -------
        EcalTriggerGeometryProvider
            Single instance of the provider
        """

        if EcalTriggerGeometryProvider.__instance == None :
            EcalTriggerGeometryProvider()

        return EcalTriggerGeometryProvider.__instance

    def __init__(self):
        if EcalTriggerGeometryProvider.__instance != None :
            raise Exception('EcalTriggerGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("EcalTriggerGeometry","ecal::EcalTriggerGeometryProvider","Ecal")
            EcalTriggerGeometryProvider.__instance = self 

# make sure global instance is created, this registers the condition
EcalTriggerGeometryProvider.getInstance()
