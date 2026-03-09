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

        if HcalGeometryProvider.__instance is None :
            HcalGeometryProvider()

        return HcalGeometryProvider.__instance

    def __init__(self):
        if HcalGeometryProvider.__instance is not None :
            raise Exception('HcalGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("HcalGeometryProvider","hcal::HcalGeometryProvider","Hcal")
            from LDMX.DetDescr import hcal_geometry
            self.hcal_geometry = hcal_geometry.HcalGeometry()
            HcalGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
HcalGeometryProvider.getInstance()


class HcalTriggerGeometryProvider(ldmxcfg.ConditionsObjectProvider):
    """Provider that provides access to Hcal geometry (ldmx::HcalGeometry)
    Parameters
    ----------
    tagName : str
        tag for generator of information
    Attributes
    ----------
    HcalGeometry : HcalGeometry
        Actual class providing precision cellular layout in Hcal
    __instance : HcalTriggerGeometryProvider
        Singleton instance of this object
    """

    __instance = None

    def getInstance() :
        """Get the single instance of the HcalTriggerGeometryProvider
        Returns
        -------
        HcalTriggerGeometryProvider
            Single instance of the provider
        """

        if HcalTriggerGeometryProvider.__instance is None :
            HcalTriggerGeometryProvider()

        return HcalTriggerGeometryProvider.__instance

    def __init__(self):
        if HcalTriggerGeometryProvider.__instance is not None :
            raise Exception('HcalTriggerGeometryProvider is a singleton class and should only be retrieved using getInstance()')
        else:
            super().__init__("HcalTriggerGeometry","hcal::HcalTriggerGeometryProvider","Hcal")
            from LDMX.DetDescr import hcal_geometry
            self.hcal_geometry = hcal_geometry.HcalGeometry()
            HcalTriggerGeometryProvider.__instance = self

# make sure global instance is created, this registers the condition
HcalTriggerGeometryProvider.getInstance()
