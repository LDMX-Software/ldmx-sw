
class KaonPhysics():

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
