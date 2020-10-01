"""ConditionsProvider for EcalHexReadout and other Ecal geometry-related aspects"""
from LDMX.Framework import ldmxcfg

class EcalGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Ecal geometry helper classes

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
            EcalGeometryProvider()

        return EcalGeometryProvider.__instance

    def __init__(self):
        if EcalGeometryProvider.__instance != None :
            raise Exception('EcalGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("EcalGeometryProvider","ldmx::EcalGeometryProvider","Ecal")
            from LDMX.DetDescr import EcalHexReadout
            self.EcalHexReadout = EcalHexReadout.EcalHexReadout()
            EcalGeometryProvider.__instance = self 
