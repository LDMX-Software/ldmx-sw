"""Configuration for Visibles veto

Examples
--------
   from LDMX.Hcal.visibles import VisiblesVetoProcessor
   p.sequence.append( VisiblesVetoProcessor)
"""

import os
import sys

from LDMX.Framework import ldmxcfg


def makeBDTPath(bdt_name) :
    """ Get the full path to the installed BDT files
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

    full_path = '@CMAKE_INSTALL_PREFIX@/data/Hcal/' + bdt_name + '.onnx'
    if not os.path.isfile(full_path) :
        print('ERROR: ONNX model file \'%s\' does not exist.' % (full_path))
        sys.exit(1)

    return full_path

class VisiblesVetoProcessor(ldmxcfg.Producer) :
    """Configuration for visibles veto"""

    def __init__(self, name = 'visiblesVeto') :
        super().__init__(name, "hcal::VisiblesVetoProcessor", "Visibles")

        self.verbose = False
        self.feature_list_name = "input"
        self.bdt_file = makeBDTPath("visibles")
        self.beam_energy = 8000.0 # in MeV
        self.disc_cut = 0.999965
        self.collection_name = "VisiblesVeto"
        self.rec_coll_name = "HcalRecHits"
        self.rec_pass_name = ''
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.track_pass_name = ''
        self.sp_coll_name = 'TargetScoringPlaneHits'
        self.sp_pass_name = ''
        self.sim_particles_pass_name = ''
