
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

