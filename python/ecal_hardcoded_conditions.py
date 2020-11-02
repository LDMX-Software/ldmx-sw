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
                                                 0,  # TOT_PEDESTAL -- currently zero
                                                 100,  # TOT_THRESHOLD -- rather large value...
                                                 8 ] # TOT_GAIN, would ideally be 7.8, but....
                                               )

EcalReconConditionsHardcode=SimpleCSVDoubleTableProvider("EcalReconConditions",["ADC_PEDESTAL","ADC_GAIN","TOT_PEDESTAL","TOT_GAIN"])

EcalReconConditionsHardcode.validForAllRows([
    50. , #ADC_PEDESTAL - should match HgcrocEmulator
    320./1024. , #ADC_GAIN - [fC/counts] - conversion to estimated charge deposited in ADC mode
    0. , #TOT_PEDESTAL - not being used at this time in the digi emulation
    4000./4096 #TOT_GAIN - [fC/counts] - conversion to estimated charge deposited in TOT mode
    ])
