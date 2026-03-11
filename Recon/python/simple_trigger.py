"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Attributes:
-------------
beam_energy : float
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
    from LDMX.Recon.simple_trigger import simple_trigger
    p.sequence.append( simple_trigger )
"""

from LDMX.Framework import Processor, processor


@processor("recon::TriggerProcessor", "Recon")
class TriggerProcessor(Processor):
    """Configuration for the (multi-electron aware but simple) trigger on the ECal reco hits"""

    beam_energy: float
    thresholds: list[float] = []
    mode: int = 0
    start_layer: int = 0
    end_layer: int = 20
    input_collection: str = "EcalRecHits"
    input_pass: str = ""
    trigger_collection: str = "Trigger"

    def __post_init__(self):
        if len(self.thresholds) == 0:
            if self.beam_energy == 4000.0:
                self.thresholds = [1500.0, 5000.0, 8200.0, 11800.0]
            else:
                self.thresholds = [3000.0, 10790.0, 18540.0, 26250.0]


simple_trigger = TriggerProcessor(
    beam_energy=8000.0,
    instance_name="simple_trigger",
)
