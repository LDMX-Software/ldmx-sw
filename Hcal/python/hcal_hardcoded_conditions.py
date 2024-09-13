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
                                                 5100.,  # TOT_THRESHOLD 
                                                 # Rounding because trigger primitives shouldn't represent floating point operations.
                                                 # See https://github.com/LDMX-Software/Hcal/issues/66#issuecomment-1719663799
                                                 round(2.5) ] # TOT_GAIN, ratio of recon TOT gain over recon ADC gain
                                               )

adc_pedestal = SimpleCSVDoubleTableProvider("hcal_adc_pedestal",["pedestal"])
adc_pedestal.validForAllRows([1.]) # should match HgcrocEmulator

adc_gain = SimpleCSVDoubleTableProvider("hcal_adc_gain",["gain"])
adc_gain.validForAllRows([1.2]) # 4 ADCs per PE - maxADCRange/readoutPadCapacitance/1024

tot_calib = SimpleCSVDoubleTableProvider("hcal_tot_calibration",
                                         ["m_adc_i","cut_point_tot","high_slope","high_offset",
                                          "low_slope","low_power","low_offset","tot_not",
                                          "channel","flagged"])
tot_calib.validForAllRows([
    1.,1.,1.,1.,
    1.,1.,1.,1.,
    1.,1.
]) # dummy value since TOT is not implemented

toa_calib =  SimpleCSVDoubleTableProvider("hcal_toa_calibration",
                                          ["bx_shift","mean_shift"])
toa_calib.validForAllRows([0., 0.]) # dummy values

# wrap our tables in the parent object that is used by the processors
from .conditions import HcalReconConditionsProvider
HcalReconConditionsProvider(adc_pedestal, adc_gain, tot_calib, toa_calib)

HcalHgcrocConditionsHardcode=SimpleCSVDoubleTableProvider("HcalHgcrocConditions", [
            "PEDESTAL",
            "NOISE",
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
    0.02*5/(5100./1023.), #NOISE - 0.02 PE with 1 PE ~ 5mV and gain = 5100./1023.
    12.5, #MEAS_TIME - ns - clock_cycle/2 - defines the point in the BX where an in-time (time=0 in times vector) hit would arrive
    13/5.1, #PAD_CAPACITANCE - pF
    200., #TOT_MAX - ns - maximum time chip would be in TOT mode
    320000./200., #DRAIN_RATE - fC/ns - to drain maximum charge of 320000fC in 200ns
    5100./1023., #GAIN - take 160 fC - 13 pC to be the linear range of ADC -> V_{adc_max} = 13pC/13/5.1pF = 5100mV
    1. + 4., #READOUT_THRESHOLD - 4 ADC counts above pedestal
    1.*(5100./1023.) + 1*5, #TOA_THRESHOLD - mV - 1 PE above pedestal ( 1 PE  - 5 mV conversion)
    5100., #TOT_THRESHOLD - mV - TOT begins at V_{adc_max}
    ])
