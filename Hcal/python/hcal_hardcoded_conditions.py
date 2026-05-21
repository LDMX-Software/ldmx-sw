"""Package to provide hard-coded conditions sources for Hcal
reconstruction and simulation

Attributes
----------
HcalReconConditionsHardcode : simple_csv_double_table_provider
    Provides a table of double conditions for hcal
    precision reconstruction
HcalHgcrocConditionsHardcode: simple_csv_double_table_provider
    Provides a table of double conditions for hcal
    hgcroc emulator
"""

from LDMX.Conditions.SimpleCSVTableProvider import (
    simple_csv_double_table_provider,
    simple_csv_integer_table_provider,
)


HcalTrigPrimConditionsHardcode = simple_csv_integer_table_provider(
    "HcalTrigPrimDigiConditions",
    ["ADC_PEDESTAL", "ADC_THRESHOLD", "TOT_PEDESTAL", "TOT_THRESHOLD", "TOT_GAIN"],
)
HcalTrigPrimConditionsHardcode.valid_for_all_rows(
    [
        1,  # ADC_PEDESTAL -- should match value from HgcrocEmulator
        5,  # ADC_THRESHOLD -- current noise is
        1,  # TOT_PEDESTAL -- currently set to match ADC pedestal
        10000,  # TOT_THRESHOLD -- rather large value...
        # Rounding because trigger primitives shouldn't use floating point
        # See https://github.com/LDMX-Software/Hcal/issues/66#issuecomment-1719663799
        round(2.5),
    ]  # TOT_GAIN, ratio of recon TOT gain over recon ADC gain
)

adc_pedestal = simple_csv_double_table_provider("hcal_adc_pedestal", ["pedestal"])
adc_pedestal.valid_for_all_rows([1.0])  # should match HgcrocEmulator

adc_gain = simple_csv_double_table_provider("hcal_adc_gain", ["gain"])
adc_gain.valid_for_all_rows(
    [1.2]
)  # 4 ADCs per PE - maxADCRange/readoutPadCapacitance/1024

tot_calib = simple_csv_double_table_provider(
    "hcal_tot_calibration",
    [
        "m_adc_i",
        "cut_point_tot",
        "high_slope",
        "high_offset",
        "low_slope",
        "low_power",
        "low_offset",
        "tot_not",
        "channel",
        "flagged",
    ],
)
tot_calib.valid_for_all_rows(
    [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
)  # dummy value since TOT is not implemented

toa_calib = simple_csv_double_table_provider(
    "hcal_toa_calibration", ["bx_shift", "mean_shift"]
)
toa_calib.valid_for_all_rows([0.0, 0.0])  # dummy values

# wrap our tables in the parent object that is used
# by the processors
from .conditions import HcalReconConditionsProvider


HcalReconConditionsProvider(
    adc_pedestal.object_name,
    adc_gain.object_name,
    tot_calib.object_name,
    toa_calib.object_name,
)

HcalHgcrocConditionsHardcode = simple_csv_double_table_provider(
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

HcalHgcrocConditionsHardcode.valid_for_all_rows(
    [
        1.0,  # PEDESTAL
        0.02 * 5 / 1.2,  # NOISE - 0.02 PE with 1 PE ~ 5mV and gain = 1.2
        12.5,  # MEAS_TIME - ns - clock_cycle/2
        # defines the point in the BX where an in-time hit would arrive
        20.0,  # PAD_CAPACITANCE - pF
        200.0,  # TOT_MAX - ns - maximum time chip would be in TOT mode
        10240.0 / 200.0,  # DRAIN_RATE - fC/ns - dummy value for now
        1.2,  # GAIN - large ADC gain for now - conversion from ADC to mV
        1.0 + 4.0,  # READOUT_THRESHOLD - 4 ADC counts above pedestal
        1.0 * 1.2
        + 1 * 5,  # TOA_THRESHOLD - mV - 1 PE above pedestal ( 1 PE  - 5 mV conversion)
        10000.0,  # TOT_THRESHOLD - mV - very large for now
    ]
)
