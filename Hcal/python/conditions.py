"""HCal conditions configuration classes

NOTE: This module is NOT a full set of conditions,
it is just here to share the configuration classes helpful
for conditions between the multiple condition sets
"""

from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider


@conditions_object_provider(
    "HcalReconConditions", "hcal::HcalReconConditionsProvider", "Hcal"
)
class HcalReconConditionsProvider(ConditionsObjectProvider):
    """The HcalReconConditions object packages the reconstructing conditions
    tables together

    This makes the processor using the recon conditions less dependent on the
    underlying structure.

    Parameters
    ----------
    adc_ped : framework::ConditionsObjectProvider
        provider for the HCal ADC pedestals
    adc_gain : framework::ConditionsObjectProvider
        provider for the HCal ADC gains
    tot_calib : framework::ConditionsObjectProvider
        provider for the HCal TOT calibrations
    toa_calib : framework::ConditionsObjectProvider
        provider for the HCal TOA calibrations

    Examples
    --------
    The hcal_hardcoded_conditions.py file provides a working example
    where each condition wrapped here are constant for all runs and all channels.
    """

    adc_ped: str
    adc_gain: str
    tot_calib: str
    toa_calib: str
