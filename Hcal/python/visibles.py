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

class VisiblesFeatureProducer(ldmxcfg.Analyzer) :
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

class VisiblesCutflow(ldmxcfg.Analyzer) :
    """Just plot the visibles features"""

    def __init__(self, name='vis') :
        super().__init__(name, 'hcal::VisiblesCutflow', 'Hcal')

        self.bdt_file = makeBDTPath("visibles_v2")
        self.feature_list_name = "float_input"
        self.disc_cut = 0.999965
       
        self.beam_energy = 8000.0
        
        self.hcal_rec_coll_name = "HcalRecHits"
        self.ecal_rec_coll_name = "EcalRecHits"
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.sp_coll_name = 'TargetScoringPlaneHits'

        self.ecal_veto_coll_name = "EcalVeto"
        self.ecal_veto_pass_name = ''
        self.ecal_disc_cut = 0.99741

        ## Define histograms ##

        self.build1DHistogram("totalevents", "totalevents", 40, 0, 2000)
        self.build1DHistogram("acceptance", "acceptance", 40, 0, 2000)
        self.build1DHistogram("passtrigger", "passtrigger", 40, 0, 2000)
        self.build1DHistogram("ecalenergy", "ecalenergy", 40, 0, 2000)
        self.build1DHistogram("passTrackerVeto", "passTrackerVeto", 40, 0, 2000)
        self.build1DHistogram("passEcalBDT", "passEcalBDT", 40, 0, 2000)
        self.build1DHistogram("hcalEnergyReq", "hcalEnergyReq", 40, 0, 2000)
        self.build1DHistogram("containment", "containment", 40, 0, 2000)
        self.build1DHistogram("passVisiblesBDT", "passVisiblesBDT", 40, 0, 2000)
        self.build1DHistogram("visiblesDisc", "visiblesDisc", 100, 0, 1)
        self.build1DHistogram("visiblesDiscHigh", "visiblesDiscHigh", 10000, 0.999, 1)
        self.build1DHistogram("visiblesDiscHighNorm", "visiblesDiscHighNorm", 10000, 0.999, 1)
        self.build2DHistogram("ecalDiscvsVisDisc", "visDisc", 1000, 0.9999, 1, "ecalDisc", 1000, 0.999, 1)

        self.build1DHistogram("beamEnergyFrac", "beamEnergyFrac", 100, 0.5, 1)
        self.build1DHistogram("beamAngle", "beamAngle", 100, 0, 0.5)

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
