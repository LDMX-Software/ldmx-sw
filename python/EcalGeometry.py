"""ConditionsProvider for EcalHexReadout and other Ecal geometry-related aspects"""
from LDMX.Framework import ldmxcfg

class EcalGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Ecal geometry (EcalHexReadout)

    Parameters
    ----------
    tagName : str
        tag for generator of information

    Attributes
    ----------
    EcalHexReadout : EcalHexReadout
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
            EcalGeometryProvider('tag')

        return EcalGeometryProvider.__instance

    def __init__(self,tagName):
        if EcalGeometryProvider.__instance != None :
            raise Exception('EcalGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("EcalHexReadout","ldmx::EcalGeometryProvider",tagName,"Ecal")
            from LDMX.DetDescr import EcalHexReadout
            self.EcalHexReadout = EcalHexReadout.EcalHexReadout()
            EcalGeometryProvider.__instance = self

class EcalTriggerGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Ecal geometry (EcalHexReadout)

    Parameters
    ----------
    tagName : str
        tag for generator of information

    Attributes
    ----------
    EcalHexReadout : EcalHexReadout
        Actual class providing precision cellular layout in Ecal
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
            EcalTriggerGeometryProvider('tag')

        return EcalTriggerGeometryProvider.__instance

    def __init__(self,tagName):
        if EcalTriggerGeometryProvider.__instance != None :
            raise Exception('EcalTriggerGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("EcalTriggerGeometry","ldmx::EcalTriggerGeometryProvider",tagName,"Ecal")
            from LDMX.DetDescr import EcalHexReadout
            self.EcalHexReadout = EcalHexReadout.EcalHexReadout()
            EcalTriggerGeometryProvider.__instance = self 

ldmxcfg.Process.declareConditionsObjectProvider(EcalGeometryProvider.getInstance())
ldmxcfg.Process.declareConditionsObjectProvider(EcalTriggerGeometryProvider.getInstance())
