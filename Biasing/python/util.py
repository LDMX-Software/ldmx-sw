"""Utility classes for Simulation"""

from LDMX.SimCore.user_action import UserAction, user_action


def biasing_utility(class_name: str):
    return user_action(
        instance_name = class_name,
        class_name = f'biasing::utility::{class_name}',
        module_name = 'Biasing_Utility'
    )


@biasing_utility("StepPrinter")
class StepPrinter(UserAction) :
    """Print each step of the input track ID

    The default track ID is 1 (the primary particle).

    Parameters
    ----------
    track_id : int, optional
        Geant4 track ID to print each step of
    """

    process_name: str = ""
    track_id: int = 1
    depth: int = 0


@biasing_utility("PartialEnergySorter")
class PartialEnergySorter(UserAction) :
    """Process particles such that all particles above
    the input threshold are processed first.

    Parameters
    ----------
    threshold : float
        Minimum energy [MeV] to process a track first
    """

    threshold: float


@biasing_utility("TrackProcessFilter")
class TrackProcessFilter(UserAction):
    """ Configuration used to tag all tracks produced via the given process to persist them to the event.

    Parameters
    ----------
    process_name : str
        The Geant4 process name (e.g. photonNuclear) via which the tracks were produced. 
    """

    process_name: str

    def __post_init__(self) :
        self.instance_name = f"{self.process_name}_track_filter"


    def photo_nuclear() :
        """ Configuration used to tag all photo-nuclear tracks to persist them to the event. 
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag photo-nuclear tracks.
        """
        return TrackProcessFilter('photonNuclear')


    def electro_nuclear() :
        """ Configuration used to tag all electro-nuclear tracks to persist them to the event. 
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag electro-nuclear tracks.
    
        """
        return TrackProcessFilter('electronNuclear')


    def electron_brem() :
        """ Configuration used to tag all electron brem tracks to persist them to the event.
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag eBrem tracks.
    
        """
        return TrackProcessFilter('eBrem')


    def conversion() :
        """ Configuration used to tag all electron conversion tracks to persist them to the event.
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag conversion tracks.
    
        """
        return TrackProcessFilter('conv')


    def dark_brem() :
        """ Configuration used to tag all dark brem tracks to persist them to the event. 
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag dark brem tracks.
    
        """
        return TrackProcessFilter('DarkBrem')

    def gamma_mumu() :
        """ Configuration used to tag all gamma --> mu+ mu- tracks to persist them to the event.

        Return
        ------
        Instance of TrackProcessFilter configured to tag gamma --> mu+ mu- tracks.
        """
        return TrackProcessFilter('GammaToMuPair')


@biasing_utility
class DecayChildrenKeeper(UserAction):
    """ Configuration used to store children of specific particle decays

    Parameters
    ----------
    parents : list[int]
        list of PDG ID of particles whose decay products we want to keep
    """

    parents: list[int]
