"""Load the April 2022 Testbeam conditions for HCal

We assume that the conditions-data repository has been cloned into LDMX_BASE.

    cd ${LDMX_BASE}
    git clone https://github.com/LDMX-Software/conditions-data

You can modify this path if you have your conditions somewhere else.
**Make sure these conditions are available to the container.

    import hcal_testbeam0422_conditions
    hcal_testbeam0422_conditions.adc_pedestal.conditions_baseURL = 'file:///full/path/my/url/'

"""

from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider, SimpleCSVDoubleTableProvider
import os

adc_pedestal = SimpleCSVDoubleTableProvider("hcal_adc_pedestal",["PEDESTAL_ADC","PEDESTAL_RMS_ADC"])
adc_pedestal.conditions_baseURL = f'file://{os.environ["LDMX_BASE"]}/conditions-data/'
adc_pedestal.entriesURL = '${LDMX_CONDITION_BASEURL}/Hcal/testbeam04-2022/pedestals/index_v1_0_0.csv'

adc_gain = SimpleCSVDoubleTableProvider("hcal_adc_gain",["MIPMPV_ADC"])
adc_gain.conditions_baseURL = f'file://{os.environ["LDMX_BASE"]}/conditions-data/'
adc_gain.entriesURL = '${LDMX_CONDITION_BASEURL}/Hcal/testbeam04-2022/mips/index_v1_0_0.csv'

# the TOT linearization parameters are very stable so we use the same set for all runs
tot_calib = SimpleCSVDoubleTableProvider("hcal_tot_calibration",
        ["m_adc_i","cut_point_tot","high_slope","high_offset",
         "low_slope","low_power","low_offset","tot_not","channel","flagged"])
tot_calib.validForever(f'file://{os.environ["LDMX_BASE"]}/conditions-data/Hcal/testbeam04-2022/tot_calibration/calibrated_tot_calib_v0_1_0.csv')

from .conditions import HcalReconConditionsProvider
HcalReconConditionsProvider(adc_pedestal, adc_gain, tot_calib)
