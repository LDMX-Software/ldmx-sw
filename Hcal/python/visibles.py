"""Configuration for Visibles veto

Examples
--------
   from LDMX.Hcal.visibles import VisiblesVetoProcessor
   p.sequence.append( VisiblesVetoProcessor)
"""

import os
import sys

from LDMX.Framework import Processor, processor


def make_bdt_path(bdt_name):
    """Get the full path to the installed BDT files
    Exits entire python script if the file does not exist.

    Parameters
    ----------
    bdt_name : str
       Name of BDT file (no extension)

    Returns
    -------
    str
       full path to installed BDT file

    Examples
    --------
       visiblesVeto.bdt_file = makeBDTPath('visibles_v1')
    """

    full_path = "@CMAKE_INSTALL_PREFIX@/data/Hcal/" + bdt_name + ".onnx"
    if not os.path.isfile(full_path):
        print(f"ERROR: ONNX model file '{full_path}' does not exist.")
        sys.exit(1)

    return full_path


@processor("hcal::VisiblesVetoProcessor", "Hcal")
class VisiblesVetoProcessor(Processor):
    feature_list_name: str = "input"
    bdt_file: str = make_bdt_path("visibles")
    beam_energy: float = 8000.0  # in MeV
    disc_cut: float = 0.999965
    collection_name: str = "VisiblesVeto"
    rec_coll_name: str = "HcalRecHits"
    rec_pass_name: str = ""
    recoil_from_tracking: bool = False
    track_collection: str = "RecoilTracks"
    track_pass_name: str = ""
    sp_coll_name: str = "TargetScoringPlaneHits"
    sp_pass_name: str = ""
    sim_particles_pass_name: str = ""
