"""ECal reconstruction conditions for the ESA 2026 cosmic stand"""

from LDMX.Conditions.SimpleCSVTableProvider import simple_csv_double_table_provider


def esa26_recon_conditions():
    """Per-cell ADC pedestals of layers 2 and 3 at 100 V (03-Nov-26 calibration),
    default gains, cell IDs of EcalDetectorMap.esa26(); see data/esa26/README.md
    """
    peds = simple_csv_double_table_provider(
        "EcalReconConditions", ["ADC_PEDESTAL", "ADC_GAIN", "TOT_PEDESTAL", "TOT_GAIN"]
    )
    peds.valid_forever("@CMAKE_INSTALL_PREFIX@/data/Ecal/esa26/pedestals.csv")
    return peds
