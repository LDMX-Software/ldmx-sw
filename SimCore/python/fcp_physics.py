"""Configuration for FCPPhysics -- fractionally charged particle processes

This module is independent of the dark brem configuration. It handles
gamma -> fcp+ fcp- conversion on SM photons.
"""


class FCPPhysics:
    """Parameters for the FCP physics constructor

    Attributes
    ----------
    enable : bool
        Should we enable fcp physics? (Default: No)
    fcp_mass : float
        Mass of the fcp in MeV (Default: 0.)
    fcp_charge : float
        Electric charge of the fcp in units of e (Default: 0.1)
    fcp_pdg_id : int
        PDG ID for the fcp particles (Default: 17)
    """

    def __init__(self):
        self.enable = False
        self.fcp_mass = 0.0  # MeV
        self.fcp_charge = 0.1  # units of e
        self.fcp_pdg_id = 17

    def activate(self, fcp_mass, fcp_charge=0.1):
        """Enable gamma -> fcp+ fcp- conversion process

        Parameters
        ----------
        fcp_mass : float
            Mass of the fractionally charged particle in MeV
        fcp_charge : float, optional
            Electric charge of the fcp in units of e (default: 0.1)
        """
        self.enable = True
        self.fcp_mass = fcp_mass
        self.fcp_charge = fcp_charge
