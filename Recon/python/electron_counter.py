"""Configuration for ElectronCounter

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon.electron_counter import electron_counter
    p.sequence.append( electron_counter )
"""

from LDMX.Framework import ldmxcfg


class ElectronCounter(ldmxcfg.Producer) :
    """Configuration for the event beam electron counter"""

    def __init__(self,n_sim_beam_electrons,name="ElectronCounter") :
        super().__init__(name,'recon::ElectronCounter','Recon')

        self.input_collection = "TriggerPadTracksY"
        self.input_pass_name = "truth"
        self.output_collection = "BeamElectronCount"
        self.simulated_electron_number = n_sim_beam_electrons
        self.use_simulated_electron_number = False
electron_counter = ElectronCounter(1, "ElectronCounter")

