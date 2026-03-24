"""Detector map for translations between EIDs and det IDs"""

from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider


@conditions_object_provider("HcalDetectorMap", "hcal::HcalDetectorMapLoader", "Hcal")
class HcalDetectorMap(ConditionsObjectProvider):
    """Provider of the Hcal detector map allowing translations between
    electronics IDs and detector IDs.

    Parameters
    ----------
    connections_table : str
        Path to table of connections in Hcal
    want_d2e : bool
        Flag determining if we should spend the time to create a detID->EID LUT
    """

    connections_table: str
    want_d2e: bool = Falase
