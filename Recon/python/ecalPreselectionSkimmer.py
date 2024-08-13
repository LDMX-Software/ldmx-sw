"""Configuration for PreselectionProcessor

Sets all the default parameters that high so it leads to no preselection.

Attributes:
------------- 

ecal_veto_name: string
    Collection Name for veto object
ecal_veto_pass: string
    Pass Name for veto object
summed_det_max: double
    Max value for summed det
summed_tight_iso_max: double
   Max value for summed tigh iso
ecal_back_energy_max: double
    Max value for ecal back energy
 n_readout_hits_max: int
    Max value for num readout hits
 shower_rms_max: int
    Max value for shower rms
 shower_y_std_max: int
   Max value for shower rms in Y
shower_x_std_max: int
    Max value for shower rms in X
max_cell_dep_max: double
    Max value for maximal cell deposition
std_layer_hit_max: int
    Max value for std layer hits
n_straight_tracks_max: int
    Max value for num straight tracks


Examples
--------
    from LDMX.Recon.ecalPreselectionSkimmer import EcalPreselectionSkimmer
    ecal_pres_skimmer = EcalPreselectionSkimmer()
    p.sequence.append( ecal_pres_skimmer )
"""

from LDMX.Framework import ldmxcfg

class EcalPreselectionSkimmer(ldmxcfg.Producer) :
    """Configuration for an ECAL-based pre-selection skimmer"""

    def __init__(self, name = "ecalPreselectionSkimmer") :
        super().__init__(name,'recon::EcalPreselectionSkimmer','Recon')

        self.ecal_veto_name = "EcalVeto"
        self.ecal_veto_pass = ""
        self.summed_det_max = 9999.
        self.summed_tight_iso_max = 9999.
        self.ecal_back_energy_max = 9999.
        self.n_readout_hits_max = 9999
        self.shower_rms_max = 9999
        self.shower_y_std_max = 9999
        self.shower_x_std_max = 9999
        self.max_cell_dep_max = 9999.
        self.std_layer_hit_max = 9999
        self.n_straight_tracks_max = 9999
