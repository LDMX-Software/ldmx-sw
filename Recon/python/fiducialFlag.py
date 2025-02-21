from LDMX.Framework import ldmxcfg

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

class RecoilFiducialityProcessor(ldmxcfg.Producer) :

    def __init__(self, name) :
        super().__init__(name,'recon::RecoilFiducialityProcessor','Recon')

        self.min_p_mag = 50. # MeV
        self.min_tracker_hits = 5
        self.ecal_collection = "EcalSimHits"
        self.hcal_collection = "HcalSimHits"
        self.recoil_collection = "RecoilSimHits"
        self.output_collection = "RecoilTruthFiducialFlags"