from LDMX.Framework import Processor, processor

"""Configuration for RecoilFiducialityProcessor

Sets all parameters to defaults.

Attributes:
-------------
min_p_mag : double
    Minimum energy of the recoil electron at production.
min_tracker_hits: int
    Minimum number of recoil electron hits in the recoil tracker.
ecal_collection : string
    Name of the Ecal hit collection.
hcal_collection : string
    Name of the Hcal hit collection.
recoil_collection : string
    Name of the recoil tracker hit collection.
output_collection : string
    Name of the output collection containing the fiduciality flags
"""


@processor("recon::RecoilFiducialityProcessor", "Recon")
class RecoilFiducialityProcessor(Processor):

    min_p_mag: float = 50.0
    min_tracker_hits: int = 5
    input_pass_name: str = ""
    ecal_collection: str = "EcalSimHits"
    hcal_collection: str = "HcalSimHits"
    recoil_collection: str = "RecoilSimHits"
    output_collection: str = "RecoilTruthFiducialFlags"
    inverse_skim: bool = False
