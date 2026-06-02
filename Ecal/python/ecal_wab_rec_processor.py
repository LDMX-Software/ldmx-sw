"""Configuration for reconstructing WABs in the Ecal"""

from LDMX.Framework import Processor, processor


@processor("ecal::EcalWABRecProcessor", "Ecal")
class EcalWABRecProcessor(Processor):
    sp_pass_name: str = ""
    collection_name: str = "EcalWABRec"
    rec_coll_name: str = "EcalRecHits"
    rec_pass_name: str = ""
    track_coll_name: str = "LinearRecoilTracks"
    track_pass_name: str = ""
    sim_particles_passname: str = ""
