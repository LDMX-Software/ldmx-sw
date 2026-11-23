"""ECal reconstruction conditions for the ESA 2026 cosmic stand"""

from LDMX.Conditions.SimpleCSVTableProvider import simple_csv_double_table_provider
from LDMX.Ecal.ecal_geometry import EcalGeometryProvider


CONDITIONS_DATA = (
    "https://raw.githubusercontent.com/LDMX-Software/conditions-data/refs/heads/main/"
)


def esa26_recon_conditions(base_url=CONDITIONS_DATA, alignment=True):
    """Per-cell ADC pedestals of layers 2 and 3 at 100 V (03-Nov-26 calibration)
    and ADC gains from cosmic MIPs (a MIP reconstructs as 1 MIP with
    EcalRecProducer.esa_cosmic_stand()), cell IDs of EcalDetectorMap.esa26(),
    from the conditions-data repository (Ecal/esa26, valid from run 230)

    Parameters
    ----------
    base_url : str
        conditions-data location, e.g. file:///path/to/conditions-data/
    alignment : bool
        also shift the ECal geometry per run (esa26_alignment)
    """
    if alignment:
        esa26_alignment(base_url)
    peds = simple_csv_double_table_provider(
        "EcalReconConditions", ["ADC_PEDESTAL", "ADC_GAIN", "TOT_PEDESTAL", "TOT_GAIN"]
    )
    peds.conditions_base_url = base_url
    # relative URLs get the base URL prepended
    peds.entries_url = "Ecal/esa26/pedestals/index_v1_2_0.csv"
    return peds


def esa26_alignment(base_url=CONDITIONS_DATA):
    """Whole-ECal shift of the stand vs ldmx-esa-cosmic26-v2 (DX, DY, DZ in mm)
    per run range, from cosmic muons with straight HCal tracks; the
    EcalGeometryProvider builds the geometry of each run with it

    Parameters
    ----------
    base_url : str
        conditions-data location, e.g. file:///path/to/conditions-data/
    """
    align = simple_csv_double_table_provider("EcalAlignment", ["DX", "DY", "DZ"])
    align.conditions_base_url = base_url
    align.entries_url = "Ecal/esa26/alignment/index_v1_1_0.csv"
    EcalGeometryProvider.get_instance().set_alignment("EcalAlignment")
    return align
