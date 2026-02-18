"""ConditionsProvider for HcalGeometry"""
from LDMX.Framework import ldmxcfg
from LDMX.DetDescr.hcal_geometry import HcalGeometry

@ldmxcfg.conditions_object_provider("HcalGeometryProvider","hcal::HcalGeometryProvider","Hcal")
class HcalGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Hcal geometry (HcalGeometry)

    Parameters
    ----------
    tag_name : str
        tag for generator of information

    Attributes
    ----------
    hcal_geometry : HcalGeometry
        Actual class providing Hcal layout
    __instance : HcalGeometryProvider
        Singleton instance of this object
    """

    __instance = None
    hcal_geometry: HcalGeometry = ldmxcfg.field(default_factory = HcalGeometry)

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

    def __post_init__(self):
        if HcalGeometryProvider.__instance != None :
            raise Exception('HcalGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            HcalGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
HcalGeometryProvider.getInstance()


@ldmxcfg.conditions_object_provider("HcalTriggerGeometry","hcal::HcalTriggerGeometryProvider","Hcal")
class HcalTriggerGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Hcal geometry (ldmx::HcalGeometry)
    Parameters
    ----------
    tag_name : str
        tag for generator of information
    Attributes
    ----------
    hcal_geometry : HcalGeometry
        Actual class providing precision cellular layout in Hcal
    __instance : HcalTriggerGeometryProvider
        Singleton instance of this object
    """

    __instance = None
    hcal_geometry: HcalGeometry = ldmxcfg.field(default_factory = HcalGeometry)

    def getInstance() :
        """Get the single instance of the HcalTriggerGeometryProvider
        Returns
        -------
        HcalTriggerGeometryProvider
            Single instance of the provider
        """

        if HcalTriggerGeometryProvider.__instance == None :
            HcalTriggerGeometryProvider()

        return HcalTriggerGeometryProvider.__instance

    def __post_init__(self):
        if HcalTriggerGeometryProvider.__instance != None :
            raise Exception('HcalTriggerGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            HcalTriggerGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
HcalTriggerGeometryProvider.getInstance()
