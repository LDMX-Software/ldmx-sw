"""Configuration for DeepTriggerProcessor

Sets all parameters to reasonable defaults.

Attributes:
-------------
beam_energy : float
    The beam energy in MeV
thresholds : list of floats
    The lower limit on Ecal reconstructed hit energy sum (in MeV) required for
    the event to pass the trigger.
    Calculated as a sum over the specified number of layers.
    The processor assumes that the first element in the list is for 1e,
    2nd is for 2e, etc:
    [ my_cut_for_1e, my_cut_for_2e, ... ]
    If the electron count exceeds the number of elements, the 1e threshold
    and a simple formula adding a multiple of the beam energy is used.
mode : int
    Legacy parameter for which energy calculation mode to use.
    Defaults to 0 (layer sums).
    mode = 1 is for tower sums, currently not implemented.
start_layer : int
    First layer used in the Ecal energy sum over layers.
end_layer : int
    First layer not used in the Ecal energy sum over layers.
    So, the last used layer is end_layer-1.
input_collection : string
    Name of the Ecal hit collection used as input
trigger_collection : string
    Name of the output collection containing the trigger result


Examples
--------
    from LDMX.Recon.deep_trigger import deep_trigger
    p.sequence.append( deep_trigger )
"""

from LDMX.Framework import Processor, processor


@processor("recon::DeepTriggerProcessor", "Recon")
class DeepTriggerProcessor(Processor):
    """Configuration for the (multi-electron aware but simple) deep-layer
    trigger on the ECal reco hits"""

    beam_energy: float = 8000.0
    thresholds: list[float] = []
    mode: int = 0
    start_layer: int = 25
    end_layer: int = 32
    input_collection: str = "EcalRecHits"
    input_pass: str = ""
    trigger_collection: str = "TriggerDeep"

    def __post_init__(self):
        if len(self.thresholds) == 0:
            self.thresholds = [100.0, 100.0, 100.0, 100.0]


deep_trigger = DeepTriggerProcessor(
    beam_energy=8000.0,
    instance_name="deep_trigger",
)
