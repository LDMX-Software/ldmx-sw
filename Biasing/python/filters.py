""" filters
Examples of how each filter is configured for biased MC generation. 

    The configurations below reflect the nominal values currently being used 
    for large scale production.  When debugging, it's best to create these
    generators directly. 
"""

from LDMX.SimCore import simcfg

class TargetBremFilter(simcfg.UserAction):
    """ Configuration for filtering events that don't see a hard brem in the target. 

    An event is vetoed if one of two conditions is satisfied:
    1) The recoil electron exits the target area with an energy above 1500 MeV
    2) The recoil electorn brems, but the energy of at least one of the brems
    isn't above 2500 MeV. 

    Parameters
    ----------
    recoil_max_p : float
        Maximum momentum the recoil electron can have [MeV]
    brem_min_e : float
        Minimum energy the brem photon can have [MeV]

    Attributes
    ----------
    kill_recoil_track : bool
        Should we kill the recoil electron track for a worst case scenario?
    """

    def __init__(self,recoil_max_p = 1500.,brem_min_e = 2500.) :
        super().__init__("target_brem_filter", "ldmx::TargetBremFilter")

        from LDMX.Biasing import include
        include.library()

        self.recoil_max_p_threshold = recoil_max_p
        self.brem_min_energy_threshold = brem_min_e
        self.kill_recoil_track = False

class EcalBremFilter(simcfg.UserAction):
    """ Configuration for filtering events that don't see a hard brem in the target. 

    The event is rejected if the primary electron brems, 
    but the energy of at least one of the brems isn't above 
    the brem_min_energy_threshold [MeV] or if the primary
    doesn't brem at all.

    Parameters
    ----------
    brem_min_e : float
        Minimum energy the brem photon can have [MeV]
    """

    def __init__(self,brem_min_e) :
        super().__init__("ecal_brem_filter", "ldmx::EcalBremFilter")

        from LDMX.Biasing import include
        include.library()

        self.brem_min_energy_threshold = brem_min_e

class EcalProcessFilter(simcfg.UserAction):
    """ Configuration for filtering events that don't see a hard brem undergo a photo-nuclear reaction in the ECal. 

    Parameters
    ----------
    process : str
        Geant4 process to look for in the ecal
    """

    def __init__(self,process = 'photonNuclear') :
        super().__init__('ecal_%s_filter'%process,'ldmx::EcalProcessFilter')

        from LDMX.Biasing import include
        include.library()

        self.process = process

class TargetENFilter(simcfg.UserAction) :
    """ Configuration for filtering electro-nuclear events in the target. 

    Parameters
    ----------
    recoil_thresh : float
        Maximum energy recoil electron is allowed to have [MeV]
    """

    def __init__(self,recoil_thresh = 2500.) :
        super().__init__("target_en_process_filter","ldmx::TargetENProcessFilter")

        from LDMX.Biasing import include
        include.library()

        self.recoilThreshold = recoil_thresh #MeV

class EcalENFilter(simcfg.UserAction) :
    """ Configuration for filtering electro-nuclear events in the ecal.

    Designed similar to EcalBremFilter, this action looks for the primary
    to have a minimum total energy "lost" to EN products within the ecal.

    Parameters
    ----------
    min_en_energy : float
        Minimum total energy of all EN products [MeV]
    """

    def __init__(self,min_en_energy) :
        super().__init__("ecal_en_process_filter","ldmx::EcalENFilter")

        from LDMX.Biasing import include
        include.library()

        self.min_total_en_energy = min_en_energy #MeV

class TargetPNFilter(simcfg.UserAction) :
    """ Configuration for filtering photo-nuclear events in the target."""

    def __init__(self) :
        super().__init__("target_process_filter", "ldmx::TargetProcessFilter")

        from LDMX.Biasing import include
        include.library()

        self.process = 'photonNuclear'

class EcalDarkBremFilter(simcfg.UserAction):
    """ Configuration for filtering A' events

    Parameters
    ----------
    minApEnergy : float
        Minimum A' energy to keep the event [MeV]
    """

    def __init__(self,minApEnergy):
        super().__init__('ecal_db_filter','ldmx::EcalDarkBremFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = minApEnergy

class TargetDarkBremFilter(simcfg.UserAction):
    """ Configuration for filtering A' events

    Parameters
    ----------
    minApEnergy : float
        Minimum A' energy to keep the event [MeV]
    """

    def __init__(self,minApEnergy):
        super().__init__('target_db_filter','ldmx::TargetDarkBremFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = minApEnergy

class TaggerVetoFilter(simcfg.UserAction): 
    """ Configuration used to reject off-energy electrons in the tagger tracker.

    Parameters
    ----------
    thresh : float
        Minimum energy [MeV] that electron should have
    """
    
    def __init__(self,thresh=3800.) :
        super().__init__('tagger_veto_filter','ldmx::TaggerVetoFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = thresh

class PrimaryToEcalFilter(simcfg.UserAction) :
    """ Configuration used to reject events where the primary doesn't reach the ecal with a mimimum energy

    Parameters
    ----------
    thresh : float
        Minimum energy [MeV] that primary electron should have when hitting ecal
    """

    def __init__(self,thresh) :
        super().__init__('primary_to_ecal_with_%d'%thresh,'ldmx::PrimaryToEcalFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = thresh

class TrackProcessFilter(simcfg.UserAction):
    """ Configuration used to tag all tracks produced via the given process to persist them to the event.

    Parameters
    ----------
    process_name : str
        The Geant4 process name (e.g. photonNuclear) via which the tracks were produced. 
    """

    def __init__(self,process_name) :
        super().__init__('%s_track_filter'%process_name, 'ldmx::TrackProcessFilter' )

        from LDMX.Biasing import include
        include.library()

        self.process = process_name

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

    def dark_brem() :
        """ Configuration used to tag all dark brem tracks to persist them to the event. 
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag dark brem tracks.
    
        """
        return TrackProcessFilter('eDarkBrem')

