"""Detector map for translations between EIDs and det IDs"""

from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider


@conditions_object_provider("HcalDetectorMap", "hcal::HcalDetectorMapLoader", "Hcal")
class HcalDetectorMap(ConditionsObjectProvider):
    """Provider of the Hcal detector map allowing translations between
    electronics IDs and detector IDs.

    Parameters
    ----------
    connections_table : str
        Path to table of connections in Hcal, either the test beam format
        (Polarfire, HGCROC, Channel, CMB, QuadBar, Bar, Plane) or an explicit
        one (Polarfire, Elink, Channel, Section, Layer, Strip, End)
    want_d2e : bool
        Flag determining if we should spend the time to create a detID->EID LUT
    """

    connections_table: str
    want_d2e: bool = False

    @staticmethod
    def esa26():
        """ESA 2026 cosmic stand, HCal ROC0, 2, 3 on one ECON-D

        Electronics IDs as written by ECONDDecoder (fiber 0, eRx, channel).
        The orientation in x and y is provisional, see data/esa26/README.md.
        """
        return HcalDetectorMap(
            connections_table="@CMAKE_INSTALL_PREFIX@/data/Hcal/esa26/connections.csv"
        )
