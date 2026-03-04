"""Provide hard-coded conditions for Hcal reconstruction and simulation

Attributes
----------
HcalReconConditionsHardcode : SimpleCSVDoubleTableProvider
    Provides a table of double conditions for hcal
    precision reconstruction
HcalHgcrocConditionsHardcode: SimpleCSVDoubleTableProvider
    Provides a table of double conditions for hcal
    hgcroc emulator
"""

from LDMX.Conditions.SimpleCSVTableProvider import (
    SimpleCSVDoubleTableProvider,
    SimpleCSVIntegerTableProvider,
)


HcalTrigPrimConditionsHardcode = SimpleCSVIntegerTableProvider(
    "HcalTrigPrimDigiConditions",
    [
        "ADC_PEDESTAL",
        "ADC_THRESHOLD",
        "TOT_PEDESTAL",
        "TOT_THRESHOLD",
        "TOT_GAIN",
    ],
)
# ADC_PEDESTAL -- should match value from HgcrocEmulator
HcalTrigPrimConditionsHardcode.valid_for_all_rows([
    1,
    5,  # ADC_THRESHOLD -- current noise is
    # TOT_PEDESTAL -- currently set to match ADC pedestal
    1,
    # TOT_THRESHOLD -- rather large value...
    10000,
    # Rounding because trigger primitives
    # shouldn't represent floating point
    # operations.
    # See LDMX-Software/Hcal/issues/66#issuecomment-1719663799
    round(2.5),  # TOT_GAIN
])

adc_pedestal = SimpleCSVDoubleTableProvider(
    "hcal_adc_pedestal", ["pedestal"],
)
# should match HgcrocEmulator
adc_pedestal.valid_for_all_rows([1.])

adc_gain = SimpleCSVDoubleTableProvider(
    "hcal_adc_gain", ["gain"],
)
# 4 ADCs per PE - maxADCRange/readoutPadCapacitance/1024
adc_gain.valid_for_all_rows([1.2])

tot_calib = SimpleCSVDoubleTableProvider(
    "hcal_tot_calibration",
    [
        "m_adc_i", "cut_point_tot",
        "high_slope", "high_offset",
        "low_slope", "low_power",
        "low_offset", "tot_not",
        "channel", "flagged",
    ],
)
tot_calib.valid_for_all_rows([
    1., 1., 1., 1.,
    1., 1., 1., 1.,
    1., 1.,
])  # dummy value since TOT is not implemented

toa_calib = SimpleCSVDoubleTableProvider(
    "hcal_toa_calibration",
    ["bx_shift", "mean_shift"],
)
toa_calib.valid_for_all_rows([0., 0.])  # dummy values

# wrap our tables in the parent object that is used
# by the processors
from .conditions import HcalReconConditionsProvider


HcalReconConditionsProvider(
    adc_pedestal, adc_gain, tot_calib, toa_calib,
)

HcalHgcrocConditionsHardcode = SimpleCSVDoubleTableProvider(
    "HcalHgcrocConditions",
    [
        "PEDESTAL",
        "NOISE",
        "MEAS_TIME",
        "PAD_CAPACITANCE",
        "TOT_MAX",
        "DRAIN_RATE",
        "GAIN",
        "READOUT_THRESHOLD",
        "TOA_THRESHOLD",
        "TOT_THRESHOLD",
    ],
)

HcalHgcrocConditionsHardcode.valid_for_all_rows([
    1.,  # PEDESTAL
    # NOISE - 0.02 PE with 1 PE ~ 5mV and gain = 1.2
    0.02 * 5 / 1.2,
    # MEAS_TIME - ns - clock_cycle/2 - defines the
    # point in the BX where an in-time (time=0 in times
    # vector) hit would arrive
    12.5,
    20.,  # PAD_CAPACITANCE - pF
    # TOT_MAX - ns - max time chip would be in TOT mode
    200.,
    # DRAIN_RATE - fC/ns - dummy value for now
    10240. / 200.,
    # GAIN - large ADC gain for now
    # conversion from ADC to mV
    1.2,
    # READOUT_THRESHOLD - 4 ADC counts above pedestal
    1. + 4.,
    # TOA_THRESHOLD - mV
    # 1 PE above pedestal (1 PE - 5 mV conversion)
    1. * 1.2 + 1 * 5,
    10000.,  # TOT_THRESHOLD - mV - very large for now
])
