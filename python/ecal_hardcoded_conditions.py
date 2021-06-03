""" Package to provide hard-coded conditions sources for Ecal reconstruction and simulation

Attributes
----------
EcalTrigPrimConditionsHardcode : SimpleCSVIntegerTableProvider
    Provides a table of integer conditions for ecal trigger primitives producer
EcalReconConditionsHardcode : SimpleCSVDoubleTableProvider
    Provides a table of double conditions for ecal precision reconstruction
"""

from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider, SimpleCSVDoubleTableProvider

EcalTrigPrimConditionsHardcode=SimpleCSVIntegerTableProvider("EcalTrigPrimDigiConditions",["ADC_PEDESTAL","ADC_THRESHOLD","TOT_PEDESTAL","TOT_THRESHOLD","TOT_GAIN"])

EcalTrigPrimConditionsHardcode.validForAllRows([ 50 , # ADC_PEDESTAL -- should match value from HgcrocEmulator
                                                 5 , # ADC_THRESHOLD -- current noise is 
                                                 50,  # TOT_PEDESTAL -- currently set to match ADC pedestal
                                                 100,  # TOT_THRESHOLD -- rather large value...
                                                 8 ] # TOT_GAIN, ratio of recon TOT gain over recon ADC gain
                                               )

EcalReconConditionsHardcode=SimpleCSVDoubleTableProvider("EcalReconConditions",["ADC_PEDESTAL","ADC_GAIN","TOT_PEDESTAL","TOT_GAIN"])

EcalReconConditionsHardcode.validForAllRows([
    50. , #ADC_PEDESTAL - should match HgcrocEmulator
    0.3125, #ADC_GAIN - 320. fC / 1024. counts - conversion to estimated charge deposited in ADC mode
    50.0 , #TOT_PEDESTAL - tweaked so that we pass the reconstruction tests
    2.5, #TOT_GAIN - 10240 fC / 4096 counts - conversion to estimated charge deposited in TOT mode
    ])

EcalHgcrocConditionsHardcode=SimpleCSVDoubleTableProvider("EcalHgcrocConditions", [
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

EcalHgcrocConditionsHardcode.validForAllRows([
    50. , #PEDESTAL 
    0.0, #MEAS_TIME - ns
    20., #PAD_CAPACITANCE - pF
    200., #TOT_MAX - ns - maximum time chip would be in TOT mode
    10240. / 200., #DRAIN_RATE - fC/ns
    320./1024/20., #GAIN - 320. fC / 1024. counts / 20 pF - conversion from ADC to mV
    50. + 3., #READOUT_THRESHOLD - 3 ADC counts above pedestal
    50.*320./1024/20. + 5 *37*0.1602/20., #TOA_THRESHOLD - mV - ~5  MIPs above pedestal
    50.*320./1024/20. + 50*37*0.1602/20., #TOT_THRESHOLD - mV - ~50 MIPs above pedestal
    ])
