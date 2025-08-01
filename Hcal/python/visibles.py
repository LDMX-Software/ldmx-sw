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

class VisiblesFeatureProducer(ldmxcfg.Analyzer) :
    """Just plot the visibles features"""

    def __init__(self, name='vis') :
        super().__init__(name, 'hcal::VisiblesFeatureProducer', 'Hcal')

        ## Parameters choose whether to save features to .txt file ##
        ## Useful for training a Python-based BDT                  ##
        self.training = False
        self.training_file = ""

        ## A few other useful parameters ##
        self.beam_energy = 8000.0 # in MeV
        self.hcal_rec_coll_name = "HcalRecHits"
        self.hcal_rec_pass_name = ''
        self.ecal_rec_coll_name = "EcalRecHits"
        self.ecal_rec_pass_name = ''
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.track_pass_name = ''
        self.sp_coll_name = 'TargetScoringPlaneHits'
        self.sp_pass_name = ''
        self.sim_particles_pass_name = ''
        
        ## Define histograms ##

        self.build1DHistogram("layers_hit", "layers_hit", 100, 0, 100);
        self.build1DHistogram("x_std", "x_std", 80, 0, 800);
        self.build1DHistogram("y_std", "y_std", 80, 0, 800);
        self.build1DHistogram("z_std", "z_std", 100, 0, 1000)
        self.build1DHistogram("x_mean", "x_meean", 80, -800, 800)
        self.build1DHistogram("y_mean", "y_mean", 80, -800, 800)
        self.build1DHistogram("r_mean", "r_mean", 80, 0, 800)
        self.build1DHistogram("iso_hits", "iso_hits", 100, 0, 100)
        self.build1DHistogram("iso_energy", "iso_energy", 80, 0, 800)
        self.build1DHistogram("n_hits", "n_hits", 200, 0, 200)
        self.build1DHistogram("total_energy", "total_energy", 100, 4800, 9800)
        self.build1DHistogram("photon_track", "photon_track", 80, 0, 800)

class VisiblesCutflow(ldmxcfg.Analyzer) :
    """Just plot the visibles features"""

    def __init__(self, name='vis') :
        super().__init__(name, 'hcal::VisiblesCutflow', 'Hcal')

        self.bdt_file = makeBDTPath("visibles_v2")
        self.feature_list_name = "float_input"
        self.disc_cut = 0.999965
       
        self.beam_energy = 8000.0
        
        self.hcal_rec_coll_name = "HcalRecHits"
        self.hcal_rec_pass_name = ''
        self.ecal_rec_coll_name = "EcalRecHits"
        self.ecal_rec_pass_name = ''
        self.recoil_from_tracking = False
        self.track_collection = 'RecoilTracks'
        self.track_pass_name = ''
        self.sp_coll_name = 'TargetScoringPlaneHits'
        self.sp_pass_name = ''
        self.sim_particles_pass_name = ''

        self.ecal_veto_coll_name = "EcalVeto"
        self.ecal_veto_pass_name = ''
        self.ecal_disc_cut = 0.99741 # assumes SegMip disc

        ## Histograms for efficiency ##
        self.build1DHistogram("total_events", "total_events", 40, 0, 2000)
        self.build1DHistogram("pass_acceptance", "pass_acceptance", 40, 0, 2000)
        self.build1DHistogram("pass_trigger", "pass_trigger", 40, 0, 2000)
        self.build1DHistogram("pass_ecal_energy", "pass_ecal_energy", 40, 0, 2000)
        self.build1DHistogram("pass_tracker_veto", "pass_tracker_veto", 40, 0, 2000)
        self.build1DHistogram("pass_ecal_bdt", "pass_ecal_bdt", 40, 0, 2000)
        self.build1DHistogram("pass_hcal_energy", "pass_hcal_energy", 40, 0, 2000)
        self.build1DHistogram("pass_hcal_containment", "pass_hcal_containment", 40, 0, 2000)
        self.build1DHistogram("pass_visibles_bdt", "pass_visibles_bdt", 40, 0, 2000)

        ## Histograms for BDT performance plots ##
        self.build1DHistogram("visibles_disc", "visibles_disc", 100, 0, 1)
        self.build1DHistogram("visibles_disc_high", "visibles_disc_high", 10000, 0.999, 1)
        self.build1DHistogram("visibles_disc_high_norm", "visibles_disc_high_norm", 10000, 0.999, 1)
        self.build2DHistogram("ecal_disc_vs_vis_disc", "vis_disc", 1000, 0.9999, 1, "ecal_disc", 1000, 0.999, 1)
        self.build1DHistogram("roc", "roc", 10000, 0.99, 1)

        ## LLP kinematics histograms ##
        self.build1DHistogram("beam_energy_frac", "beam_energy_frac", 100, 0.5, 1)
        self.build1DHistogram("beam_angle", "beam_angle", 100, 0, 0.5)

        ## BDT feature histograms ##
        self.build1DHistogram("layers_hit", "layers_hit", 100, 0, 100);
        self.build1DHistogram("x_std", "x_std", 80, 0, 800);
        self.build1DHistogram("y_std", "y_std", 80, 0, 800);
        self.build1DHistogram("z_std", "z_std", 100, 0, 1000)
        self.build1DHistogram("x_mean", "x_mean", 80, -800, 800)
        self.build1DHistogram("y_mean", "y_mean", 80, -800, 800)
        self.build1DHistogram("r_mean", "r_mean", 80, 0, 800)
        self.build1DHistogram("iso_hits", "iso_hits", 100, 0, 100)
        self.build1DHistogram("iso_energy", "iso_energy", 80, 0, 800)
        self.build1DHistogram("n_hits", "n_hits", 200, 0, 200)
        self.build1DHistogram("total_energy", "total_energy", 100, 4800, 9800)
        self.build1DHistogram("photon_track", "photon_track", 80, 0, 800)
