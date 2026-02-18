"""ConditionsProvider for EcalGeometry and other Ecal geometry-related aspects"""
from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider, field
from LDMX.DetDescr.ecal_geometry import EcalGeometry

print('importing ecal_geometry')

@conditions_object_provider("EcalGeometry", "ecal::EcalGeometryProvider", "Ecal")
class EcalGeometryProvider(ConditionsObjectProvider):
    """Provider that provides access to Ecal geometry (ecal::EcalGeometry)

    Parameters
    ----------
    tag_name : str
        tag for generator of information

    Attributes
    ----------
    EcalGeometry : EcalGeometry
        Actual class providing precision cellular layout in Ecal
    __instance : EcalGeometryProvider
        Singleton instance of this object
    """

    __instance = None
    geometries: list[EcalGeometry] = []

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

    def __post_init__(self):
        print('creating EcalGeometryProvider')
        if EcalGeometryProvider.__instance != None :
            raise Exception('EcalGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            self.geometries = EcalGeometry.geometries()
            EcalGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
EcalGeometryProvider.getInstance()

@conditions_object_provider("EcalTriggerGeometry", "ecal::EcalTriggerGeometryProvider", "Ecal")
class EcalTriggerGeometryProvider(ConditionsObjectProvider):
    """Provider that provides access to Ecal geometry (ldmx::EcalGeometry)

    Parameters
    ----------
    tag_name : str
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

    def __post_init__(self):
        if EcalTriggerGeometryProvider.__instance != None :
            raise Exception('EcalTriggerGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            EcalTriggerGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
EcalTriggerGeometryProvider.getInstance()
