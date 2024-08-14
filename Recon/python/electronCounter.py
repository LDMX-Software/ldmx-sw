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

        self.input_collection = "TriggerPadTracksY"
        self.input_pass_name = "truth"
        self.output_collection = "BeamElectronCount"
        self.simulated_electron_number = nSimBeamElectrons
        self.use_simulated_electron_number = False
electronCounter = ElectronCounter(1, "ElectronCounter")

class ElectronCounter2(ldmxcfg.Producer) :
    """Configuration for a test beam electron counter combining
    TS track position information with Ecal cluster position information"""

    def __init__(self, name="ElectronCounter2") :
        super().__init__(name,'recon::ElectronCounter2','Recon')

        self.input_collections = ["TriggerPadTracksY", "ecalClusters"]
        self.input_pass_name = ""   # Take any pass
        self.output_collection = "BeamElectronCount2"
        self.x_tolerance = 0.
        self.y_tolerance = 0.
        self.ecal_position_shift_xy = [-4.8158640226/2., 0.]
        self.ecal_peak_mean = [7000., 14000., 21500., 29500.] # Ecal mean of peak of 1e, 2e, 3e and 4e total energy distribution
        self.ts_peak_mean = [85., 170., 255., 340.]           # TS mean of peak of 1e, 2e, 3e, 4e total PE distribution
        self.verbose = True