"""Configuration for Visibles veto

Examples
--------
   from LDMX.Hcal.visibles import VisiblesVetoProcessor
   p.sequence.append( VisiblesVetoProcessor)
"""

from LDMX.Framework import ldmxcfg
import os, sys

def makeBDTPath(BDTname) :
    """ Get the full path to the installed BDT files
    Exits entire python script if the file does not exist.

    Parameters
    ----------
    BDTname : str
       Name of BDT file (no extension)

    Returns
    -------
    str
       full path to installed BDT file

    Examples
    --------
       visiblesVeto.bdt_file = makeBDTPath('visibles_v1')
    """

    fullPath = '@CMAKE_INSTALL_PREFIX@/data/Hcal/' + BDTname + '.onnx'
    if not os.path.isfile(fullPath) :
        print('ERROR: ONNX model file \'%s\' does not exist.' % (fullPath))
        sys.exit(1)

    return fullPath

class VisiblesVetoProcessor(ldmxcfg.Producer) :
    """Configuration for visibles veto"""

    def __init__(self, name = 'visiblesVeto') :
        super().__init__(name, "hcal::VisiblesVetoProcessor", "Visibles")

        self.verbose = False
        self.feature_list_name = "input"
        self.bdt_file = makeBDTPath("visibles_v1")
        self.beam_energy = 8000.0 # in MeV
        self.disc_cut = 0.99
        self.collection_name = "VisiblesVeto"
        self.rec_coll_name = "HcalRecHits"
        self.rec_pass_name = ''
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
