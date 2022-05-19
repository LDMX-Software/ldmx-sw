""" Package to provide hard-coded conditions sources for Hcal reconstruction and simulation

Attributes
----------
HcalReconConditionsHardcode : SimpleCSVDoubleTableProvider
    Provides a table of double conditions for hcal precision reconstruction
HcalHgcrocConditionsHardcode: SimpleCSVDoubleTableProvider
    Provides a table of double conditions for hcal hgcroc emulator
"""

from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider, SimpleCSVDoubleTableProvider

HcalTrigPrimConditionsHardcode=SimpleCSVIntegerTableProvider("HcalTrigPrimDigiConditions",["ADC_PEDESTAL","ADC_THRESHOLD","TOT_PEDESTAL","TOT_THRESHOLD","TOT_GAIN"])
HcalTrigPrimConditionsHardcode.validForAllRows([ 1 , # ADC_PEDESTAL -- should match value from HgcrocEmulator
                                                 5 , # ADC_THRESHOLD -- current noise is 
                                                 1,  # TOT_PEDESTAL -- currently set to match ADC pedestal
                                                 10000,  # TOT_THRESHOLD -- rather large value...
                                                 2.5 ] # TOT_GAIN, ratio of recon TOT gain over recon ADC gain
                                               )
from LDMX.Framework import ldmxcfg

class HcalReconConditionsProvider(ldmxcfg.ConditionsObjectProvider) :
    def __init__(self,adc_ped,adc_gain,tot_ped,tot_gain) :
        super().__init__("HcalReconConditions","hcal::HcalReconConditionsProvider","Hcal")

        def extract_obj_name(co) :
            """If it is a COP, then get the obj name, otherwise return itself"""

            if isinstance(co,ldmxcfg.ConditionsObjectProvider) :
                return co.objectName
            else :
                return co

        self.adc_ped = extract_obj_name(adc_ped)
        self.adc_gain = extract_obj_name(adc_gain)
        self.tot_ped = extract_obj_name(tot_ped)
        self.tot_gain = extract_obj_name(tot_gain)


adc_pedestal = SimpleCSVDoubleTableProvider("hcal_adc_pedestal",["pedestal"])
adc_pedestal.validForAllRows([1.]) # should match HgcrocEmulator

adc_gain = SimpleCSVDoubleTableProvider("hcal_adc_gain",["gain"])
adc_gain.validForAllRows([1.2]) # 4 ADCs per PE - maxADCRange/readoutPadCapacitance/1024

tot_pedestal = SimpleCSVDoubleTableProvider("hcal_tot_pedestal",["pedestal"])
tot_pedestal.validForAllRows([1.]) # dummy value since TOT is not implemented

tot_gain = SimpleCSVDoubleTableProvider("hcal_tot_gain",["gain"])
tot_gain.validForAllRows([2.5]) # dummy value - conversion to estimated charge deposited in TOT mode

# wrap our tables in the parent object that is used by the processors
HcalReconConditionsProvider(adc_pedestal, adc_gain, tot_pedestal, tot_gain)

HcalHgcrocConditionsHardcode=SimpleCSVDoubleTableProvider("HcalHgcrocConditions", [
            "PEDESTAL",
            "MEAS_TIME",
            "PAD_CAPACITANCE",
            "TOT_MAX",
            "DRAIN_RATE",
            "GAIN",
            "READOUT_THRESHOLD",
            "TOA_THRESHOLD",
            "TOT_THRESHOLD"
        ])

HcalHgcrocConditionsHardcode.validForAllRows([
    1. , #PEDESTAL 
    12.5, #MEAS_TIME - ns - clock_cycle/2 - defines the point in the BX where an in-time (time=0 in times vector) hit would arrive
    20., #PAD_CAPACITANCE - pF
    200., #TOT_MAX - ns - maximum time chip would be in TOT mode
    10240. / 200., #DRAIN_RATE - fC/ns - dummy value for now
    1.2, #GAIN - large ADC gain for now - conversion from ADC to mV
    1. + 4., #READOUT_THRESHOLD - 4 ADC counts above pedestal
    1.*1.2 + 1*5, #TOA_THRESHOLD - mV - 1 PE above pedestal ( 1 PE  - 5 mV conversion)
    10000., #TOT_THRESHOLD - mV - very large for now
    ])
