
class KaonPhysics():
    """Parameters that determine the physics of kaons in the simulation

    Parameters
    ----------
    kplus_branching_ratios : list[float]
    kminus_branching_ratios : list[float]
    k0l_branching_ratios : list[float]
    k0s_branching_ratios : list[float]
        List of the desired branching ratios for kaons. The entries
        correspond go the following processes

        For charged kaons
        - 0: K -> mu + v
        - 1: K -> pi + pi0
        - 2: K -> 3 pi
        - 3: K -> pi + 2 pi0
        - 4: K -> pi0 + e + v
        - 5: K -> pi0 + mu + v

        For Klong
        - 0: K -> 3 pi0
        - 1: K -> pi0 + pi+ + pi-
        - 2: K -> pi- + e+ + v
        - 3: K -> pi+ + e- + v
        - 4: K -> pi- + mu+ + v
        - 5: K -> pi+ + mu- + v

        For Kshort:
        - 0: K -> pi+ + pi-
        - 1: K -> 2 pi0

        See the sources for Geant4's definitions and branching ratios
        (G4KaonMinus.cc, G4KaonPlus.cc, KaonZeroLong.cc, KaonZeroShort.cc). The
        default branching ratios correspond to the Geant4 defaults.

    kplus_lifetime_factor : float
    kminus_lifetime_factor : float
    k0l_lifetime_factor : float
    k0s_lifetime_factor : float

        Multiplicative factor to scale the lifetime of kaons by. Default is to
        scale by 1 (no scaling)

    verbosity : int

        If > 0: Dump details about the decay after the update
        If > 1: Also dump details about the decay before the update to show the
        difference

    """
    def __init__(self):
        self.kplus_branching_ratios = [
            0.6355,  # K^+ -> mu^+ + nu_mu
            0.2066,  # K^+ -> pi^+ + pi^0
            0.0559,  # K^+ -> pi^+ + pi^- + pi^+
            0.0507,  # K^+ -> pi^0 + e^+ + nu_e
            0.0335,  # K^+ -> pi^0 + mu^+ + nu_mu
            0.01761, # K^+ -> pi^+ + pi^0 + pi^0
        ]
        self.kminus_branching_ratios = [
            0.6355,  # K^- -> mu^- + anti_nu_mu
            0.2066,  # K^- -> pi^- + pi^0
            0.0559,  # K^- -> pi^- + pi^+ + pi^-
            0.0507,  # K-+ -> pi^0 + e^- + anti_nu_e
            0.0335,  # K-+ -> pi^0 + mu^- + anti_nu_mu
            0.01761, # K-+ -> pi^- + pi^0 + pi^0
        ]
        self.k0l_branching_ratios = [
            0.2027, # K^0_L -> pi^- + e^+ + nu_e
            0.2027, # K^0_L -> pi^+ + e^- + anti_nu_e
            0.1952, # K^0_L -> pi^0 + pi^0 + pi^0
            0.1352, # K^0_L -> pi^- + mu^+ + nu_mu
            0.1352, # K^0_L -> pi^+ + mu^- + anti_nu_mu
            0.1254, # K^0_L -> pi^0 + pi^+ + pi^-
        ]
        self.k0s_branching_ratios = [
            0.6920, # K^0_S -> pi^+ + pi^-
            0.3069, # K^0_S -> pi^0 + pi^0
        ]
        self.kplus_lifetime_factor = 1.
        self.kminus_lifetime_factor = 1.
        self.k0l_lifetime_factor = 1.
        self.k0s_lifetime_factor = 1.

        self.verbosity=0


    def upKaons():
        """Returns a configuration of the kaon physics corresponding
        to the changes that were made in
        https://github.com/ldmx-software/geant4/tree/LDMX.upKaons_mod

        Reduces the charged kaon lifetimes by a factor 1/50 and forces
        decays to be into one of the leptonic decay modes.

        See
        https://github.com/LDMX-Software/geant4/commit/25228b8b1fbad913b4933b7c0d3951ebe36d404c
        """
        kaon_physics = KaonPhysics()
        kaon_physics.kplus_branching_ratios = [
            0.8831,  # K^+ -> mu^+ + nu_mu
            0.,      # K^+ -> pi^+ + pi^0
            0.,      # K^+ -> pi^+ + pi^- + pi^+
            0.0704,  # K^+ -> pi^0 + e^+ + nu_e
            0.0465,  # K^+ -> pi^0 + mu^+ + nu_mu
            0.,      # K^+ -> pi^+ + pi^0 + pi^0
        ]
        kaon_physics.kminus_branching_ratios = [
            0.8831,  # K^- -> mu^- + anti_nu_mu
            0.,      # K^- -> pi^- + pi^0
            0.,      # K^- -> pi^- + pi^+ + pi^-
            0.0704,  # K^- -> pi^0 + e^- + anti_nu_e
            0.0464,  # K^- -> pi^0 + mu^- + anti_nu_mu
            0.,      # K^- -> pi^- + pi^0 + pi^0
        ]
        kaon_physics.kplus_lifetime_factor = 1/50.
        kaon_physics.kminus_lifetime_factor = 1/50.
        kaon_physics.verbosity = 2
        return kaon_physics

    def __setattr__(self, key, value):
        """Ensure that attempts to set the branching ratios give a total of
        something close to 1 and that the lifetime factors are positive.

        Note that Geant4's defaults are not exactly equal to one, but something
        significantly different from one is likely to be a mistake.

        """
        if 'branching_ratios' in key:
            if not isinstance(value, list):
                raise TypeError(f'Values of branching ratios ({key}) need to be lists')
            total_branching_ratio = sum(value)
            if abs(total_branching_ratio- 1) > 0.05:
                raise ValueError(f'Total of branching ratios in {key} significantly different from one, was {total_branching_ratio}: {value}.')
        elif 'lifetime' in key:
            if not isinstance(value, float):
                raise TypeError(f'Lifetime parameter ({key}) needs to be floating-point')
            if value < 0:
                raise ValueError(f'Lifetime parameter ({key}) needs to be positive')

        # Everything ok!
        super().__setattr__(key, value)

    def __repr__(self):
        return (
            f"KaonPhysics(\n"
            f"  kplus_branching_ratios={self.kplus_branching_ratios},\n"
            f"  kminus_branching_ratios={self.kminus_branching_ratios},\n"
            f"  k0l_branching_ratios={self.k0l_branching_ratios},\n"
            f"  k0s_branching_ratios={self.k0s_branching_ratios},\n"
            f"  kplus_lifetime_factor={self.kplus_lifetime_factor},\n"
            f"  kminus_lifetime_factor={self.kminus_lifetime_factor},\n"
            f"  k0l_lifetime_factor={self.k0l_lifetime_factor},\n"
            f"  k0s_lifetime_factor={self.k0s_lifetime_factor}\n"
            f")"
        )
