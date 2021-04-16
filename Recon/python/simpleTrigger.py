"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Attributes:
------------- 
beamEnergy : float
    The beam energy in MeV
thresholds : list of floats
    The upper limit on Ecal reconstructed hit energy sum (in MeV) allowed for the event to pass the trigger. 
    Calculated as a sum over the specified number of layers. 
    The processor assumes that the first element in the list is for 1e, 2nd is for 2e, etc:
    [ my_cut_for_1e, my_cut_for_2e, ... ]
    If the electron count exceeds the number of elements, the 1e threshold and a simple formula subtracting a multiple of the beam energy is used. 
mode : int
    Legacy parameter for which energy calculation mode to use. Defaults to 0 (layer sums). 
    mode = 1 is for tower sums, currently not implemented. 
start_layer : int
    First layer used in the Ecal energy sum over layers.
end_layer : int
    First layer not used in the Ecal energy sum over layers. So, the last used layer is end_layer-1. 
input_collection : string
    Name of the Ecal hit collection used as input
trigger_collection : string
    Name of the output collection containing the trigger result


Examples
--------
    from LDMX.Recon.simpleTrigger import simpleTrigger
    p.sequence.append( simpleTrigger )
"""

from LDMX.Framework import ldmxcfg

class TriggerProcessor(ldmxcfg.Producer) :
    """Configuration for the (multi-electron aware but simple) trigger on the ECal reco hits"""

    def __init__(self,name) :
        super().__init__(name,'recon::TriggerProcessor','Recon')

        self.beamEnergy = 4000.
        self.thresholds = [ 1500.0, 1000. + self.beamEnergy, 500. + 2*self.beamEnergy, 100. + 3*self.beamEnergy ]  #toy something up 
        self.mode = 0
        self.start_layer = 1
        self.end_layer = 20
        self.input_collection = "EcalRecHits"
        self.trigger_collection = "Trigger"

simpleTrigger = TriggerProcessor("simpleTrigger")

