"""Configuration for PreselectionProcessor

Sets all the default parameters that high so it leads to no preselection.

Attributes:
-------------

use_rechits: bool
    If True, use rechit-based preselection.
    If False, use veto-based preselection (default: False)

Rechit-based mode parameters:
ecal_rec_hit_coll: string
    Collection Name for ecal rechits
ecal_rec_hit_pass: string
    Pass Name for ecal rechits

Veto-based mode parameters:
ecal_veto_name: string
    Collection Name for veto object
ecal_veto_pass: string
    Pass Name for veto object
ecal_mip_name: string
    Collection Name for mip result object
ecal_mip_pass: string
    Pass Name for mip result object

Shared parameters (used in both modes):
summed_det_max: double
    Max value for summed det (veto mode) or total rechit energy sum (rechit mode, MeV)
 n_readout_hits_max: int
    Max value for num readout hits (veto mode) or num rechits (rechit mode)
summed_tight_iso_max: double
   Max value for summed tigh iso (veto mode only)
ecal_back_energy_max: double
    Max value for ecal back energy (veto mode only)
 shower_rms_max: double
    Max value for shower rms
 shower_y_std_max: double
   Max value for shower rms in Y
shower_x_std_max: double
    Max value for shower rms in X
max_cell_dep_max: double
    Max value for maximal cell deposition
std_layer_hit_max: int
    Max value for std layer hits
n_straight_tracks_max: int
    Max value for num straight tracks
bdt_disc_min: double
    Min value for the BDT disc variable
fiducial_level: int
    0: don't care if it's fiducial or not,
    1: keep fiducial events only,
    2: keep non-fid events only



Examples
--------
    from LDMX.Recon.ecalPreselectionSkimmer import EcalPreselectionSkimmer

    # Veto-based mode (default):
    ecal_pres_skimmer = EcalPreselectionSkimmer()
    p.sequence.append( ecal_pres_skimmer )

    # Rechit-based mode:
    ecal_pres_skimmer = EcalPreselectionSkimmer()
    ecal_pres_skimmer.use_rechits = True
    ecal_pres_skimmer.ecal_rec_hit_coll = "EcalRecHits"
    ecal_pres_skimmer.ecal_rec_hit_pass = ""
    ecal_pres_skimmer.summed_det_max = 4000.0  # MeV (total rechit energy)
    ecal_pres_skimmer.n_readout_hits_max = 90  # num rechits
    p.sequence.append( ecal_pres_skimmer )
"""

from LDMX.Framework import Processor, processor


@processor("recon::EcalPreselectionSkimmer", "Recon")
class EcalPreselectionSkimmer(Processor):
    """Configuration for an ECAL-based pre-selection skimmer"""

    # Mode selection (default: rechit-based)
    use_rechits: bool = True

    # Rechit-based parameters
    ecal_rec_hit_coll: str = "EcalRecHits"
    ecal_rec_hit_pass: str = ""

    # Veto-based parameters
    ecal_veto_name: str = "EcalVeto"
    ecal_veto_pass: str = ""
    ecal_mip_name: str = "EcalMipInfo"
    ecal_mip_pass: str = ""

    # Cut values
    summed_det_max: float = 4000.0
    summed_tight_iso_max: float = 9999.0
    ecal_back_energy_max: float = 9999.0
    n_readout_hits_max: int = 90
    shower_rms_max: float = 9999.0
    shower_y_std_max: float = 9999.0
    shower_x_std_max: float = 9999.0
    max_cell_dep_max: float = 9999.0
    std_layer_hit_max: int = 9999
    n_straight_tracks_max: int = 9999
    bdt_disc_min: float = 0.0
    fiducial_level: int = 0
