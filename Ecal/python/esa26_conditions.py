"""ECal reconstruction conditions for the ESA 2026 cosmic stand"""

from LDMX.Conditions.SimpleCSVTableProvider import simple_csv_double_table_provider


def esa26_recon_conditions(
    base_url="https://raw.githubusercontent.com/LDMX-Software/conditions-data/refs/heads/main/",
):
    """Per-cell ADC pedestals of layers 2 and 3 at 100 V (03-Nov-26 calibration),
    default gains, cell IDs of EcalDetectorMap.esa26(), from the conditions-data
    repository (Ecal/esa26, valid from run 230)

    Parameters
    ----------
    base_url : str
        conditions-data location, e.g. file:///path/to/conditions-data/
    """
    peds = simple_csv_double_table_provider(
        "EcalReconConditions", ["ADC_PEDESTAL", "ADC_GAIN", "TOT_PEDESTAL", "TOT_GAIN"]
    )
    peds.conditions_base_url = base_url
    # relative URLs get the base URL prepended
    peds.entries_url = "Ecal/esa26/pedestals/index_v1_0_0.csv"
    return peds
