"""Load the April 2022 Testbeam conditions for HCal"""

from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider, SimpleCSVDoubleTableProvider

adc_pedestal = SimpleCSVDoubleTableProvider("hcal_adc_pedestal",["ADC_PEDESTAL","ADC_RMS"])
adc_pedestal.conditions_baseURL = os.environ['LDMX_BASE']+'/conditions-data/'
adc_pedestal.entriesURL = '${LDMX_CONDITIONS_BASEURL}/Hcal/testbeam04-22/pedestals/index-v1_0_0.csv'

adc_gain = SimpleCSVDoubleTableProvider("hcal_adc_gain",["gain"])
adc_gain.validForAllRows([1.2]) # 4 ADCs per PE - maxADCRange/readoutPadCapacitance/1024

tot_pedestal = SimpleCSVDoubleTableProvider("hcal_tot_pedestal",["pedestal"])
tot_pedestal.validForAllRows([1.]) # dummy value since TOT is not implemented

tot_gain = SimpleCSVDoubleTableProvider("hcal_tot_gain",["gain"])
tot_gain.validForAllRows([2.5]) # dummy value - conversion to estimated charge deposited in TOT mode

from .conditions import HcalReconConditionsProvider
HcalReconConditionsProvider(adc_pedestal, adc_gain, tot_pedestal, tot_gain)

