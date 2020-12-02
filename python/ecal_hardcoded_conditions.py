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
    50. , #TOT_PEDESTAL - using the same pedestal as ADC right now
    2.5, #TOT_GAIN - 10240 fC / 4096 counts - conversion to estimated charge deposited in TOT mode
    ])
