""" Package to provide hard-coded conditions sources for Hcal reconstruction and simulation

Attributes
----------
HcalReconConditionsHardcode : SimpleCSVDoubleTableProvider
    Provides a table of double conditions for hcal precision reconstruction
HcalHgcrocConditionsHardcode: SimpleCSVDoubleTableProvider
    Provides a table of double conditions for hcal hgcroc emulator
"""

from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider, SimpleCSVDoubleTableProvider

# TODO copied from Ecal trigger (ecal_hardcoded_conditions.py)
HcalTrigPrimConditionsHardcode=SimpleCSVIntegerTableProvider("HcalTrigPrimDigiConditions",["ADC_PEDESTAL","ADC_THRESHOLD","TOT_PEDESTAL","TOT_THRESHOLD","TOT_GAIN"])
HcalTrigPrimConditionsHardcode.validForAllRows([ 50 , # ADC_PEDESTAL -- should match value from HgcrocEmulator
                                                 5 , # ADC_THRESHOLD -- current noise is 
                                                 50,  # TOT_PEDESTAL -- currently set to match ADC pedestal
                                                 100,  # TOT_THRESHOLD -- rather large value...
                                                 8 ] # TOT_GAIN, ratio of recon TOT gain over recon ADC gain
                                               )


HcalReconConditionsHardcode=SimpleCSVDoubleTableProvider("HcalReconConditions",["ADC_PEDESTAL","ADC_GAIN","TOT_PEDESTAL","TOT_GAIN"])

HcalReconConditionsHardcode.validForAllRows([
    1. , #ADC_PEDESTAL - should match HgcrocEmulator
    1.2, #ADC_GAIN - 4 ADCS per PE - maxADCRange/readoutPadCapacitance/1024
    1 , #TOT_PEDESTAL - dummy value since TOT is not implemented
    2.5, #TOT_GAIN - dummy value - conversion to estimated charge deposited in TOT mode
    ])

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
