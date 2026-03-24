"""configuration for event filtering user actions"""

from LDMX.SimCore.user_actions import UserAction, user_action


@user_action("biasing::TargetBremFilter", "Biasing")
class TargetBremFilter(UserAction):
    """Configuration for filtering events that don't see a hard brem in the target.

    An event is vetoed if one of two conditions is satisfied:
    1) The recoil electron exits the target area with an energy above 1500 MeV
    2) The recoil electorn brems, but the energy of at least one of the brems
    isn't above 2500 MeV.

    Parameters
    ----------
    recoil_max_p_threshold : float
        Maximum momentum the recoil electron can have [MeV]
    brem_min_energy_threshold : float
        Minimum energy the brem photon can have [MeV]

    Attributes
    ----------
    kill_recoil_track : bool
        Should we kill the recoil electron track for a worst case scenario?
    """

    recoil_max_p_threshold: float
    brem_min_energy_threshold: float
    kill_recoil_track: bool = False


@user_action("biasing::NonFiducialFilter", "Biasing")
class NonFiducialFilter(UserAction):
    """Configuration for rejecting events that are fiducial.

    Parameters
    ----------
    recoil_max_p : float
        Maximum momentum the recoil electron can have [MeV]
    abort_fiducial: bool
        If true, aborts fiducial events. Otherwise, the fiducial events will be
        only tagged
    """

    recoil_max_p: float
    abort_fiducial: bool = True


@user_action("biasing::EcalProcessFilter", "Biasing")
class EcalProcessFilter(UserAction):
    """Configuration for filtering events that don't see a hard brem
    undergo a photo-nuclear reaction in the ECal.

    Parameters
    ----------
    process : str
        Geant4 process to look for in the ecal
    """

    process: str = "photonNuclear"


@user_action("biasing::DeepEcalProcessFilter", "Biasing")
class DeepEcalProcessFilter(UserAction):
    """Configuration for keeping events where the pn happens deep in the ECAL.

    Parameters
    ----------
    bias_threshold: double
        Threshold for minimum energy that the products should have
    processes: vector of str
        The allowed processes that can happen deep inside the ECAL,
        default is conversion (conv) and photoelectron (photo)
    ecal_min_z: double
        Minimum Z location where the deep process should happen
    require_photon_from_target: bool
        Require that the hard brem photon originates from the target
        Default is False
    """

    bias_threshold: float
    processes: list[str]
    ecal_min_z: float
    require_photon_from_target: bool


@user_action("biasing::TargetENProcessFilter", "Biasing")
class TargetENFilter(UserAction):
    """Configuration for filtering electro-nuclear events in the target.

    Parameters
    ----------
    recoil_thresh : float
        Maximum energy recoil electron is allowed to have [MeV]
    """

    recoil_thresh: float


@user_action("biasing::TargetProcessFilter", "Biasing")
class TargetProcessFilter(UserAction):
    """Configuration for filtering photo-nuclear events in the target."""

    process: str = "photonNuclear"

    def photo_nuclear():
        return TargetProcessFilter("photonNuclear")

    def gamma_mu_mu():
        return TargetProcessFilter("GammaToMuPair")


    def aprime_to_fcp():
        return TargetProcessFilter("APrimeToFCPPair")

    def gamma_to_fcp():
        return TargetProcessFilter("GammaToFCPPair")


@user_action("biasing::EcalDarkBremFilter", "Biasing")
class EcalDarkBremFilter(UserAction):
    """Configuration for filtering A' events

    Parameters
    ----------
    threshold : float
        Minimum A' energy to keep the event [MeV]
    """

    threshold: float


@user_action("biasing::TargetDarkBremFilter", "Biasing")
class TargetDarkBremFilter(UserAction):
    """Configuration for filtering A' events

    Parameters
    ----------
    threshold : float
        Minimum A' energy to keep the event [MeV]
    """

    threshold: float


@user_action("biasing::TaggerVetoFilter", "Biasing")
class TaggerVetoFilter(UserAction):
    """Configuration used to reject off-energy electrons in the tagger tracker.

    Parameters
    ----------
    threshold : float
        Minimum energy [MeV] that electron should have
    reject_events_missing_tagger : bool
        Also veto events where the primary particle misses the tagger region
    """

    threshold: float
    reject_events_missing_tagger: bool = True


@user_action("biasing::PrimaryToEcalFilter", "Biasing")
class PrimaryToEcalFilter(UserAction):
    """Configuration used to reject events where the primary doesn't reach the ecal
    with a mimimum energy

    Parameters
    ----------
    threshold : float
        Minimum energy [MeV] that primary electron should have when hitting ecal
    """

    threshold: float


@user_action("biasing::MidShowerNuclearBkgdFilter", "Biasing")
class MidShowerNuclearBkgdFilter(UserAction):
    """Configuration used to reject events that don't have enough energy given to the
    input process.

    Parameters
    ----------
    threshold : float
        Minimum energy [MeV] that the kinetic energy of the products needs to sum to
    """

    threshold: float


@user_action("biasing::MidShowerDiMuonBkgdFilter", "Biasing")
class MidShowerDiMuonBkgdFilter(UserAction):
    """Configuration used to reject events that don't have enough energy given to
    muon conversion

    Parameters
    ----------
    threshold : float
        Minimum energy [MeV] that needs to go into creating the muons
    """

    threshold: float


@user_action("biasing::TaggerHitFilter", "Biasing")
class TaggerHitFilter(UserAction):
    """Configuration used to reject off-energy electrons in the tagger tracker.

    Parameters
    ----------
    layers_hit : int
        Minimum number of tagger layers with a hit needed to persist the event.
    """

    layers_hit: int = 8
