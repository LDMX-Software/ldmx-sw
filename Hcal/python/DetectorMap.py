"""Detector map for translations between EIDs and det IDs"""

from LDMX.Framework import ldmxcfg

class HcalDetectorMap(ldmxcfg.ConditionsObjectProvider) :
    """Provider of the Hcal detector map allowing translations between
    electronics IDs and detector IDs.

    Parameters
    ----------
    connections_table : str
        Path to table of connections in Hcal
    want_d2e : bool
        Flag determining if we should spend the time to create a detID->EID LUT
    """

    def __init__(self, connections_table, want_d2e = False) :
        super().__init__('HcalDetectorMap','hcal::HcalDetectorMapLoader','Hcal')
        self.connections_table = connections_table
        self.want_d2e = want_d2e

