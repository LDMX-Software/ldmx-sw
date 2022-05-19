"""HCal conditions configuration classes

NOTE: This module is NOT a full set of conditions,
it is just here to share the configuration classes helpful
for conditions between the multiple condition sets
"""


from LDMX.Framework import ldmxcfg

class HcalReconConditionsProvider(ldmxcfg.ConditionsObjectProvider) :
    """The HcalReconConditions object packages the reconstructing conditions tables together
    
    This makes the processor using the recon conditions less dependent on the underlying structure,
    and (for example) a user can have very specific ADC pedestals provided by a per-channel table
    while keeping the TOT pedestal as an estimated constant.

    Parameters
    ----------
    adc_ped : framework::ConditionsObjectProvider
        provider for the HCal ADC pedestals
    adc_gain : framework::ConditionsObjectProvider
        provider for the HCal ADC gains
    tot_ped : framework::ConditionsObjectProvider
        provider for the HCal TOT pedestals
    tot_gain : framework::ConditionsObjectProvider
        provider for the HCal TOT gains

    Examples
    --------
    The hcal_hardcoded_conditions.py file provides a working example where each condition
    wrapped here are constant for all runs and all channels.
    """

    def __init__(self,adc_ped,adc_gain,tot_ped,tot_gain) :
        super().__init__("HcalReconConditions","hcal::HcalReconConditionsProvider","Hcal")

        # our COP only needs the object names but providing the full parent COPs
        #   ensures that they exist
        self.adc_ped = adc_ped.objectName
        self.adc_gain = adc_gain.objectName
        self.tot_ped = tot_ped.objectName
        self.tot_gain = tot_gain.objectName

