""" Package to provide hard-coded conditions sources for Ecal reconstruction and simulation
"""
from LDMX.Framework.ldmxcfg import ConditionsObjectProvider
from LDMX.Framework.ldmxcfg import Process
from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider

EcalTrigPrimConditionsHardcode=SimpleCSVIntegerTableProvider("EcalTrigPrimDigiConditions",["ADC_PEDESTAL","ADC_THRESHOLD","TOT_PEDESTAL","TOT_THRESHOLD","TOT_GAIN"])

EcalTrigPrimConditionsHardcode.validForAllRows([ 50 , # ADC_PEDESTAL -- should match value from HgcrocEmulator
                                                 5 , # ADC_THRESHOLD -- current noise is 
                                                 0,  # TOT_PEDESTAL -- currently zero
                                                 100,  # TOT_THRESHOLD -- rather large value...
                                                 8 ] # TOT_GAIN, would ideally be 7.8, but....
                                               )
