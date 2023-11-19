
class KaonPhysics():
    """Parameters that determine the physics of kaons in the simulation

    Parameters
    ----------
    kplus_branching_ratios : list[float]
    kminus_branching_ratios : list[float]
        List of the desired branching ratios for charged kaons. The entries
        correspond go the following processes

        - 0: K -> mu + v
        - 1: K -> pi + pi0
        - 2: K -> 3 pi
        - 3: K -> pi + 2 pi0
        - 4: K -> pi0 + e + v
        - 5: K -> pi0 + mu + v

        See the sources for Geant4's definitions and branching ratios
        (G4KaonMinus.cc, G4KaonPlus.cc). The default branching ratios
        correspond to the Geant4 defaults.

    kplus_lifetime_factor : float
    kminus_lifetime_factor : float

        Multiplicative factor to scale the lifetime of charged kaons by.
        Default is to scale by 1 (no scaling)
    """
    def __init__(self):
        self.kplus_branching_ratios = [
            0.6355,  # K^+ -> mu^+ + nu_mu
            0.2066,  # K^+ -> pi^+ + pi^0
            0.0559,  # K^+ -> pi^+ + pi^- + pi^+
            0.01761, # K^+ -> pi^+ + pi^0 + pi^0
            0.0507,  # K^+ -> pi^0 + e^+ + nu_e
            0.0335,  # K^+ -> pi^0 + mu^+ + nu_mu
        ]
        self.kminus_branching_ratios = [
            0.6355,  # K^- -> mu^- + anti_nu_mu
            0.2066,  # K^- -> pi^- + pi^0
            0.0559,  # K^- -> pi^- + pi^+ + pi^-
            0.01761, # K-+ -> pi^- + pi^0 + pi^0
            0.0507,  # K-+ -> pi^0 + e^- + anti_nu_e
            0.0335,  # K-+ -> pi^0 + mu^- + anti_nu_mu
        ]
        self.kplus_lifetime_factor = 1.
        self.kminus_lifetime_factor = 1.


    def upKaons():
        """Returns a configuration of the kaon physics corresponding
        to the changes that were made in
        https://github.com/ldmx-software/geant4/tree/LDMX.upKaons_mod

        Reduces the charged kaon lifetimes by a factor 1/50 and forces
        decays to be into one of the leptonic decay modes.
        """
        kaon_physics = KaonPhysics()
        kaon_physics.kplus_branching_ratios = [
            0.8831,  # K^+ -> mu^+ + nu_mu
            0.,  # K^+ -> pi^+ + pi^0
            0.,  # K^+ -> pi^+ + pi^- + pi^+
            0., # K^+ -> pi^+ + pi^0 + pi^0
            0.0704,  # K^+ -> pi^0 + e^+ + nu_e
            0.0465,  # K^+ -> pi^0 + mu^+ + nu_mu
        ]
        kaon_physics.kminus_branching_ratios = [
            0.8831,  # K^- -> mu^- + anti_nu_mu
            0.,  # K^- -> pi^- + pi^0
            0.,  # K^- -> pi^- + pi^+ + pi^-
            0., # K-+ -> pi^- + pi^0 + pi^0
            0.0704,  # K-+ -> pi^0 + e^- + anti_nu_e
            0.0464,  # K-+ -> pi^0 + mu^- + anti_nu_mu
        ]
        kaon_physics.kplus_lifetime_factor = 1/50.
        kaon_physics.kminus_lifetime_factor = 1/50.
        return kaon_physics
