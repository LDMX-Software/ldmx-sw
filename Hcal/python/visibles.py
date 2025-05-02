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
        self.disc_cut = 0.9999825
        self.collection_name = "VisiblesVeto"
        self.rec_coll_name = "HcalRecHits"
        self.rec_pass_name = ''
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.track_pass_name = ''
        self.sp_coll_name = 'TargetScoringPlaneHits'
        self.sp_pass_name = ''

class GetVisiblesFeatures(ldmxcfg.Analyzer) :
    """Just plot the visibles features"""

    def __init__(self, name='vis') :
        super().__init__(name, 'hcal::VisiblesFeatureProducer', 'Hcal')

        ## Parameters choose whether to save features to .txt file ##
        self.training = False
        self.training_file = ""

        ## A few other useful parameters ##
        self.beam_energy = 8000.0 # in MeV
        self.hcal_rec_coll_name = "HcalRecHits"
        self.ecal_rec_coll_name = "EcalRecHits"
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.sp_coll_name = 'TargetScoringPlaneHits'
        
        ## Define histograms ##

        self.build1DHistogram("layershit", "layershit", 100, 0, 100);
        self.build1DHistogram("xStd", "xStd", 80, 0, 800);
        self.build1DHistogram("yStd", "yStd", 80, 0, 800);
        self.build1DHistogram("zStd", "zStd", 100, 0, 1000)
        self.build1DHistogram("xMean", "xMeean", 80, -800, 800)
        self.build1DHistogram("yMean", "yMean", 80, -800, 800)
        self.build1DHistogram("rMean", "rMean", 80, 0, 800)
        self.build1DHistogram("isoHits", "isoHits", 100, 0, 100)
        self.build1DHistogram("isoE", "isoE", 80, 0, 800)
        self.build1DHistogram("nHits", "nHits", 200, 0, 200)
        self.build1DHistogram("Etot", "Etot", 100, 4800, 9800)
        self.build1DHistogram("photonProj", "photonProj", 80, 0, 800)
