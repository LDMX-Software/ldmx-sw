"""Configuration for ElectronCounter

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon.electronCounter import electronCounter
    p.sequence.append( electronCounter )
"""

from LDMX.Framework import ldmxcfg

class ElectronCounter(ldmxcfg.Producer) :
    """Configuration for the event beam electron counter"""

    def __init__(self,nSimBeamElectrons,name="ElectronCounter") :
        super().__init__(name,'recon::ElectronCounter','Recon')

        self.input_collection = "TriggerPadTracks"
        self.input_pass_name = "truth"
        self.output_collection = "BeamElectronCount"
        self.simulated_electron_number = nSimBeamElectrons
        self.use_simulated_electron_number = False
electronCounter = ElectronCounter(1, "ElectronCounter")

